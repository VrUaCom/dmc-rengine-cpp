#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::rengine::reverse {

// Recovered views; original class names and arena initialization remain open.
struct BlockAllocationState {
    std::uint64_t callback_head;
    std::uint64_t arena;
    std::uint64_t data;
    std::int32_t first;
    std::uint32_t blocks;
};
struct BlockArenaState {
    std::uint64_t occupancy;
    std::uint64_t data;
    std::int32_t capacity;
    std::uint32_t block_bytes;
    std::uint32_t availability_gate;
    std::uint32_t preserved_1c;
    std::uint32_t live_allocations;
    std::uint32_t preserved_24;
};
static_assert(sizeof(BlockAllocationState) == 0x20);
static_assert(offsetof(BlockAllocationState, first) == 0x18);
static_assert(sizeof(BlockArenaState) == 0x28);
static_assert(offsetof(BlockArenaState, live_allocations) == 0x20);
inline constexpr std::uint64_t runtime_block_arenas_address = 0x140ca8910;

struct BlockArenaView {
    std::uint64_t address;
    BlockArenaState* state;
    std::span<const std::byte> occupancy; // Starts at state->occupancy.
};
struct BlockMemoryServices {
    void* context;
    // Checked host implementation of the CRT memset boundary.
    void (*fill)(void*, std::uint64_t address, std::uint8_t value, std::uint64_t bytes);
    // 0x1403292A0: callback queue draining remains an explicit boundary.
    void (*drain_callbacks)(void*, BlockAllocationState&);
};
struct BlockAllocatorView {
    std::span<BlockArenaView> arenas;
    BlockMemoryServices services;
};
struct BlockRoutingFlags { std::uint8_t fallback; std::uint8_t force_arena2; };

// 0x1403374A0..0x1403375F1, including its chained .pdata fragments.
// Nonbinary occupancy has deliberately inconsistent treatment in the EXE.
std::int32_t find_runtime_block_run(const BlockArenaView&, std::uint32_t blocks);
// 0x140337600. Size/product arithmetic retains original uint32 truncation.
std::uint64_t allocate_runtime_blocks(BlockAllocationState&, std::uint64_t arena,
                                     std::uint32_t bytes, BlockAllocatorView&);
// 0x1402C6150. Mode -2 -> arena 1; -1 -> signed size <=1024 ? 1 : 0.
// force_arena2 is active only at value 1. Fallback eligibility uses ORIGINAL mode.
std::uint64_t allocate_routed_blocks(BlockAllocationState&, std::uint32_t bytes,
                                     std::int32_t mode, BlockRoutingFlags, BlockAllocatorView&);
// 0x140337710, also reached by tail thunk 0x1402C6260.
std::uint8_t release_runtime_blocks(BlockAllocationState&, BlockAllocatorView&);

// Host bounds/division guards are not original EXE validation or SEH.
// Arena state, allocation handles and occupancy/data buffers must not alias.
} // namespace dmc::rengine::reverse
