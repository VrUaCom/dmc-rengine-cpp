#include "rengine/lsg/eye_runtime.hpp"

#include <algorithm>
#include <cmath>

namespace rengine::lsg {

float pupil_target_from_luminance(float scene_luminance,
                                  float pupil_bias) noexcept {
  if (!std::isfinite(scene_luminance)) scene_luminance = 1.0f;
  if (!std::isfinite(pupil_bias)) pupil_bias = 0.5f;

  const float luminance = std::max(scene_luminance, 0.0001f);
  const float log_luminance = std::log2(luminance + 1.0f);
  const float light_response = std::clamp(log_luminance / 3.0f, 0.0f, 1.0f);
  const float bias = std::clamp(pupil_bias, 0.0f, 1.0f);

  const float dark_radius = 0.145f;
  const float bright_radius = 0.082f;
  const float biased = std::clamp(light_response + (0.5f - bias) * 0.22f, 0.0f, 1.0f);
  return dark_radius + (bright_radius - dark_radius) * biased;
}

void update_eye_runtime(EyeRuntimeState& state,
                        float scene_luminance,
                        float pupil_bias,
                        float delta_seconds) noexcept {
  if (!std::isfinite(delta_seconds) || delta_seconds < 0.0f) delta_seconds = 0.0f;
  delta_seconds = std::min(delta_seconds, 0.25f);

  state.scene_luminance = std::max(std::isfinite(scene_luminance) ? scene_luminance : 1.0f, 0.0f);
  state.target_pupil_radius = pupil_target_from_luminance(state.scene_luminance, pupil_bias);

  constexpr float response_rate = 3.2f;
  const float alpha = 1.0f - std::exp(-response_rate * delta_seconds);
  state.pupil_radius += (state.target_pupil_radius - state.pupil_radius) * alpha;
  state.pupil_radius = std::clamp(state.pupil_radius, 0.070f, 0.155f);
}

}  // namespace rengine::lsg
