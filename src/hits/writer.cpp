#include "dmc_rengine/hits/writer.hpp"

#include "dmc_rengine/hits/edit.hpp"
#include "dmc_rengine/hits/runtime.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <limits>
#include <set>
#include <string>
#include <utility>

namespace dmc::rengine::hits::writer {
namespace {

using formats::ParseDiagnostic;
using formats::ParseSeverity;
using formats::hits::Header;
using formats::hits::Triangle;
using formats::hits::Vec3;

constexpr std::size_t canonical_spatial_offset = 0x44U;
constexpr std::size_t pointer_table_offset = canonical_spatial_offset;

struct DVec3 final {
    double x{};
    double y{};
    double z{};
};

struct PreparedSurface final {
    StableSurfaceId stable_id{};
    Triangle triangle;
};

struct AxisRange final {
    std::uint32_t minimum{};
    std::uint32_t maximum{};
};

void add_diagnostic(
    RebuildResult& result,
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

[[nodiscard]] bool finite(const Vec3& value) noexcept {
    return std::isfinite(value.x) && std::isfinite(value.y) &&
           std::isfinite(value.z);
}

[[nodiscard]] DVec3 to_double(const Vec3& value) noexcept {
    return DVec3{
        .x = static_cast<double>(value.x),
        .y = static_cast<double>(value.y),
        .z = static_cast<double>(value.z),
    };
}

[[nodiscard]] DVec3 subtract(const DVec3& left, const DVec3& right) noexcept {
    return DVec3{
        .x = left.x - right.x,
        .y = left.y - right.y,
        .z = left.z - right.z,
    };
}

[[nodiscard]] DVec3 cross(const DVec3& left, const DVec3& right) noexcept {
    return DVec3{
        .x = left.y * right.z - left.z * right.y,
        .y = left.z * right.x - left.x * right.z,
        .z = left.x * right.y - left.y * right.x,
    };
}

[[nodiscard]] double dot(const DVec3& left, const DVec3& right) noexcept {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

[[nodiscard]] double length_squared(const DVec3& value) noexcept {
    return dot(value, value);
}

[[nodiscard]] bool overlap_on_axis(
    const std::array<DVec3, 3U>& vertices,
    const DVec3& half_extents,
    const DVec3& axis,
    double epsilon) noexcept {
    if (length_squared(axis) <= epsilon * epsilon) {
        return true;
    }
    const auto p0 = dot(vertices[0], axis);
    const auto p1 = dot(vertices[1], axis);
    const auto p2 = dot(vertices[2], axis);
    const auto minimum = std::min({p0, p1, p2});
    const auto maximum = std::max({p0, p1, p2});
    const auto radius =
        half_extents.x * std::fabs(axis.x) +
        half_extents.y * std::fabs(axis.y) +
        half_extents.z * std::fabs(axis.z);
    return !(minimum > radius + epsilon || maximum < -radius - epsilon);
}

[[nodiscard]] bool triangle_intersects_box(
    const Triangle& triangle,
    const Vec3& box_min,
    const Vec3& box_max,
    double epsilon) noexcept {
    const DVec3 minimum = to_double(box_min);
    const DVec3 maximum = to_double(box_max);
    const DVec3 center{
        .x = (minimum.x + maximum.x) * 0.5,
        .y = (minimum.y + maximum.y) * 0.5,
        .z = (minimum.z + maximum.z) * 0.5,
    };
    const DVec3 half_extents{
        .x = (maximum.x - minimum.x) * 0.5,
        .y = (maximum.y - minimum.y) * 0.5,
        .z = (maximum.z - minimum.z) * 0.5,
    };
    const std::array<DVec3, 3U> vertices{
        subtract(to_double(triangle.point_a), center),
        subtract(to_double(triangle.point_b), center),
        subtract(to_double(triangle.point_c), center),
    };
    const std::array<DVec3, 3U> edges{
        subtract(vertices[1], vertices[0]),
        subtract(vertices[2], vertices[1]),
        subtract(vertices[0], vertices[2]),
    };
    constexpr std::array<DVec3, 3U> box_axes{
        DVec3{1.0, 0.0, 0.0},
        DVec3{0.0, 1.0, 0.0},
        DVec3{0.0, 0.0, 1.0},
    };

    for (const auto& axis : box_axes) {
        if (!overlap_on_axis(vertices, half_extents, axis, epsilon)) {
            return false;
        }
    }
    const auto triangle_normal = cross(edges[0], edges[1]);
    if (!overlap_on_axis(vertices, half_extents, triangle_normal, epsilon)) {
        return false;
    }
    for (const auto& edge : edges) {
        for (const auto& box_axis : box_axes) {
            if (!overlap_on_axis(
                    vertices,
                    half_extents,
                    cross(edge, box_axis),
                    epsilon)) {
                return false;
            }
        }
    }
    return true;
}

[[nodiscard]] bool point_within_bounds(
    const Header& header,
    const Vec3& point,
    double epsilon) noexcept {
    return static_cast<double>(point.x) >=
               static_cast<double>(header.bounds_min.x) - epsilon &&
           static_cast<double>(point.y) >=
               static_cast<double>(header.bounds_min.y) - epsilon &&
           static_cast<double>(point.z) >=
               static_cast<double>(header.bounds_min.z) - epsilon &&
           static_cast<double>(point.x) <=
               static_cast<double>(header.bounds_max.x) + epsilon &&
           static_cast<double>(point.y) <=
               static_cast<double>(header.bounds_max.y) + epsilon &&
           static_cast<double>(point.z) <=
               static_cast<double>(header.bounds_max.z) + epsilon;
}

[[nodiscard]] Vec3 minimum_point(const Triangle& triangle) noexcept {
    return Vec3{
        .x = std::min({triangle.point_a.x, triangle.point_b.x, triangle.point_c.x}),
        .y = std::min({triangle.point_a.y, triangle.point_b.y, triangle.point_c.y}),
        .z = std::min({triangle.point_a.z, triangle.point_b.z, triangle.point_c.z}),
    };
}

[[nodiscard]] Vec3 maximum_point(const Triangle& triangle) noexcept {
    return Vec3{
        .x = std::max({triangle.point_a.x, triangle.point_b.x, triangle.point_c.x}),
        .y = std::max({triangle.point_a.y, triangle.point_b.y, triangle.point_c.y}),
        .z = std::max({triangle.point_a.z, triangle.point_b.z, triangle.point_c.z}),
    };
}

// Closed cell boxes share their boundary planes. A triangle whose minimum lies
// exactly on a cell boundary therefore has to test the preceding cell too.
// The 16-file / 4,420-triangle stock corpus reproduces its serialized cell
// memberships exactly with this candidate range followed by SAT filtering.
[[nodiscard]] std::optional<AxisRange> closed_candidate_axis_range(
    float triangle_minimum,
    float triangle_maximum,
    float grid_minimum,
    std::uint32_t cell_extent,
    std::uint32_t cell_count) noexcept {
    if (!std::isfinite(triangle_minimum) || !std::isfinite(triangle_maximum) ||
        !std::isfinite(grid_minimum) || triangle_maximum < triangle_minimum ||
        cell_extent == 0U || cell_count == 0U) {
        return std::nullopt;
    }

    const auto extent = static_cast<double>(cell_extent);
    const auto normalized_minimum =
        (static_cast<double>(triangle_minimum) -
         static_cast<double>(grid_minimum)) / extent;
    const auto normalized_maximum =
        (static_cast<double>(triangle_maximum) -
         static_cast<double>(grid_minimum)) / extent;
    const auto last = static_cast<double>(cell_count - 1U);
    const auto lower = std::clamp(
        std::ceil(normalized_minimum) - 1.0, 0.0, last);
    const auto upper = std::clamp(
        std::floor(normalized_maximum), 0.0, last);
    if (lower > upper) {
        return std::nullopt;
    }
    return AxisRange{
        .minimum = static_cast<std::uint32_t>(lower),
        .maximum = static_cast<std::uint32_t>(upper),
    };
}

[[nodiscard]] std::optional<runtime::GridRange> closed_triangle_candidate_range(
    const Header& header,
    const Triangle& triangle) noexcept {
    if (!runtime::valid_runtime_grid(header)) {
        return std::nullopt;
    }
    const auto minimum = minimum_point(triangle);
    const auto maximum = maximum_point(triangle);
    const auto x = closed_candidate_axis_range(
        minimum.x, maximum.x, header.bounds_min.x,
        header.cell_size.x, header.grid_count_x);
    const auto y = closed_candidate_axis_range(
        minimum.y, maximum.y, header.bounds_min.y,
        header.cell_size.y, header.grid_count_y);
    const auto z = closed_candidate_axis_range(
        minimum.z, maximum.z, header.bounds_min.z,
        header.cell_size.z, header.grid_count_z);
    if (!x || !y || !z) {
        return std::nullopt;
    }
    return runtime::GridRange{
        .minimum = runtime::GridCoordinate{
            .x = x->minimum,
            .y = y->minimum,
            .z = z->minimum,
        },
        .maximum = runtime::GridCoordinate{
            .x = x->maximum,
            .y = y->maximum,
            .z = z->maximum,
        },
    };
}

[[nodiscard]] bool compute_fit_grid(
    Header& header,
    std::span<const PreparedSurface> surfaces,
    float padding) noexcept {
    if (surfaces.empty() || !std::isfinite(padding) || padding < 0.0F ||
        header.cell_size.x == 0U || header.cell_size.y == 0U ||
        header.cell_size.z == 0U) {
        return false;
    }
    Vec3 minimum = minimum_point(surfaces.front().triangle);
    Vec3 maximum = maximum_point(surfaces.front().triangle);
    for (const auto& surface : surfaces.subspan(1U)) {
        const auto local_minimum = minimum_point(surface.triangle);
        const auto local_maximum = maximum_point(surface.triangle);
        minimum.x = std::min(minimum.x, local_minimum.x);
        minimum.y = std::min(minimum.y, local_minimum.y);
        minimum.z = std::min(minimum.z, local_minimum.z);
        maximum.x = std::max(maximum.x, local_maximum.x);
        maximum.y = std::max(maximum.y, local_maximum.y);
        maximum.z = std::max(maximum.z, local_maximum.z);
    }

    // Stock corpus rule: bounds_min is floor(global vertex minimum). Padding is
    // applied before the floor so authoring mode remains conservative.
    minimum.x = std::floor(minimum.x - padding);
    minimum.y = std::floor(minimum.y - padding);
    minimum.z = std::floor(minimum.z - padding);
    maximum.x += padding;
    maximum.y += padding;
    maximum.z += padding;
    if (!finite(minimum) || !finite(maximum)) {
        return false;
    }

    const auto count_for_axis = [](float minimum_value,
                                   float maximum_value,
                                   std::uint32_t cell_extent)
        -> std::optional<std::uint32_t> {
        if (!std::isfinite(minimum_value) || !std::isfinite(maximum_value) ||
            cell_extent == 0U || maximum_value < minimum_value) {
            return std::nullopt;
        }
        const auto extent = static_cast<double>(maximum_value) -
                            static_cast<double>(minimum_value);
        const auto raw = std::max(
            1.0,
            std::ceil(extent / static_cast<double>(cell_extent)));
        if (raw > static_cast<double>(std::numeric_limits<std::uint32_t>::max())) {
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(raw);
    };

    const auto count_x = count_for_axis(minimum.x, maximum.x, header.cell_size.x);
    const auto count_y = count_for_axis(minimum.y, maximum.y, header.cell_size.y);
    const auto count_z = count_for_axis(minimum.z, maximum.z, header.cell_size.z);
    if (!count_x || !count_y || !count_z) {
        return false;
    }

    header.bounds_min = minimum;
    header.grid_count_x = *count_x;
    header.grid_count_y = *count_y;
    header.grid_count_z = *count_z;
    header.bounds_max = Vec3{
        .x = minimum.x + static_cast<float>(header.cell_size.x) *
            static_cast<float>(*count_x),
        .y = minimum.y + static_cast<float>(header.cell_size.y) *
            static_cast<float>(*count_y),
        .z = minimum.z + static_cast<float>(header.cell_size.z) *
            static_cast<float>(*count_z),
    };
    return finite(header.bounds_max);
}

void write_u32(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint32_t value) noexcept {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_i32(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::int32_t value) noexcept {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
}

void write_f32(
    std::span<std::byte> bytes,
    std::size_t offset,
    float value) noexcept {
    write_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void write_vec3(
    std::span<std::byte> bytes,
    std::size_t offset,
    const Vec3& value) noexcept {
    write_f32(bytes, offset, value.x);
    write_f32(bytes, offset + 4U, value.y);
    write_f32(bytes, offset + 8U, value.z);
}

[[nodiscard]] std::optional<std::size_t> checked_add(
    std::size_t left,
    std::size_t right) noexcept {
    if (right > std::numeric_limits<std::size_t>::max() - left) {
        return std::nullopt;
    }
    return left + right;
}

[[nodiscard]] std::optional<std::size_t> checked_multiply(
    std::size_t left,
    std::size_t right) noexcept {
    if (left != 0U && right > std::numeric_limits<std::size_t>::max() / left) {
        return std::nullopt;
    }
    return left * right;
}

[[nodiscard]] std::optional<std::size_t> align16(
    std::size_t value) noexcept {
    const auto with_padding = checked_add(value, 15U);
    if (!with_padding) {
        return std::nullopt;
    }
    return *with_padding & ~std::size_t{15U};
}

} // namespace

bool RebuildResult::ok() const noexcept {
    return !bytes.empty() && std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

std::vector<Surface> SpatialWriter::surfaces_from_scan(
    const formats::hits::ScanResult& scan,
    StableSurfaceId first_stable_id) {
    std::vector<Surface> surfaces;
    surfaces.reserve(scan.triangles.size());
    auto stable_id = first_stable_id;
    for (const auto& triangle : scan.triangles) {
        surfaces.push_back(Surface{
            .stable_id = stable_id,
            .flags = triangle.flags,
            .point_a = triangle.point_a,
            .point_b = triangle.point_b,
            .point_c = triangle.point_c,
        });
        if (stable_id != std::numeric_limits<StableSurfaceId>::max()) {
            ++stable_id;
        }
    }
    return surfaces;
}

RebuildResult SpatialWriter::rebuild(
    const formats::hits::ScanResult& source_scan,
    std::span<const std::byte> source_bytes,
    std::span<const Surface> surfaces,
    RebuildOptions options) {
    RebuildResult result;
    static_cast<void>(source_bytes);
    if (!source_scan.ok()) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.invalid_source_scan",
            "The source HITS scan must be recognized and free of structural errors.");
        return result;
    }
    if (!std::isfinite(options.bounds_padding) ||
        options.bounds_padding < 0.0F ||
        !std::isfinite(options.overlap_epsilon) ||
        options.overlap_epsilon < 0.0) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.invalid_options",
            "Bounds padding and overlap epsilon must be finite and non-negative.");
        return result;
    }
    if (options.assignment_policy != AssignmentPolicy::capcom_triangle_box_sat) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.unsupported_assignment_policy",
            "The requested spatial assignment policy is not implemented.");
        return result;
    }
    if (surfaces.size() >
        static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.too_many_surfaces",
            "The HITS triangle count exceeds the 32-bit header field.");
        return result;
    }

