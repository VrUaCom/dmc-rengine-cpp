#pragma once

#include <array>
#include <cmath>
#include <cstddef>

// Small row-vector math shared by the DMC3 effect ports (P particles, G
// generators, effect parents). DMC3 multiplies row vectors: p' = p * M, with
// the translation in row 3 (values[12..14]).
namespace dmc::rengine::profiles::dmc3::fx {

struct Vec2 final {
    float u{};
    float v{};
};

struct Vec3 final {
    float x{};
    float y{};
    float z{};
};

struct Matrix4 final {
    std::array<float, 16> values{
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F,
    };
};

// Rz * Ry * Rx with the given translation row, as 0x140030F10 / 0x140030FC0 /
// 0x140031080 build the three rotations (row-vector convention).
[[nodiscard]] inline Matrix4 local_matrix_zyx(const std::array<float, 3>& translation,
                                              const std::array<float, 3>& rotation_xyz_radians) noexcept {
    const float cx = std::cos(rotation_xyz_radians[0]), sx = std::sin(rotation_xyz_radians[0]);
    const float cy = std::cos(rotation_xyz_radians[1]), sy = std::sin(rotation_xyz_radians[1]);
    const float cz = std::cos(rotation_xyz_radians[2]), sz = std::sin(rotation_xyz_radians[2]);
    const std::array<float, 9> rx{1.0F, 0.0F, 0.0F, 0.0F, cx, sx, 0.0F, -sx, cx};
    const std::array<float, 9> ry{cy, 0.0F, -sy, 0.0F, 1.0F, 0.0F, sy, 0.0F, cy};
    const std::array<float, 9> rz{cz, sz, 0.0F, -sz, cz, 0.0F, 0.0F, 0.0F, 1.0F};
    const auto mul = [](const std::array<float, 9>& a, const std::array<float, 9>& b) {
        std::array<float, 9> out{};
        for (std::size_t r = 0U; r < 3U; ++r) {
            for (std::size_t c = 0U; c < 3U; ++c) {
                for (std::size_t k = 0U; k < 3U; ++k) out[r * 3U + c] += a[r * 3U + k] * b[k * 3U + c];
            }
        }
        return out;
    };
    const auto m = mul(mul(rz, ry), rx);
    Matrix4 out;
    for (std::size_t r = 0U; r < 3U; ++r) {
        for (std::size_t c = 0U; c < 3U; ++c) out.values[r * 4U + c] = m[r * 3U + c];
    }
    out.values[12] = translation[0];
    out.values[13] = translation[1];
    out.values[14] = translation[2];
    return out;
}

// a * b for row-vector matrices (apply a, then b).
[[nodiscard]] inline Matrix4 multiply(const Matrix4& a, const Matrix4& b) noexcept {
    Matrix4 r;
    for (std::size_t i = 0U; i < 4U; ++i) {
        for (std::size_t j = 0U; j < 4U; ++j) {
            float s = 0.0F;
            for (std::size_t k = 0U; k < 4U; ++k) s += a.values[i * 4U + k] * b.values[k * 4U + j];
            r.values[i * 4U + j] = s;
        }
    }
    return r;
}

}  // namespace dmc::rengine::profiles::dmc3::fx

namespace dmc::rengine::profiles::dmc3 {
using fx::Matrix4;
using fx::Vec2;
using fx::Vec3;
}  // namespace dmc::rengine::profiles::dmc3
