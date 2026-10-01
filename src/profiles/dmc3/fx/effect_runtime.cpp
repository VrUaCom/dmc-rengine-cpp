#include "dmc_rengine/profiles/dmc3/fx/effect_runtime.hpp"

#include <cmath>

namespace dmc::rengine::profiles::dmc3::fx::runtime {

Matrix4 apply_mode(const Matrix4& source, MatrixMode mode) noexcept {
    Matrix4 out = source;
    switch (mode) {
    case MatrixMode::Copy:
        break;
    case MatrixMode::PositionOnly:
        out = Matrix4{};
        out.values[12] = source.values[12];
        out.values[13] = source.values[13];
        out.values[14] = source.values[14];
        break;
    case MatrixMode::RotationOnly:
        out.values[12] = 0.0F;
        out.values[13] = 0.0F;
        out.values[14] = 0.0F;
        break;
    case MatrixMode::Normalized:
        for (std::size_t row = 0U; row < 3U; ++row) {
            const float x = source.values[row * 4U], y = source.values[row * 4U + 1U],
                        z = source.values[row * 4U + 2U];
            const float length = std::sqrt(x * x + y * y + z * z);
            if (length > 0.0F) {
                out.values[row * 4U] = x / length;
                out.values[row * 4U + 1U] = y / length;
                out.values[row * 4U + 2U] = z / length;
            }
        }
        break;
    }
    return out;
}

Matrix4 ca0_spawn_matrix(const Matrix4* source) noexcept {
    Matrix4 out = source != nullptr ? *source : Matrix4{};
    out.values[13] += kCa0LiftY;
    return out;
}

}  // namespace dmc::rengine::profiles::dmc3::fx::runtime
