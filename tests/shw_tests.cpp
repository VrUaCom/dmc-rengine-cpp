#include "dmc_rengine/formats/shw.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
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
    for (std::size_t index = 0U; index < 4U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>((value >> (index * 8U)) & 0xFFU));
    }
}

void put_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>((value >> (index * 8U)) & 0xFFU));
    }
}

void put_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void put_triangle(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t a,
    std::uint32_t b,
    std::uint32_t c) {
    put_u32(bytes, offset + 0x00U, a);
    put_u32(bytes, offset + 0x04U, b);
    put_u32(bytes, offset + 0x08U, c);
    put_u32(bytes, offset + 0x0CU, 0U);
}

void put_adjacency(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint16_t a,
    std::uint16_t b,
    std::uint16_t c) {
    put_u16(bytes, offset + 0x00U, a);
    put_u16(bytes, offset + 0x02U, b);
    put_u16(bytes, offset + 0x04U, c);
    put_u16(bytes, offset + 0x06U, 0U);
}

void put_vertex(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float x,
    float y,
    float z) {
    put_f32(bytes, offset + 0x00U, x);
    put_f32(bytes, offset + 0x04U, y);
    put_f32(bytes, offset + 0x08U, z);
    put_f32(bytes, offset + 0x0CU, 1.0F);
}

[[nodiscard]] std::vector<std::byte> tetrahedron_shw() {
    // One closed tetrahedral hull. The layout mirrors the hash-bound real SHW
    // grammar without copying proprietary payload bytes.
    std::vector<std::byte> bytes(0x110U, std::byte{0});
    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'H'};
    bytes[2] = std::byte{'W'};
    bytes[3] = std::byte{' '};
    put_f32(bytes, 0x04U, 0.5F);
    put_u8(bytes, 0x10U, 1U);

    const std::size_t hull = 0x20U;
    put_u16(bytes, hull + 0x00U, 4U);
    put_u16(bytes, hull + 0x02U, 4U);
    put_u64(bytes, hull + 0x10U, 0x60U);
    put_u64(bytes, hull + 0x18U, 0xA0U);
    put_u64(bytes, hull + 0x20U, 0xC0U);
    put_u64(bytes, hull + 0x28U, 0x100U);

    put_triangle(bytes, 0x60U, 0U, 1U, 2U);
    put_triangle(bytes, 0x70U, 0U, 3U, 1U);
    put_triangle(bytes, 0x80U, 1U, 3U, 2U);
    put_triangle(bytes, 0x90U, 2U, 3U, 0U);

    // Every tetrahedron face shares one edge with each of the other faces.
    put_adjacency(bytes, 0xA0U, 1U, 2U, 3U);
    put_adjacency(bytes, 0xA8U, 0U, 2U, 3U);
    put_adjacency(bytes, 0xB0U, 0U, 1U, 3U);
    put_adjacency(bytes, 0xB8U, 0U, 1U, 2U);

    put_vertex(bytes, 0xC0U, 0.0F, 0.0F, 0.0F);
    put_vertex(bytes, 0xD0U, 1.0F, 0.0F, 0.0F);
    put_vertex(bytes, 0xE0U, 0.0F, 1.0F, 0.0F);
    put_vertex(bytes, 0xF0U, 0.0F, 0.0F, 1.0F);

    put_u8(bytes, 0x100U, 0U);
    put_u8(bytes, 0x101U, 1U);
    put_u8(bytes, 0x102U, 2U);
    put_u8(bytes, 0x103U, 3U);
    return bytes;
}

[[nodiscard]] bool has_diag(
    const dmc::rengine::formats::shw::ParseResult& parsed,
    std::string_view code) {
    return std::any_of(
        parsed.diagnostics.begin(), parsed.diagnostics.end(),
        [code](const dmc::rengine::formats::ParseDiagnostic& diagnostic) {
            return diagnostic.code == code;
        });
}

} // namespace

