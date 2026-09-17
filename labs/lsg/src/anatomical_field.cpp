#include "rengine/lsg/anatomical_field.hpp"

#include <algorithm>

namespace rengine::lsg {
namespace {

float smooth01(float value) noexcept {
  const float x = std::clamp(value, 0.0f, 1.0f);
  return x * x * (3.0f - 2.0f * x);
}

float smooth_range(float value, float lo, float hi) noexcept {
  if (!(hi > lo)) return value >= hi ? 1.0f : 0.0f;
  return smooth01((value - lo) / (hi - lo));
}

float smooth_band(float value, float rise0, float rise1, float fall0, float fall1) noexcept {
  return smooth_range(value, rise0, rise1) * (1.0f - smooth_range(value, fall0, fall1));
}

} // namespace

AnatomicalWeights sample_anatomical_weights(float body_y01) noexcept {
  const float y = std::clamp(body_y01, 0.0f, 1.0f);
  AnatomicalWeights out{};
  out.shoulder = smooth_band(y, 0.55f, 0.64f, 0.78f, 0.87f);
  out.chest = smooth_band(y, 0.50f, 0.58f, 0.70f, 0.79f);
  out.waist = smooth_band(y, 0.39f, 0.46f, 0.55f, 0.63f);
  out.pelvis = smooth_band(y, 0.29f, 0.36f, 0.47f, 0.55f);
  out.head = smooth_range(y, 0.78f, 0.89f);
  return out;
}

AnatomicalPoint deform_anatomy_rest(
    AnatomicalPoint centered_position_m,
    const DerivedCharacterParameters& p) noexcept {
  const float body_y01 = centered_position_m.y / 1.75f + 0.5f;
  const auto w = sample_anatomical_weights(body_y01);

  const float fat_delta = p.body_fat_scale - 1.0f;
  const float muscle_delta = p.muscle_scale - 1.0f;
  float width_scale = 1.0f + 0.55f * fat_delta;
  float depth_scale = 1.0f + 0.70f * fat_delta;

  width_scale += w.shoulder * ((p.shoulder_scale - 1.0f) + 0.55f * muscle_delta);
  width_scale += w.waist * (p.waist_scale - 1.0f);
  width_scale += w.pelvis * (p.pelvis_scale - 1.0f);
  width_scale += w.head * (p.head_scale - 1.0f);

  depth_scale += w.chest * ((p.chest_depth_scale - 1.0f) + 0.35f * muscle_delta);
  depth_scale += w.waist * 0.50f * (p.waist_scale - 1.0f);
  depth_scale += w.pelvis * 0.50f * (p.pelvis_scale - 1.0f);
  depth_scale += w.head * (p.head_scale - 1.0f);

  width_scale = std::clamp(width_scale, 0.75f, 1.25f);
  depth_scale = std::clamp(depth_scale, 0.75f, 1.25f);

  return {
      centered_position_m.x * width_scale,
      centered_position_m.y * p.height_scale,
      centered_position_m.z * depth_scale};
}

} // namespace rengine::lsg
