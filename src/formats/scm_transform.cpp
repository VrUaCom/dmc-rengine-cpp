#include "dmc_rengine/formats/scm_transform.hpp"

#include <cmath>

namespace dmc::rengine::formats::scm {

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

} // namespace dmc::rengine::formats::scm
