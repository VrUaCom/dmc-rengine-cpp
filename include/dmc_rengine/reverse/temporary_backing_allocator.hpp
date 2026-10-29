#pragma once

#include <cstddef>
#include <cstdint>

namespace dmc::rengine::reverse {

// Memory layouts recovered from 0x140337A30 and 0x1403379B0..0x140337A18.
struct TemporaryBackingArena {
    std::uint64_t base;
    std::uint64_t end;
    std::uint64_t cursor;
    std::uint32_t bytes;
    std::uint32_t active_leases;
    std::uint32_t alignment_shift;
    std::uint32_t reserved_24;
};
struct TemporaryBackingLease {
    std::uint64_t arena;
    std::uint64_t cursor_snapshot;
    std::uint32_t lease_depth;
    std::uint32_t reserved_14;
};
static_assert(sizeof(TemporaryBackingArena) == 0x28);
static_assert(offsetof(TemporaryBackingArena, cursor) == 0x10);
static_assert(offsetof(TemporaryBackingArena, active_leases) == 0x1c);
static_assert(sizeof(TemporaryBackingLease) == 0x18);
static_assert(offsetof(TemporaryBackingLease, lease_depth) == 0x10);

// 0x140337A30. The caller supplies the already allocated backing span.
void initialize_temporary_backing_arena(TemporaryBackingArena&, std::uint64_t base,
                                        std::uint32_t bytes,
                                        std::uint32_t alignment_shift);
// 0x1403379B0 / 0x1403379E0. Leases are nestable and must be released LIFO.
bool acquire_temporary_backing_lease(TemporaryBackingLease&, std::uint64_t arena_address,
                                     TemporaryBackingArena&);
bool release_temporary_backing_lease(TemporaryBackingLease&, TemporaryBackingArena&);
// 0x140337920. Allocates from the current lease and rounds each request up to
// the backing alignment. Returns zero when no valid lease or capacity remains.
std::uint64_t allocate_temporary_backing(TemporaryBackingLease&, TemporaryBackingArena&,
                                         std::uint32_t bytes);

} // namespace dmc::rengine::reverse
