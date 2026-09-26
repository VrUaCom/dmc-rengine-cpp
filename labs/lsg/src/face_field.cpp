#include "rengine/lsg/face_field.hpp"
#include "../include/rengine/lsg/face_field_contract.inc"

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
  return smooth01(std::clamp(
      head_weight / static_cast<float>(RENGINE_FACE_ACTIVATION_FULL_WEIGHT), 0.0f, 1.0f));
}

FacePoint canonical_face_point(FacePoint raw,
                               const CarrierFaceFieldMetadataV0& metadata) noexcept {
  return {
      raw.x / metadata.face_half_width_m,
      (raw.y - metadata.head_pivot_y) / metadata.face_half_height_m,
      raw.z / metadata.face_depth_m,
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

FaceFieldWeights sample_face_field_weights(
    FacePoint raw_point, const CarrierFaceFieldMetadataV0& metadata) noexcept {
  const FacePoint q = canonical_face_point(raw_point, metadata);
  FaceFieldWeights weights{};
  weights.front = smooth_range(q.z, RENGINE_FACE_FRONT_LOW, RENGINE_FACE_FRONT_HIGH);
  weights.center = 1.0f - smooth_range(std::abs(q.x), RENGINE_FACE_CENTER_LOW, RENGINE_FACE_CENTER_HIGH);
  weights.cheek = smooth_band(q.y, RENGINE_FACE_CHEEK_R0, RENGINE_FACE_CHEEK_R1, RENGINE_FACE_CHEEK_F0, RENGINE_FACE_CHEEK_F1) * weights.front;
  weights.jaw = smooth_band(q.y, RENGINE_FACE_JAW_R0, RENGINE_FACE_JAW_R1, RENGINE_FACE_JAW_F0, RENGINE_FACE_JAW_F1) * weights.front;
  weights.chin = smooth_band(q.y, RENGINE_FACE_CHIN_R0, RENGINE_FACE_CHIN_R1, RENGINE_FACE_CHIN_F0, RENGINE_FACE_CHIN_F1) * weights.front;
  weights.eyes = smooth_band(q.y, RENGINE_FACE_EYES_R0, RENGINE_FACE_EYES_R1, RENGINE_FACE_EYES_F0, RENGINE_FACE_EYES_F1) * weights.front;
  weights.nose = smooth_band(q.y, RENGINE_FACE_NOSE_R0, RENGINE_FACE_NOSE_R1, RENGINE_FACE_NOSE_F0, RENGINE_FACE_NOSE_F1) * weights.front * weights.center;
  weights.brow = smooth_band(q.y, RENGINE_FACE_BROW_R0, RENGINE_FACE_BROW_R1, RENGINE_FACE_BROW_F0, RENGINE_FACE_BROW_F1) * weights.front;
  weights.forehead = smooth_band(q.y, RENGINE_FACE_FOREHEAD_R0, RENGINE_FACE_FOREHEAD_R1, RENGINE_FACE_FOREHEAD_F0, RENGINE_FACE_FOREHEAD_F1) * weights.front;
  weights.mouth = smooth_band(q.y, RENGINE_FACE_MOUTH_R0, RENGINE_FACE_MOUTH_R1, RENGINE_FACE_MOUTH_F0, RENGINE_FACE_MOUTH_F1) * weights.front;
  weights.upper_lip = smooth_band(q.y, RENGINE_FACE_UPPER_LIP_R0, RENGINE_FACE_UPPER_LIP_R1, RENGINE_FACE_UPPER_LIP_F0, RENGINE_FACE_UPPER_LIP_F1) * weights.front * weights.center;
  weights.lower_lip = smooth_band(q.y, RENGINE_FACE_LOWER_LIP_R0, RENGINE_FACE_LOWER_LIP_R1, RENGINE_FACE_LOWER_LIP_F0, RENGINE_FACE_LOWER_LIP_F1) * weights.front * weights.center;
  return weights;
}

FacePoint deform_face_field(FacePoint p, FacePoint raw, float head_weight,
                            const FaceFieldParameters& f,
                            const CarrierFaceFieldMetadataV0& metadata) noexcept {
  const float activation = face_activation(head_weight);
  if (activation <= 0.0f) return p;
  const FaceFieldWeights w = sample_face_field_weights(raw, metadata);

  p.x *= 1.0f + RENGINE_FACE_SKULL_WIDTH_COEFF * f.skull_width * activation;
  p.y = metadata.head_pivot_y +
        (p.y - metadata.head_pivot_y) * (1.0f + RENGINE_FACE_SKULL_HEIGHT_COEFF * f.skull_height * activation);

  p.y += RENGINE_FACE_LENGTH_COEFF * f.face_length * (w.jaw + w.chin) * activation;
  p.y += RENGINE_FACE_FOREHEAD_HEIGHT_COEFF * f.forehead_height * w.forehead * activation;
  p.z += RENGINE_FACE_BROW_DEPTH_COEFF * f.brow_depth * w.brow * activation;
  p.x += sign_symmetric(raw.x) * RENGINE_FACE_EYE_SPACING_COEFF * f.eye_spacing * w.eyes * activation;
  p.x *= 1.0f + RENGINE_FACE_EYE_SIZE_SURFACE_COEFF * f.eye_size * w.eyes * activation;
  p.y += sign_symmetric(raw.x) * RENGINE_FACE_EYE_TILT_COEFF * f.eye_tilt * w.eyes * activation;

  p.y -= RENGINE_FACE_NOSE_LENGTH_COEFF * f.nose_length * w.nose * activation;
  p.x *= 1.0f + RENGINE_FACE_NOSE_WIDTH_COEFF * f.nose_width * w.nose * activation;
  p.z += RENGINE_FACE_NOSE_PROJECTION_COEFF * f.nose_projection * w.nose * activation;
  p.x *= 1.0f + RENGINE_FACE_CHEEKBONE_WIDTH_COEFF * f.cheekbone_width * w.cheek * activation;
  p.z += RENGINE_FACE_CHEEK_FULLNESS_COEFF * f.cheek_fullness * w.cheek * activation;
  p.x *= 1.0f + RENGINE_FACE_JAW_WIDTH_COEFF * f.jaw_width * w.jaw * activation;
  p.x *= 1.0f + RENGINE_FACE_CHIN_WIDTH_COEFF * f.chin_width * w.chin * activation;
  p.z += RENGINE_FACE_CHIN_PROJECTION_COEFF * f.chin_projection * w.chin * activation;
  p.x *= 1.0f + RENGINE_FACE_MOUTH_WIDTH_COEFF * f.mouth_width * w.mouth * activation;
  p.z += RENGINE_FACE_UPPER_LIP_COEFF * f.upper_lip_fullness * w.upper_lip * activation;
  p.z += RENGINE_FACE_LOWER_LIP_COEFF * f.lower_lip_fullness * w.lower_lip * activation;
  p.z += RENGINE_FACE_LIP_PROJECTION_COEFF * f.lip_projection * w.mouth * w.center * activation;
  return p;
}

FacePoint deform_eye_socket_field(FacePoint p, FacePoint raw, float head_weight,
                                  const FaceFieldParameters& f,
                                  const CarrierFaceFieldMetadataV0& metadata) noexcept {
  const float activation = face_activation(head_weight);
  if (activation <= 0.0f) return p;

  p.x *= 1.0f + RENGINE_FACE_SKULL_WIDTH_COEFF * f.skull_width * activation;
  p.y = metadata.head_pivot_y +
        (p.y - metadata.head_pivot_y) * (1.0f + RENGINE_FACE_SKULL_HEIGHT_COEFF * f.skull_height * activation);

  const float side = side_nonzero(raw.x);
  p.x += side * RENGINE_FACE_EYE_SPACING_COEFF * f.eye_spacing * activation;
  const float eye_center_x = side * metadata.eye_center_x_abs_m;
  const float eye_center_y = metadata.eye_center_y_m;
  p.x = eye_center_x + (p.x - eye_center_x) * (1.0f + RENGINE_FACE_EYE_SIZE_X_COEFF * f.eye_size * activation);
  p.y = eye_center_y + (p.y - eye_center_y) * (1.0f + RENGINE_FACE_EYE_SIZE_Y_COEFF * f.eye_size * activation);
  p.y += side * RENGINE_FACE_EYE_TILT_COEFF * f.eye_tilt * activation;
  return p;
}

} // namespace rengine::lsg
