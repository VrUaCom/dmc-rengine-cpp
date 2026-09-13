#pragma once

#include "dmc_rengine/formats/hits.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <unordered_set>
#include <vector>

namespace dmc::rengine::hits::runtime {

using formats::hits::Cell;
using formats::hits::Header;
using formats::hits::Triangle;
using formats::hits::Vec3;

struct GridCoordinate final {
    std::uint32_t x{};
    std::uint32_t y{};
    std::uint32_t z{};

    friend bool operator==(const GridCoordinate&, const GridCoordinate&) = default;
};

struct GridRange final {
    GridCoordinate minimum;
    GridCoordinate maximum;
};

enum class StaticSource : std::uint8_t {
    source_0_member_3 = 0,
    source_1_member_6 = 1,
};

class StaticSourceSelector final {
public:
    constexpr StaticSourceSelector(
        bool source_0_available,
        bool source_1_available) noexcept
        : source_0_available_(source_0_available),
          source_1_available_(source_1_available) {}

    [[nodiscard]] constexpr StaticSource selected() const noexcept {
        return selected_;
    }

    [[nodiscard]] constexpr bool available(StaticSource source) const noexcept {
        switch (source) {
        case StaticSource::source_0_member_3:
            return source_0_available_;
        case StaticSource::source_1_member_6:
            return source_1_available_;
        }
        return false;
    }

    [[nodiscard]] constexpr bool select(StaticSource source) noexcept {
        if (!available(source)) {
            return false;
        }
        selected_ = source;
        return true;
    }

