#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/mod/runtime_postload.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

void put_u8(std::vector<std::byte>& bytes, std::size_t offset, std::uint8_t value) {
    bytes[offset] = static_cast<std::byte>(value);
}

void put_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    put_u8(bytes, offset + 0U, static_cast<std::uint8_t>(value & 0xFFU));
    put_u8(bytes, offset + 1U, static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
}

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        put_u8(bytes, offset + i,
               static_cast<std::uint8_t>((value >> (i * 8U)) & 0xFFU));
    }
}

void put_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t i = 0U; i < 8U; ++i) {
        put_u8(bytes, offset + i,
               static_cast<std::uint8_t>((value >> (i * 8U)) & 0xFFU));
    }
}

void put_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void put_ascii(std::vector<std::byte>& bytes, std::size_t offset, std::string_view text) {
    for (std::size_t i = 0U; i < text.size(); ++i) {
        put_u8(bytes, offset + i, static_cast<std::uint8_t>(text[i]));
    }
}

[[nodiscard]] std::uint16_t get_u16(
    const std::span<const std::byte> bytes,
    std::size_t offset) {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint8_t>(bytes[offset])) |
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(
                std::to_integer<std::uint8_t>(bytes[offset + 1U])) << 8U);
}

[[nodiscard]] std::uint32_t get_u32(
    const std::span<const std::byte> bytes,
    std::size_t offset) {
    std::uint32_t value{};
    for (std::size_t i = 0U; i < 4U; ++i) {
        value |= static_cast<std::uint32_t>(
            std::to_integer<std::uint8_t>(bytes[offset + i])) << (i * 8U);
    }
    return value;
}

[[nodiscard]] std::uint64_t get_u64(
    const std::span<const std::byte> bytes,
    std::size_t offset) {
    std::uint64_t value{};
    for (std::size_t i = 0U; i < 8U; ++i) {
        value |= static_cast<std::uint64_t>(
            std::to_integer<std::uint8_t>(bytes[offset + i])) << (i * 8U);
    }
    return value;
}

std::vector<std::byte> make_valid_mod() {
    std::vector<std::byte> bytes(0x260U, std::byte{0});
    put_ascii(bytes, 0x00U, "MOD ");
    put_f32(bytes, 0x04U, 1.01F);
    put_u8(bytes, 0x10U, 1U); // outer records
    put_u8(bytes, 0x11U, 1U); // transform-domain nodes
    put_u8(bytes, 0x12U, 8U); // serialized texture-slot-domain mirror
    put_u8(bytes, 0x13U, 0x5AU); // raw runtime-carried mode byte
    put_u32(bytes, 0x14U, 0x12345678U); // raw runtime-carried metadata
    put_u64(bytes, 0x20U, 0x200U);

    put_u8(bytes, 0x40U, 1U); // one inner mesh
    put_u8(bytes, 0x41U, 0x90U); // raw MOD alpha/control
    put_u16(bytes, 0x42U, 1U); // aggregate elements
    put_u64(bytes, 0x48U, 0x80U);
    put_u32(bytes, 0x50U, 0x00004000U); // nearest-filter source flag
    put_f32(bytes, 0x70U, 10.0F);  // bounding center x
    put_f32(bytes, 0x74U, -20.0F); // bounding center y
    put_f32(bytes, 0x78U, 30.0F);  // bounding center z
    put_f32(bytes, 0x7CU, 42.5F);  // bounding radius

    put_u16(bytes, 0x80U, 1U);
    put_u16(bytes, 0x82U, 7U); // texture slot
    put_u16(bytes, 0x84U, 1U); // GS CLAMP MINU
    put_u16(bytes, 0x86U, 2U); // GS CLAMP MAXU
    put_u16(bytes, 0x88U, 3U); // GS CLAMP MINV
    put_u16(bytes, 0x8AU, 4U); // GS CLAMP MAXV
    put_u64(bytes, 0x90U, 0xD0U);  // positions
    put_u64(bytes, 0x98U, 0xE0U);  // normals
    put_u64(bytes, 0xA0U, 0xF0U);  // UV
    put_u64(bytes, 0xA8U, 0x100U); // blend indices
    put_u64(bytes, 0xB0U, 0x110U); // packed weights/topology
    put_u64(bytes, 0xB8U, 0U);
    put_u64(bytes, 0xC0U, 0xA0U);  // record-relative -> 0x120
    put_u32(bytes, 0xC8U, 0U);
    put_u32(bytes, 0xCCU, 0U);

    put_f32(bytes, 0xD0U, 1.0F);
    put_f32(bytes, 0xD4U, 2.0F);
    put_f32(bytes, 0xD8U, 3.0F);
    put_f32(bytes, 0xE0U, 0.0F);
    put_f32(bytes, 0xE4U, 1.0F);
    put_f32(bytes, 0xE8U, 0.0F);
    put_u16(bytes, 0xF0U, 4096U);
    put_u16(bytes, 0xF2U, 2048U);
    put_u16(bytes, 0x110U, 0x001FU); // one influence, weight 31/31

    // Canonical one-node transform-domain document at 0x200.
    // Arrays are count bytes with align4 placement; the 0x20-byte local
    // transform record begins at align16(0x20 + 3*align4(1)) == 0x30.
    put_u32(bytes, 0x200U, 0x20U); // parentByOrderPosition
    put_u32(bytes, 0x204U, 0x24U); // nodeAtOrderPosition
    put_u32(bytes, 0x208U, 0x28U); // adapter domain
    put_u32(bytes, 0x20CU, 0x30U); // localTransformByNodeIndex
    put_u8(bytes, 0x220U, 0xFFU); // root parent
    put_u8(bytes, 0x224U, 0U);    // complete permutation
    put_u8(bytes, 0x228U, 0U);    // adapter byte preserved/undecoded
    // 0x230..0x24F remains zero: finite identity-rotation / zero-translation.
    return bytes;
}

