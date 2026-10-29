#include "dmc_rengine/reverse/runtime_block_allocator.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <stdexcept>

namespace dmc::rengine::reverse {
namespace {
BlockArenaView& arena_at(BlockAllocatorView& memory, std::uint64_t address) {
    for (auto& arena : memory.arenas) if (arena.address == address) return arena;
    throw std::out_of_range("unrepresented runtime block arena");
}
std::uint8_t cell(const BlockArenaView& arena, std::int64_t index) {
    if (index < 0 || static_cast<std::uint64_t>(index) >= arena.occupancy.size())
        throw std::out_of_range("runtime block occupancy read");
    return std::to_integer<std::uint8_t>(arena.occupancy[static_cast<std::size_t>(index)]);
}
} // namespace

std::int32_t find_runtime_block_run(const BlockArenaView& arena, std::uint32_t blocks) {
    if ((arena.state->occupancy & 7U) != 0) return -1;
    const auto capacity = static_cast<std::int64_t>(arena.state->capacity);
    const auto rounded = capacity & ~std::int64_t{7};
    const auto needed = static_cast<std::int64_t>(std::bit_cast<std::int32_t>(blocks));
    std::int64_t cursor = 0, group = 0;
    while (cursor < rounded) {
        bool all_ones = true;
        for (std::int64_t i = 0; i < 8; ++i) all_ones &= cell(arena, group * 8 + i) == 1;
        if (all_ones) { ++group; cursor += 8; continue; }
        std::int64_t first_zero = 0;
        while (first_zero < 8 && cell(arena, group * 8 + first_zero) != 0) ++first_zero;
        if (first_zero == 8) return -1; // Not a general "nonzero means occupied" scan.
        auto start = group * 8 + first_zero;
        std::int64_t run = 0, accumulated_free = 0;
        for (;;) {
            if (run == needed) return static_cast<std::int32_t>(start);
            const auto current = start + run;
            if (current >= capacity) return -1;
            if (cell(arena, current) == 1) {
                if (accumulated_free >= 8) {
                    cursor = current + 1; group = cursor / 8; break;
                }
                start = current + 1; run = 0; // accumulated_free is NOT reset.
            } else { ++run; ++accumulated_free; }
        }
    }
    while (cursor < capacity) {
        if (cell(arena, cursor) == 1) { ++cursor; continue; }
        std::int64_t run = 0;
        for (;;) {
            if (run == needed) return static_cast<std::int32_t>(cursor);
            if (cursor + run >= capacity) return -1;
            if (cell(arena, cursor + run) == 1) { cursor += run + 1; break; }
            ++run;
        }
    }
    return -1;
}

bool initialize_runtime_block_arena(BlockArenaState& arena, std::span<std::byte> backing,
                                    std::uint64_t backing_address,
                                    std::uint32_t block_bytes,
                                    std::uint32_t backing_bytes,
                                    std::uint32_t alignment_shift) {
    const auto shift = alignment_shift & 31U;
    const auto alignment = std::uint32_t{1} << shift;
    arena.backing_bytes = backing_bytes;
    const auto minimum = static_cast<std::uint64_t>(alignment) + block_bytes;
    if (static_cast<std::uint64_t>(backing_bytes) < minimum) {
        arena.backing_bytes = 0;
        return false;
    }
    if (backing.size() < backing_bytes || block_bytes == UINT32_MAX)
        throw std::out_of_range("runtime block arena backing span");

    const auto aligned_block_bytes = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(block_bytes) + alignment - 1U) &
        ~static_cast<std::uint64_t>(alignment - 1U));
    const auto denominator = static_cast<std::uint64_t>(aligned_block_bytes) + 1U;
    const auto capacity = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(backing_bytes) - alignment) / denominator);
    const auto data_unaligned = backing_address + capacity;
    const auto data = (data_unaligned + alignment - 1U) &
                      ~static_cast<std::uint64_t>(alignment - 1U);

    arena.occupancy = backing_address;
    arena.data = data;
    arena.capacity = std::bit_cast<std::int32_t>(capacity);
    arena.block_bytes = aligned_block_bytes;
    arena.alignment_shift = alignment_shift;
    arena.live_allocations = 0;
    std::fill_n(backing.begin(), backing_bytes, std::byte{0xff});
    std::fill_n(backing.begin(), capacity, std::byte{0});
    return true;
}

std::uint64_t allocate_runtime_blocks(BlockAllocationState& node, std::uint64_t address,
                                     std::uint32_t bytes, BlockAllocatorView& memory) {
    if (bytes == 0 || address == 0) return 0;
    auto& arena = arena_at(memory, address);
    auto& state = *arena.state;
    if (state.data == 0 || state.backing_bytes == 0 || node.arena != 0) return 0;
    if (state.block_bytes == 0) throw std::domain_error("original runtime division by zero");
    const auto blocks = bytes / state.block_bytes + static_cast<std::uint32_t>(bytes % state.block_bytes != 0);
    node.arena = address;
    node.first = find_runtime_block_run(arena, blocks);
    if (node.first == -1) { node.arena = 0; return 0; } // data/blocks/head retained.
    node.blocks = blocks;
    memory.services.fill(memory.services.context,
        state.occupancy + static_cast<std::uint64_t>(static_cast<std::int64_t>(node.first)), 1, blocks);
    ++state.live_allocations;
    node.data = state.data + state.block_bytes * static_cast<std::uint32_t>(node.first);
    memory.services.fill(memory.services.context, node.data, 0, bytes); // Not rounded capacity.
    return node.data;
}

std::uint64_t allocate_routed_blocks(BlockAllocationState& node, std::uint32_t bytes,
    std::int32_t mode, BlockRoutingFlags flags, BlockAllocatorView& memory) {
    const bool fallback = flags.fallback != 0 && mode != 0;
    if (flags.force_arena2 == 1) mode = 2;
    else if (mode == -2) mode = 1;
    else if (mode == -1) mode = std::bit_cast<std::int32_t>(bytes) <= 0x400 ? 1 : 0;
    const auto address = [](std::int32_t index) {
        return runtime_block_arenas_address + static_cast<std::uint64_t>(static_cast<std::int64_t>(index)) * 0x28;
    };
    auto result = allocate_runtime_blocks(node, address(mode), bytes, memory);
    if (fallback && result == 0) {
        const auto other = mode == 1 ? 0 : mode == 0 ? 1 : mode;
        result = allocate_runtime_blocks(node, address(other), bytes, memory);
    }
    return result;
}

std::uint8_t release_runtime_blocks(BlockAllocationState& node, BlockAllocatorView& memory) {
    const bool had_arena = node.arena != 0;
    if (had_arena) {
        auto& arena = *arena_at(memory, node.arena).state;
        memory.services.fill(memory.services.context,
            arena.occupancy + static_cast<std::uint64_t>(static_cast<std::int64_t>(node.first)), 0, node.blocks);
        --arena.live_allocations;
        memory.services.drain_callbacks(memory.services.context, node);
    }
    node.arena = 0; node.data = 0; node.first = -1; node.blocks = 0;
    return static_cast<std::uint8_t>(had_arena);
}
} // namespace dmc::rengine::reverse
