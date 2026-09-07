#pragma once

#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/mod_skin.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace dmc::rengine::analysis::mod {

// Serialized MOD inner-mesh bytes that are intentionally kept separate from
// runtime-live stream/state fields. Canonical post-load 0x1402FE3B0 and runtime
// mesh construction 0x1402FE6A0 do not relocate or copy these fields in the
// confirmed MOD load path. That negative runtime evidence does NOT authorize a
// writer to clear them; they remain byte-preservation obligations.
struct MeshSerializedPreservationAbi final {
    static constexpr std::size_t record_size = 0x50U;
    static constexpr std::size_t preserved0c_u32_field = 0x0CU;
    static constexpr std::size_t preserved38_u64_field = 0x38U;
    static constexpr std::size_t preserved4c_u32_field = 0x4CU;
};

struct MeshSerializedPreservation final {
    std::uint32_t preserved0c_u32{};
    std::uint64_t preserved38_u64{};
    std::uint32_t preserved4c_u32{};
};

[[nodiscard]] constexpr std::optional<MeshSerializedPreservation>
decode_mesh_serialized_preservation(
    std::span<const std::byte> bytes,
    std::size_t mesh_record_offset) noexcept {
    constexpr std::size_t required_end =
        MeshSerializedPreservationAbi::record_size;
    if (mesh_record_offset > bytes.size() ||
        required_end > bytes.size() - mesh_record_offset) {
        return std::nullopt;
    }

    const auto read_u32_le = [&](std::size_t relative) constexpr {
        std::uint32_t value{};
        for (std::size_t i = 0U; i < 4U; ++i) {
            value |= static_cast<std::uint32_t>(
                std::to_integer<std::uint8_t>(
                    bytes[mesh_record_offset + relative + i])) << (8U * i);
        }
        return value;
    };

    const auto read_u64_le = [&](std::size_t relative) constexpr {
        std::uint64_t value{};
        for (std::size_t i = 0U; i < 8U; ++i) {
            value |= static_cast<std::uint64_t>(
                std::to_integer<std::uint8_t>(
                    bytes[mesh_record_offset + relative + i])) << (8U * i);
        }
        return value;
    };

    return MeshSerializedPreservation{
        .preserved0c_u32 = read_u32_le(
            MeshSerializedPreservationAbi::preserved0c_u32_field),
        .preserved38_u64 = read_u64_le(
            MeshSerializedPreservationAbi::preserved38_u64_field),
        .preserved4c_u32 = read_u32_le(
            MeshSerializedPreservationAbi::preserved4c_u32_field),
    };
}

[[nodiscard]] inline std::optional<MeshSerializedPreservation>
read_mesh_serialized_preservation(
    const dmc::rengine::formats::mod::Document& document,
    std::size_t outer_index,
    std::size_t mesh_index) noexcept {
    if (outer_index >= document.outer_models.size()) return std::nullopt;
    const auto& outer = document.outer_models[outer_index];
    if (mesh_index >= outer.meshes.size()) return std::nullopt;

    const auto offset64 = outer.meshes[mesh_index].record_offset;
    if (offset64 > static_cast<std::uint64_t>(document.source_bytes.size()))
        return std::nullopt;

    return decode_mesh_serialized_preservation(
        std::span<const std::byte>(document.source_bytes),
        static_cast<std::size_t>(offset64));
}

// These constants already drive the canonical read-only MOD skin decoder.
// Their evidence is now independently closed by the canonical executable:
// - CPU consumer 0x1402F3D0A..0x1402F3D0F reads BLENDINDICES lane[1] and
//   divides the matrix-row offset by four to obtain a node/bone index.
// - runtime-selected vertex shader DMC3_MOD.hlsl (descriptor tag 5) decodes
//   three 5-bit PSIZE weights with denominator 31 and uses matIndex.y/z/w as
//   the corresponding four-row matrix starts in extraMatrices[].
static_assert(dmc::rengine::formats::mod::matrix_row_stride == 4U);
static_assert(dmc::rengine::formats::mod::quantized_weight_sum == 31U);
static_assert(dmc::rengine::formats::mod::topology_break_mask == 0x8000U);
static_assert(dmc::rengine::formats::mod::packed_weight_mask == 0x7FFFU);

static_assert(MeshSerializedPreservationAbi::record_size == 0x50U);
static_assert(MeshSerializedPreservationAbi::preserved0c_u32_field == 0x0CU);
static_assert(MeshSerializedPreservationAbi::preserved38_u64_field == 0x38U);
static_assert(MeshSerializedPreservationAbi::preserved4c_u32_field == 0x4CU);

} // namespace dmc::rengine::analysis::mod
