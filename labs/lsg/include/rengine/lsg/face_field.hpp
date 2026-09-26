#pragma once

#include "genome.hpp"

#include <cstdint>

namespace rengine::lsg {

struct FacePoint {
  float x{};
  float y{};
  float z{};
};

struct FaceFieldParameters {
  float skull_width{};
  float skull_height{};
  float face_length{};
  float forehead_height{};
  float brow_depth{};
  float eye_spacing{};
  float eye_size{};
  float eye_tilt{};
  float nose_length{};
  float nose_width{};
  float nose_projection{};
  float cheekbone_width{};
  float cheek_fullness{};
  float jaw_width{};
  float chin_width{};
  float chin_projection{};
  float mouth_width{};
  float upper_lip_fullness{};
  float lower_lip_fullness{};
  float lip_projection{};
};

struct FaceFieldWeights {
  float front{};
  float center{};
  float cheek{};
  float jaw{};
  float chin{};
  float eyes{};
  float nose{};
  float brow{};
  float forehead{};
  float mouth{};
  float upper_lip{};
  float lower_lip{};
};

[[nodiscard]] FaceFieldParameters derive_face_field_parameters(const FaceGenomeV0& face) noexcept;
[[nodiscard]] FaceFieldWeights sample_face_field_weights(
    FacePoint raw_point, const CarrierFaceFieldMetadataV0& metadata) noexcept;
[[nodiscard]] FacePoint deform_face_field(
    FacePoint shaped_point, FacePoint raw_point, float head_weight,
    const FaceFieldParameters& parameters,
    const CarrierFaceFieldMetadataV0& metadata) noexcept;
[[nodiscard]] FacePoint deform_eye_socket_field(
    FacePoint shaped_point, FacePoint raw_point, float head_weight,
    const FaceFieldParameters& parameters,
    const CarrierFaceFieldMetadataV0& metadata) noexcept;

} // namespace rengine::lsg
