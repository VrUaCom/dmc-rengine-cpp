#pragma once
#include <cstdint>

namespace rengine::lsg {

inline constexpr float kPupilRadiusMin = 0.045f;
inline constexpr float kPupilRadiusMax = 0.135f;
inline constexpr float kPupilDarkRadius = 0.118f;
inline constexpr float kPupilBrightRadius = 0.055f;
inline constexpr float kPupilInitialRadius = 0.095f;

struct EyeRuntimeState {
  float scene_luminance{1.0f};
  float target_pupil_radius{kPupilInitialRadius};
  float pupil_radius{kPupilInitialRadius};
};

[[nodiscard]] float pupil_target_from_luminance(float scene_luminance,
                                                 float pupil_bias) noexcept;

void update_eye_runtime(EyeRuntimeState& state,
                        float scene_luminance,
                        float pupil_bias,
                        float delta_seconds) noexcept;

}  // namespace rengine::lsg
