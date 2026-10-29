#include "dmc_rengine/reverse/deferred_callback_queue.hpp"
#include "dmc_rengine/reverse/runtime_block_allocator.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace dmc::rengine::reverse;

namespace {
struct Invocation {
    std::uint64_t function;
    std::uint64_t context;
    std::uint64_t payload;
};
void invoke(void* opaque, std::uint64_t function, std::uint64_t context,
            std::uint64_t payload, std::uint64_t*) {
    static_cast<std::vector<Invocation>*>(opaque)->push_back({function, context, payload});
}
} // namespace

int main() {
    {
        BlockArenaState arena{};
        std::vector<std::byte> backing(0x1000, std::byte{0xa5});
        const bool initialized = initialize_runtime_block_arena(
            arena, backing, 0x70000000, 0x200, 0x1000, 6);
        assert(initialized);
        assert(arena.occupancy == 0x70000000);
        assert(arena.capacity == 7);
        assert(arena.block_bytes == 0x200);
        assert(arena.backing_bytes == 0x1000);
        assert(arena.alignment_shift == 6);
        assert(arena.live_allocations == 0);
        assert(arena.data == 0x70000040);
        for (std::size_t i = 0; i < 7; ++i) assert(backing[i] == std::byte{0});
        for (std::size_t i = 7; i < backing.size(); ++i) assert(backing[i] == std::byte{0xff});
    }
    {
        BlockArenaState arena{};
        arena.data = 0x1234;
        arena.capacity = 17;
        std::vector<std::byte> backing(0x1000, std::byte{0xa5});
        const bool initialized = initialize_runtime_block_arena(
            arena, backing, 0x70000000, 0x800, 0x840 - 1, 6);
        assert(!initialized);
        assert(arena.backing_bytes == 0);
        assert(arena.data == 0x1234);
        assert(arena.capacity == 17);
        assert(backing.front() == std::byte{0xa5});
    }
    {
        std::vector<std::byte> storage(4 * sizeof(DeferredCallbackRecord), std::byte{0xa5});
        DeferredCallbackPool pool{0x71000000, storage, 0, 4};
        std::uint64_t queue = 0;
        const auto first = enqueue_deferred_callback(pool, queue, 0x101, 0x201, 0x301);
        const auto second = enqueue_deferred_callback(pool, queue, 0x102, 0x202, 0x302);
        assert(first != second);
        assert(queue == second);
        assert(pool.free_head != 0);

        std::vector<Invocation> invocations;
        drain_deferred_callbacks(pool, queue, {&invocations, invoke});
        assert(queue == 0);
        assert(invocations.size() == 2);
        assert(invocations[0].function == 0x102 && invocations[0].context == 0x202 &&
               invocations[0].payload == 0x302);
        assert(invocations[1].function == 0x101 && invocations[1].context == 0x201 &&
               invocations[1].payload == 0x301);
        assert(pool.free_head == first);
        const auto recycled = acquire_deferred_callback_record(pool);
        assert(recycled == first);
    }
}
