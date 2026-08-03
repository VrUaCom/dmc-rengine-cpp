#pragma once

#include "dmc_rengine/formats/hits.hpp"

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::rengine::hits::edit {

using formats::hits::Triangle;
using formats::hits::Vec3;

struct TriangleGeometry final {
    Vec3 point_a;
    Vec3 point_b;
    Vec3 point_c;
    Vec3 normal;
    float plane_d{};
};

struct SafeTriangleEdit final {
    std::uint32_t triangle_index{};
    std::uint64_t record_offset{};
    std::uint32_t preserved_flags{};
    TriangleGeometry geometry;
};

[[nodiscard]] inline bool finite(const Vec3& value) noexcept {
    return std::isfinite(value.x) && std::isfinite(value.y) &&
           std::isfinite(value.z);
}

[[nodiscard]] inline Vec3 subtract(const Vec3& left, const Vec3& right) noexcept {
    return Vec3{
        .x = left.x - right.x,
        .y = left.y - right.y,
        .z = left.z - right.z,
    };
}

[[nodiscard]] inline Vec3 cross(const Vec3& left, const Vec3& right) noexcept {
    return Vec3{
        .x = left.y * right.z - left.z * right.y,
        .y = left.z * right.x - left.x * right.z,
        .z = left.x * right.y - left.y * right.x,
    };
}

[[nodiscard]] inline float dot(const Vec3& left, const Vec3& right) noexcept {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

[[nodiscard]] inline float length_squared(const Vec3& value) noexcept {
    return dot(value, value);
}

[[nodiscard]] inline std::optional<TriangleGeometry> recompute_geometry(
    const Vec3& point_a,
    const Vec3& point_b,
    const Vec3& point_c,
    float degeneracy_epsilon = 1.0e-12F) noexcept {
    if (!finite(point_a) || !finite(point_b) || !finite(point_c) ||
        !std::isfinite(degeneracy_epsilon) || degeneracy_epsilon <= 0.0F) {
        return std::nullopt;
    }

    const auto edge_ab = subtract(point_b, point_a);
    const auto edge_ac = subtract(point_c, point_a);
    const auto raw_normal = cross(edge_ab, edge_ac);
    const auto squared = length_squared(raw_normal);
    if (!std::isfinite(squared) || squared <= degeneracy_epsilon) {
        return std::nullopt;
    }

    const auto inverse_length = 1.0F / std::sqrt(squared);
    const Vec3 normal{
        .x = raw_normal.x * inverse_length,
        .y = raw_normal.y * inverse_length,
        .z = raw_normal.z * inverse_length,
    };
    const auto plane_d = -dot(normal, point_a);
    if (!finite(normal) || !std::isfinite(plane_d)) {
        return std::nullopt;
    }

    return TriangleGeometry{
        .point_a = point_a,
        .point_b = point_b,
        .point_c = point_c,
        .normal = normal,
        .plane_d = plane_d,
    };
}

[[nodiscard]] inline std::optional<SafeTriangleEdit> prepare_safe_edit(
    const Triangle& original,
    std::uint32_t triangle_index,
    const Vec3& point_a,
    const Vec3& point_b,
    const Vec3& point_c) noexcept {
    const auto geometry = recompute_geometry(point_a, point_b, point_c);
    if (!geometry) {
        return std::nullopt;
    }
    return SafeTriangleEdit{
        .triangle_index = triangle_index,
        .record_offset = original.offset,
        .preserved_flags = original.flags,
        .geometry = *geometry,
    };
}

inline void write_u32_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint32_t value) noexcept {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

inline void write_f32_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    float value) noexcept {
    write_u32_le(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

inline void write_vec3_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    const Vec3& value) noexcept {
    write_f32_le(bytes, offset, value.x);
    write_f32_le(bytes, offset + 4U, value.y);
    write_f32_le(bytes, offset + 8U, value.z);
}

[[nodiscard]] inline bool apply_topology_preserving_edit(
    std::span<std::byte> bytes,
    const SafeTriangleEdit& edit) noexcept {
    const auto offset = static_cast<std::size_t>(edit.record_offset);
    if (offset > bytes.size() ||
        formats::hits::triangle_size > bytes.size() - offset ||
        !finite(edit.geometry.point_a) || !finite(edit.geometry.point_b) ||
        !finite(edit.geometry.point_c) || !finite(edit.geometry.normal) ||
        !std::isfinite(edit.geometry.plane_d)) {
        return false;
    }

    // Preserve the original raw flags and patch only the 13 float fields.
    write_u32_le(bytes, offset, edit.preserved_flags);
    write_vec3_le(bytes, offset + 0x04U, edit.geometry.point_a);
    write_vec3_le(bytes, offset + 0x10U, edit.geometry.point_b);
    write_vec3_le(bytes, offset + 0x1CU, edit.geometry.point_c);
    write_vec3_le(bytes, offset + 0x28U, edit.geometry.normal);
    write_f32_le(bytes, offset + 0x34U, edit.geometry.plane_d);
    return true;
}

[[nodiscard]] inline std::optional<std::vector<std::byte>>
serialize_topology_preserving_copy(
    std::span<const std::byte> source,
    const SafeTriangleEdit& edit) {
    std::vector<std::byte> output(source.begin(), source.end());
    if (!apply_topology_preserving_edit(output, edit)) {
        return std::nullopt;
    }
    return output;
}

} // namespace dmc::rengine::hits::edit
