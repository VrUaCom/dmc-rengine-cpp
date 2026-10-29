#pragma once

#include "dmc_rengine/reverse/ptx_manager_state.hpp"
#include <array>
#include <span>

namespace dmc::rengine::reverse {

// Runtime state, not a serialized PTX file. Slot values are canonical addresses.
struct PtxPayloadState {
    std::array<std::uint64_t, 64> records;
    std::uint32_t texture_count;
    std::uint32_t records_per_texture;
};
static_assert(sizeof(PtxPayloadState) == 0x208);
static_assert(offsetof(PtxPayloadState, texture_count) == 0x200);
static_assert(offsetof(PtxPayloadState, records_per_texture) == 0x204);

inline constexpr std::uint64_t ptx_pool_address = 0x140d5fb70;
inline constexpr std::size_t ptx_record_size = 0x50;
inline constexpr std::size_t ptx_record_count = 128;
inline constexpr std::size_t ptx_allocation_map_offset = 0x2800;
inline constexpr std::size_t ptx_pool_base_units_offset = 0xcb08;
inline constexpr std::size_t ptx_scratch_count_offset = 0xcb20;
inline constexpr std::size_t ptx_scratch_records_offset = 0xcb28;

// Includes the 128 records, occupancy bytes and downstream scratch state.
// Bytes without a proven meaning are retained verbatim.
using PtxPoolImage = std::array<std::byte, 0xcb48>;
using PtxTextureInputImage = std::array<std::byte, 0x40>;

// Loaders reuse a stack descriptor without initializing it. Its initial bytes
// are an explicit input here; the parser owns which bytes it writes.
struct PtxLoadWorkspace { PtxTextureInputImage descriptor; };

struct PtxBundleServices {
    void* context;
    int (*parse_texture)(void*, std::uint64_t source, PtxTextureInputImage&); // 0x1403365B0
    int (*materialize_records)(void*, PtxTextureInputImage&, std::int32_t selector,
                               std::uint64_t argument, PtxPoolImage&); // 0x1403366E0
    int (*finalize_payload)(void*, PtxPayloadState&, PtxPoolImage&); // 0x140331A80
};

// 0x1403313F0: reserve the first record with word +0 == 0. Only +0/+2 change.
std::uint64_t allocate_ptx_record(PtxPoolImage&) noexcept;
// 0x140330F60: release occupancy spans; record bytes themselves are retained.
void release_ptx_record_spans(PtxPoolImage&, std::uint64_t record);
// 0x140331420: inclusive address-range test, optional spans, then clear 0x50 bytes.
// The original performs no record-alignment check.
void release_ptx_record(PtxPoolImage&, std::uint64_t record);
// 0x1403317D0: release slots in order, then clear all 0x208 payload bytes.
void release_ptx_payload(PtxPayloadState&, PtxPoolImage&);

// 0x140336BB0 / 0x140336A70. Source bytes are borrowed and remain unchanged.
// source_address identifies the same byte span in the original address space.
// Host guards throw for reads/writes outside represented memory or nonpositive
// do-while span lengths. These guards are not original EXE validation/SEH.
int load_ptx_bundle(std::span<const std::byte> source, std::uint64_t source_address,
                    PtxPayloadState&, PtxPoolImage&, PtxLoadWorkspace&, const PtxBundleServices&);
int load_ptx_bundle_variant(std::span<const std::byte> source, std::uint64_t source_address,
                            PtxPayloadState&, PtxPoolImage&, PtxLoadWorkspace&,
                            std::uint64_t argument, const PtxBundleServices&);

PtxPayloadState decode_ptx_payload_state(const PtxPayload&) noexcept;
void encode_ptx_payload_state(const PtxPayloadState&, PtxPayload&) noexcept;

} // namespace dmc::rengine::reverse
