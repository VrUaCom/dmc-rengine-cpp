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

std::array<float, 2> receiver_depth_gradient(
    std::array<float, 3> dx, std::array<float, 3> dy) noexcept {
  const float det = dx[0] * dy[1] - dx[1] * dy[0];
  const float scale = std::max(std::hypot(dx[0], dx[1]) *
                               std::hypot(dy[0], dy[1]), 1e-20f);
  if (std::abs(det) <= 1e-5f * scale) return {};
  const float u = (dy[1] * dx[2] - dx[1] * dy[2]) / det;
  const float v = (dx[0] * dy[2] - dy[0] * dx[2]) / det;
  if (!std::isfinite(u) || !std::isfinite(v)) return {};
  return {std::clamp(u, -64.0f, 64.0f), std::clamp(v, -64.0f, 64.0f)};
}

float cinematic_shadow_visibility_reference(
    const std::array<float, 25>& depths, std::array<float, 2> pixel_fraction,
    float receiver_depth, std::array<float, 2> depth_gradient,
    std::uint32_t map_size, float bias, float max_correction) noexcept {
  if (map_size == 0u) return 1.0f;
  float visible = 0.0f, total = 0.0f;
  for (int y = -2; y <= 2; ++y) for (int x = -2; x <= 2; ++x) {
    const float du = static_cast<float>(x) + 0.5f - pixel_fraction[0];
    const float dv = static_cast<float>(y) + 0.5f - pixel_fraction[1];
    const float weight = std::max(2.5f - std::abs(du), 0.0f) *
                         std::max(2.5f - std::abs(dv), 0.0f);
    const float correction = std::clamp(
        (depth_gradient[0] * du + depth_gradient[1] * dv) /
          static_cast<float>(map_size), -max_correction, max_correction);
    const auto i = static_cast<std::size_t>((y + 2) * 5 + x + 2);
    if (receiver_depth + correction - bias <= depths[i]) visible += weight;
    total += weight;
  }
  return visible / std::max(total, 1e-6f);
}

} // namespace rengine::lsg
