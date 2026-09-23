#include "dmc_rengine/reverse/temporary_backing_allocator.hpp"

#include <cstddef>
#include <cstdint>

using namespace dmc::rengine::reverse;

extern "C" void temporary_backing_init_step(void* state, std::uint64_t base,
    std::uint32_t bytes, std::uint32_t shift) {
    initialize_temporary_backing_arena(*static_cast<TemporaryBackingArena*>(state),
                                       base, bytes, shift);
}

extern "C" std::uint8_t temporary_backing_acquire_step(void* lease,
    std::uint64_t arena_address, void* arena) {
    return static_cast<std::uint8_t>(acquire_temporary_backing_lease(
        *static_cast<TemporaryBackingLease*>(lease), arena_address,
        *static_cast<TemporaryBackingArena*>(arena)));
}

extern "C" std::uint64_t temporary_backing_alloc_step(void* lease, void* arena,
    std::uint32_t bytes) {
    return allocate_temporary_backing(*static_cast<TemporaryBackingLease*>(lease),
        *static_cast<TemporaryBackingArena*>(arena), bytes);
}

extern "C" std::uint8_t temporary_backing_release_step(void* lease, void* arena) {
    return static_cast<std::uint8_t>(release_temporary_backing_lease(
        *static_cast<TemporaryBackingLease*>(lease),
        *static_cast<TemporaryBackingArena*>(arena)));
}
