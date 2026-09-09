#include "dmc_rengine/formats/mod_writer.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

void put_u8(std::vector<std::byte>& bytes,
            std::size_t offset,
            std::uint8_t value) {
    bytes[offset] = static_cast<std::byte>(value);
}

void put_u16(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint16_t value) {
    put_u8(bytes, offset + 0U,
           static_cast<std::uint8_t>(value & 0xFFU));
    put_u8(bytes, offset + 1U,
           static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
}

void put_u32(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(
                   (value >> (index * 8U)) & 0xFFU));
    }
}

void put_u64(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(
                   (value >> (index * 8U)) & 0xFFU));
    }
}

void put_f32(std::vector<std::byte>& bytes,
             std::size_t offset,
             float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void put_ascii(std::vector<std::byte>& bytes,
               std::size_t offset,
               std::string_view text) {
    for (std::size_t index = 0U; index < text.size(); ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(text[index]));
    }
}

std::vector<std::byte> make_valid_mod() {
    std::vector<std::byte> bytes(0x260U, std::byte{0});
    put_ascii(bytes, 0x00U, "MOD ");
    put_f32(bytes, 0x04U, 1.01F);
    put_u8(bytes, 0x10U, 1U);
    put_u8(bytes, 0x11U, 1U);
    put_u8(bytes, 0x12U, 8U);
    put_u8(bytes, 0x13U, 0x5AU);
    put_u32(bytes, 0x14U, 0x12345678U);
    put_u64(bytes, 0x20U, 0x200U);

    put_u8(bytes, 0x40U, 1U);
    put_u8(bytes, 0x41U, 0x90U);
    put_u16(bytes, 0x42U, 1U);
    put_u64(bytes, 0x48U, 0x80U);
    put_u32(bytes, 0x50U, 0x00004000U);
    put_f32(bytes, 0x70U, 10.0F);
    put_f32(bytes, 0x74U, -20.0F);
    put_f32(bytes, 0x78U, 30.0F);
    put_f32(bytes, 0x7CU, 42.5F);

    put_u16(bytes, 0x80U, 1U);
    put_u16(bytes, 0x82U, 7U);
    put_u16(bytes, 0x84U, 1U);
    put_u16(bytes, 0x86U, 2U);
    put_u16(bytes, 0x88U, 3U);
    put_u16(bytes, 0x8AU, 4U);
    put_u64(bytes, 0x90U, 0xD0U);
    put_u64(bytes, 0x98U, 0xE0U);
    put_u64(bytes, 0xA0U, 0xF0U);
    put_u64(bytes, 0xA8U, 0x100U);
    put_u64(bytes, 0xB0U, 0x110U);
    put_u64(bytes, 0xB8U, 0U);
    put_u64(bytes, 0xC0U, 0xA0U);
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
    put_u16(bytes, 0x110U, 0x001FU);

    put_u32(bytes, 0x200U, 0x20U);
    put_u32(bytes, 0x204U, 0x24U);
    put_u32(bytes, 0x208U, 0x28U);
    put_u32(bytes, 0x20CU, 0x30U);
    put_u8(bytes, 0x220U, 0xFFU);
    put_u8(bytes, 0x224U, 0U);
    put_u8(bytes, 0x228U, 0U);
    return bytes;
}

bool has_diagnostic(
    const dmc::rengine::formats::mod::WriteResult& result,
    std::string_view code) {
    for (const auto& diagnostic : result.diagnostics) {
        if (diagnostic.code == code) return true;
    }
    return false;
}

bool allowed_edit_byte(std::size_t offset) {
    return (offset >= 0x7CU && offset < 0x80U) ||
           (offset >= 0xD0U && offset < 0xD4U) ||
           (offset >= 0xF0U && offset < 0xF2U);
}

} // namespace

int main() {
    namespace mod = dmc::rengine::formats::mod;

    const auto source = make_valid_mod();
    const auto source_span = std::span<const std::byte>{
        source.data(), source.size()};
    const auto parsed = mod::Parser::parse(source_span);
    assert(parsed.ok());

    {
        const auto written = mod::Writer::write(source_span, parsed.document);
        assert(written.ok());
        assert(written.bytes == source);
        assert(written.reparsed.document.outer_models[0].bounding_radius ==
               42.5F);
        assert(written.receipt.valid());
        assert(written.receipt.source_sha256 == written.receipt.output_sha256);
        assert(written.receipt.byte_count == source.size());
        assert(written.receipt.modified_byte_count == 0U);
        assert(written.receipt.source_image_matches_document);
        assert(written.receipt.unauthorized_bytes_unchanged);
        assert(written.receipt.output_reparse_ok);
        assert(written.receipt.no_edit_byte_identical);
    }

    {
        auto edited = parsed.document;
        auto& outer = edited.outer_models[0];
        auto& mesh = outer.meshes[0];
        outer.bounding_radius = 50.0F;
        mesh.positions[0].x = 9.0F;
        mesh.uvs[0].u = -1024;

        const auto written = mod::Writer::write(source_span, edited);
        assert(written.ok());
        assert(written.bytes.size() == source.size());
        assert(written.reparsed.document.outer_models[0].bounding_radius ==
               50.0F);
        assert(written.reparsed.document.outer_models[0]
                   .meshes[0].positions[0].x == 9.0F);
        assert(written.reparsed.document.outer_models[0]
                   .meshes[0].uvs[0].u == -1024);
        assert(written.receipt.modified_byte_count > 0U);
        assert(written.receipt.source_sha256 != written.receipt.output_sha256);
        assert(written.receipt.unauthorized_bytes_unchanged);
        assert(!written.receipt.no_edit_byte_identical);

        for (std::size_t offset = 0U; offset < source.size(); ++offset) {
            if (!allowed_edit_byte(offset)) {
                assert(written.bytes[offset] == source[offset]);
            }
        }
    }

    {
        auto edited = parsed.document;
        edited.outer_models[0].source_flags ^= 0x00004000U;
        const auto written = mod::Writer::write(source_span, edited);
        assert(!written.ok());
        assert(has_diagnostic(written,
                              "mod.writer.unsupported-object-edit"));
    }

    {
        auto edited = parsed.document;
        edited.outer_models[0].meshes[0].reserved4c = 1U;
        const auto written = mod::Writer::write(source_span, edited);
        assert(!written.ok());
        assert(has_diagnostic(written,
                              "mod.writer.unsupported-mesh-edit"));
    }

    {
        auto edited = parsed.document;
        edited.outer_models[0].meshes[0].positions.push_back(
            mod::Vec3f{1.0F, 2.0F, 3.0F});
        const auto written = mod::Writer::write(source_span, edited);
        assert(!written.ok());
        assert(has_diagnostic(written,
                              "mod.writer.stream-size-change"));
    }

    {
        auto edited = parsed.document;
        edited.transform_domain.local_transform_records_by_node_index[0]
            .translation.x = 1.0F;
        const auto written = mod::Writer::write(source_span, edited);
        assert(!written.ok());
        assert(has_diagnostic(written,
                              "mod.writer.unsupported-transform-edit"));
    }

    {
        auto edited = parsed.document;
        edited.source_bytes[0x14U] ^= std::byte{0x01};
        const auto written = mod::Writer::write(source_span, edited);
        assert(!written.ok());
        assert(has_diagnostic(written,
                              "mod.writer.source-image-mismatch"));
    }

    return 0;
}