    [[nodiscard]] constexpr bool restore_default() noexcept {
        return select(StaticSource::source_0_member_3);
    }

private:
    bool source_0_available_{};
    bool source_1_available_{};
    StaticSource selected_{StaticSource::source_0_member_3};
};

[[nodiscard]] inline bool valid_runtime_grid(const Header& header) noexcept {
    return header.grid_count_x != 0U &&
        header.grid_count_y != 0U &&
        header.grid_count_z != 0U &&
        header.cell_size.x != 0U &&
        header.cell_size.y != 0U &&
        header.cell_size.z != 0U &&
        std::isfinite(header.bounds_min.x) &&
        std::isfinite(header.bounds_min.y) &&
        std::isfinite(header.bounds_min.z) &&
        std::isfinite(header.bounds_max.x) &&
        std::isfinite(header.bounds_max.y) &&
        std::isfinite(header.bounds_max.z);
}

[[nodiscard]] inline std::optional<GridCoordinate> world_to_grid(
    const Header& header,
    const Vec3& point) noexcept {
    if (!valid_runtime_grid(header) ||
        !std::isfinite(point.x) ||
        !std::isfinite(point.y) ||
        !std::isfinite(point.z)) {
        return std::nullopt;
    }

    // Canonical EXE 0x1402D2F50 truncates the float world delta to integer,
    // performs integer division by the serialized DWORD cell extent, and then
    // clamps to [0, count-1]. For positive in-grid deltas this is equivalent
    // to floor(delta / extent); explicit boundary clamps avoid unsafe casts.
    const auto axis = [](float value, float minimum, std::uint32_t cell_size,
                         std::uint32_t count) -> std::uint32_t {
        if (value <= minimum) {
            return 0U;
        }
        const auto delta = static_cast<double>(value) -
            static_cast<double>(minimum);
        const auto grid_extent = static_cast<double>(cell_size) *
            static_cast<double>(count);
        if (delta >= grid_extent) {
            return count - 1U;
        }
        const auto raw = static_cast<std::uint64_t>(
            std::floor(delta / static_cast<double>(cell_size)));
        return static_cast<std::uint32_t>(std::min<std::uint64_t>(
            raw, static_cast<std::uint64_t>(count - 1U)));
    };

    return GridCoordinate{
        .x = axis(point.x, header.bounds_min.x,
                  header.cell_size.x, header.grid_count_x),
        .y = axis(point.y, header.bounds_min.y,
                  header.cell_size.y, header.grid_count_y),
        .z = axis(point.z, header.bounds_min.z,
                  header.cell_size.z, header.grid_count_z),
    };
}

[[nodiscard]] inline std::optional<std::uint32_t> flatten(
    const Header& header,
    GridCoordinate coordinate) noexcept {
    if (!valid_runtime_grid(header) ||
        coordinate.x >= header.grid_count_x ||
        coordinate.y >= header.grid_count_y ||
        coordinate.z >= header.grid_count_z) {
        return std::nullopt;
    }

    const auto value =
        (static_cast<std::uint64_t>(coordinate.x) * header.grid_count_y +
         coordinate.y) * header.grid_count_z + coordinate.z;
    if (value > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(value);
}

[[nodiscard]] inline std::optional<GridRange> world_range_to_grid(
    const Header& header,
    const Vec3& first,
    const Vec3& second) noexcept {
    const auto first_cell = world_to_grid(header, first);
    const auto second_cell = world_to_grid(header, second);
    if (!first_cell || !second_cell) {
        return std::nullopt;
    }

    return GridRange{
        .minimum = GridCoordinate{
            .x = std::min(first_cell->x, second_cell->x),
            .y = std::min(first_cell->y, second_cell->y),
            .z = std::min(first_cell->z, second_cell->z),
        },
        .maximum = GridCoordinate{
            .x = std::max(first_cell->x, second_cell->x),
            .y = std::max(first_cell->y, second_cell->y),
            .z = std::max(first_cell->z, second_cell->z),
        },
    };
}

[[nodiscard]] inline std::optional<std::vector<std::uint32_t>>
enumerate_cells(
    const Header& header,
    const Vec3& first,
    const Vec3& second) {
    const auto range = world_range_to_grid(header, first, second);
    if (!range) {
        return std::nullopt;
    }

    const auto count_x = static_cast<std::uint64_t>(
        range->maximum.x - range->minimum.x + 1U);
    const auto count_y = static_cast<std::uint64_t>(
        range->maximum.y - range->minimum.y + 1U);
    const auto count_z = static_cast<std::uint64_t>(
        range->maximum.z - range->minimum.z + 1U);
    const auto total = count_x * count_y * count_z;
    if (total > std::numeric_limits<std::size_t>::max()) {
        return std::nullopt;
    }

    std::vector<std::uint32_t> indices;
    indices.reserve(static_cast<std::size_t>(total));
    for (auto x = range->minimum.x; x <= range->maximum.x; ++x) {
        for (auto y = range->minimum.y; y <= range->maximum.y; ++y) {
            for (auto z = range->minimum.z; z <= range->maximum.z; ++z) {
                const auto index = flatten(header, GridCoordinate{x, y, z});
                if (!index) {
                    return std::nullopt;
                }
                indices.push_back(*index);
            }
        }
    }
    return indices;
}

[[nodiscard]] constexpr bool rejected_by_upper_mask(
    std::uint32_t flags,
    std::uint16_t caller_reject_mask) noexcept {
    return ((flags >> 16U) & caller_reject_mask) != 0U;
}

[[nodiscard]] inline std::optional<std::vector<std::uint32_t>>
collect_unique_triangle_offsets(
    std::span<const Cell> cells,
    std::span<const std::uint32_t> cell_indices) {
    std::vector<std::uint32_t> offsets;
    std::unordered_set<std::uint32_t> seen;

    for (const auto cell_index : cell_indices) {
        if (cell_index >= cells.size()) {
            return std::nullopt;
        }
        for (const auto offset : cells[cell_index].triangle_byte_offsets) {
            if (seen.insert(offset).second) {
                offsets.push_back(offset);
            }
        }
    }
    return offsets;
}

[[nodiscard]] inline float maximum_plane_residual(
    const Triangle& triangle) noexcept {
    return std::max({
        std::fabs(formats::hits::evaluate_plane(triangle, triangle.point_a)),
        std::fabs(formats::hits::evaluate_plane(triangle, triangle.point_b)),
        std::fabs(formats::hits::evaluate_plane(triangle, triangle.point_c)),
    });
}

} // namespace dmc::rengine::hits::runtime
