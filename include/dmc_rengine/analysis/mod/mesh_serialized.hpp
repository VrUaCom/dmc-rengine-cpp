#pragma once

#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/mod_skin.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace dmc::rengine::analysis::mod {

// Serialized MOD inner-mesh bytes intentionally kept separate from runtime-live
// stream/state fields. Canonical post-load 0x1402FE3B0 and runtime mesh
// construction 0x1402FE6A0 provide bounded negative evidence for these source
// fields. Negative runtime evidence never authorizes a writer to clear them;
// they remain byte-preservation obligations.
//
// Evidence boundaries:
// - MOD +0x0C is a u32 between the 12-byte material/CLAMP prefix and the first
//   u64 stream pointer. It is zero in the current bounded corpus, but neither
//   the zero histogram nor its physical position proves padding/alignment.
//   Status remains PRESERVED_UNDECODED until a complete consumer census closes
//   it.
// - +0x38 is not generic padding. EFM post-load 0x1402F7A90 relocates the
//   homologous slot and EFM runtime builder 0x1402F7D60 forwards it as the
//   extra COLOR0 stream. Canonical MOD builder 0x1402FE6A0 instead explicitly
//   zeroes the corresponding runtime auxiliary-stream slots and does not copy
//   serialized MOD +0x38. The shared physical description is therefore a
//   family-specific auxiliary-stream slot; MOD does not inherit EFM COLOR0
//   semantics by offset similarity. Source bytes are still preserved.
// - MOD +0x4C is the trailing u32 after runtime-generated count +0x48. It is
//   zero in the bounded MOD corpus and has no positive consumer in the audited
//   load/build paths. That is not sufficient to call it reserved or padding;
//   its global semantic remains PRESERVED_UNDECODED.
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

// These constants drive the canonical read-only MOD skin decoder. CPU consumer
// 0x1402F3D0A reads BLENDINDICES lane[1] and divides the matrix-row offset by
// four. Canonical MOD shader families use matIndex.y/z/w and no matIndex.x use
// was found. Lane X is nevertheless not called reserved: its global CPU role
// is not exhaustively closed and the serialized byte remains
// PRESERVED_UNDECODED.
static_assert(dmc::rengine::formats::mod::matrix_row_stride == 4U);
static_assert(dmc::rengine::formats::mod::quantized_weight_sum == 31U);
static_assert(dmc::rengine::formats::mod::topology_break_mask == 0x8000U);
static_assert(dmc::rengine::formats::mod::packed_weight_mask == 0x7FFFU);

static_assert(MeshSerializedPreservationAbi::record_size == 0x50U);
static_assert(MeshSerializedPreservationAbi::preserved0c_u32_field == 0x0CU);
static_assert(MeshSerializedPreservationAbi::preserved38_u64_field == 0x38U);
static_assert(MeshSerializedPreservationAbi::preserved4c_u32_field == 0x4CU);

} // namespace dmc::rengine::analysis::mod
