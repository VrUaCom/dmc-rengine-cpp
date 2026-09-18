#include "rengine/lsg/lighting_runtime.hpp"

#include <algorithm>
#include <cmath>

namespace rengine::lsg {
namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float degrees(float value) noexcept {
  return value * (kPi / 180.0f);
}

struct LightingBase {
  float elevation_deg;
  float azimuth_deg;
  float direct;
  float sky;
  float exposure;
  float luminance;
  std::array<float, 3> sun_tint;
  std::array<float, 3> zenith_tint;
  std::array<float, 3> horizon_tint;
};

constexpr LightingBase base_for(LightingPreset preset) noexcept {
  switch (preset) {
    case LightingPreset::morning:
      return {27.0f, 105.0f, 0.72f, 0.70f, 1.08f, 1.10f,
              {1.00f, 0.72f, 0.50f},
              {0.11f, 0.24f, 0.50f},
              {0.62f, 0.48f, 0.40f}};
    case LightingPreset::noon:
      return {55.0f, 180.0f, 1.00f, 1.00f, 1.00f, 2.40f,
              {1.00f, 0.95f, 0.86f},
              {0.16f, 0.32f, 0.58f},
              {0.36f, 0.48f, 0.62f}};
    case LightingPreset::evening:
      return {16.0f, 255.0f, 0.48f, 0.52f, 1.18f, 0.72f,
              {1.00f, 0.49f, 0.26f},
              {0.09f, 0.16f, 0.34f},
              {0.72f, 0.34f, 0.22f}};
    case LightingPreset::night:
      return {-8.0f, 310.0f, 0.015f, 0.085f, 1.85f, 0.035f,
              {0.34f, 0.40f, 0.55f},
              {0.018f, 0.035f, 0.090f},
              {0.045f, 0.060f, 0.110f}};
  }
  return {55.0f, 180.0f, 1.00f, 1.00f, 1.00f, 2.40f,
          {1.00f, 0.95f, 0.86f},
          {0.16f, 0.32f, 0.58f},
          {0.36f, 0.48f, 0.62f}};
}

void apply_filter(OpticalFilterPreset filter,
                  float& transmission,
                  float& polarization_strength,
                  std::array<float, 3>& tint) noexcept {
  switch (filter) {
    case OpticalFilterPreset::clear:
      transmission = 1.0f;
      polarization_strength = 0.0f;
      tint = {1.0f, 1.0f, 1.0f};
      return;
    case OpticalFilterPreset::tinted:
      transmission = 0.58f;
      polarization_strength = 0.0f;
      tint = {0.90f, 0.94f, 0.98f};
      return;
    case OpticalFilterPreset::polarized_approx:
      transmission = 0.78f;
      polarization_strength = 0.68f;
      tint = {0.97f, 0.985f, 1.0f};
      return;
  }
  transmission = 1.0f;
  polarization_strength = 0.0f;
  tint = {1.0f, 1.0f, 1.0f};
}

std::array<float, 3> sun_direction(float elevation_rad,
                                   float azimuth_rad) noexcept {
  const float ce = std::cos(elevation_rad);
  const float x = ce * std::sin(azimuth_rad);
  const float y = std::sin(elevation_rad);
  const float z = ce * std::cos(azimuth_rad);
  const float length = std::sqrt(x * x + y * y + z * z);
  if (!(length > 0.0f) || !std::isfinite(length)) return {0.0f, 1.0f, 0.0f};
  return {x / length, y / length, z / length};
}

bool finite3(const std::array<float, 3>& v) noexcept {
  return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]);
}

}  // namespace

LightingRuntimeState lighting_for(LightingPreset preset,
                                  OpticalFilterPreset filter) noexcept {
  const auto base = base_for(preset);

  LightingRuntimeState state{};
  state.preset = preset;
  state.filter = filter;
  state.sun_elevation_rad = degrees(base.elevation_deg);
  state.sun_azimuth_rad = degrees(base.azimuth_deg);
  state.sun_direction = sun_direction(state.sun_elevation_rad, state.sun_azimuth_rad);
  state.direct_sun_intensity = base.direct;
  state.sky_intensity = base.sky;
  state.exposure = base.exposure;
  state.scene_luminance = base.luminance;
  state.sun_tint = base.sun_tint;
  state.sky_zenith_tint = base.zenith_tint;
  state.sky_horizon_tint = base.horizon_tint;

  apply_filter(filter, state.filter_transmission,
               state.polarization_strength, state.filter_tint);

  // This is a render-control luminance, not a calibrated photometric measurement.
  // Polarization attenuation is intentionally left for the GPU sky/glare pass; the
  // pupil sees bulk transmitted light here.
  state.effective_eye_luminance =
      std::max(0.0f, state.scene_luminance * state.filter_transmission);
  return state;
}

bool valid_lighting_state(const LightingRuntimeState& state) noexcept {
  if (!std::isfinite(state.sun_elevation_rad) ||
      !std::isfinite(state.sun_azimuth_rad) ||
      !finite3(state.sun_direction) ||
      !std::isfinite(state.direct_sun_intensity) ||
      !std::isfinite(state.sky_intensity) ||
      !std::isfinite(state.exposure) ||
      !std::isfinite(state.scene_luminance) ||
      !std::isfinite(state.effective_eye_luminance) ||
      !std::isfinite(state.filter_transmission) ||
      !std::isfinite(state.polarization_strength) ||
      !finite3(state.sun_tint) ||
      !finite3(state.sky_zenith_tint) ||
      !finite3(state.sky_horizon_tint) ||
      !finite3(state.filter_tint)) {
    return false;
  }

  const float sun_len = std::sqrt(
      state.sun_direction[0] * state.sun_direction[0] +
      state.sun_direction[1] * state.sun_direction[1] +
      state.sun_direction[2] * state.sun_direction[2]);

  return std::abs(sun_len - 1.0f) < 0.0005f &&
         state.direct_sun_intensity >= 0.0f &&
         state.sky_intensity >= 0.0f &&
         state.exposure > 0.0f &&
         state.scene_luminance >= 0.0f &&
         state.effective_eye_luminance >= 0.0f &&
         state.filter_transmission > 0.0f &&
         state.filter_transmission <= 1.0f &&
         state.polarization_strength >= 0.0f &&
         state.polarization_strength <= 1.0f &&
         state.sun_tint[0] >= 0.0f && state.sun_tint[0] <= 1.0f &&
         state.sun_tint[1] >= 0.0f && state.sun_tint[1] <= 1.0f &&
         state.sun_tint[2] >= 0.0f && state.sun_tint[2] <= 1.0f &&
         state.sky_zenith_tint[0] >= 0.0f && state.sky_zenith_tint[0] <= 1.0f &&
         state.sky_zenith_tint[1] >= 0.0f && state.sky_zenith_tint[1] <= 1.0f &&
         state.sky_zenith_tint[2] >= 0.0f && state.sky_zenith_tint[2] <= 1.0f &&
         state.sky_horizon_tint[0] >= 0.0f && state.sky_horizon_tint[0] <= 1.0f &&
         state.sky_horizon_tint[1] >= 0.0f && state.sky_horizon_tint[1] <= 1.0f &&
         state.sky_horizon_tint[2] >= 0.0f && state.sky_horizon_tint[2] <= 1.0f &&
         state.filter_tint[0] >= 0.0f && state.filter_tint[0] <= 1.0f &&
         state.filter_tint[1] >= 0.0f && state.filter_tint[1] <= 1.0f &&
         state.filter_tint[2] >= 0.0f && state.filter_tint[2] <= 1.0f;
}

}  // namespace rengine::lsg
