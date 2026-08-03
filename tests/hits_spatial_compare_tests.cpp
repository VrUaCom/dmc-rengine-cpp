#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/spatial_compare.hpp"
#include "dmc_rengine/hits/writer.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
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
    constexpr std::size_t triangle_offset = 0x80U;
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
    write_u32(bytes, 0x40U, 0x78U);
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[0x44U + index] = static_cast<std::byte>(0xB0U + index);
    }
    write_i32(bytes, 0x4CU, 0x4CU);
    write_i32(bytes, 0x50U, 0x54U);
    write_i32(bytes, 0x54U, 0);
    write_i32(bytes, 0x58U, -1);
    write_i32(bytes, 0x5CU, 0x38);
    write_i32(bytes, 0x60U, -1);
    write_triangle(bytes, triangle_offset, 0x18060001U, -2.0F);
    write_triangle(bytes, triangle_offset + 0x38U, 0x00000001U, 3.0F);
    return bytes;
}

[[nodiscard]] bool near(double left, double right) {
    return std::fabs(left - right) < 1.0e-12;
}

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

[[nodiscard]] double numeric_value(
    const dmc::rengine::core::json::Value& value) {
    if (const auto* floating = value.as_double(); floating != nullptr) {
        return *floating;
    }
    if (const auto* unsigned_integer = value.as_u64();
        unsigned_integer != nullptr) {
        return static_cast<double>(*unsigned_integer);
    }
    if (const auto* signed_integer = value.as_i64();
        signed_integer != nullptr) {
        return static_cast<double>(*signed_integer);
    }
    assert(false);
    return 0.0;
}

