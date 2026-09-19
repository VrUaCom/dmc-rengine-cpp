#pragma once

#include "rengine/lsg/camera.hpp"

#include <cstdint>

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

} // namespace rengine::lsg
