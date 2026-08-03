#pragma once

#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/runtime.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::rengine::hits::contact {

using formats::hits::ScanResult;
using formats::hits::Triangle;
using formats::hits::Vec3;
using runtime::StaticSource;

// Neutral names are intentional. The EXE confirms three normal-Y classes,
// but the original gameplay names remain RESEARCH REQUIRED.
enum class NormalYClass : std::uint8_t {
    low = 0,
    middle = 1,
    high = 2,
};

struct TriangleCandidate final {
    StaticSource source{StaticSource::source_0_member_3};
    std::uint32_t triangle_index{};
    std::uint32_t triangle_byte_offset{};
    std::uint32_t flags{};
    const Triangle* triangle{};
};

[[nodiscard]] inline NormalYClass classify_normal_y(
    const Vec3& normal) noexcept {
    constexpr float sin_30_degrees = 0.5F;
    constexpr float sin_45_degrees = 0.7071067811865475F;
    const auto absolute_y = std::fabs(normal.y);
    if (absolute_y < sin_30_degrees) {
        return NormalYClass::low;
    }
    if (absolute_y < sin_45_degrees) {
        return NormalYClass::middle;
    }
    return NormalYClass::high;
}

[[nodiscard]] inline std::optional<Vec3> apply_normal_correction(
    const Vec3& position,
    const Vec3& normal,
    float correction) noexcept {
    if (!std::isfinite(position.x) ||
        !std::isfinite(position.y) ||
        !std::isfinite(position.z) ||
        !std::isfinite(normal.x) ||
        !std::isfinite(normal.y) ||
        !std::isfinite(normal.z) ||
        !std::isfinite(correction)) {
        return std::nullopt;
    }

    return Vec3{
        .x = position.x + normal.x * correction,
        .y = position.y + normal.y * correction,
        .z = position.z + normal.z * correction,
    };
}

[[nodiscard]] inline std::optional<std::vector<TriangleCandidate>>
resolve_static_candidates(
    const ScanResult& scan,
    StaticSource source,
    std::span<const std::uint32_t> cell_indices,
    std::uint16_t caller_reject_mask) {
    if (!scan.ok()) {
        return std::nullopt;
    }

    const auto offsets = runtime::collect_unique_triangle_offsets(
        scan.cells, cell_indices);
    if (!offsets) {
        return std::nullopt;
    }

    std::vector<TriangleCandidate> candidates;
    candidates.reserve(offsets->size());
    for (const auto byte_offset : *offsets) {
        if ((byte_offset % formats::hits::triangle_size) != 0U) {
            return std::nullopt;
        }
        const auto triangle_index =
            byte_offset / static_cast<std::uint32_t>(formats::hits::triangle_size);
        if (triangle_index >= scan.triangles.size()) {
            return std::nullopt;
        }

        const auto& triangle = scan.triangles[triangle_index];
        if (runtime::rejected_by_upper_mask(
                triangle.flags, caller_reject_mask)) {
            continue;
        }

        candidates.push_back(TriangleCandidate{
            .source = source,
            .triangle_index = triangle_index,
            .triangle_byte_offset = byte_offset,
            .flags = triangle.flags,
            .triangle = &triangle,
        });
    }
    return candidates;
}

[[nodiscard]] inline std::optional<std::vector<TriangleCandidate>>
query_static_candidates(
    const ScanResult& scan,
    StaticSource source,
    const Vec3& first,
    const Vec3& second,
    std::uint16_t caller_reject_mask) {
    const auto cells = runtime::enumerate_cells(scan.header, first, second);
    if (!cells) {
        return std::nullopt;
    }
    return resolve_static_candidates(
        scan, source, *cells, caller_reject_mask);
}

} // namespace dmc::rengine::hits::contact
