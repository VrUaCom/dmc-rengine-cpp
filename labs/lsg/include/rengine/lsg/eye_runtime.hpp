#pragma once
#include <cstdint>

namespace rengine::lsg {

struct EyeRuntimeState {
  float scene_luminance{1.0f};
  float target_pupil_radius{0.105f};
  float pupil_radius{0.105f};
};

[[nodiscard]] float pupil_target_from_luminance(float scene_luminance,
                                                 float pupil_bias) noexcept;

void update_eye_runtime(EyeRuntimeState& state,
                        float scene_luminance,
                        float pupil_bias,
                        float delta_seconds) noexcept;

}  // namespace rengine::lsg
