#include "dmc_rengine/hits/spatial_compare.hpp"

#include <algorithm>
#include <bit>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::hits::spatial_compare {
namespace {

using formats::ParseDiagnostic;
using formats::ParseSeverity;
using formats::hits::Header;
using writer::StableSurfaceId;

void add_diagnostic(
    SpatialComparisonReport& report,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset = 0U) {
    report.diagnostics.push_back(ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

[[nodiscard]] bool same_float(float left, float right) noexcept {
    return std::bit_cast<std::uint32_t>(left) ==
           std::bit_cast<std::uint32_t>(right);
}

[[nodiscard]] bool same_vec3(
    const formats::hits::Vec3& left,
    const formats::hits::Vec3& right) noexcept {
    return same_float(left.x, right.x) &&
           same_float(left.y, right.y) &&
           same_float(left.z, right.z);
}

[[nodiscard]] bool compatible_grid(
    const Header& original,
    const Header& candidate) noexcept {
    return same_vec3(original.bounds_min, candidate.bounds_min) &&
           same_vec3(original.bounds_max, candidate.bounds_max) &&
           original.cell_size == candidate.cell_size &&
           original.grid_count_x == candidate.grid_count_x &&
           original.grid_count_y == candidate.grid_count_y &&
           original.grid_count_z == candidate.grid_count_z;
}

template <typename T>
[[nodiscard]] std::vector<T> difference(
    const std::set<T>& left,
    const std::set<T>& right) {
    std::vector<T> output;
    std::set_difference(
        left.begin(), left.end(),
        right.begin(), right.end(),
        std::back_inserter(output));
    return output;
}

template <typename T>
[[nodiscard]] std::uint64_t intersection_count(
    const std::set<T>& left,
    const std::set<T>& right) {
    std::uint64_t count{};
    auto left_iterator = left.begin();
    auto right_iterator = right.begin();
    while (left_iterator != left.end() &&
           right_iterator != right.end()) {
        if (*left_iterator < *right_iterator) {
            ++left_iterator;
        } else if (*right_iterator < *left_iterator) {
            ++right_iterator;
        } else {
            ++count;
            ++left_iterator;
            ++right_iterator;
        }
    }
    return count;
}

[[nodiscard]] double ratio(
    std::uint64_t numerator,
    std::uint64_t denominator,
    double empty_value) noexcept {
    if (denominator == 0U) {
        return empty_value;
    }
    return static_cast<double>(numerator) /
           static_cast<double>(denominator);
}

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hexadecimal[] = "0123456789abcdef";
    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00"
                       << hexadecimal[(character >> 4U) & 0x0FU]
                       << hexadecimal[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

void write_string(std::ostringstream& output, std::string_view value) {
    output << '"' << escape_json(value) << '"';
}

template <typename T>
void write_numeric_array(
    std::ostringstream& output,
    const std::vector<T>& values) {
    output << '[';
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) {
            output << ", ";
        }
        output << values[index];
    }
    output << ']';
}

[[nodiscard]] std::string_view severity_name(
    ParseSeverity severity) noexcept {
    switch (severity) {
    case ParseSeverity::info: return "info";
    case ParseSeverity::warning: return "warning";
    case ParseSeverity::error: return "error";
    }
    return "error";
}

struct MappingIndex final {
    std::map<std::uint32_t, StableSurfaceId> original_by_offset;
    std::map<std::uint32_t, StableSurfaceId> candidate_by_offset;
    std::vector<StableSurfaceId> stable_ids;
};

[[nodiscard]] bool validate_mapping(
    SpatialComparisonReport& report,
    const formats::hits::ScanResult& original_scan,
    const formats::hits::ScanResult& candidate_scan,
    std::span<const SurfaceOffsetMapping> mappings,
    MappingIndex& index) {
    if (mappings.size() != original_scan.triangles.size() ||
        mappings.size() != candidate_scan.triangles.size()) {
        add_diagnostic(
            report,
            ParseSeverity::error,
            "hits.spatial_compare.mapping_count_mismatch",
            "The stable-surface mapping must cover every original and candidate triangle exactly once.");
        return false;
    }

    std::set<StableSurfaceId> stable_ids;
    std::set<std::uint32_t> original_offsets;
    std::set<std::uint32_t> candidate_offsets;
    const auto original_byte_size =
        static_cast<std::uint64_t>(original_scan.triangles.size()) *
        formats::hits::triangle_size;
    const auto candidate_byte_size =
        static_cast<std::uint64_t>(candidate_scan.triangles.size()) *
        formats::hits::triangle_size;

    for (std::size_t mapping_index = 0U;
         mapping_index < mappings.size();
         ++mapping_index) {
        const auto& mapping = mappings[mapping_index];
        if (mapping.stable_id == 0U ||
            !stable_ids.insert(mapping.stable_id).second ||
            mapping.original_triangle_byte_offset %
                    formats::hits::triangle_size != 0U ||
            mapping.candidate_triangle_byte_offset %
                    formats::hits::triangle_size != 0U ||
            mapping.original_triangle_byte_offset >= original_byte_size ||
            mapping.candidate_triangle_byte_offset >= candidate_byte_size ||
            !original_offsets.insert(
                mapping.original_triangle_byte_offset).second ||
            !candidate_offsets.insert(
                mapping.candidate_triangle_byte_offset).second) {
            add_diagnostic(
                report,
                ParseSeverity::error,
                "hits.spatial_compare.invalid_mapping_entry",
                "A stable-surface mapping entry has a duplicate, zero, unaligned, or out-of-range identity/offset.",
                mapping_index);
            return false;
        }
        index.original_by_offset.emplace(
            mapping.original_triangle_byte_offset,
            mapping.stable_id);
        index.candidate_by_offset.emplace(
            mapping.candidate_triangle_byte_offset,
            mapping.stable_id);
    }

    for (std::size_t triangle_index = 0U;
         triangle_index < mappings.size();
         ++triangle_index) {
        const auto expected = static_cast<std::uint32_t>(
            triangle_index * formats::hits::triangle_size);
        if (original_offsets.find(expected) == original_offsets.end() ||
            candidate_offsets.find(expected) == candidate_offsets.end()) {
            add_diagnostic(
                report,
                ParseSeverity::error,
                "hits.spatial_compare.incomplete_offset_domain",
                "The stable-surface mapping does not cover the complete contiguous triangle-offset domain.",
                expected);
            return false;
        }
    }

    index.stable_ids.assign(stable_ids.begin(), stable_ids.end());
    return true;
}

[[nodiscard]] bool cell_surface_set(
    SpatialComparisonReport& report,
    const formats::hits::Cell& cell,
    const std::map<std::uint32_t, StableSurfaceId>& mapping,
    std::set<StableSurfaceId>& output,
    std::string_view side) {
    std::set<std::uint32_t> seen_offsets;
    for (const auto offset : cell.triangle_byte_offsets) {
        if (!seen_offsets.insert(offset).second) {
            add_diagnostic(
                report,
                ParseSeverity::error,
                "hits.spatial_compare.duplicate_cell_reference",
                std::string(side) +
                    " cell list contains a duplicate triangle byte offset.",
                cell.list_offset);
            return false;
        }
        const auto iterator = mapping.find(offset);
        if (iterator == mapping.end()) {
            add_diagnostic(
                report,
                ParseSeverity::error,
                "hits.spatial_compare.unmapped_cell_reference",
                std::string(side) +
                    " cell list references a triangle offset absent from the stable-surface mapping.",
                offset);
            return false;
        }
        output.insert(iterator->second);
    }
    return true;
}

} // namespace

std::vector<SurfaceOffsetMapping> make_surface_mapping(
    const formats::hits::ScanResult& original_scan,
    std::span<const writer::Surface> original_surfaces,
    std::span<const writer::SurfaceLocation> candidate_locations) {
    if (!original_scan.ok() ||
        original_surfaces.size() != original_scan.triangles.size() ||
        candidate_locations.size() != original_surfaces.size()) {
        return {};
    }

    std::map<StableSurfaceId, std::uint32_t> candidate_offsets;
    for (const auto& location : candidate_locations) {
        if (location.stable_id == 0U ||
            !candidate_offsets.emplace(
                location.stable_id,
                location.triangle_byte_offset).second) {
            return {};
        }
    }

    std::set<StableSurfaceId> original_ids;
    std::vector<SurfaceOffsetMapping> mappings;
    mappings.reserve(original_surfaces.size());
    for (std::size_t index = 0U;
         index < original_surfaces.size();
         ++index) {
        const auto stable_id = original_surfaces[index].stable_id;
        const auto candidate = candidate_offsets.find(stable_id);
        if (stable_id == 0U ||
            !original_ids.insert(stable_id).second ||
            candidate == candidate_offsets.end()) {
            return {};
        }
        const auto original_offset_u64 =
            static_cast<std::uint64_t>(index) *
            formats::hits::triangle_size;
        if (original_offset_u64 >
            std::numeric_limits<std::uint32_t>::max()) {
            return {};
        }
        mappings.push_back(SurfaceOffsetMapping{
            .stable_id = stable_id,
            .original_triangle_byte_offset =
                static_cast<std::uint32_t>(original_offset_u64),
            .candidate_triangle_byte_offset = candidate->second,
        });
    }
    return mappings;
}

SpatialComparisonReport compare(
    const formats::hits::ScanResult& original_scan,
    const formats::hits::ScanResult& candidate_scan,
    std::span<const SurfaceOffsetMapping> mappings,
    std::string report_id,
    std::string original_identity,
    std::string candidate_identity,
    std::string assignment_policy) {
    SpatialComparisonReport report{
        .report_id = std::move(report_id),
        .original_identity = std::move(original_identity),
        .candidate_identity = std::move(candidate_identity),
        .assignment_policy = std::move(assignment_policy),
    };

    if (!original_scan.ok()) {
        report.status = ComparisonStatus::invalid_original_scan;
        add_diagnostic(
            report,
            ParseSeverity::error,
            "hits.spatial_compare.invalid_original_scan",
            "The original HITS scan is not structurally valid.");
        return report;
    }
    if (!candidate_scan.ok()) {
        report.status = ComparisonStatus::invalid_candidate_scan;
        add_diagnostic(
            report,
            ParseSeverity::error,
            "hits.spatial_compare.invalid_candidate_scan",
            "The candidate HITS scan is not structurally valid.");
        return report;
    }
    if (!compatible_grid(original_scan.header, candidate_scan.header) ||
        original_scan.cells.size() != candidate_scan.cells.size()) {
        report.status = ComparisonStatus::incompatible_grid;
        add_diagnostic(
            report,
            ParseSeverity::error,
            "hits.spatial_compare.incompatible_grid",
            "Spatial-list comparison requires bit-identical bounds/cell extents and identical grid dimensions.");
        return report;
    }

    MappingIndex mapping_index;
    if (!validate_mapping(
            report,
            original_scan,
            candidate_scan,
            mappings,
            mapping_index)) {
        report.status = ComparisonStatus::invalid_surface_mapping;
        return report;
    }

    std::map<StableSurfaceId, std::set<std::uint32_t>> original_cells;
    std::map<StableSurfaceId, std::set<std::uint32_t>> candidate_cells;
    for (const auto stable_id : mapping_index.stable_ids) {
        original_cells.emplace(stable_id, std::set<std::uint32_t>{});
        candidate_cells.emplace(stable_id, std::set<std::uint32_t>{});
    }

    report.cells.reserve(original_scan.cells.size());
    for (std::size_t cell_index = 0U;
         cell_index < original_scan.cells.size();
         ++cell_index) {
        std::set<StableSurfaceId> original_surfaces;
        std::set<StableSurfaceId> candidate_surfaces;
        if (!cell_surface_set(
                report,
                original_scan.cells[cell_index],
                mapping_index.original_by_offset,
                original_surfaces,
                "Original") ||
            !cell_surface_set(
                report,
                candidate_scan.cells[cell_index],
                mapping_index.candidate_by_offset,
                candidate_surfaces,
                "Candidate")) {
            report.status = ComparisonStatus::invalid_reference_stream;
            return report;
        }

        for (const auto stable_id : original_surfaces) {
            original_cells[stable_id].insert(
                static_cast<std::uint32_t>(cell_index));
        }
        for (const auto stable_id : candidate_surfaces) {
            candidate_cells[stable_id].insert(
                static_cast<std::uint32_t>(cell_index));
        }

        const auto missing = difference(
            original_surfaces,
            candidate_surfaces);
        const auto extra = difference(
            candidate_surfaces,
            original_surfaces);
        const auto shared = intersection_count(
            original_surfaces,
            candidate_surfaces);
        report.metrics.original_reference_count +=
            original_surfaces.size();
        report.metrics.candidate_reference_count +=
            candidate_surfaces.size();
        report.metrics.shared_reference_count += shared;
        report.metrics.missing_reference_count += missing.size();
        report.metrics.extra_reference_count += extra.size();
        if (missing.empty() && extra.empty()) {
            ++report.metrics.exact_cell_count;
        }
        report.cells.push_back(CellDifference{
            .cell_index = static_cast<std::uint32_t>(cell_index),
            .original_surfaces = std::vector<StableSurfaceId>(
                original_surfaces.begin(), original_surfaces.end()),
            .candidate_surfaces = std::vector<StableSurfaceId>(
                candidate_surfaces.begin(), candidate_surfaces.end()),
            .missing_surfaces = missing,
            .extra_surfaces = extra,
        });
    }

    report.surfaces.reserve(mapping_index.stable_ids.size());
    for (const auto stable_id : mapping_index.stable_ids) {
        const auto& original = original_cells.at(stable_id);
        const auto& candidate = candidate_cells.at(stable_id);
        auto missing = difference(original, candidate);
        auto extra = difference(candidate, original);
        if (missing.empty() && extra.empty()) {
            ++report.metrics.exact_surface_count;
        }
        report.surfaces.push_back(SurfaceDifference{
            .stable_id = stable_id,
            .original_cells = std::vector<std::uint32_t>(
                original.begin(), original.end()),
            .candidate_cells = std::vector<std::uint32_t>(
                candidate.begin(), candidate.end()),
            .missing_cells = std::move(missing),
            .extra_cells = std::move(extra),
        });
    }

    report.metrics.precision = ratio(
        report.metrics.shared_reference_count,
        report.metrics.candidate_reference_count,
        report.metrics.original_reference_count == 0U ? 1.0 : 0.0);
    report.metrics.recall = ratio(
        report.metrics.shared_reference_count,
        report.metrics.original_reference_count,
        1.0);
    const auto union_count =
        report.metrics.shared_reference_count +
        report.metrics.missing_reference_count +
        report.metrics.extra_reference_count;
    report.metrics.jaccard = ratio(
        report.metrics.shared_reference_count,
        union_count,
        1.0);
    report.status = ComparisonStatus::comparable;
    return report;
}

std::string report_json(const SpatialComparisonReport& report) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::setprecision(17);
    output << "{\n  \"schema_version\": 1,\n"
           << "  \"report_id\": ";
    write_string(output, report.report_id);
    output << ",\n  \"original_identity\": ";
    write_string(output, report.original_identity);
    output << ",\n  \"candidate_identity\": ";
    write_string(output, report.candidate_identity);
    output << ",\n  \"assignment_policy\": ";
    write_string(output, report.assignment_policy);
    output << ",\n  \"status\": ";
    write_string(output, to_string(report.status));
    output << ",\n  \"comparable\": "
           << (report.comparable() ? "true" : "false")
           << ",\n  \"exact\": "
           << (report.exact() ? "true" : "false")
           << ",\n  \"metrics\": {\n"
           << "    \"original_reference_count\": "
           << report.metrics.original_reference_count
           << ",\n    \"candidate_reference_count\": "
           << report.metrics.candidate_reference_count
           << ",\n    \"shared_reference_count\": "
           << report.metrics.shared_reference_count
           << ",\n    \"missing_reference_count\": "
           << report.metrics.missing_reference_count
           << ",\n    \"extra_reference_count\": "
           << report.metrics.extra_reference_count
           << ",\n    \"exact_cell_count\": "
           << report.metrics.exact_cell_count
           << ",\n    \"exact_surface_count\": "
           << report.metrics.exact_surface_count
           << ",\n    \"precision\": "
           << report.metrics.precision
           << ",\n    \"recall\": "
           << report.metrics.recall
           << ",\n    \"jaccard\": "
           << report.metrics.jaccard
           << "\n  },\n  \"cells\": [";
    if (!report.cells.empty()) {
        output << '\n';
        for (std::size_t index = 0U;
             index < report.cells.size();
             ++index) {
            const auto& cell = report.cells[index];
            output << "    {\n      \"cell_index\": "
                   << cell.cell_index
                   << ",\n      \"exact\": "
                   << (cell.exact() ? "true" : "false")
                   << ",\n      \"original_surfaces\": ";
            write_numeric_array(output, cell.original_surfaces);
            output << ",\n      \"candidate_surfaces\": ";
            write_numeric_array(output, cell.candidate_surfaces);
            output << ",\n      \"missing_surfaces\": ";
            write_numeric_array(output, cell.missing_surfaces);
            output << ",\n      \"extra_surfaces\": ";
            write_numeric_array(output, cell.extra_surfaces);
            output << "\n    }";
            if (index + 1U != report.cells.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"surfaces\": [";
    if (!report.surfaces.empty()) {
        output << '\n';
        for (std::size_t index = 0U;
             index < report.surfaces.size();
             ++index) {
            const auto& surface = report.surfaces[index];
            output << "    {\n      \"stable_id\": "
                   << surface.stable_id
                   << ",\n      \"exact\": "
                   << (surface.exact() ? "true" : "false")
                   << ",\n      \"original_cells\": ";
            write_numeric_array(output, surface.original_cells);
            output << ",\n      \"candidate_cells\": ";
            write_numeric_array(output, surface.candidate_cells);
            output << ",\n      \"missing_cells\": ";
            write_numeric_array(output, surface.missing_cells);
            output << ",\n      \"extra_cells\": ";
            write_numeric_array(output, surface.extra_cells);
            output << "\n    }";
            if (index + 1U != report.surfaces.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"diagnostics\": [";
    if (!report.diagnostics.empty()) {
        output << '\n';
        for (std::size_t index = 0U;
             index < report.diagnostics.size();
             ++index) {
            const auto& diagnostic = report.diagnostics[index];
            output << "    {\n      \"severity\": ";
            write_string(output, severity_name(diagnostic.severity));
            output << ",\n      \"code\": ";
            write_string(output, diagnostic.code);
            output << ",\n      \"message\": ";
            write_string(output, diagnostic.message);
            output << ",\n      \"offset\": "
                   << diagnostic.offset
                   << "\n    }";
            if (index + 1U != report.diagnostics.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "]\n}\n";
    return output.str();
}

} // namespace dmc::rengine::hits::spatial_compare
