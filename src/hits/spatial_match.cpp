#include "dmc_rengine/hits/spatial_match.hpp"

#include <array>
#include <bit>
#include <limits>
#include <map>
#include <string>
#include <utility>

namespace dmc::rengine::hits::spatial_match {
namespace {

using GeometryKey = std::array<std::uint32_t, 10U>;
using formats::ParseDiagnostic;
using formats::ParseSeverity;
using formats::hits::Triangle;

void add_diagnostic(
    GeometryMatchResult& result,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset = 0U) {
    result.diagnostics.push_back(ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

[[nodiscard]] GeometryKey geometry_key(const Triangle& triangle) noexcept {
    return GeometryKey{
        triangle.flags,
        std::bit_cast<std::uint32_t>(triangle.point_a.x),
        std::bit_cast<std::uint32_t>(triangle.point_a.y),
        std::bit_cast<std::uint32_t>(triangle.point_a.z),
        std::bit_cast<std::uint32_t>(triangle.point_b.x),
        std::bit_cast<std::uint32_t>(triangle.point_b.y),
        std::bit_cast<std::uint32_t>(triangle.point_b.z),
        std::bit_cast<std::uint32_t>(triangle.point_c.x),
        std::bit_cast<std::uint32_t>(triangle.point_c.y),
        std::bit_cast<std::uint32_t>(triangle.point_c.z),
    };
}

using OffsetBuckets = std::map<GeometryKey, std::vector<std::uint32_t>>;

[[nodiscard]] OffsetBuckets build_buckets(
    const formats::hits::ScanResult& scan) {
    OffsetBuckets buckets;
    for (std::size_t index = 0U; index < scan.triangles.size(); ++index) {
        const auto offset = static_cast<std::uint32_t>(
            index * formats::hits::triangle_size);
        buckets[geometry_key(scan.triangles[index])].push_back(offset);
    }
    return buckets;
}

} // namespace

GeometryMatchResult match_unique_geometry(
    const formats::hits::ScanResult& original_scan,
    const formats::hits::ScanResult& candidate_scan,
    writer::StableSurfaceId first_stable_id) {
    GeometryMatchResult result;
    if (!original_scan.ok()) {
        result.status = GeometryMatchStatus::invalid_original_scan;
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.spatial_match.invalid_original_scan",
            "The original HITS scan is not structurally valid.");
        return result;
    }
    if (!candidate_scan.ok()) {
        result.status = GeometryMatchStatus::invalid_candidate_scan;
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.spatial_match.invalid_candidate_scan",
            "The candidate HITS scan is not structurally valid.");
        return result;
    }
    if (original_scan.triangles.size() != candidate_scan.triangles.size()) {
        result.status = GeometryMatchStatus::triangle_count_mismatch;
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.spatial_match.triangle_count_mismatch",
            "Automatic geometry matching requires identical triangle counts.");
        return result;
    }
    if (!original_scan.triangles.empty() &&
        first_stable_id == 0U) {
        result.status = GeometryMatchStatus::stable_id_overflow;
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.spatial_match.invalid_first_stable_id",
            "Stable surface IDs must begin at a non-zero value.");
        return result;
    }
    if (original_scan.triangles.size() >
        static_cast<std::size_t>(
            std::numeric_limits<writer::StableSurfaceId>::max() -
            first_stable_id + 1U)) {
        result.status = GeometryMatchStatus::stable_id_overflow;
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.spatial_match.stable_id_overflow",
            "The triangle count cannot be represented in the requested stable-ID range.");
        return result;
    }

    const auto original_buckets = build_buckets(original_scan);
    const auto candidate_buckets = build_buckets(candidate_scan);
    if (original_buckets.size() != candidate_buckets.size()) {
        result.status = GeometryMatchStatus::unmatched_geometry;
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.spatial_match.geometry_domain_mismatch",
            "Original and candidate files do not contain the same unique geometry domain.");
        return result;
    }

    std::map<std::uint32_t, std::uint32_t> candidate_by_original_offset;
    for (const auto& [key, original_offsets] : original_buckets) {
        const auto candidate = candidate_buckets.find(key);
        if (candidate == candidate_buckets.end() ||
            candidate->second.size() != original_offsets.size()) {
            result.status = GeometryMatchStatus::unmatched_geometry;
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.spatial_match.unmatched_geometry",
                "At least one original triangle geometry has no one-to-one candidate match.",
                original_offsets.empty() ? 0U : original_offsets.front());
            return result;
        }
        if (original_offsets.size() != 1U) {
            result.status = GeometryMatchStatus::ambiguous_geometry;
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.spatial_match.ambiguous_duplicate_geometry",
                "Duplicate flags plus A/B/C geometry make automatic stable identity ambiguous; use an explicit authoring manifest.",
                original_offsets.front());
            return result;
        }
        candidate_by_original_offset.emplace(
            original_offsets.front(),
            candidate->second.front());
    }

    result.mappings.reserve(original_scan.triangles.size());
    auto stable_id = first_stable_id;
    for (std::size_t index = 0U;
         index < original_scan.triangles.size();
         ++index) {
        const auto original_offset = static_cast<std::uint32_t>(
            index * formats::hits::triangle_size);
        const auto candidate = candidate_by_original_offset.find(
            original_offset);
        if (candidate == candidate_by_original_offset.end()) {
            result.status = GeometryMatchStatus::unmatched_geometry;
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.spatial_match.missing_original_offset",
                "The unique geometry index failed to cover an original triangle offset.",
                original_offset);
            result.mappings.clear();
            return result;
        }
        result.mappings.push_back(
            spatial_compare::SurfaceOffsetMapping{
                .stable_id = stable_id,
                .original_triangle_byte_offset = original_offset,
                .candidate_triangle_byte_offset = candidate->second,
            });
        ++stable_id;
    }
    result.status = GeometryMatchStatus::matched;
    return result;
}

} // namespace dmc::rengine::hits::spatial_match
