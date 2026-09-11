#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/contact.hpp"
#include "dmc_rengine/hits/edit.hpp"
#include "dmc_rengine/hits/result.hpp"
#include "dmc_rengine/hits/runtime.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
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

void write_triangle(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t flags,
    float origin_x) {
    write_u32(bytes, offset, flags);
    write_vec3(bytes, offset + 0x04U, origin_x, 0.0F, 0.0F);
    write_vec3(bytes, offset + 0x10U, origin_x + 1.0F, 0.0F, 0.0F);
    write_vec3(bytes, offset + 0x1CU, origin_x, 0.0F, 1.0F);
    write_vec3(bytes, offset + 0x28U, 0.0F, -1.0F, 0.0F);
    write_f32(bytes, offset + 0x34U, 0.0F);
}

[[nodiscard]] std::vector<std::byte> make_fixture() {
    constexpr std::size_t pointer_table_offset = 0x44U;
    constexpr std::size_t lists_offset = 0x4CU;
    constexpr std::size_t triangle_offset = 0x64U;
    constexpr std::size_t end_offset = triangle_offset + 2U * 0x38U;
    std::vector<std::byte> bytes(end_offset, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    write_u32(bytes, 0x04U, static_cast<std::uint32_t>(end_offset));
    write_vec3(bytes, 0x08U, -10.0F, -2.0F, -10.0F);
    write_vec3(bytes, 0x14U, 10.0F, 2.0F, 10.0F);
    write_u32(bytes, 0x20U, 10U);
    write_u32(bytes, 0x24U, 4U);
    write_u32(bytes, 0x28U, 20U);
    write_u32(bytes, 0x2CU, 2U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 2U);
    write_u32(bytes, 0x3CU, 0x3CU);
    write_u32(bytes, 0x40U, static_cast<std::uint32_t>(triangle_offset - 8U));

    write_i32(bytes, pointer_table_offset,
              static_cast<std::int32_t>(lists_offset - 8U));
    write_i32(bytes, pointer_table_offset + 4U,
              static_cast<std::int32_t>(lists_offset + 8U - 8U));
    write_i32(bytes, lists_offset, 0);
    write_i32(bytes, lists_offset + 4U, -1);
    write_i32(bytes, lists_offset + 8U, 0x38);
    write_i32(bytes, lists_offset + 12U, -1);

    write_triangle(bytes, triangle_offset, 0x18060001U, -2.0F);
    write_triangle(bytes, triangle_offset + 0x38U, 0x00000001U, 3.0F);
    return bytes;
}

} // namespace

