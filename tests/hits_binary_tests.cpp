#include "dmc_rengine/formats/hits_binary.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

void write_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_i32(std::vector<std::byte>& bytes, std::size_t offset, std::int32_t value) {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
}

void write_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    write_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void write_vec3(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float x,
    float y,
    float z) {
    write_f32(bytes, offset, x);
    write_f32(bytes, offset + 4U, y);
    write_f32(bytes, offset + 8U, z);
}

[[nodiscard]] std::vector<std::byte> make_fixture() {
    constexpr std::size_t pointer_table_offset = 0x44U;
    constexpr std::size_t list_offset = 0x48U;
    constexpr std::size_t triangle_offset = 0x50U;
    constexpr std::size_t end_offset = triangle_offset + 0x38U;
    std::vector<std::byte> bytes(end_offset, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    write_u32(bytes, 0x04U, static_cast<std::uint32_t>(end_offset));
    write_vec3(bytes, 0x08U, -1.0F, -1.0F, -1.0F);
    write_vec3(bytes, 0x14U, 1.0F, 1.0F, 1.0F);
    write_u32(bytes, 0x20U, 2U);
    write_u32(bytes, 0x24U, 2U);
    write_u32(bytes, 0x28U, 2U);
    write_u32(bytes, 0x2CU, 1U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 1U);
    write_u32(bytes, 0x3CU, 0x3CU);
    write_u32(bytes, 0x40U, static_cast<std::uint32_t>(triangle_offset - 8U));
    write_i32(bytes, pointer_table_offset,
              static_cast<std::int32_t>(list_offset - 8U));
    write_i32(bytes, list_offset, 0);
    write_i32(bytes, list_offset + 4U, -1);

    write_u32(bytes, triangle_offset, 0x10040001U);
    write_vec3(bytes, triangle_offset + 0x04U, 0.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x10U, 1.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x1CU, 0.0F, 0.0F, 1.0F);
    write_vec3(bytes, triangle_offset + 0x28U, 0.0F, 1.0F, 0.0F);
    write_f32(bytes, triangle_offset + 0x34U, 0.0F);
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "synthetic",
            .logical_path = "stage/collision.hits",
            .container_chain = "PAC[3]",
            .offset = 4096,
            .size = size,
        },
        .display_name = "collision.hits",
        .format = "hits",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

} // namespace

int main() {
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::build_binary_document;

    const auto bytes = make_fixture();
    const auto scan = RecordScanner::scan(bytes);
    assert(scan.ok());

    auto document = build_binary_document(resource(bytes.size()), bytes, scan);
    assert(document.has_value());
    assert(document->regions().size() == 3U);
    assert(document->ownership().size() == 3U);
    assert(document->conflicts().empty());
    assert(document->ownership_conflicts().empty());

    const auto* header = document->find_region("hits-header");
    assert(header != nullptr);
    assert(header->range.size == 0x44U);

    const auto* cell_x = document->find_field("hits-cell-size-x");
    const auto* cell_y = document->find_field("hits-cell-size-y");
    const auto* cell_z = document->find_field("hits-cell-size-z");
    assert(cell_x != nullptr && cell_x->display_value == "2");
    assert(cell_y != nullptr && cell_y->display_value == "2");
    assert(cell_z != nullptr && cell_z->display_value == "2");

    const auto* spatial = document->find_region("hits-spatial-index");
    assert(spatial != nullptr);
    assert(spatial->range.offset == 0x44U);

    const auto* triangle = document->find_region("hits-triangle-00000050");
    assert(triangle != nullptr);
    assert(triangle->range.size == 0x38U);

    const auto* flags = document->find_field("hits-triangle-00000050-flags");
    assert(flags != nullptr);
    assert(flags->display_value == "0x10040001");

    const auto* normal_y = document->find_field(
        "hits-triangle-00000050-normal-y");
    assert(normal_y != nullptr);
    assert(normal_y->display_value == "1");

    const auto* plane_d = document->find_field(
        "hits-triangle-00000050-plane-d");
    assert(plane_d != nullptr);
    assert(plane_d->display_value == "0");

    const auto selection = document->selection_at(0x50U);
    assert(selection.regions.size() == 1U);
    assert(selection.fields.size() == 2U);
    assert(selection.owners.size() == 1U);

    assert(!build_binary_document(
        resource(bytes.size() + 1U), bytes, scan).has_value());

    return 0;
}
