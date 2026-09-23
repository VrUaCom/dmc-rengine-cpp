#include "dmc_rengine/reverse/deferred_callback_queue.hpp"
#include "dmc_rengine/reverse/runtime_block_allocator.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

using namespace dmc::rengine::reverse;

extern "C" int initialize_runtime_arena_step(void* state, std::byte* backing,
    std::size_t backing_span, std::uint64_t backing_address, std::uint32_t block_bytes,
    std::uint32_t backing_bytes, std::uint32_t alignment_shift) {
    auto& arena = *static_cast<BlockArenaState*>(state);
    try {
        return initialize_runtime_block_arena(arena, {backing, backing_span}, backing_address,
            block_bytes, backing_bytes, alignment_shift) ? 1 : 0;
    } catch (...) {
        return -1;
    }
}

namespace {
struct CallLog {
    std::uint64_t* events;
    std::size_t capacity;
    std::size_t count;
};
void invoke(void* opaque, std::uint64_t function, std::uint64_t callback_context,
            std::uint64_t payload, std::uint64_t*) {
    auto& log = *static_cast<CallLog*>(opaque);
    if (log.count < log.capacity) {
        const auto index = log.count++ * 3;
        log.events[index] = function;
        log.events[index + 1] = callback_context;
        log.events[index + 2] = payload;
    }
}
} // namespace

extern "C" std::uint64_t deferred_callback_queue_step(std::byte* storage,
    std::size_t storage_size, std::uint64_t storage_address, std::uint64_t* free_head,
    std::uint64_t* queue_head, std::uint64_t operation, std::uint64_t function,
    std::uint64_t callback_context, std::uint64_t payload, std::uint64_t* events,
    std::size_t event_capacity, std::size_t* event_count) {
    DeferredCallbackPool pool{storage_address, {storage, storage_size}, *free_head, 32};
    CallLog log{events, event_capacity, 0};
    std::uint64_t result = 0;
    if (operation == 0) {
        result = enqueue_deferred_callback(pool, *queue_head, function, callback_context, payload);
    } else {
        drain_deferred_callbacks(pool, *queue_head, {&log, invoke});
    }
    *free_head = pool.free_head;
    *event_count = log.count;
    return result;
}