int main() {
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::Vec3;
    using dmc::rengine::formats::hits::triangle_size;
    using dmc::rengine::hits::contact::NormalYClass;
    using dmc::rengine::hits::contact::apply_normal_correction;
    using dmc::rengine::hits::contact::classify_normal_y;
    using dmc::rengine::hits::contact::query_static_candidates;
    using dmc::rengine::hits::edit::SpatialSafety;
    using dmc::rengine::hits::edit::classify_spatial_safety;
    using dmc::rengine::hits::edit::prepare_safe_edit;
    using dmc::rengine::hits::edit::recompute_geometry;
    using dmc::rengine::hits::edit::serialize_safe_copy;
    using dmc::rengine::hits::edit::serialize_topology_preserving_copy;
    using dmc::rengine::hits::runtime::ContactResult;
    using dmc::rengine::hits::runtime::GridCoordinate;
    using dmc::rengine::hits::runtime::StaticSource;
    using dmc::rengine::hits::runtime::StaticSourceSelector;
    using dmc::rengine::hits::runtime::enumerate_cells;
    using dmc::rengine::hits::runtime::flatten;
    using dmc::rengine::hits::runtime::make_contact_result;
    using dmc::rengine::hits::runtime::maximum_plane_residual;
    using dmc::rengine::hits::runtime::select_nearest_contact;
    using dmc::rengine::hits::runtime::world_to_grid;

    static_assert(triangle_size == 0x38U);

    const auto bytes = make_fixture();
    const auto scan = RecordScanner::scan(std::span<const std::byte>{bytes});
    assert(scan.recognized);
    assert(scan.ok());
    assert(scan.header.cell_size.x == 10U);
    assert(scan.header.cell_size.y == 4U);
    assert(scan.header.cell_size.z == 20U);
    assert(scan.header.cell_count() == 2U);
    assert(scan.header.spatial_offset() == 0x44U);
    assert(scan.header.triangle_offset() == 0x64U);
    assert(scan.cells.size() == 2U);
    assert(scan.triangles.size() == 2U);
    assert(scan.triangles[0].flags == 0x18060001U);
    assert(maximum_plane_residual(scan.triangles[0]) == 0.0F);

    const auto first_cell = world_to_grid(
        scan.header, Vec3{-9.0F, 0.0F, -9.0F});
    assert((first_cell == GridCoordinate{0U, 0U, 0U}));
    const auto second_cell = world_to_grid(
        scan.header, Vec3{9.0F, 0.0F, 9.0F});
    assert((second_cell == GridCoordinate{1U, 0U, 0U}));
    assert(flatten(scan.header, GridCoordinate{1U, 0U, 0U}) == 1U);
    assert(!flatten(scan.header, GridCoordinate{2U, 0U, 0U}));
    const auto cells = enumerate_cells(
        scan.header,
        Vec3{-9.0F, 0.0F, -9.0F},
        Vec3{9.0F, 0.0F, 9.0F});
    assert(cells == std::vector<std::uint32_t>({0U, 1U}));

    StaticSourceSelector selector(true, true);
    assert(selector.select(StaticSource::source_1_member_6));
    assert(selector.restore_default());
    assert(selector.selected() == StaticSource::source_0_member_3);

    assert(classify_normal_y(Vec3{1.0F, 0.0F, 0.0F}) == NormalYClass::low);
    assert(classify_normal_y(Vec3{0.0F, 0.6F, 0.0F}) == NormalYClass::middle);
    assert(classify_normal_y(Vec3{0.0F, 1.0F, 0.0F}) == NormalYClass::high);
    const auto corrected = apply_normal_correction(
        Vec3{1.0F, 2.0F, 3.0F}, Vec3{0.0F, 1.0F, 0.0F}, 2.5F);
    assert(corrected.has_value());
    assert((corrected.value() == Vec3{1.0F, 4.5F, 3.0F}));

    const auto candidates = query_static_candidates(
        scan,
        StaticSource::source_0_member_3,
        Vec3{-9.0F, 0.0F, -9.0F},
        Vec3{9.0F, 0.0F, 9.0F},
        0U);
    assert(candidates.has_value());
    assert(candidates->size() == 2U);
    const auto first_contact = make_contact_result(
        (*candidates)[0], Vec3{-1.5F, 0.0F, 0.25F}, 2.0F);
    const auto second_contact = make_contact_result(
        (*candidates)[1], Vec3{3.25F, 0.0F, 0.25F}, 1.0F);
    assert(first_contact.has_value());
    assert(second_contact.has_value());
    const std::vector<ContactResult> contacts{*first_contact, *second_contact};
    const auto nearest = select_nearest_contact(contacts);
    assert(nearest.has_value());
    assert(nearest->triangle_index == 1U);

    const auto edit = prepare_safe_edit(
        scan.triangles[0],
        0U,
        Vec3{-2.0F, 1.0F, 0.0F},
        Vec3{-1.0F, 1.0F, 0.0F},
        Vec3{-2.0F, 1.0F, 1.0F});
    assert(edit.has_value());
    assert(edit->preserved_flags == 0x18060001U);
    assert(edit->geometry.normal.y == -1.0F);
    assert(edit->geometry.plane_d == 1.0F);
    assert(classify_spatial_safety(scan, *edit) ==
           SpatialSafety::safe_without_rebuild);

    const auto edited_bytes = serialize_safe_copy(scan, bytes, *edit);
    assert(edited_bytes.has_value());
    assert(edited_bytes->size() == bytes.size());
    assert(std::equal(bytes.begin(), bytes.begin() + 0x64U, edited_bytes->begin()));
    const auto edited_scan = RecordScanner::scan(*edited_bytes);
    assert(edited_scan.ok());
    assert(edited_scan.triangles.size() == scan.triangles.size());
    assert(edited_scan.triangles[0].flags == scan.triangles[0].flags);
    assert(edited_scan.triangles[0].point_a.y == 1.0F);
    assert(edited_scan.triangles[0].normal.y == -1.0F);
    assert(edited_scan.triangles[0].plane_d == 1.0F);
    assert(maximum_plane_residual(edited_scan.triangles[0]) == 0.0F);
    assert(edited_scan.cells[0].triangle_byte_offsets ==
           scan.cells[0].triangle_byte_offsets);
    assert(edited_scan.cells[1].triangle_byte_offsets ==
           scan.cells[1].triangle_byte_offsets);

    const auto moved_to_other_cell = prepare_safe_edit(
        scan.triangles[0],
        0U,
        Vec3{3.0F, 0.0F, 0.0F},
        Vec3{4.0F, 0.0F, 0.0F},
        Vec3{3.0F, 0.0F, 1.0F});
    assert(moved_to_other_cell.has_value());
    assert(classify_spatial_safety(scan, *moved_to_other_cell) ==
           SpatialSafety::requires_spatial_rebuild);
    assert(!serialize_safe_copy(scan, bytes, *moved_to_other_cell));
    assert(serialize_topology_preserving_copy(bytes, *moved_to_other_cell));

    assert(!recompute_geometry(
        Vec3{0.0F, 0.0F, 0.0F},
        Vec3{1.0F, 0.0F, 0.0F},
        Vec3{2.0F, 0.0F, 0.0F}));

    const std::vector<std::byte> wrong_magic{
        std::byte{'N'}, std::byte{'O'}, std::byte{'P'}, std::byte{'E'}};
    assert(!RecordScanner::scan(wrong_magic).recognized);
    const std::vector<std::byte> truncated{
        std::byte{'H'}, std::byte{'I'}, std::byte{'T'}, std::byte{'S'}};
    const auto truncated_result = RecordScanner::scan(truncated);
    assert(truncated_result.recognized);
    assert(!truncated_result.ok());
    assert(truncated_result.diagnostics[0].severity == ParseSeverity::error);

    return 0;
}
