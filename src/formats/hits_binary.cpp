#include "dmc_rengine/formats/hits_binary.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::formats::hits {
namespace {

inline constexpr std::string_view evidence_id =
    "dmc3-hits-exe-confirmed";

[[nodiscard]] std::string offset_id(
    std::string_view prefix,
    std::uint64_t offset) {
    std::ostringstream output;
    output << prefix << '-' << std::hex << std::setfill('0')
           << std::setw(8) << offset;
    return output.str();
}

[[nodiscard]] std::string number_text(std::uint64_t value) {
    return std::to_string(value);
}

[[nodiscard]] std::string hex_text(std::uint32_t value) {
    std::ostringstream output;
    output << "0x" << std::uppercase << std::hex << std::setfill('0')
           << std::setw(8) << value;
    return output.str();
}

[[nodiscard]] std::string float_text(float value) {
    std::ostringstream output;
    output << std::setprecision(9) << value;
    return output.str();
}

bool add_field(
    binary::Document& document,
    std::string id,
    std::string name,
    std::uint64_t offset,
    std::uint64_t size,
    binary::FieldKind kind,
    std::string type,
    std::string value,
    std::string parent = {}) {
    return document.add_field(binary::Field{
        .id = std::move(id),
        .name = std::move(name),
        .range = {.offset = offset, .size = size},
        .kind = kind,
        .type_name = std::move(type),
        .display_value = std::move(value),
        .parent_id = std::move(parent),
        .evidence_id = std::string(evidence_id),
    });
}

bool add_vec3(
    binary::Document& document,
    std::string_view id,
    std::string_view name,
    std::uint64_t offset,
    const Vec3& value,
    std::string_view parent) {
    const auto structure_id = std::string(id);
    if (!add_field(document, structure_id, std::string(name), offset, 12U,
            binary::FieldKind::structure, "Vec3", {}, std::string(parent))) {
        return false;
    }
    return add_field(document, structure_id + "-x", "X", offset, 4U,
               binary::FieldKind::floating_point, "f32_le", float_text(value.x), structure_id) &&
        add_field(document, structure_id + "-y", "Y", offset + 4U, 4U,
               binary::FieldKind::floating_point, "f32_le", float_text(value.y), structure_id) &&
        add_field(document, structure_id + "-z", "Z", offset + 8U, 4U,
               binary::FieldKind::floating_point, "f32_le", float_text(value.z), structure_id);
}

bool add_cell_extent(
    binary::Document& document,
    std::uint64_t offset,
    const CellExtent& value,
    std::string_view parent) {
    const auto structure_id = std::string("hits-cell-size");
    if (!add_field(document, structure_id, "Cell extent", offset, 12U,
            binary::FieldKind::structure, "u32_le[3]", {}, std::string(parent))) {
        return false;
    }
    return add_field(document, structure_id + "-x", "X extent", offset, 4U,
               binary::FieldKind::unsigned_integer, "u32_le",
               number_text(value.x), structure_id) &&
        add_field(document, structure_id + "-y", "Y extent", offset + 4U, 4U,
               binary::FieldKind::unsigned_integer, "u32_le",
               number_text(value.y), structure_id) &&
        add_field(document, structure_id + "-z", "Z extent", offset + 8U, 4U,
               binary::FieldKind::unsigned_integer, "u32_le",
               number_text(value.z), structure_id);
}

} // namespace