int main() {
    using dmc::rengine::formats::shw::Parser;

    const auto valid_bytes = tetrahedron_shw();
    const auto valid = Parser::parse(valid_bytes);
    assert(valid.recognized);
    assert(valid.ok());
    assert(valid.document.header.version == 0.5F);
    assert(valid.document.header.hull_count == 1U);
    assert(valid.document.hulls.size() == 1U);
    const auto& hull = valid.document.hulls.front();
    assert(hull.vertex_count == 4U);
    assert(hull.triangle_count == 4U);
    assert(hull.triangles.size() == 4U);
    assert(hull.adjacency.size() == 4U);
    assert(hull.vertices.size() == 4U);
    assert(hull.transform_selectors.size() == 4U);
    assert(hull.transform_selectors[3] == 3U);
    assert(valid.document.source_bytes == valid_bytes);

    auto wrong_magic = valid_bytes;
    wrong_magic[0] = std::byte{'X'};
    const auto unrecognized = Parser::parse(wrong_magic);
    assert(!unrecognized.recognized);
    assert(!unrecognized.ok());

    std::vector<std::byte> truncated(valid_bytes.begin(), valid_bytes.begin() + 0x10U);
    const auto short_header = Parser::parse(truncated);
    assert(short_header.recognized);
    assert(!short_header.ok());
    assert(has_diag(short_header, "shw.truncated-header"));

    auto bad_pointer = valid_bytes;
    put_u64(bad_pointer, 0x20U + 0x10U, 0x1000U);
    const auto pointer_error = Parser::parse(bad_pointer);
    assert(pointer_error.recognized);
    assert(!pointer_error.ok());
    assert(has_diag(pointer_error, "shw.triangle-span-out-of-bounds"));

    auto bad_vertex_index = valid_bytes;
    put_u32(bad_vertex_index, 0x60U, 99U);
    const auto vertex_error = Parser::parse(bad_vertex_index);
    assert(vertex_error.recognized);
    assert(!vertex_error.ok());
    assert(has_diag(vertex_error, "shw.triangle-vertex-out-of-range"));

    auto variant_version = valid_bytes;
    put_f32(variant_version, 0x04U, 0.6F);
    const auto variant = Parser::parse(variant_version);
    assert(variant.recognized);
    assert(variant.ok());
    assert(has_diag(variant, "shw.unconfirmed-version"));

    auto variant_adjacency = valid_bytes;
    put_u16(variant_adjacency, 0xA0U, 0U);
    const auto adjacency_variant = Parser::parse(variant_adjacency);
    assert(adjacency_variant.recognized);
    assert(adjacency_variant.ok());
    assert(has_diag(
        adjacency_variant,
        "shw.adjacency-not-complete-edge-neighborhood"));

    using dmc::rengine::gdspaces::ResourceId;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRef;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::integration::WorkspaceContext;
    using dmc::rengine::integration::WorkspaceEventType;

    const ResourceRef shw_resource{
        .id = ResourceId{
            .source_id = "shw-native-reader-test",
            .logical_path = "model/test.shw",
            .container_chain = "NBZ[0]/PAC[0]",
            .offset = 0U,
            .size = valid_bytes.size(),
        },
        .display_name = "test.shw",
        .format = "shw",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };

    ProjectWorkspace project;
    assert(project.create_session(ResourcePayload{
        .resource = shw_resource,
        .bytes = valid_bytes,
        .diagnostics = {},
    }, WorkspaceContext{}));

    const auto report = ResourceAnalyzer::analyze(project, shw_resource.id);
    assert(report.ok());
    assert(report.parser_available);
    assert(report.recognized);
    assert(report.parser_id == "formats.shw-structural-v1");
    assert(!report.binary_document_attached);

    const auto* session = project.find_session(shw_resource.id);
    assert(session != nullptr);
    assert(session->format() != nullptr);
    assert(session->parser_validation() != nullptr);
    assert(session->parser_validation()->parser_id == "formats.shw-structural-v1");
    assert(session->events().by_type(WorkspaceEventType::parser_completed).size() == 1U);

    return 0;
}
