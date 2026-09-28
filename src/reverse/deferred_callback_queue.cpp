#include "dmc_rengine/reverse/deferred_callback_queue.hpp"

#include <cstring>
#include <stdexcept>

namespace dmc::rengine::reverse {
namespace {

std::size_t offset_of(const DeferredCallbackPool& pool, std::uint64_t address) {
    if (address < pool.storage_address || address - pool.storage_address > pool.storage.size() ||
        sizeof(DeferredCallbackRecord) > pool.storage.size() - (address - pool.storage_address) ||
        ((address - pool.storage_address) % sizeof(DeferredCallbackRecord)) != 0)
        throw std::out_of_range("deferred callback record address");
    return static_cast<std::size_t>(address - pool.storage_address);
}

DeferredCallbackRecord load(const DeferredCallbackPool& pool, std::uint64_t address) {
    DeferredCallbackRecord record{};
    std::memcpy(&record, pool.storage.data() + offset_of(pool, address), sizeof(record));
    return record;
}

void store(DeferredCallbackPool& pool, std::uint64_t address,
           const DeferredCallbackRecord& record) {
    std::memcpy(pool.storage.data() + offset_of(pool, address), &record, sizeof(record));
}

} // namespace

std::uint64_t acquire_deferred_callback_record(DeferredCallbackPool& pool) {
    if (pool.free_head == 0) {
        const auto capacity = pool.storage.size() / sizeof(DeferredCallbackRecord);
        if (pool.preallocation_count > capacity)
            throw std::out_of_range("deferred callback pool capacity");
        for (std::uint32_t i = 0; i < pool.preallocation_count; ++i) {
            const auto address = pool.storage_address +
                                 static_cast<std::uint64_t>(i) * sizeof(DeferredCallbackRecord);
            store(pool, address, DeferredCallbackRecord{0, 0, 0, pool.free_head});
            pool.free_head = address;
        }
    }
    if (pool.free_head == 0) throw std::out_of_range("empty deferred callback pool");
    const auto result = pool.free_head;
    auto record = load(pool, result);
    pool.free_head = record.next;
    record.next = 0;
    store(pool, result, record);
    return result;
}

std::uint64_t enqueue_deferred_callback(DeferredCallbackPool& pool, std::uint64_t& queue_head,
                                        std::uint64_t function,
                                        std::uint64_t callback_context,
                                        std::uint64_t payload) {
    const auto address = acquire_deferred_callback_record(pool);
    store(pool, address, DeferredCallbackRecord{function, callback_context, payload, queue_head});
    queue_head = address;
    return address;
}

void drain_deferred_callbacks(DeferredCallbackPool& pool, std::uint64_t& queue_head,
                              DeferredCallbackServices services) {
    if (services.invoke == nullptr) throw std::invalid_argument("deferred callback dispatcher");
    while (queue_head != 0) {
        const auto address = queue_head;
        const auto record = load(pool, address);
        services.invoke(services.context, record.function, record.context, record.payload,
                        &queue_head);

        // The native function rereads *head after the callback. It then puts
        // that value onto the global free list before advancing to saved next.
        const auto current_head = queue_head;
        const auto old_free_head = pool.free_head;
        pool.free_head = current_head;
        auto recycled = load(pool, current_head);
        recycled.next = old_free_head;
        store(pool, current_head, recycled);
        queue_head = record.next;
    }
}

} // namespace dmc::rengine::reverse
