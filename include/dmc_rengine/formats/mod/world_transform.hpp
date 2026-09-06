#pragma once

#include "dmc_rengine/formats/mod/transform_domain.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace dmc::rengine::formats::mod::world_transform {

struct Matrix4f final {
    // Row-major storage matching the recovered DMC3 matrix helpers.
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

// Instance-level capability gate for spatial MOD preview. A format-level
// "transforms" flag is not sufficient: the concrete document must also have a
// valid topological hierarchy and a complete finite local-transform array.
[[nodiscard]] bool supports_spatial_hierarchy(
    const transform_domain::ParseResult& domain) noexcept;

// Reconstruct 0x1400312B0: result = left * right in the recovered DMC3
// row-major / row-vector convention.
[[nodiscard]] Matrix4f multiply_dmc3_matrices(
    const Matrix4f& left,
    const Matrix4f& right) noexcept;

// Reconstruct the MOD/EFM local matrix built by 0x1402FA080 through
// 0x140330450 and 0x140031200. Serialized +0x0C is not homogeneous W.
[[nodiscard]] Matrix4f build_local_matrix(
    const transform_domain::LocalTransformRecord& transform) noexcept;

// Reconstruct 0x1402F9700 for the MOD/EFM manager ABI established by
// 0x1402FA080. The caller-provided root base corresponds to manager+0x1B0.
[[nodiscard]] std::optional<std::vector<Matrix4f>> build_world_matrices(
    const transform_domain::ParseResult& domain,
    const Matrix4f& root_base) noexcept;

// Canonical model-space hierarchy: identical runtime composition with an
// identity external/root base. This is sufficient for a truthful 3D MOD
// hierarchy overlay without inventing positions from mesh vertices.
[[nodiscard]] std::optional<std::vector<Matrix4f>>
build_model_space_world_matrices(
    const transform_domain::ParseResult& domain) noexcept;

// With DMC3's recovered row-vector affine convention, transforming the origin
// yields row 3 XYZ. These are therefore the canonical node positions for the
// selected root base.
[[nodiscard]] constexpr transform_domain::Vec3f world_position(
    const Matrix4f& world) noexcept {
    return transform_domain::Vec3f{
        world.values[12],
        world.values[13],
        world.values[14],
    };
}

} // namespace dmc::rengine::formats::mod::world_transform
