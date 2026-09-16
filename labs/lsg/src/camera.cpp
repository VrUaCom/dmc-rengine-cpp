#include "rengine/lsg/camera.hpp"

#include <algorithm>
#include <cmath>

namespace rengine::lsg {
namespace {
constexpr float kMinPitch = -1.10f;
constexpr float kMaxPitch = 1.10f;
constexpr float kMinDistance = 0.32f;
constexpr float kMaxDistance = 12.0f;
constexpr float kOrbitYawPerScreen = 3.14159265f;
constexpr float kOrbitPitchPerScreen = 2.10f;

float fit_distance(float half_extent_m, float fov_y_radians, float fill_fraction) noexcept {
  const float tangent = std::tan(fov_y_radians * 0.5f);
  return half_extent_m / std::max(0.05f, tangent * fill_fraction);
}
}

CameraController::CameraController() noexcept { apply_preset(CameraPreset::full_body); }

void CameraController::set_subject_height(float height_m) noexcept {
  subject_height_m_ = std::clamp(height_m, 0.5f, 3.0f);
  apply_preset(state_.preset);
}

void CameraController::apply_preset(CameraPreset preset) noexcept {
  state_.preset = preset;
  state_.fov_y_radians = 0.4712389f;
  switch (preset) {
    case CameraPreset::full_body:
      state_.target_y_m = -subject_height_m_ * 0.055f;
      state_.distance_m = fit_distance(subject_height_m_ * 0.50f, state_.fov_y_radians, 0.78f);
      state_.pitch_radians = 0.0f;
      break;
    case CameraPreset::portrait:
      state_.target_y_m = subject_height_m_ * 0.30f;
      state_.distance_m = fit_distance(subject_height_m_ * 0.22f, state_.fov_y_radians, 0.80f);
      state_.pitch_radians = 0.0f;
      break;
    case CameraPreset::extreme_close_up:
      state_.target_y_m = subject_height_m_ * 0.40f;
      state_.distance_m = fit_distance(subject_height_m_ * 0.105f, state_.fov_y_radians, 0.82f);
      state_.pitch_radians = 0.0f;
      break;
  }
}

void CameraController::set_preset(CameraPreset preset) noexcept { apply_preset(preset); }

void CameraController::orbit(float normalized_dx, float normalized_dy) noexcept {
  state_.yaw_radians = std::remainder(state_.yaw_radians + normalized_dx * kOrbitYawPerScreen, 6.28318531f);
  state_.pitch_radians = std::clamp(state_.pitch_radians + normalized_dy * kOrbitPitchPerScreen,
                                    kMinPitch, kMaxPitch);
}

void CameraController::zoom(float scale) noexcept {
  if (!std::isfinite(scale) || scale <= 0.01f) return;
  state_.distance_m = std::clamp(state_.distance_m / scale, kMinDistance, kMaxDistance);
}

} // namespace rengine::lsg
