#include "dmc_rengine/formats/hits.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

namespace dmc::rengine::formats::hits {
namespace {

void add_diagnostic(
    ScanResult& result,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset) {
    result.diagnostics.push_back(ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

[[nodiscard]] bool read_vec3(
    const binary::Reader& reader,
    std::size_t offset,
    Vec3& value) {
    const auto x = reader.f32_le(offset);
    const auto y = reader.f32_le(offset + 4U);
    const auto z = reader.f32_le(offset + 8U);
    if (!x || !y || !z) {
        return false;
    }
    value = Vec3{*x, *y, *z};
    return true;
}

[[nodiscard]] std::optional<std::int32_t> read_i32(
    const binary::Reader& reader,
    std::size_t offset) {
    const auto value = reader.u32_le(offset);
    if (!value) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*value);
}

[[nodiscard]] bool finite(const Vec3& value) noexcept {
    return std::isfinite(value.x) && std::isfinite(value.y) &&
        std::isfinite(value.z);
}

} // namespace

std::uint64_t Header::cell_count() const noexcept {
    return static_cast<std::uint64_t>(grid_count_x) * grid_count_y *
        grid_count_z;
}

std::uint64_t Header::spatial_offset() const noexcept {
    return 8U + spatial_relative_offset;
}

std::uint64_t Header::triangle_offset() const noexcept {
    return 8U + triangle_relative_offset;
}

bool ScanResult::ok() const noexcept {
    return recognized && std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

float evaluate_plane(const Triangle& triangle, const Vec3& point) noexcept {
    return triangle.normal.x * point.x + triangle.normal.y * point.y +
        triangle.normal.z * point.z + triangle.plane_d;
}

ScanResult RecordScanner::scan(std::span<const std::byte> bytes) {
    ScanResult result;
    const binary::Reader reader(bytes);

    if (!reader.matches(0U, "HITS")) {
        add_diagnostic(result, ParseSeverity::warning,
            "hits.unrecognized", "The resource does not begin with the four-byte HITS magic.", 0U);
        return result;
    }
    result.recognized = true;

    if (!reader.contains(0U, header_size)) {
        add_diagnostic(result, ParseSeverity::error,
            "hits.truncated_header", "The HITS header is shorter than 0x44 bytes.", 0U);
        return result;
    }

    const auto end_offset = reader.u32_le(0x04U);
    const auto grid_x = reader.u32_le(0x2CU);
    const auto grid_y = reader.u32_le(0x30U);
    const auto grid_z = reader.u32_le(0x34U);
    const auto triangle_count = reader.u32_le(0x38U);
    const auto spatial_relative = reader.u32_le(0x3CU);
    const auto triangle_relative = reader.u32_le(0x40U);
    if (!end_offset || !grid_x || !grid_y || !grid_z || !triangle_count ||
        !spatial_relative || !triangle_relative ||
        !read_vec3(reader, 0x08U, result.header.bounds_min) ||
        !read_vec3(reader, 0x14U, result.header.bounds_max) ||
        !read_vec3(reader, 0x20U, result.header.cell_size)) {
        add_diagnostic(result, ParseSeverity::error,
            "hits.invalid_header", "The HITS header could not be decoded completely.", 0U);
        return result;
    }

    result.header.end_offset = *end_offset;
    result.header.grid_count_x = *grid_x;
    result.header.grid_count_y = *grid_y;
    result.header.grid_count_z = *grid_z;
    result.header.triangle_count = *triangle_count;
    result.header.spatial_relative_offset = *spatial_relative;
    result.header.triangle_relative_offset = *triangle_relative;

    if (*grid_x == 0U || *grid_y == 0U || *grid_z == 0U ||
        !finite(result.header.bounds_min) || !finite(result.header.bounds_max) ||
        !finite(result.header.cell_size) || result.header.cell_size.x <= 0.0F ||
        result.header.cell_size.y <= 0.0F || result.header.cell_size.z <= 0.0F) {
        add_diagnostic(result, ParseSeverity::error,
            "hits.invalid_grid", "HITS grid dimensions and cell sizes must be finite and non-zero.", 0x20U);
        return result;
    }

    const auto cell_count = result.header.cell_count();
    const auto spatial_offset = result.header.spatial_offset();
    const auto triangle_offset = result.header.triangle_offset();
    const auto triangle_bytes = static_cast<std::uint64_t>(*triangle_count) * triangle_size;
    if (cell_count > std::numeric_limits<std::size_t>::max() ||
        spatial_offset > bytes.size() || triangle_offset > bytes.size() ||
        triangle_bytes > bytes.size() - triangle_offset ||
        *end_offset > bytes.size()) {
        add_diagnostic(result, ParseSeverity::error,
            "hits.out_of_range_header", "A HITS header offset, count, or terminal array exceeds the resource.", 0x38U);
        return result;
    }

    const auto expected_end = triangle_offset + triangle_bytes;
    if (*end_offset != expected_end) {
        add_diagnostic(result, ParseSeverity::warning,
            "hits.end_offset_mismatch", "The declared end offset does not equal triangleBase + triangleCount * 0x38.", 0x04U);
    }

    const auto pointer_table = spatial_offset + 8U;
    const auto pointer_bytes = cell_count * 4U;
    if (pointer_table > bytes.size() || pointer_bytes > bytes.size() - pointer_table) {
        add_diagnostic(result, ParseSeverity::error,
            "hits.truncated_cell_table", "The spatial cell pointer table exceeds the resource.", spatial_offset);
        return result;
    }

    result.cells.reserve(static_cast<std::size_t>(cell_count));
    for (std::uint64_t index = 0; index < cell_count; ++index) {
        const auto pointer_offset = pointer_table + index * 4U;
        const auto relative = read_i32(reader, static_cast<std::size_t>(pointer_offset));
        if (!relative || *relative < 0) {
            add_diagnostic(result, ParseSeverity::error,
                "hits.invalid_cell_pointer", "A spatial cell contains a negative or unreadable list pointer.", pointer_offset);
            return result;
        }
        const auto list_offset = 8U + static_cast<std::uint64_t>(*relative);
        if (list_offset >= bytes.size()) {
            add_diagnostic(result, ParseSeverity::error,
                "hits.cell_list_out_of_range", "A spatial cell list pointer exceeds the resource.", pointer_offset);
            return result;
        }

        Cell cell{
            .index = static_cast<std::uint32_t>(index),
            .pointer_offset = pointer_offset,
            .list_offset = list_offset,
            .triangle_byte_offsets = {},
        };
        auto cursor = list_offset;
        bool terminated = false;
        while (cursor + 4U <= bytes.size()) {
            const auto reference = read_i32(reader, static_cast<std::size_t>(cursor));
            if (!reference) {
                break;
            }
            cursor += 4U;
            if (*reference == cell_list_terminator) {
                terminated = true;
                break;
            }
            if (*reference < 0 || (*reference % static_cast<std::int32_t>(triangle_size)) != 0 ||
                static_cast<std::uint64_t>(*reference) >= triangle_bytes) {
                add_diagnostic(result, ParseSeverity::error,
                    "hits.invalid_triangle_reference", "A cell list entry is not a valid 0x38-byte triangle offset.", cursor - 4U);
                return result;
            }
            cell.triangle_byte_offsets.push_back(static_cast<std::uint32_t>(*reference));
        }
        if (!terminated) {
            add_diagnostic(result, ParseSeverity::error,
                "hits.unterminated_cell_list", "A spatial cell reference list has no -1 terminator.", list_offset);
            return result;
        }
        result.cells.push_back(std::move(cell));
    }

    result.triangles.reserve(*triangle_count);
    for (std::uint32_t index = 0; index < *triangle_count; ++index) {
        const auto offset = triangle_offset + static_cast<std::uint64_t>(index) * triangle_size;
        Triangle triangle{.offset = offset};
        const auto flags = reader.u32_le(static_cast<std::size_t>(offset));
        const auto plane_d = reader.f32_le(static_cast<std::size_t>(offset + 0x34U));
        if (!flags || !plane_d ||
            !read_vec3(reader, static_cast<std::size_t>(offset + 0x04U), triangle.point_a) ||
            !read_vec3(reader, static_cast<std::size_t>(offset + 0x10U), triangle.point_b) ||
            !read_vec3(reader, static_cast<std::size_t>(offset + 0x1CU), triangle.point_c) ||
            !read_vec3(reader, static_cast<std::size_t>(offset + 0x28U), triangle.normal)) {
            add_diagnostic(result, ParseSeverity::error,
                "hits.truncated_triangle", "A declared HITS triangle could not be decoded completely.", offset);
            return result;
        }
        triangle.flags = *flags;
        triangle.plane_d = *plane_d;
        if (!finite(triangle.point_a) || !finite(triangle.point_b) ||
            !finite(triangle.point_c) || !finite(triangle.normal) ||
            !std::isfinite(triangle.plane_d)) {
            add_diagnostic(result, ParseSeverity::error,
                "hits.nonfinite_triangle", "A HITS triangle contains a non-finite geometry value.", offset);
            return result;
        }
        const auto residual = std::max({
            std::fabs(evaluate_plane(triangle, triangle.point_a)),
            std::fabs(evaluate_plane(triangle, triangle.point_b)),
            std::fabs(evaluate_plane(triangle, triangle.point_c)),
        });
        if (residual > 0.01F) {
            add_diagnostic(result, ParseSeverity::warning,
                "hits.plane_residual", "Triangle vertices do not closely satisfy dot(normal, point) + planeD = 0.", offset + 0x28U);
        }
        result.triangles.push_back(triangle);
    }

    if (result.triangles.empty()) {
        add_diagnostic(result, ParseSeverity::warning,
            "hits.no_triangles", "The HITS header is valid but declares no triangles.", 0x38U);
    }
    return result;
}

} // namespace dmc::rengine::formats::hits
