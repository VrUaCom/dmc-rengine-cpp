#pragma once

#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/mod_skin.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace dmc::rengine::analysis::mod {

// Serialized MOD inner-mesh bytes intentionally remain separate from
// runtime-live stream/state fields. Canonical whole-image provenance closure
// now covers both ways the executable reaches the 0x50-byte source mesh:
//
// 1) direct object.mesh_table(+0x08) + mesh_index*0x50 derivation; and
// 2) the live source backreference stored by 0x1402FE713 at runtime mesh +0x10.
//
// The whole executable contains exactly six direct 0x50-table derivation
// functions. The runtime-mesh surface has 34 exact index*0x1A0 + array(+0x20)
// derivation functions; typed propagation covers 43 reachable functions and
// 20 direct typed calls with zero serialized-source pointer escapes and zero
// serialized-source indirect calls. Live backreference consumers read source
// offsets +0x00/+0x02/+0x04/+0x06/+0x08/+0x0A/+0x40/+0x48, but never +0x0C
// or +0x4C. This closes canonical runtime consumption of +0x0C/+0x4C as
// EXE_CONFIRMED dormant/no effect. It does NOT rename either serialized field
// padding/reserved and does not authorize a writer to normalize it.
//
// Evidence boundaries:
// - MOD +0x0C is a raw u32 between the material/CLAMP prefix and the first u64
//   stream pointer. It is zero in the bounded MOD/EFM corpus and has no
//   provenance-confirmed canonical consumer. Serialized semantic remains
//   PRESERVED_UNDECODED; exact source bits remain a writer obligation.
// - +0x38 is not generic padding. EFM post-load 0x1402F7A90 relocates the
//   homologous slot and EFM runtime builder 0x1402F7D60 forwards it as the
//   extra COLOR0 stream. Canonical MOD builder 0x1402FE6A0 instead explicitly
//   zeroes the corresponding runtime auxiliary-stream slots and does not copy
//   serialized MOD +0x38. The shared physical description is therefore a
//   family-specific auxiliary-stream slot; MOD does not inherit EFM COLOR0
//   semantics by offset similarity. Source bytes are still preserved.
// - MOD +0x4C is the raw trailing u32 after the separate generated topology
//   count at +0x48. +0x48 is live in build and post-build consumers, including
//   the runtime-mesh +0x10 backreference path; +0x4C is not. It is zero in the
//   bounded MOD/EFM corpus, canonical runtime behavior is dormant/no effect,
//   and serialized semantic remains PRESERVED_UNDECODED. The physical
//   topology-workspace span align16(6*(vertex_count-2)) belongs to +0x40, not
//   to serialized +0x48 or +0x4C.
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

// These constants drive the canonical read-only MOD skin decoder. The direct
// CPU path has a provenance-confirmed lane-Y read at 0x1402F3D0A and no lane-X
// dereference. The former indirect GPU escape is also closed for the canonical
// executable: every BLENDINDICES DXBC input signature uses ReadWriteMask=0xE,
// so compiled shaders consume Y/Z/W and not X. Lane X is still part of the
// serialized u8x4 ABI and remains source-preserved rather than being renamed
// padding/reserved or force-normalized to zero.
static_assert(dmc::rengine::formats::mod::matrix_row_stride == 4U);
static_assert(dmc::rengine::formats::mod::quantized_weight_sum == 31U);
static_assert(dmc::rengine::formats::mod::topology_break_mask == 0x8000U);
static_assert(dmc::rengine::formats::mod::packed_weight_mask == 0x7FFFU);

static_assert(MeshSerializedPreservationAbi::record_size == 0x50U);
static_assert(MeshSerializedPreservationAbi::preserved0c_u32_field == 0x0CU);
static_assert(MeshSerializedPreservationAbi::preserved38_u64_field == 0x38U);
static_assert(MeshSerializedPreservationAbi::preserved4c_u32_field == 0x4CU);

} // namespace dmc::rengine::analysis::mod
