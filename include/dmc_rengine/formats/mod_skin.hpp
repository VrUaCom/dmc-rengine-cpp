#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::formats::mod {

enum class SkinDecodeStatus : std::uint8_t {
    ok,
    quantized_sum_mismatch,
    active_index_not_matrix_aligned,
    bone_index_out_of_range,
    duplicate_active_bone,
};

[[nodiscard]] constexpr std::string_view to_string(
    SkinDecodeStatus status) noexcept {
    switch (status) {
    case SkinDecodeStatus::ok: return "ok";
    case SkinDecodeStatus::quantized_sum_mismatch:
        return "quantized-sum-mismatch";
    case SkinDecodeStatus::active_index_not_matrix_aligned:
        return "active-index-not-matrix-aligned";
    case SkinDecodeStatus::bone_index_out_of_range:
        return "bone-index-out-of-range";
    case SkinDecodeStatus::duplicate_active_bone:
        return "duplicate-active-bone";
    }
    return "quantized-sum-mismatch";
}

struct SkinInfluence final {
    std::uint8_t bone_index{};
    std::uint8_t quantized_weight{};
    float weight{};
};

struct VertexSkin final {
    std::array<SkinInfluence, 3> influences{};
    std::uint8_t influence_count{};
    bool topology_break{};
};

struct SkinDecodeResult final {
    SkinDecodeStatus status{SkinDecodeStatus::quantized_sum_mismatch};
    VertexSkin skin{};

    [[nodiscard]] bool ok() const noexcept {
        return status == SkinDecodeStatus::ok;
    }
};

inline constexpr std::uint16_t topology_break_mask = 0x8000U;
inline constexpr std::uint16_t packed_weight_mask = 0x7FFFU;
inline constexpr std::uint8_t quantized_weight_sum = 31U;
inline constexpr std::uint8_t matrix_row_stride = 4U;

// EXE-confirmed read-only decoder for the recovered DMC3-HD MOD revision.
// CPU consumer 0x1402F3D0A..0x1402F3D0F reads BLENDINDICES lane[1] as a
// float4-row offset and divides by four before indexing node/world matrices.
// Runtime-selected vertex shader descriptor tag 5 resolves to DMC3_MOD.hlsl;
// its DXBC/SPDB source decodes three 5-bit PSIZE weights with denominator 31
// and binds them to matIndex.y/z/w four-row matrix starts in extraMatrices[].
// The high 0x8000 source bit is independent topology state consumed/cleared by
// canonical MOD post-load before the packed 15-bit weight payload is retained.
// blend_indices[0] remains reserved/constant in the currently bound corpus.
[[nodiscard]] SkinDecodeResult decode_vertex_skin(
    const std::array<std::uint8_t, 4>& blend_indices,
    std::uint16_t packed_weights_and_topology,
    std::uint8_t node_count) noexcept;

} // namespace dmc::rengine::formats::mod
