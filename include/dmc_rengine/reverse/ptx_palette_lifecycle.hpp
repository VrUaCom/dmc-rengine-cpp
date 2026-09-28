#pragma once

#include "dmc_rengine/reverse/ptx_palette_state.hpp"
#include "dmc_rengine/reverse/ptx_record_placement.hpp"
#include "dmc_rengine/reverse/runtime_block_allocator.hpp"

namespace dmc::rengine::reverse {

struct PtxPaletteContextImage {
    PtxPaletteContextState prefix;
    std::array<BlockAllocationState, 2> allocations;
};
static_assert(sizeof(PtxPaletteContextImage) == 0x58);
static_assert(offsetof(PtxPaletteContextImage, allocations) == 0x18);

// 0x140331180. Nonpositive counts skip a bank without clearing old state.
// Each enabled bank reserves a pool record, places/marks it, publishes the
// pointer/counters, then allocates data. Every failure returns without rollback.
std::uint8_t initialize_ptx_palette_context(PtxPaletteContextImage&, PtxPoolImage&,
    std::int32_t count13, std::int32_t count14, PtxPlacementConfig,
    BlockRoutingFlags, BlockAllocatorView&);
// 0x140331460. Record clear, pointer/counter reset, then allocation release,
// in bank order. Called even with invalid/null bank record pointers.
void destroy_ptx_palette_context(PtxPaletteContextImage&, PtxPoolImage&, BlockAllocatorView&);

} // namespace dmc::rengine::reverse