    std::set<StableSurfaceId> stable_ids;
    std::vector<PreparedSurface> prepared;
    prepared.reserve(surfaces.size());
    for (std::size_t index = 0U; index < surfaces.size(); ++index) {
        const auto& surface = surfaces[index];
        if (surface.stable_id == 0U || !stable_ids.insert(surface.stable_id).second) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.writer.invalid_stable_surface_id",
                "Every rebuilt surface requires a unique non-zero stable ID.",
                index);
            return result;
        }
        const auto geometry = edit::recompute_geometry(
            surface.point_a,
            surface.point_b,
            surface.point_c);
        if (!geometry) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.writer.invalid_surface_geometry",
                "A surface contains non-finite or degenerate triangle geometry.",
                index);
            return result;
        }
        prepared.push_back(PreparedSurface{
            .stable_id = surface.stable_id,
            .triangle = Triangle{
                .offset = 0U,
                .flags = surface.flags,
                .point_a = geometry->point_a,
                .point_b = geometry->point_b,
                .point_c = geometry->point_c,
                .normal = geometry->normal,
                .plane_d = geometry->plane_d,
            },
        });
    }

    result.header = source_scan.header;
    result.header.triangle_count = static_cast<std::uint32_t>(prepared.size());
    result.header.spatial_table_relative_offset = 0x3CU;
    if (options.grid_policy == GridPolicy::fit_bounds_preserve_cell_size) {
        if (!compute_fit_grid(result.header, prepared, options.bounds_padding)) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.writer.fit_grid_failed",
                "The rebuilt bounds and grid dimensions could not be derived.");
            return result;
        }
    } else if (options.grid_policy == GridPolicy::preserve_source_grid) {
        for (std::size_t index = 0U; index < prepared.size(); ++index) {
            const auto& triangle = prepared[index].triangle;
            if (!point_within_bounds(result.header, triangle.point_a, options.overlap_epsilon) ||
                !point_within_bounds(result.header, triangle.point_b, options.overlap_epsilon) ||
                !point_within_bounds(result.header, triangle.point_c, options.overlap_epsilon)) {
                add_diagnostic(
                    result,
                    ParseSeverity::error,
                    "hits.writer.surface_outside_preserved_grid",
                    "A surface leaves the preserved source bounds; use fit-bounds rebuild mode.",
                    index);
                return result;
            }
        }
    } else {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.unsupported_grid_policy",
            "The requested grid policy is not implemented.");
        return result;
    }

    const auto cell_count_u64 = result.header.cell_count();
    if (cell_count_u64 == 0U ||
        cell_count_u64 > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max())) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.invalid_cell_count",
            "The rebuilt grid has an invalid or unrepresentable cell count.");
        return result;
    }
    const auto cell_count = static_cast<std::size_t>(cell_count_u64);
    std::vector<std::vector<std::uint32_t>> references(cell_count);

    for (std::size_t triangle_index = 0U;
         triangle_index < prepared.size();
         ++triangle_index) {
        const auto relative_offset_u64 =
            static_cast<std::uint64_t>(triangle_index) * formats::hits::triangle_size;
        if (relative_offset_u64 > static_cast<std::uint64_t>(
                std::numeric_limits<std::int32_t>::max())) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.writer.triangle_reference_overflow",
                "A triangle byte offset cannot be represented by the signed 32-bit cell-list ABI.",
                triangle_index);
            return result;
        }
        const auto relative_offset = static_cast<std::uint32_t>(relative_offset_u64);
        const auto& triangle = prepared[triangle_index].triangle;
        const auto range = closed_triangle_candidate_range(result.header, triangle);
        if (!range) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.writer.grid_range_failed",
                "A surface could not be mapped to the rebuilt grid.",
                triangle_index);
            return result;
        }

        std::size_t assigned_cells = 0U;
        for (auto x = range->minimum.x; x <= range->maximum.x; ++x) {
            for (auto y = range->minimum.y; y <= range->maximum.y; ++y) {
                for (auto z = range->minimum.z; z <= range->maximum.z; ++z) {
                    const auto flat = runtime::flatten(
                        result.header,
                        runtime::GridCoordinate{x, y, z});
                    if (!flat || *flat >= references.size()) {
                        add_diagnostic(
                            result,
                            ParseSeverity::error,
                            "hits.writer.flatten_failed",
                            "A rebuilt grid coordinate could not be flattened safely.",
                            triangle_index);
                        return result;
                    }
                    const Vec3 cell_min{
                        .x = result.header.bounds_min.x +
                            static_cast<float>(result.header.cell_size.x) *
                                static_cast<float>(x),
                        .y = result.header.bounds_min.y +
                            static_cast<float>(result.header.cell_size.y) *
                                static_cast<float>(y),
                        .z = result.header.bounds_min.z +
                            static_cast<float>(result.header.cell_size.z) *
                                static_cast<float>(z),
                    };
                    const Vec3 cell_max{
                        .x = cell_min.x + static_cast<float>(result.header.cell_size.x),
                        .y = cell_min.y + static_cast<float>(result.header.cell_size.y),
                        .z = cell_min.z + static_cast<float>(result.header.cell_size.z),
                    };
                    if (triangle_intersects_box(
                            triangle,
                            cell_min,
                            cell_max,
                            options.overlap_epsilon)) {
                        references[*flat].push_back(relative_offset);
                        ++assigned_cells;
                    }
                }
            }
        }
        if (assigned_cells == 0U) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.writer.surface_unassigned",
                "A valid surface did not intersect any rebuilt grid cell.",
                triangle_index);
            return result;
        }
    }

    const auto pointer_bytes = checked_multiply(cell_count, 4U);
    if (!pointer_bytes) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.pointer_table_overflow",
            "The cell pointer table size overflowed.");
        return result;
    }
    const auto lists_start = checked_add(pointer_table_offset, *pointer_bytes);
    if (!lists_start) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.layout_overflow",
            "The spatial-list start offset overflowed.");
        return result;
    }

    std::vector<std::size_t> list_offsets;
    list_offsets.reserve(cell_count);
    auto cursor = *lists_start;
    for (const auto& list : references) {
        list_offsets.push_back(cursor);
        const auto entries = checked_add(list.size(), 1U);
        const auto list_bytes = entries ? checked_multiply(*entries, 4U) : std::nullopt;
        const auto next = list_bytes ? checked_add(cursor, *list_bytes) : std::nullopt;
        if (!next || cursor < formats::hits::relative_offset_base ||
            cursor - formats::hits::relative_offset_base >
                static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.writer.cell_list_overflow",
                "A cell list pointer or size exceeds the signed 32-bit ABI.");
            return result;
        }
        cursor = *next;
    }

    const auto triangle_offset = cursor;
    const auto triangle_bytes = checked_multiply(
        prepared.size(), formats::hits::triangle_size);
    const auto end_offset = triangle_bytes
        ? checked_add(triangle_offset, *triangle_bytes)
        : std::nullopt;
    const auto file_size = end_offset ? align16(*end_offset) : std::nullopt;
    if (!triangle_bytes || !end_offset || !file_size ||
        triangle_offset < formats::hits::relative_offset_base ||
        triangle_offset - formats::hits::relative_offset_base >
            std::numeric_limits<std::uint32_t>::max() ||
        *end_offset > std::numeric_limits<std::uint32_t>::max()) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.output_layout_overflow",
            "The rebuilt HITS output exceeds its 32-bit offset ABI.");
        return result;
    }

    result.header.triangle_array_relative_offset = static_cast<std::uint32_t>(
        triangle_offset - formats::hits::relative_offset_base);
    result.header.end_offset = static_cast<std::uint32_t>(*end_offset);
    result.bytes.assign(*file_size, std::byte{0});
    auto output = std::span<std::byte>{result.bytes};

    output[0] = std::byte{'H'};
    output[1] = std::byte{'I'};
    output[2] = std::byte{'T'};
    output[3] = std::byte{'S'};
    write_u32(output, 0x04U, result.header.end_offset);
    write_vec3(output, 0x08U, result.header.bounds_min);
    write_vec3(output, 0x14U, result.header.bounds_max);
    write_u32(output, 0x20U, result.header.cell_size.x);
    write_u32(output, 0x24U, result.header.cell_size.y);
    write_u32(output, 0x28U, result.header.cell_size.z);
    write_u32(output, 0x2CU, result.header.grid_count_x);
    write_u32(output, 0x30U, result.header.grid_count_y);
    write_u32(output, 0x34U, result.header.grid_count_z);
    write_u32(output, 0x38U, result.header.triangle_count);
    write_u32(output, 0x3CU, result.header.spatial_table_relative_offset);
    write_u32(output, 0x40U, result.header.triangle_array_relative_offset);

    for (std::size_t cell_index = 0U;
         cell_index < cell_count;
         ++cell_index) {
        const auto relative =
            list_offsets[cell_index] - formats::hits::relative_offset_base;
        write_i32(
            output,
            pointer_table_offset + cell_index * 4U,
            static_cast<std::int32_t>(relative));
        auto list_cursor = list_offsets[cell_index];
        for (const auto reference : references[cell_index]) {
            write_i32(output, list_cursor, static_cast<std::int32_t>(reference));
            list_cursor += 4U;
        }
        write_i32(output, list_cursor, formats::hits::cell_list_terminator);
    }

    result.locations.reserve(prepared.size());
    for (std::size_t index = 0U; index < prepared.size(); ++index) {
        const auto record_offset =
            triangle_offset + index * formats::hits::triangle_size;
        auto& triangle = prepared[index].triangle;
        triangle.offset = record_offset;
        write_u32(output, record_offset, triangle.flags);
        write_vec3(output, record_offset + 0x04U, triangle.point_a);
        write_vec3(output, record_offset + 0x10U, triangle.point_b);
        write_vec3(output, record_offset + 0x1CU, triangle.point_c);
        write_vec3(output, record_offset + 0x28U, triangle.normal);
        write_f32(output, record_offset + 0x34U, triangle.plane_d);
        result.locations.push_back(SurfaceLocation{
            .stable_id = prepared[index].stable_id,
            .triangle_index = static_cast<std::uint32_t>(index),
            .triangle_byte_offset = static_cast<std::uint32_t>(
                index * formats::hits::triangle_size),
            .file_offset = record_offset,
        });
    }

    const auto round_trip = formats::hits::RecordScanner::scan(result.bytes);
    if (!round_trip.ok() ||
        round_trip.header.triangle_count != result.header.triangle_count ||
        round_trip.cells.size() != cell_count ||
        round_trip.triangles.size() != prepared.size()) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "hits.writer.round_trip_failed",
            "The rebuilt HITS output did not pass the canonical parser round trip.");
        return result;
    }

    add_diagnostic(
        result,
        ParseSeverity::warning,
        "hits.writer.modified_topology_game_validation_required",
        "The stock-corpus cell assignment is reproduced by boundary-inclusive candidate cells plus triangle-box SAT; modified topology still requires controlled in-game validation.");
    return result;
}

} // namespace dmc::rengine::hits::writer