std::vector<std::byte> make_runtime_postload_fixture() {
    std::vector<std::byte> bytes(0x400U, std::byte{0});
    put_u8(bytes, 0x10U, 1U);
    put_u64(bytes, 0x20U, 0x300U);

    put_u8(bytes, 0x40U, 1U);
    put_u64(bytes, 0x48U, 0x100U);

    put_u16(bytes, 0x100U, 6U);
    put_u64(bytes, 0x110U, 0x310U);
    put_u64(bytes, 0x118U, 0x320U);
    put_u64(bytes, 0x120U, 0x330U);
    put_u64(bytes, 0x128U, 0x340U);
    put_u64(bytes, 0x130U, 0x200U);
    put_u64(bytes, 0x138U, 0x350U); // MOD +0x38 remains untouched
    put_u64(bytes, 0x140U, 0x80U);  // mesh-relative -> 0x180

    constexpr std::array<std::uint16_t, 6> packed{
        0U, 1U, 2U, 3U,
        static_cast<std::uint16_t>(0x8000U | 4U),
        5U,
    };
    for (std::size_t index = 0U; index < packed.size(); ++index) {
        put_u16(bytes, 0x200U + index * 2U, packed[index]);
    }
    return bytes;
}

bool has_diagnostic(const dmc::rengine::formats::mod::ParseResult& parsed,
                    std::string_view code) {
    for (const auto& diagnostic : parsed.diagnostics) {
        if (diagnostic.code == code) return true;
    }
    return false;
}

} // namespace

