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

// Reconstruct the rigid-transform inverse helper 0x140030DC0. A 2026-09-05
// provenance correction established that the direct initialization use of
// this helper belongs to the MOD/EFM transform path (0x1402FA080), not the
// SCM-specific initializer. The arithmetic utility remains valid, but this
// declaration no longer claims an SCM runtime inverse-cache owner.
[[nodiscard]] Matrix4f invert_dmc3_rigid_transform(
    const Matrix4f& matrix) noexcept;

// Reconstruct the exact rotation matrix sequence used by the canonical SCM
// transform initializer 0x1402FA360 -> 0x140330450. The homologous MOD/EFM
// initializer 0x1402FA080 calls the same rotation helper on the same +0x10
// transform lane, which is now recorded in Model Family evidence.
//
// Serialized angles are X/Y/Z Euler radians. The original code applies the
// axis rotations in X -> Y -> Z order through its matrix multiplier, which
// yields the matrix product Rz * Ry * Rx from an identity starting basis.
[[nodiscard]] Matrix4f build_rotation_xyz_radians(
    const Vec3f& rotation_xyz_radians) noexcept;

// Reconstruct the local SCM scene-node matrix produced by 0x1402FA360 using
// 0x140330450 followed by 0x140031200. Translation occupies row 3 XYZ.
// The serialized translation-magnitude lane (+0x0C) is deliberately excluded
// from homogeneous W by the original helper; homogeneous W remains 1.
[[nodiscard]] Matrix4f build_local_transform(
    const SceneTransform& transform) noexcept;

} // namespace dmc::rengine::formats::scm
