#include "dmc_rengine/formats/scm_transform.hpp"

#include <cmath>

namespace dmc::rengine::formats::scm {

Matrix4f multiply_dmc3_matrices(
    const Matrix4f& left,
    const Matrix4f& right) noexcept {
    Matrix4f result{};
    for (std::size_t row = 0U; row < 4U; ++row) {
        for (std::size_t column = 0U; column < 4U; ++column) {
            float value = 0.0F;
            for (std::size_t k = 0U; k < 4U; ++k)
                value += left(row, k) * right(k, column);
            result.values[row * 4U + column] = value;
        }
    }
    return result;
}

Matrix4f invert_dmc3_rigid_transform(const Matrix4f& matrix) noexcept {
    Matrix4f result = identity_matrix();

    for (std::size_t row = 0U; row < 3U; ++row) {
        for (std::size_t column = 0U; column < 3U; ++column)
            result.values[row * 4U + column] = matrix(column, row);
    }

    const auto tx = matrix(3U, 0U);
    const auto ty = matrix(3U, 1U);
    const auto tz = matrix(3U, 2U);
    result.values[12] = -(tx * matrix(0U, 0U) +
                          ty * matrix(0U, 1U) +
                          tz * matrix(0U, 2U));
    result.values[13] = -(tx * matrix(1U, 0U) +
                          ty * matrix(1U, 1U) +
                          tz * matrix(1U, 2U));
    result.values[14] = -(tx * matrix(2U, 0U) +
                          ty * matrix(2U, 1U) +
                          tz * matrix(2U, 2U));
    return result;
}

Matrix4f build_rotation_xyz_radians(
    const Vec3f& rotation_xyz_radians) noexcept {
    const auto cx = std::cos(rotation_xyz_radians.x);
    const auto sx = std::sin(rotation_xyz_radians.x);
    const auto cy = std::cos(rotation_xyz_radians.y);
    const auto sy = std::sin(rotation_xyz_radians.y);
    const auto cz = std::cos(rotation_xyz_radians.z);
    const auto sz = std::sin(rotation_xyz_radians.z);

    // Exact row-major expansion of Rz * Ry * Rx for the axis matrices
    // recovered from 0x140030F10 / 0x140030FC0 / 0x140031080.
    return Matrix4f{{
        cy * cz,
        cx * sz + cz * sx * sy,
        -cx * cz * sy + sx * sz,
        0.0F,

        -cy * sz,
        cx * cz - sx * sy * sz,
        cx * sy * sz + cz * sx,
        0.0F,

        sy,
        -cy * sx,
        cx * cy,
        0.0F,

        0.0F,
        0.0F,
        0.0F,
        1.0F,
    }};
}

Matrix4f build_local_transform(const SceneTransform& transform) noexcept {
    auto result = build_rotation_xyz_radians(transform.rotation_xyz_radians);

    // 0x140031200 copies the basis and adds serialized translation XYZ to
    // row 3. Its runtime-initialized mask is {0,0,0,0xFFFFFFFF}, preserving
    // the original homogeneous W instead of adding transform +0x0C.
    result.values[12] += transform.translation.x;
    result.values[13] += transform.translation.y;
    result.values[14] += transform.translation.z;
    return result;
}

} // namespace dmc::rengine::formats::scm
