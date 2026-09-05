#include "dmc_rengine/formats/mod.hpp"

#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
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

std::vector<std::byte> make_valid_mod() {
    std::vector<std::byte> bytes(0x240U, std::byte{0});
    put_ascii(bytes, 0x00U, "MOD ");
    put_f32(bytes, 0x04U, 1.01F);
    put_u8(bytes, 0x10U, 1U); // outer records
    put_u8(bytes, 0x11U, 1U); // transform-domain nodes
    put_u64(bytes, 0x20U, 0x200U);

    put_u8(bytes, 0x40U, 1U); // one inner mesh
    put_u16(bytes, 0x42U, 1U); // aggregate elements
    put_u64(bytes, 0x48U, 0x80U);

    put_u16(bytes, 0x80U, 1U);
    put_u64(bytes, 0x90U, 0xD0U);  // positions
    put_u64(bytes, 0x98U, 0xE0U);  // normals
    put_u64(bytes, 0xA0U, 0xF0U);  // UV
    put_u64(bytes, 0xA8U, 0x100U); // blend indices
    put_u64(bytes, 0xB0U, 0x110U); // packed weights/control
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

    // Transform-domain document. Offsets are relative to 0x200.
    put_u32(bytes, 0x200U, 0x10U);
    put_u32(bytes, 0x204U, 0x20U);
    put_u32(bytes, 0x208U, 0x30U);
    put_u8(bytes, 0x210U, 0xFFU); // root reference
    put_u8(bytes, 0x220U, 0U);    // complete permutation
    put_u8(bytes, 0x230U, 0U);    // third table preserved/unknown here
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
        assert(parsed.document.outer_models.size() == 1U);
        const auto& outer = parsed.document.outer_models.front();
        assert(outer.aggregate_element_count == 1U);
        assert(outer.meshes.size() == 1U);
        const auto& mesh = outer.meshes.front();
        assert(mesh.element_count == 1U);
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
        assert(parsed.document.transform_domain.hierarchy_candidate_is_acyclic);
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
        put_u64(bytes, 0x90U, 0x238U);
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
        // table2 relative pointer escapes the payload. This previously slipped
        // through transform_domain because only table0/table1 were read.
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

    return 0;
}
