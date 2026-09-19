#include "rengine/lsg/shadow_quality.hpp"

#include <algorithm>
#include <cmath>

namespace rengine::lsg {

CloseShadowLevel select_close_shadow_level(
    CameraPreset preset, float camera_distance_m) noexcept {
  const bool finite_distance =
      std::isfinite(camera_distance_m) && camera_distance_m > 0.0f;

  if (preset == CameraPreset::extreme_close_up ||
      (finite_distance && camera_distance_m <= 1.35f)) {
    return CloseShadowLevel::cinematic;
  }
  if (preset == CameraPreset::portrait ||
      (finite_distance && camera_distance_m <= 2.25f)) {
    return CloseShadowLevel::portrait;
  }
  return CloseShadowLevel::baseline;
}

CloseShadowConfig close_shadow_config(
    CameraPreset preset,
    float camera_distance_m,
    std::uint32_t shadow_map_size) noexcept {
  CloseShadowConfig out{};
  out.level = select_close_shadow_level(preset, camera_distance_m);

  switch (out.level) {
    case CloseShadowLevel::baseline:
      out.half_extent_m = 1.20f;
      out.depth_half_extent_m = 2.00f;
      break;
    case CloseShadowLevel::portrait:
      out.half_extent_m = 0.85f;
      out.depth_half_extent_m = 1.50f;
      break;
    case CloseShadowLevel::cinematic:
      out.half_extent_m = 0.55f;
      out.depth_half_extent_m = 1.10f;
      break;
  }

  const auto safe_size = std::max<std::uint32_t>(shadow_map_size, 1u);
  out.texel_size_m =
      (2.0f * out.half_extent_m) / static_cast<float>(safe_size);
  return out;
}

float snap_shadow_axis(float value_m, float texel_size_m) noexcept {
  if (!std::isfinite(value_m) ||
      !std::isfinite(texel_size_m) ||
      texel_size_m <= 0.0f) {
    return value_m;
  }
  return std::round(value_m / texel_size_m) * texel_size_m;
}

} // namespace rengine::lsg