int main() {
    namespace mod = dmc::rengine::formats::mod;

    {
        const auto parsed = mod::Parser::parse(make_valid_mod());
        assert(parsed.recognized);
        assert(parsed.ok());
        assert(parsed.document.header.texture_slot_count == 8U);
        assert(parsed.document.header.runtime_mode_byte == 0x5AU);
        assert(parsed.document.header.runtime_metadata_u32 == 0x12345678U);
        assert(parsed.document.outer_models.size() == 1U);
        const auto& outer = parsed.document.outer_models.front();
        assert(outer.aggregate_element_count == 1U);
        assert(outer.alpha_control == 0x90U);
        assert(outer.source_flags == 0x00004000U);
        assert(outer.bounding_center.x == 10.0F);
        assert(outer.bounding_center.y == -20.0F);
        assert(outer.bounding_center.z == 30.0F);
        assert(outer.bounding_radius == 42.5F);
        assert(outer.meshes.size() == 1U);
        const auto& mesh = outer.meshes.front();
        assert(mesh.element_count == 1U);
        assert(mesh.texture_slot == 7U);
        assert(mesh.gs_clamp_region_repeat.min_u == 1U);
        assert(mesh.gs_clamp_region_repeat.max_u == 2U);
        assert(mesh.gs_clamp_region_repeat.min_v == 3U);
        assert(mesh.gs_clamp_region_repeat.max_v == 4U);
        assert(mesh.positions.size() == 1U);
        assert(mesh.positions[0].x == 1.0F);
        assert(mesh.positions[0].y == 2.0F);
        assert(mesh.positions[0].z == 3.0F);
        assert(mesh.uvs[0].u == 4096);
        assert(mesh.skin.size() == 1U);
        assert(mesh.skin[0].ok());
        assert(mesh.skin[0].skin.influence_count == 1U);
        assert(mesh.skin[0].skin.influences[0].bone_index == 0U);
        assert(parsed.document.transform_domain.permutation_is_complete);
        assert(parsed.document.transform_domain.hierarchy_is_topological);
        assert(parsed.document.transform_domain.hierarchy_candidate_is_acyclic);
        assert(parsed.document.transform_domain.transform_records_complete);
        assert(parsed.document.transform_domain.transform_records_finite);
    }

    {
        auto bytes = make_valid_mod();
        put_u16(bytes, 0x84U, 0x0400U);
        const auto parsed = mod::Parser::parse(bytes);
        assert(parsed.ok());
        assert(has_diagnostic(parsed, "mod.gs-clamp-field-out-of-range"));
        assert(parsed.document.outer_models[0].meshes[0]
                   .gs_clamp_region_repeat.min_u == 0x0400U);
    }

    {
        auto bytes = make_valid_mod();
        bytes.resize(0x20U);
        const auto parsed = mod::Parser::parse(bytes);
        assert(parsed.recognized);
        assert(!parsed.ok());
        assert(has_diagnostic(parsed, "mod.truncated-header"));
    }

    {
        auto bytes = make_valid_mod();
        put_u16(bytes, 0x42U, 2U);
        const auto parsed = mod::Parser::parse(bytes);
        assert(!parsed.ok());
        assert(has_diagnostic(parsed, "mod.aggregate-element-count-mismatch"));
    }

    {
        auto bytes = make_valid_mod();
        put_u64(bytes, 0x90U, 0x258U);
        const auto parsed = mod::Parser::parse(bytes);
        assert(!parsed.ok());
        assert(has_diagnostic(parsed, "mod.mesh-stream-out-of-bounds"));
    }

    {
        auto bytes = make_valid_mod();
        put_u16(bytes, 0x110U, 0U);
        const auto parsed = mod::Parser::parse(bytes);
        assert(parsed.ok());
        assert(has_diagnostic(parsed, "mod.skin-invariant-mismatch"));
        assert(parsed.document.outer_models[0].meshes[0].skin_decode_failures == 1U);
    }

    {
        auto bytes = make_valid_mod();
        // Adapter relative pointer escapes the payload. All four canonical
        // relative pointers are range-checked before any table is consumed.
        put_u32(bytes, 0x208U, 0x1000U);
        const auto transform = mod::transform_domain::parse(bytes);
        assert(transform.recognized);
        assert(!transform.ok());
        bool saw_range_error = false;
        for (const auto& diagnostic : transform.diagnostics) {
            if (diagnostic.code == "mod.transform_domain.table_range") {
                saw_range_error = true;
            }
        }
        assert(saw_range_error);
    }

    {
        auto bytes = make_runtime_postload_fixture();
        const auto base = static_cast<std::uint64_t>(
            reinterpret_cast<std::uintptr_t>(bytes.data()));
        const auto result = mod::runtime_postload::apply_in_place(bytes);
        assert(result.ok());
        assert(result.object_count == 1U);
        assert(result.mesh_count == 1U);
        assert(result.generated_word_count == 9U);

        assert(get_u64(bytes, 0x20U) == base + 0x300U);
        assert(get_u64(bytes, 0x48U) == base + 0x100U);
        assert(get_u64(bytes, 0x110U) == base + 0x310U);
        assert(get_u64(bytes, 0x118U) == base + 0x320U);
        assert(get_u64(bytes, 0x120U) == base + 0x330U);
        assert(get_u64(bytes, 0x128U) == base + 0x340U);
        assert(get_u64(bytes, 0x130U) == base + 0x200U);
        assert(get_u64(bytes, 0x138U) == 0x350U);
        assert(get_u64(bytes, 0x140U) == base + 0x180U);

        for (std::uint16_t index = 0U; index < 6U; ++index) {
            assert(get_u16(bytes, 0x200U + index * 2U) == index);
        }
        constexpr std::array<std::uint16_t, 9> expected{
            0U, 1U, 2U, 3U, 3U, 3U, 3U, 4U, 5U,
        };
        for (std::size_t index = 0U; index < expected.size(); ++index) {
            assert(get_u16(bytes, 0x180U + index * 2U) == expected[index]);
        }
        assert(get_u32(bytes, 0x148U) == expected.size());
    }

    {
        auto bytes = make_runtime_postload_fixture();
        put_u64(bytes, 0x140U, 0x1000U);
        const auto header_before = get_u64(bytes, 0x20U);
        const auto mesh_table_before = get_u64(bytes, 0x48U);
        const auto result = mod::runtime_postload::apply_in_place(bytes);
        assert(!result.ok());
        assert(result.status ==
               mod::runtime_postload::Status::generated_workspace_out_of_bounds);
        // Safe reconstruction validates the whole pass before pointer mutation.
        assert(get_u64(bytes, 0x20U) == header_before);
        assert(get_u64(bytes, 0x48U) == mesh_table_before);
    }

    return 0;
}