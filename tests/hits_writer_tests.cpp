#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/evidence_profile.hpp"
#include "dmc_rengine/hits/runtime.hpp"
#include "dmc_rengine/hits/writer.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_i32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::int32_t value) {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
}

void write_f32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float value) {
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
    write_vec3(bytes, 0x20U, 10.0F, 4.0F, 20.0F);
    write_u32(bytes, 0x2CU, 2U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 2U);
    write_u32(bytes, 0x3CU, 0x3CU);
    write_u32(bytes, 0x40U, static_cast<std::uint32_t>(triangle_offset - 8U));

    write_i32(bytes, pointer_table_offset, static_cast<std::int32_t>(lists_offset - 8U));
    write_i32(bytes, pointer_table_offset + 4U, static_cast<std::int32_t>(lists_offset + 8U - 8U));
    write_i32(bytes, lists_offset, 0);
    write_i32(bytes, lists_offset + 4U, -1);
    write_i32(bytes, lists_offset + 8U, 0x38);
    write_i32(bytes, lists_offset + 12U, -1);

    write_triangle(bytes, triangle_offset, 0x18060001U, -2.0F);
    write_triangle(bytes, triangle_offset + 0x38U, 0x00000001U, 3.0F);
    return bytes;
}

[[nodiscard]] bool has_diagnostic(
    const dmc::rengine::hits::writer::RebuildResult& result,
    std::string_view code) {
    return std::any_of(
        result.diagnostics.begin(), result.diagnostics.end(),
        [code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
}

[[nodiscard]] std::int32_t read_i32(
    std::span<const std::byte> bytes,
    std::size_t offset) {
    std::uint32_t value{};
    for (std::size_t index = 0U; index < 4U; ++index) {
        value |= static_cast<std::uint32_t>(bytes[offset + index])
                 << static_cast<unsigned>(index * 8U);
    }
    return static_cast<std::int32_t>(value);
}

} // namespace

int main() {
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::Vec3;
    using dmc::rengine::hits::evidence::StaticSourceProfile;
    using dmc::rengine::hits::evidence::profile;
    using dmc::rengine::hits::evidence::split_flags;
    using dmc::rengine::hits::runtime::maximum_plane_residual;
    using dmc::rengine::hits::writer::GridPolicy;
    using dmc::rengine::hits::writer::SpatialWriter;
    using dmc::rengine::hits::writer::Surface;

    const auto source_bytes = make_fixture();
    const auto source_scan = RecordScanner::scan(source_bytes);
    assert(source_scan.ok());
    assert(source_scan.header.spatial_offset() == 0x44U);
    assert(source_scan.header.triangle_offset() == 0x64U);
    assert(source_scan.cells[0].pointer_offset == 0x44U);
    assert(source_scan.cells[0].list_offset == 0x4CU);
    assert(source_scan.cells[1].pointer_offset == 0x48U);
    assert(source_scan.cells[1].list_offset == 0x54U);

    const auto flags = split_flags(0x182E0001U);
    assert(flags.raw == 0x182E0001U);
    assert(flags.upper_query_mask == 0x182EU);
    assert(flags.lower_surface_value == 0x0001U);
    const auto source_1 = profile(StaticSourceProfile::source_1_member_6);
    assert(source_1.label == "Source 1 / PAC member 6");
    assert(source_1.semantic_boundary.find("RESEARCH REQUIRED") != std::string_view::npos);

    auto surfaces = SpatialWriter::surfaces_from_scan(source_scan, 100U);
    assert(surfaces.size() == 2U);
    assert(surfaces[0].stable_id == 100U);
    assert(surfaces[1].stable_id == 101U);

    const auto rebuilt = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        surfaces);
    assert(rebuilt.ok());
    assert(has_diagnostic(
        rebuilt,
        "hits.writer.modified_topology_game_validation_required"));
    assert(rebuilt.bytes.size() % 16U == 0U);
    assert(rebuilt.header.end_offset <= rebuilt.bytes.size());
    assert(rebuilt.header.spatial_offset() == 0x44U);
    assert(rebuilt.header.triangle_count == 2U);
    assert(rebuilt.locations.size() == 2U);
    assert(rebuilt.locations[0].stable_id == 100U);
    assert(rebuilt.locations[0].triangle_index == 0U);
    assert(rebuilt.locations[0].triangle_byte_offset == 0U);
    assert(rebuilt.locations[1].stable_id == 101U);
    assert(rebuilt.locations[1].triangle_byte_offset == 0x38U);

    const auto triangle_offset = static_cast<std::size_t>(
        rebuilt.header.triangle_offset());
    assert(read_i32(rebuilt.bytes, triangle_offset - 4U) == -1);
    assert(std::all_of(
        rebuilt.bytes.begin() +
            static_cast<std::ptrdiff_t>(rebuilt.header.end_offset),
        rebuilt.bytes.end(),
        [](std::byte value) { return value == std::byte{0}; }));

    const auto parsed = RecordScanner::scan(rebuilt.bytes);
    assert(parsed.ok());
    assert(parsed.cells.size() == 2U);
    assert(parsed.cells[0].pointer_offset == 0x44U);
    assert(parsed.cells[1].pointer_offset == 0x48U);
    assert(parsed.cells[0].list_offset != parsed.cells[1].list_offset);
    assert(parsed.cells[0].triangle_byte_offsets ==
           std::vector<std::uint32_t>{0U});
    assert(parsed.cells[1].triangle_byte_offsets ==
           std::vector<std::uint32_t>{0x38U});
    assert(parsed.triangles[0].flags == 0x18060001U);
    assert(parsed.triangles[1].flags == 0x00000001U);
    assert(maximum_plane_residual(parsed.triangles[0]) == 0.0F);
    assert(maximum_plane_residual(parsed.triangles[1]) == 0.0F);

    const auto deterministic = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        surfaces);
    assert(deterministic.ok());
    assert(deterministic.bytes == rebuilt.bytes);

    std::reverse(surfaces.begin(), surfaces.end());
    const auto reordered = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        surfaces);
    assert(reordered.ok());
    assert(reordered.locations[0].stable_id == 101U);
    assert(reordered.locations[1].stable_id == 100U);
    const auto reordered_scan = RecordScanner::scan(reordered.bytes);
    assert(reordered_scan.ok());
    assert(reordered_scan.triangles[0].flags == 0x00000001U);
    assert(reordered_scan.triangles[1].flags == 0x18060001U);
    assert(reordered_scan.cells[0].triangle_byte_offsets ==
           std::vector<std::uint32_t>{0x38U});
    assert(reordered_scan.cells[1].triangle_byte_offsets ==
           std::vector<std::uint32_t>{0U});

    auto with_crossing_surface = SpatialWriter::surfaces_from_scan(
        source_scan,
        200U);
    with_crossing_surface.push_back(Surface{
        .stable_id = 202U,
        .flags = 0x18260001U,
        .point_a = Vec3{-1.0F, 0.0F, 0.0F},
        .point_b = Vec3{1.0F, 0.0F, 0.0F},
        .point_c = Vec3{-1.0F, 0.0F, 1.0F},
    });
    const auto added = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        with_crossing_surface);
    assert(added.ok());
    const auto added_scan = RecordScanner::scan(added.bytes);
    assert(added_scan.ok());
    assert(added_scan.triangles.size() == 3U);
    assert(std::find(
        added_scan.cells[0].triangle_byte_offsets.begin(),
        added_scan.cells[0].triangle_byte_offsets.end(),
        0x70U) != added_scan.cells[0].triangle_byte_offsets.end());
    assert(std::find(
        added_scan.cells[1].triangle_byte_offsets.begin(),
        added_scan.cells[1].triangle_byte_offsets.end(),
        0x70U) != added_scan.cells[1].triangle_byte_offsets.end());

    auto one_surface = SpatialWriter::surfaces_from_scan(source_scan, 300U);
    one_surface.erase(one_surface.begin());
    const auto deleted = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        one_surface);
    assert(deleted.ok());
    const auto deleted_scan = RecordScanner::scan(deleted.bytes);
    assert(deleted_scan.ok());
    assert(deleted_scan.triangles.size() == 1U);
    assert(deleted_scan.cells[0].triangle_byte_offsets.empty());
    assert(deleted_scan.cells[1].triangle_byte_offsets ==
           std::vector<std::uint32_t>{0U});

    auto moved = SpatialWriter::surfaces_from_scan(source_scan, 400U);
    moved[0].point_a = Vec3{25.0F, 0.0F, 0.0F};
    moved[0].point_b = Vec3{26.0F, 0.0F, 0.0F};
    moved[0].point_c = Vec3{25.0F, 0.0F, 1.0F};
    const auto rejected_preserve = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        moved);
    assert(!rejected_preserve.ok());
    assert(has_diagnostic(
        rejected_preserve,
        "hits.writer.surface_outside_preserved_grid"));

    const auto fitted = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        moved,
        dmc::rengine::hits::writer::RebuildOptions{
            .grid_policy = GridPolicy::fit_bounds_preserve_cell_size,
            .assignment_policy =
                dmc::rengine::hits::writer::AssignmentPolicy::
                    capcom_triangle_box_sat,
            .bounds_padding = 1.0F,
            .overlap_epsilon = 1.0e-7,
        });
    assert(fitted.ok());
    const auto fitted_scan = RecordScanner::scan(fitted.bytes);
    assert(fitted_scan.ok());
    assert(fitted_scan.header.bounds_min.x <= 2.0F);
    assert(fitted_scan.header.bounds_max.x >= 26.0F);
    assert(fitted_scan.header.cell_size == source_scan.header.cell_size);
    assert(fitted_scan.triangles.size() == 2U);

    auto duplicate_ids = SpatialWriter::surfaces_from_scan(source_scan, 500U);
    duplicate_ids[1].stable_id = duplicate_ids[0].stable_id;
    const auto duplicate_rejected = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        duplicate_ids);
    assert(!duplicate_rejected.ok());
    assert(has_diagnostic(
        duplicate_rejected,
        "hits.writer.invalid_stable_surface_id"));

    auto degenerate = SpatialWriter::surfaces_from_scan(source_scan, 600U);
    degenerate[0].point_a = Vec3{0.0F, 0.0F, 0.0F};
    degenerate[0].point_b = Vec3{1.0F, 0.0F, 0.0F};
    degenerate[0].point_c = Vec3{2.0F, 0.0F, 0.0F};
    const auto degenerate_rejected = SpatialWriter::rebuild(
        source_scan,
        source_bytes,
        degenerate);
    assert(!degenerate_rejected.ok());
    assert(has_diagnostic(
        degenerate_rejected,
        "hits.writer.invalid_surface_geometry"));

    return 0;
}
