#pragma once

#include "dmc_rengine/reverse/ptx_payload_state.hpp"

namespace dmc::rengine::reverse {

// Raw field read through [0x140D6D300] + 0x4C by the automatic path.
// Its containing graphics-configuration type is not recovered here.
struct PtxPlacementConfig { std::uint16_t word_4c; std::uint16_t word_4e; };
inline constexpr std::size_t ptx_pool_limit_units_offset = 0xcb0c;

// 0x140331910 clears 0xCB50 bytes. Earlier operations address only the prefix;
// the additional eight bytes have no promoted meaning beyond that clear.
struct PtxPoolStorageImage {
    PtxPoolImage prefix;
    std::array<std::byte, 8> preserved_tail;
};
static_assert(sizeof(PtxPoolStorageImage) == 0xcb50);
void initialize_ptx_pool(PtxPoolStorageImage&, PtxPlacementConfig);
// 0x140331D90: wrap32(blocks << 5), update bounds, then reset the global
// manager's keys/counts. Pool records, occupancy, payloads and tail are retained.
std::uint8_t configure_ptx_pool_reservation(PtxPoolImage&, PtxManagerImage&,
                                           std::uint32_t blocks, PtxPlacementConfig) noexcept;

// 0x140331520. requested_index == 0 selects automatic scan; any other
// signed 32-bit value selects the explicit-index path. If length_mode == 0,
// record +0x0E and the eligible +0x26 length become 1 before the search.
// This assigns record addresses; the occupancy map is not marked here.
// Original range checks and their omissions are preserved. Accesses outside
// PtxPoolImage or nonpositive active lengths throw host exceptions, not SEH.
std::uint8_t place_ptx_record(PtxPoolImage&, std::uint64_t record,
                             std::int32_t requested_index, std::int32_t length_mode,
                             PtxPlacementConfig);

} // namespace dmc::rengine::reverse