std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ScanResult& scan) {
    if (!resource.valid() || resource.id.size != bytes.size() ||
        !scan.recognized || bytes.size() < header_size) {
        return std::nullopt;
    }

    binary::Document document(std::move(resource), bytes.size());
    if (!document.add_region(binary::Region{
            .id = "hits-header",
            .name = "HITS header",
            .range = {.offset = 0U, .size = header_size},
            .kind = binary::RegionKind::header,
            .type_name = "HitsHeader_EXE",
            .evidence_id = std::string(evidence_id),
        })) {
        return std::nullopt;
    }

    const auto header_parent = std::string("hits-header-struct");
    if (!add_field(document, header_parent, "HITS header structure", 0U,
            header_size, binary::FieldKind::structure, "HitsHeader_EXE", {}) ||
        !add_field(document, "hits-magic", "Magic", 0U, 4U,
            binary::FieldKind::string, "char[4]", "HITS", header_parent) ||
        !add_field(document, "hits-end-offset", "Terminal array end offset", 0x04U, 4U,
            binary::FieldKind::unsigned_integer, "u32_le",
            number_text(scan.header.end_offset), header_parent) ||
        !add_vec3(document, "hits-bounds-min", "Bounds minimum", 0x08U,
            scan.header.bounds_min, header_parent) ||
        !add_vec3(document, "hits-bounds-max", "Bounds maximum", 0x14U,
            scan.header.bounds_max, header_parent) ||
        !add_cell_extent(document, 0x20U,
            scan.header.cell_size, header_parent) ||
        !add_field(document, "hits-grid-x", "Grid count X", 0x2CU, 4U,
            binary::FieldKind::unsigned_integer, "u32_le",
            number_text(scan.header.grid_count_x), header_parent) ||
        !add_field(document, "hits-grid-y", "Grid count Y", 0x30U, 4U,
            binary::FieldKind::unsigned_integer, "u32_le",
            number_text(scan.header.grid_count_y), header_parent) ||
        !add_field(document, "hits-grid-z", "Grid count Z", 0x34U, 4U,
            binary::FieldKind::unsigned_integer, "u32_le",
            number_text(scan.header.grid_count_z), header_parent) ||
        !add_field(document, "hits-triangle-count", "Triangle count", 0x38U, 4U,
            binary::FieldKind::unsigned_integer, "u32_le",
            number_text(scan.header.triangle_count), header_parent) ||
        !add_field(document, "hits-spatial-relative", "Spatial table relative offset", 0x3CU, 4U,
            binary::FieldKind::pointer, "u32_le relative to fileBase + 8",
            number_text(scan.header.spatial_table_relative_offset), header_parent) ||
        !add_field(document, "hits-triangle-relative", "Triangle array relative offset", 0x40U, 4U,
            binary::FieldKind::pointer, "u32_le relative to fileBase + 8",
            number_text(scan.header.triangle_array_relative_offset), header_parent)) {
        return std::nullopt;
    }

    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = "formats.hits.header",
        .range = {.offset = 0U, .size = header_size},
        .rationale = "EXE-confirmed HITS header fields, including integer cell extents at +0x20.",
    }));

    const auto spatial_offset = scan.header.spatial_offset();
    const auto triangle_offset = scan.header.triangle_offset();
    if (triangle_offset > spatial_offset) {
        if (!document.add_region(binary::Region{
                .id = "hits-spatial-index",
                .name = "HITS spatial index",
                .range = {.offset = spatial_offset,
                          .size = triangle_offset - spatial_offset},
                .kind = binary::RegionKind::table,
                .type_name = "HitsSpatialIndex",
                .evidence_id = std::string(evidence_id),
            })) {
            return std::nullopt;
        }
        static_cast<void>(document.add_ownership(binary::OwnershipClaim{
            .owner_id = "formats.hits.spatial-index",
            .range = {.offset = spatial_offset,
                      .size = triangle_offset - spatial_offset},
            .rationale = "Cell pointers and -1 terminated triangle-offset lists.",
        }));
    }

    for (const auto& triangle : scan.triangles) {
        const auto id = offset_id("hits-triangle", triangle.offset);
        if (!document.add_region(binary::Region{
                .id = id,
                .name = "HITS triangle-plane record",
                .range = {.offset = triangle.offset, .size = triangle_size},
                .kind = binary::RegionKind::record,
                .type_name = "HitsTriangle_EXE",
                .evidence_id = std::string(evidence_id),
            }) ||
            !add_field(document, id + "-struct", "Triangle-plane structure",
                triangle.offset, triangle_size, binary::FieldKind::structure,
                "HitsTriangle_EXE", {}) ||
            !add_field(document, id + "-flags", "Raw flags",
                triangle.offset, 4U, binary::FieldKind::unsigned_integer,
                "u32_le bitfield", hex_text(triangle.flags), id + "-struct") ||
            !add_vec3(document, id + "-point-a", "Point A",
                triangle.offset + 0x04U, triangle.point_a, id + "-struct") ||
            !add_vec3(document, id + "-point-b", "Point B",
                triangle.offset + 0x10U, triangle.point_b, id + "-struct") ||
            !add_vec3(document, id + "-point-c", "Point C",
                triangle.offset + 0x1CU, triangle.point_c, id + "-struct") ||
            !add_vec3(document, id + "-normal", "Plane normal",
                triangle.offset + 0x28U, triangle.normal, id + "-struct") ||
            !add_field(document, id + "-plane-d", "Plane D",
                triangle.offset + 0x34U, 4U,
                binary::FieldKind::floating_point, "f32_le",
                float_text(triangle.plane_d), id + "-struct")) {
            return std::nullopt;
        }
        static_cast<void>(document.add_ownership(binary::OwnershipClaim{
            .owner_id = "formats.hits.triangles",
            .range = {.offset = triangle.offset, .size = triangle_size},
            .rationale = "EXE-confirmed 0x38 triangle-plane record.",
        }));
    }

    return document;
}

} // namespace dmc::rengine::formats::hits
