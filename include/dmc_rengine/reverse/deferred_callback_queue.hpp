#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::rengine::reverse {

struct DeferredCallbackRecord {
    std::uint64_t function;
    std::uint64_t context;
    std::uint64_t payload;
    std::uint64_t next;
};
static_assert(sizeof(DeferredCallbackRecord) == 0x20);
static_assert(offsetof(DeferredCallbackRecord, next) == 0x18);

struct DeferredCallbackPool {
    std::uint64_t storage_address;
    std::span<std::byte> storage;
    std::uint64_t free_head;
    std::uint32_t preallocation_count;
};

struct DeferredCallbackServices {
    void* context;
    void (*invoke)(void*, std::uint64_t function, std::uint64_t callback_context,
                   std::uint64_t payload, std::uint64_t* queue_head);
};

// Canonical dmc3.exe: 0x140329300 allocates from the shared 32-byte free list;
// 0x140329240 publishes a callback record at the head of a caller-owned queue.
std::uint64_t acquire_deferred_callback_record(DeferredCallbackPool&);
std::uint64_t enqueue_deferred_callback(DeferredCallbackPool&, std::uint64_t& queue_head,
                                        std::uint64_t function,
                                        std::uint64_t callback_context,
                                        std::uint64_t payload);

// 0x1403292A0 invokes each callback, then recycles a record through the global
// free list. Callbacks are allowed to mutate queue_head; the EXE rereads it.
void drain_deferred_callbacks(DeferredCallbackPool&, std::uint64_t& queue_head,
                              DeferredCallbackServices);

} // namespace dmc::rengine::reverse
