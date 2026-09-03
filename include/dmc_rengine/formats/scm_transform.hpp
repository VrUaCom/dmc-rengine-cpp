#pragma once

#include "dmc_rengine/formats/scm.hpp"

#include <array>
#include <cstddef>

namespace dmc::rengine::formats::scm {

struct Matrix4f final {
    // Row-major storage matching the recovered DMC3 matrix helper layout.
    std::array<float, 16> values{};

    [[nodiscard]] constexpr float operator()(
        std::size_t row,
        std::size_t column) const noexcept {
        return values[row * 4U + column];
    }
};

[[nodiscard]] constexpr Matrix4f identity_matrix() noexcept {
    return Matrix4f{{
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F,
    }};
}

// Reconstruct 0x1400312B0: result = left * right in the recovered
// DMC3 row-major / row-vector matrix convention.
[[nodiscard]] Matrix4f multiply_dmc3_matrices(
    const Matrix4f& left,
    const Matrix4f& right) noexcept;

// Reconstruct 0x140030DC0 for the rigid SCM node transforms produced by the
// original scene path: transpose the 3x3 rotation basis and compute the
// inverse translation -T * R^T. This is the inverse-world cache form written
// into the runtime node by 0x1402FA080.
[[nodiscard]] Matrix4f invert_dmc3_rigid_transform(
    const Matrix4f& matrix) noexcept;

// Reconstruct the exact rotation matrix sequence used by the canonical DMC3
// scene-transform consumer 0x1402FA080 -> 0x140330450.
//
// Serialized angles are X/Y/Z Euler radians. The original code applies the
// axis rotations in X -> Y -> Z order through its matrix multiplier, which
// yields the matrix product Rz * Ry * Rx from an identity starting basis.
[[nodiscard]] Matrix4f build_rotation_xyz_radians(
    const Vec3f& rotation_xyz_radians) noexcept;

// Reconstruct the local scene-node matrix produced by 0x1402FA080 using
// 0x140330450 followed by 0x140031200. Translation occupies row 3 XYZ.
// The serialized translation-magnitude lane (+0x0C) is deliberately masked
// out by the original helper; homogeneous W remains 1.
[[nodiscard]] Matrix4f build_local_transform(
    const SceneTransform& transform) noexcept;

} // namespace dmc::rengine::formats::scm
