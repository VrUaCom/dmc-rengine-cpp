#pragma once

#include <cstdint>

namespace rengine::lsg {

enum class CameraPreset : std::uint8_t { full_body = 0, portrait = 1, extreme_close_up = 2 };

struct CameraState {
  float yaw_radians{};
  float pitch_radians{};
  float distance_m{4.5f};
  float target_y_m{};
  float fov_y_radians{0.4712389f};
  CameraPreset preset{CameraPreset::full_body};
};

class CameraController {
public:
  CameraController() noexcept;
  void set_subject_height(float height_m) noexcept;
  void set_preset(CameraPreset preset) noexcept;
  void orbit(float normalized_dx, float normalized_dy) noexcept;
  void zoom(float scale) noexcept;
  [[nodiscard]] const CameraState& state() const noexcept { return state_; }

private:
  void apply_preset(CameraPreset preset) noexcept;
  float subject_height_m_{1.75f};
  CameraState state_{};
};

} // namespace rengine::lsg
