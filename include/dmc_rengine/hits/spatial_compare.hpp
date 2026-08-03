#pragma once

#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/writer.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::hits::spatial_compare {

enum class ComparisonStatus : std::uint8_t {
    comparable = 0,
    invalid_original_scan = 1,
    invalid_candidate_scan = 2,
    incompatible_grid = 3,
    invalid_surface_mapping = 4,
    invalid_reference_stream = 5,
};

[[nodiscard]] constexpr std::string_view to_string(
    ComparisonStatus status) noexcept {
    switch (status) {
    case ComparisonStatus::comparable: return "comparable";
    case ComparisonStatus::invalid_original_scan:
        return "invalid-original-scan";
    case ComparisonStatus::invalid_candidate_scan:
        return "invalid-candidate-scan";
    case ComparisonStatus::incompatible_grid:
        return "incompatible-grid";
    case ComparisonStatus::invalid_surface_mapping:
        return "invalid-surface-mapping";
    case ComparisonStatus::invalid_reference_stream:
        return "invalid-reference-stream";
    }
    return "invalid-surface-mapping";
}

struct SurfaceOffsetMapping final {
    writer::StableSurfaceId stable_id{};
    std::uint32_t original_triangle_byte_offset{};
    std::uint32_t candidate_triangle_byte_offset{};
};

struct CellDifference final {
    std::uint32_t cell_index{};
    std::vector<writer::StableSurfaceId> original_surfaces;
    std::vector<writer::StableSurfaceId> candidate_surfaces;
    std::vector<writer::StableSurfaceId> missing_surfaces;
    std::vector<writer::StableSurfaceId> extra_surfaces;

    [[nodiscard]] bool exact() const noexcept {
        return missing_surfaces.empty() && extra_surfaces.empty();
    }
};

struct SurfaceDifference final {
    writer::StableSurfaceId stable_id{};
    std::vector<std::uint32_t> original_cells;
    std::vector<std::uint32_t> candidate_cells;
    std::vector<std::uint32_t> missing_cells;
    std::vector<std::uint32_t> extra_cells;

    [[nodiscard]] bool exact() const noexcept {
        return missing_cells.empty() && extra_cells.empty();
    }
};

struct ComparisonMetrics final {
    std::uint64_t original_reference_count{};
    std::uint64_t candidate_reference_count{};
    std::uint64_t shared_reference_count{};
    std::uint64_t missing_reference_count{};
    std::uint64_t extra_reference_count{};
    std::uint64_t exact_cell_count{};
    std::uint64_t exact_surface_count{};
    double precision{};
    double recall{};
    double jaccard{};

    [[nodiscard]] bool exact() const noexcept {
        return missing_reference_count == 0U &&
               extra_reference_count == 0U;
    }
};

struct SpatialComparisonReport final {
    std::string report_id;
    std::string original_identity;
    std::string candidate_identity;
    std::string assignment_policy;
    ComparisonStatus status{ComparisonStatus::invalid_surface_mapping};
    ComparisonMetrics metrics;
    std::vector<CellDifference> cells;
    std::vector<SurfaceDifference> surfaces;
    std::vector<formats::ParseDiagnostic> diagnostics;

    [[nodiscard]] bool comparable() const noexcept {
        return status == ComparisonStatus::comparable;
    }

    [[nodiscard]] bool exact() const noexcept {
        return comparable() && metrics.exact();
    }
};

[[nodiscard]] std::vector<SurfaceOffsetMapping> make_surface_mapping(
    const formats::hits::ScanResult& original_scan,
    std::span<const writer::Surface> original_surfaces,
    std::span<const writer::SurfaceLocation> candidate_locations);

[[nodiscard]] SpatialComparisonReport compare(
    const formats::hits::ScanResult& original_scan,
    const formats::hits::ScanResult& candidate_scan,
    std::span<const SurfaceOffsetMapping> mappings,
    std::string report_id,
    std::string original_identity,
    std::string candidate_identity,
    std::string assignment_policy);

[[nodiscard]] std::string report_json(
    const SpatialComparisonReport& report);

} // namespace dmc::rengine::hits::spatial_compare