[[nodiscard]] bool has_diagnostic(
    const dmc::rengine::hits::spatial_compare::SpatialComparisonReport& report,
    std::string_view code) {
    return std::any_of(
        report.diagnostics.begin(), report.diagnostics.end(),
        [code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
}

} // namespace

int main() {
    using namespace dmc::rengine;
    using formats::hits::RecordScanner;
    using formats::hits::Vec3;
    using hits::spatial_compare::ComparisonStatus;
    using hits::spatial_compare::compare;
    using hits::spatial_compare::make_surface_mapping;
    using hits::spatial_compare::report_json;
    using hits::writer::GridPolicy;
    using hits::writer::RebuildOptions;
    using hits::writer::SpatialWriter;

    const auto source_bytes = make_fixture();
    const auto original = RecordScanner::scan(source_bytes);
    assert(original.ok());
    const auto original_surfaces = SpatialWriter::surfaces_from_scan(
        original, 1000U);

    const auto exact_rebuild = SpatialWriter::rebuild(
        original, source_bytes, original_surfaces);
    assert(exact_rebuild.ok());
    const auto exact_candidate = RecordScanner::scan(exact_rebuild.bytes);
    const auto exact_mapping = make_surface_mapping(
        original, original_surfaces, exact_rebuild.locations);
    assert(exact_candidate.ok());
    assert(exact_mapping.size() == 2U);

    const auto exact_report = compare(
        original,
        exact_candidate,
        exact_mapping,
        "hits-spatial-exact",
        "synthetic-original",
        "synthetic-sat-writer",
        "rengine-triangle-box-sat-v1");
    assert(exact_report.comparable());
    assert(exact_report.exact());
    assert(exact_report.status == ComparisonStatus::comparable);
    assert(exact_report.metrics.original_reference_count == 2U);
    assert(exact_report.metrics.candidate_reference_count == 2U);
    assert(exact_report.metrics.shared_reference_count == 2U);
    assert(exact_report.metrics.missing_reference_count == 0U);
    assert(exact_report.metrics.extra_reference_count == 0U);
    assert(exact_report.metrics.exact_cell_count == 2U);
    assert(exact_report.metrics.exact_surface_count == 2U);
    assert(near(exact_report.metrics.precision, 1.0));
    assert(near(exact_report.metrics.recall, 1.0));
    assert(near(exact_report.metrics.jaccard, 1.0));

    const auto exact_json = report_json(exact_report);
    assert(!exact_json.empty());
    assert(exact_json == report_json(exact_report));
    const auto parsed_json = core::json::Parser::parse(exact_json);
    assert(parsed_json.ok());
    const auto* root = parsed_json.value->as_object();
    assert(root != nullptr);
    const auto* status = member(*root, "status");
    const auto* exact = member(*root, "exact");
    const auto* metrics_value = member(*root, "metrics");
    assert(status != nullptr && status->as_string() != nullptr);
    assert(*status->as_string() == "comparable");
    assert(exact != nullptr && exact->as_bool() != nullptr);
    assert(*exact->as_bool());
    assert(metrics_value != nullptr && metrics_value->as_object() != nullptr);
    const auto* metrics = metrics_value->as_object();
    const auto* shared = member(*metrics, "shared_reference_count");
    const auto* precision = member(*metrics, "precision");
    assert(shared != nullptr && shared->as_u64() != nullptr);
    assert(*shared->as_u64() == 2U);
    assert(precision != nullptr);
    assert(near(numeric_value(*precision), 1.0));

    auto reordered_surfaces = original_surfaces;
    std::reverse(reordered_surfaces.begin(), reordered_surfaces.end());
    const auto reordered_rebuild = SpatialWriter::rebuild(
        original, source_bytes, reordered_surfaces);
    assert(reordered_rebuild.ok());
    const auto reordered_candidate = RecordScanner::scan(
        reordered_rebuild.bytes);
    const auto reordered_mapping = make_surface_mapping(
        original, original_surfaces, reordered_rebuild.locations);
    assert(reordered_mapping.size() == 2U);
    assert(reordered_mapping[0].stable_id == 1000U);
    assert(reordered_mapping[0].candidate_triangle_byte_offset == 0x38U);
    assert(reordered_mapping[1].candidate_triangle_byte_offset == 0U);
    const auto reordered_report = compare(
        original,
        reordered_candidate,
        reordered_mapping,
        "hits-spatial-reordered",
        "synthetic-original",
        "synthetic-reordered",
        "rengine-triangle-box-sat-v1");
    assert(reordered_report.comparable());
    assert(reordered_report.exact());

    auto crossing_surfaces = original_surfaces;
    crossing_surfaces[0].point_a = Vec3{-1.0F, 0.0F, 0.0F};
    crossing_surfaces[0].point_b = Vec3{1.0F, 0.0F, 0.0F};
    crossing_surfaces[0].point_c = Vec3{-1.0F, 0.0F, 1.0F};
    const auto crossing_rebuild = SpatialWriter::rebuild(
        original, source_bytes, crossing_surfaces);
    assert(crossing_rebuild.ok());
    const auto crossing_report = compare(
        original,
        RecordScanner::scan(crossing_rebuild.bytes),
        make_surface_mapping(
            original, original_surfaces, crossing_rebuild.locations),
        "hits-spatial-extra",
        "synthetic-original",
        "synthetic-cross-cell",
        "rengine-triangle-box-sat-v1");
    assert(crossing_report.comparable());
    assert(!crossing_report.exact());
    assert(crossing_report.metrics.original_reference_count == 2U);
    assert(crossing_report.metrics.candidate_reference_count == 3U);
    assert(crossing_report.metrics.shared_reference_count == 2U);
    assert(crossing_report.metrics.missing_reference_count == 0U);
    assert(crossing_report.metrics.extra_reference_count == 1U);
    assert(crossing_report.metrics.exact_cell_count == 1U);
    assert(crossing_report.metrics.exact_surface_count == 1U);
    assert(near(crossing_report.metrics.precision, 2.0 / 3.0));
    assert(near(crossing_report.metrics.recall, 1.0));
    assert(near(crossing_report.metrics.jaccard, 2.0 / 3.0));
    assert(crossing_report.cells[1].extra_surfaces ==
           std::vector<hits::writer::StableSurfaceId>{1000U});
    assert(crossing_report.surfaces[0].extra_cells ==
           std::vector<std::uint32_t>{1U});

    auto moved_surfaces = original_surfaces;
    moved_surfaces[0].point_a = Vec3{2.0F, 0.0F, 0.0F};
    moved_surfaces[0].point_b = Vec3{3.0F, 0.0F, 0.0F};
    moved_surfaces[0].point_c = Vec3{2.0F, 0.0F, 1.0F};
    const auto moved_rebuild = SpatialWriter::rebuild(
        original, source_bytes, moved_surfaces);
    assert(moved_rebuild.ok());
    const auto moved_report = compare(
        original,
        RecordScanner::scan(moved_rebuild.bytes),
        make_surface_mapping(
            original, original_surfaces, moved_rebuild.locations),
        "hits-spatial-moved",
        "synthetic-original",
        "synthetic-moved",
        "rengine-triangle-box-sat-v1");
    assert(moved_report.comparable());
    assert(!moved_report.exact());
    assert(moved_report.metrics.original_reference_count == 2U);
    assert(moved_report.metrics.candidate_reference_count == 2U);
    assert(moved_report.metrics.shared_reference_count == 1U);
    assert(moved_report.metrics.missing_reference_count == 1U);
    assert(moved_report.metrics.extra_reference_count == 1U);
    assert(near(moved_report.metrics.precision, 0.5));
    assert(near(moved_report.metrics.recall, 0.5));
    assert(near(moved_report.metrics.jaccard, 1.0 / 3.0));
    assert(moved_report.cells[0].missing_surfaces ==
           std::vector<hits::writer::StableSurfaceId>{1000U});
    assert(moved_report.cells[1].extra_surfaces ==
           std::vector<hits::writer::StableSurfaceId>{1000U});

    auto outside_surfaces = original_surfaces;
    outside_surfaces[0].point_a = Vec3{25.0F, 0.0F, 0.0F};
    outside_surfaces[0].point_b = Vec3{26.0F, 0.0F, 0.0F};
    outside_surfaces[0].point_c = Vec3{25.0F, 0.0F, 1.0F};
    const auto fit_rebuild = SpatialWriter::rebuild(
        original,
        source_bytes,
        outside_surfaces,
        RebuildOptions{
            .grid_policy = GridPolicy::fit_bounds_preserve_cell_size,
            .assignment_policy =
                hits::writer::AssignmentPolicy::rengine_triangle_box_sat,
            .bounds_padding = 1.0F,
            .overlap_epsilon = 1.0e-7,
        });
    assert(fit_rebuild.ok());
    const auto incompatible = compare(
        original,
        RecordScanner::scan(fit_rebuild.bytes),
        make_surface_mapping(
            original, original_surfaces, fit_rebuild.locations),
        "hits-spatial-incompatible-grid",
        "synthetic-original",
        "synthetic-fit-grid",
        "rengine-triangle-box-sat-v1");
    assert(!incompatible.comparable());
    assert(incompatible.status == ComparisonStatus::incompatible_grid);
    assert(has_diagnostic(
        incompatible, "hits.spatial_compare.incompatible_grid"));

    auto bad_mapping = exact_mapping;
    bad_mapping[1].stable_id = bad_mapping[0].stable_id;
    const auto invalid_mapping = compare(
        original,
        exact_candidate,
        bad_mapping,
        "hits-spatial-invalid-mapping",
        "synthetic-original",
        "synthetic-candidate",
        "rengine-triangle-box-sat-v1");
    assert(!invalid_mapping.comparable());
    assert(invalid_mapping.status ==
           ComparisonStatus::invalid_surface_mapping);
    assert(has_diagnostic(
        invalid_mapping, "hits.spatial_compare.invalid_mapping_entry"));

    auto duplicate_reference_scan = exact_candidate;
    duplicate_reference_scan.cells[0].triangle_byte_offsets.push_back(0U);
    const auto invalid_reference = compare(
        original,
        duplicate_reference_scan,
        exact_mapping,
        "hits-spatial-duplicate-reference",
        "synthetic-original",
        "synthetic-invalid-reference",
        "rengine-triangle-box-sat-v1");
    assert(!invalid_reference.comparable());
    assert(invalid_reference.status ==
           ComparisonStatus::invalid_reference_stream);
    assert(has_diagnostic(
        invalid_reference,
        "hits.spatial_compare.duplicate_cell_reference"));

    const auto incomplete_mapping = std::vector<
        hits::spatial_compare::SurfaceOffsetMapping>{exact_mapping.front()};
    const auto incomplete = compare(
        original,
        exact_candidate,
        incomplete_mapping,
        "hits-spatial-incomplete-mapping",
        "synthetic-original",
        "synthetic-candidate",
        "rengine-triangle-box-sat-v1");
    assert(!incomplete.comparable());
    assert(has_diagnostic(
        incomplete, "hits.spatial_compare.mapping_count_mismatch"));

    return 0;
}
