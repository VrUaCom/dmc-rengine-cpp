#include "rengine/lsg/face_field.hpp"

#include <algorithm>
#include <cmath>

namespace rengine::lsg {
namespace {

float smooth01(float value) noexcept {
  value = std::clamp(value, 0.0f, 1.0f);
  return value * value * (3.0f - 2.0f * value);
}

float smooth_range(float value, float low, float high) noexcept {
  return smooth01((value - low) / std::max(high - low, 1.0e-6f));
}

float smooth_band(float value, float rise0, float rise1, float fall0, float fall1) noexcept {
  return smooth_range(value, rise0, rise1) * (1.0f - smooth_range(value, fall0, fall1));
}

float sign_symmetric(float value) noexcept {
  if (value < 0.0f) return -1.0f;
  if (value > 0.0f) return 1.0f;
  return 0.0f;
}

float side_nonzero(float value) noexcept {
  return value < 0.0f ? -1.0f : 1.0f;
}

float face_activation(float head_weight) noexcept {
  return smooth01(std::clamp(head_weight / 0.25f, 0.0f, 1.0f));
}

FacePoint canonical_face_point(FacePoint raw) noexcept {
  return {
      raw.x / kCanonicalFaceHalfWidthM,
      (raw.y - kCanonicalHeadPivotY) / kCanonicalFaceHalfHeightM,
      raw.z / kCanonicalFaceDepthM,
  };
}

} // namespace

FaceFieldParameters derive_face_field_parameters(const FaceGenomeV0& face) noexcept {
  return {
      decode_snorm16(face.skull_width), decode_snorm16(face.skull_height),
      decode_snorm16(face.face_length), decode_snorm16(face.forehead_height),
      decode_snorm16(face.brow_depth), decode_snorm16(face.eye_spacing),
      decode_snorm16(face.eye_size), decode_snorm16(face.eye_tilt),
      decode_snorm16(face.nose_length), decode_snorm16(face.nose_width),
      decode_snorm16(face.nose_projection), decode_snorm16(face.cheekbone_width),
      decode_snorm16(face.cheek_fullness), decode_snorm16(face.jaw_width),
      decode_snorm16(face.chin_width), decode_snorm16(face.chin_projection),
      decode_snorm16(face.mouth_width), decode_snorm16(face.upper_lip_fullness),
      decode_snorm16(face.lower_lip_fullness), decode_snorm16(face.lip_projection),
  };
}

FaceFieldWeights sample_face_field_weights(FacePoint raw_point) noexcept {
  const FacePoint q = canonical_face_point(raw_point);
  FaceFieldWeights weights{};
  weights.front = smooth_range(q.z, 0.125f, 0.750f);
  weights.center = 1.0f - smooth_range(std::abs(q.x), 0.20f, 0.75f);
  weights.cheek = smooth_band(q.y, -0.306f, -0.111f, 0.250f, 0.472f) * weights.front;
  weights.jaw = smooth_band(q.y, -0.750f, -0.556f, -0.139f, 0.111f) * weights.front;
  weights.chin = smooth_band(q.y, -0.861f, -0.722f, -0.444f, -0.250f) * weights.front;
  weights.eyes = smooth_band(q.y, 0.083f, 0.222f, 0.500f, 0.667f) * weights.front;
  weights.nose = smooth_band(q.y, -0.361f, -0.167f, 0.472f, 0.639f) * weights.front * weights.center;
  weights.brow = smooth_band(q.y, 0.333f, 0.444f, 0.611f, 0.750f) * weights.front;
  weights.forehead = smooth_band(q.y, 0.472f, 0.611f, 0.889f, 1.028f) * weights.front;
  weights.mouth = smooth_band(q.y, -0.472f, -0.333f, 0.000f, 0.139f) * weights.front;
  weights.upper_lip = smooth_band(q.y, -0.267f, -0.211f, -0.122f, -0.056f) * weights.front * weights.center;
  weights.lower_lip = smooth_band(q.y, -0.378f, -0.300f, -0.206f, -0.139f) * weights.front * weights.center;
  return weights;
}

FacePoint deform_face_field(FacePoint p, FacePoint raw, float head_weight,
                            const FaceFieldParameters& f) noexcept {
  const float activation = face_activation(head_weight);
  if (activation <= 0.0f) return p;
  const FaceFieldWeights w = sample_face_field_weights(raw);

  p.x *= 1.0f + 0.060f * f.skull_width * activation;
  p.y = kCanonicalHeadPivotY +
        (p.y - kCanonicalHeadPivotY) * (1.0f + 0.050f * f.skull_height * activation);

  p.y += 0.014f * f.face_length * (w.jaw + w.chin) * activation;
  p.y += 0.010f * f.forehead_height * w.forehead * activation;
  p.z += 0.010f * f.brow_depth * w.brow * activation;
  p.x += sign_symmetric(raw.x) * 0.008f * f.eye_spacing * w.eyes * activation;
  p.x *= 1.0f + 0.030f * f.eye_size * w.eyes * activation;
  p.y += sign_symmetric(raw.x) * 0.006f * f.eye_tilt * w.eyes * activation;

  p.y -= 0.009f * f.nose_length * w.nose * activation;
  p.x *= 1.0f + 0.120f * f.nose_width * w.nose * activation;
  p.z += 0.020f * f.nose_projection * w.nose * activation;
  p.x *= 1.0f + 0.080f * f.cheekbone_width * w.cheek * activation;
  p.z += 0.012f * f.cheek_fullness * w.cheek * activation;
  p.x *= 1.0f + 0.085f * f.jaw_width * w.jaw * activation;
  p.x *= 1.0f + 0.100f * f.chin_width * w.chin * activation;
  p.z += 0.014f * f.chin_projection * w.chin * activation;
  p.x *= 1.0f + 0.080f * f.mouth_width * w.mouth * activation;
  p.z += 0.009f * f.upper_lip_fullness * w.upper_lip * activation;
  p.z += 0.010f * f.lower_lip_fullness * w.lower_lip * activation;
  p.z += 0.008f * f.lip_projection * w.mouth * w.center * activation;
  return p;
}

FacePoint deform_eye_socket_field(FacePoint p, FacePoint raw, float head_weight,
                                  const FaceFieldParameters& f) noexcept {
  const float activation = face_activation(head_weight);
  if (activation <= 0.0f) return p;

  p.x *= 1.0f + 0.060f * f.skull_width * activation;
  p.y = kCanonicalHeadPivotY +
        (p.y - kCanonicalHeadPivotY) * (1.0f + 0.050f * f.skull_height * activation);

  const float side = side_nonzero(raw.x);
  p.x += side * 0.008f * f.eye_spacing * activation;
  const float eye_center_x = side * 0.032f;
  constexpr float eye_center_y = 0.752f;
  p.x = eye_center_x + (p.x - eye_center_x) * (1.0f + 0.070f * f.eye_size * activation);
  p.y = eye_center_y + (p.y - eye_center_y) * (1.0f + 0.055f * f.eye_size * activation);
  p.y += side * 0.006f * f.eye_tilt * activation;
  return p;
}

} // namespace rengine::lsg
