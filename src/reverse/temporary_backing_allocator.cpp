#include "dmc_rengine/reverse/temporary_backing_allocator.hpp"

namespace dmc::rengine::reverse {

void initialize_temporary_backing_arena(TemporaryBackingArena& arena, std::uint64_t base,
                                        std::uint32_t bytes,
                                        std::uint32_t alignment_shift) {
    const auto alignment = std::uint64_t{1} << (alignment_shift & 31U);
    arena.base = base;
    arena.bytes = bytes;
    arena.alignment_shift = alignment_shift;
    arena.cursor = (base + alignment - 1U) & ~(alignment - 1U);
    arena.end = base + bytes;
    arena.active_leases = 0;
    arena.reserved_24 = 0;
}

bool acquire_temporary_backing_lease(TemporaryBackingLease& lease,
                                     std::uint64_t arena_address,
                                     TemporaryBackingArena& arena) {
    if (lease.arena != 0 || arena_address == 0) return false;
    lease.arena = arena_address;
    lease.cursor_snapshot = arena.cursor;
    ++arena.active_leases;
    lease.lease_depth = arena.active_leases;
    ++arena.reserved_24;
    return true;
}

bool release_temporary_backing_lease(TemporaryBackingLease& lease,
                                     TemporaryBackingArena& arena) {
    if (lease.arena == 0) return false;
    if (lease.lease_depth != arena.active_leases) return false;
    --arena.reserved_24;
    --arena.active_leases;
    arena.cursor = lease.cursor_snapshot;
    lease.arena = 0;
    lease.cursor_snapshot = 0;
    lease.lease_depth = 0;
    return true;
}

std::uint64_t allocate_temporary_backing(TemporaryBackingLease& lease,
                                         TemporaryBackingArena& arena,
                                         std::uint32_t bytes) {
    if (lease.arena == 0) return 0;
    if (lease.lease_depth != arena.active_leases) return 0;
    if (arena.cursor > arena.end || bytes > arena.end - arena.cursor) return 0;

    const auto shift = arena.alignment_shift & 31U;
    const auto alignment = std::uint32_t{1} << shift;
    const auto rounded = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(bytes) + alignment - 1U) &
        ~static_cast<std::uint64_t>(alignment - 1U));
    const auto result = arena.cursor;
    arena.cursor += rounded;
    return result;
}

} // namespace dmc::rengine::reverse
