#pragma once

#include "rengine/lsg/camera.hpp"

#include <cstdint>
#include <array>

namespace rengine::lsg {

enum class CloseShadowLevel : std::uint8_t {
  baseline = 0,
  portrait = 1,
  cinematic = 2,
};

struct CloseShadowConfig {
  CloseShadowLevel level{CloseShadowLevel::baseline};
  float half_extent_m{1.20f};
  float depth_half_extent_m{2.00f};
  float texel_size_m{};
};

[[nodiscard]] CloseShadowLevel select_close_shadow_level(
    CameraPreset preset, float camera_distance_m) noexcept;

[[nodiscard]] CloseShadowConfig close_shadow_config(
    CameraPreset preset,
    float camera_distance_m,
    std::uint32_t shadow_map_size = 2048u) noexcept;

[[nodiscard]] float snap_shadow_axis(float value_m,
                                     float texel_size_m) noexcept;

// CPU reference for the cinematic GLSL filter, used with synthetic depth fields.
// dx/dy contain screen derivatives of normalized shadow UV and depth.
[[nodiscard]] std::array<float, 2> receiver_depth_gradient(
    std::array<float, 3> dx, std::array<float, 3> dy) noexcept;

[[nodiscard]] float cinematic_shadow_visibility_reference(
    const std::array<float, 25>& depths, std::array<float, 2> pixel_fraction,
    float receiver_depth, std::array<float, 2> depth_gradient,
    std::uint32_t map_size, float bias, float max_correction) noexcept;

} // namespace rengine::lsg
