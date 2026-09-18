#pragma once

#include <array>
#include <cstdint>

namespace rengine::lsg {

enum class LightingPreset : std::uint8_t {
  morning = 0,
  noon = 1,
  evening = 2,
  night = 3,
};

enum class OpticalFilterPreset : std::uint8_t {
  clear = 0,
  tinted = 1,
  polarized_approx = 2,
};

struct LightingRuntimeState {
  LightingPreset preset{LightingPreset::noon};
  OpticalFilterPreset filter{OpticalFilterPreset::clear};

  float sun_elevation_rad{};
  float sun_azimuth_rad{};
  std::array<float, 3> sun_direction{0.0f, 1.0f, 0.0f};

  float direct_sun_intensity{1.0f};
  float sky_intensity{1.0f};
  float exposure{1.0f};
  float scene_luminance{1.0f};
  float effective_eye_luminance{1.0f};

  std::array<float, 3> sun_tint{1.0f, 0.95f, 0.86f};
  std::array<float, 3> sky_zenith_tint{0.16f, 0.32f, 0.58f};
  std::array<float, 3> sky_horizon_tint{0.36f, 0.48f, 0.62f};

  float filter_transmission{1.0f};
  float polarization_strength{};
  std::array<float, 3> filter_tint{1.0f, 1.0f, 1.0f};
};

[[nodiscard]] LightingRuntimeState lighting_for(
    LightingPreset preset,
    OpticalFilterPreset filter) noexcept;

[[nodiscard]] bool valid_lighting_state(const LightingRuntimeState& state) noexcept;

}  // namespace rengine::lsg
