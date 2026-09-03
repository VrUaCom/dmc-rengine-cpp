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

// Reconstruct the exact rotation matrix sequence used by the canonical DMC3
// scene-transform consumer 0x1402FA080 -> 0x140330450.
//
// Serialized angles are X/Y/Z Euler radians. The original code applies the
// axis rotations in X -> Y -> Z order through its matrix multiplier, which
// yields the matrix product Rz * Ry * Rx from an identity starting basis.
[[nodiscard]] Matrix4f build_rotation_xyz_radians(
    const Vec3f& rotation_xyz_radians) noexcept;

} // namespace dmc::rengine::formats::scm
