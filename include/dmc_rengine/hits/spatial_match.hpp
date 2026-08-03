#pragma once

#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/spatial_compare.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace dmc::rengine::hits::spatial_match {

enum class GeometryMatchStatus : std::uint8_t {
    matched = 0,
    invalid_original_scan = 1,
    invalid_candidate_scan = 2,
    triangle_count_mismatch = 3,
    unmatched_geometry = 4,
    ambiguous_geometry = 5,
    stable_id_overflow = 6,
};

[[nodiscard]] constexpr std::string_view to_string(
    GeometryMatchStatus status) noexcept {
    switch (status) {
    case GeometryMatchStatus::matched: return "matched";
    case GeometryMatchStatus::invalid_original_scan:
        return "invalid-original-scan";
    case GeometryMatchStatus::invalid_candidate_scan:
        return "invalid-candidate-scan";
    case GeometryMatchStatus::triangle_count_mismatch:
        return "triangle-count-mismatch";
    case GeometryMatchStatus::unmatched_geometry:
        return "unmatched-geometry";
    case GeometryMatchStatus::ambiguous_geometry:
        return "ambiguous-geometry";
    case GeometryMatchStatus::stable_id_overflow:
        return "stable-id-overflow";
    }
    return "unmatched-geometry";
}

struct GeometryMatchResult final {
    GeometryMatchStatus status{GeometryMatchStatus::unmatched_geometry};
    std::vector<spatial_compare::SurfaceOffsetMapping> mappings;
    std::vector<formats::ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        return status == GeometryMatchStatus::matched;
    }
};

[[nodiscard]] GeometryMatchResult match_unique_geometry(
    const formats::hits::ScanResult& original_scan,
    const formats::hits::ScanResult& candidate_scan,
    writer::StableSurfaceId first_stable_id = 1U);

} // namespace dmc::rengine::hits::spatial_match
