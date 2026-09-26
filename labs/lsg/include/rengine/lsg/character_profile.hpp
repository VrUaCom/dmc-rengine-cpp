#pragma once
#include <cstdint>

namespace rengine::lsg {

// Carrier-local basis required by the universal FaceField.
// Runtime carrier/profile identity lives in CharacterRegistry; this type is only metadata.
struct CarrierFaceFieldMetadataV0 {
  std::uint32_t version{1};
  float head_pivot_y{0.690f};
  float face_half_width_m{0.100f};
  float face_half_height_m{0.180f};
  float face_depth_m{0.120f};
  float eye_center_x_abs_m{0.032f};
  float eye_center_y_m{0.752f};
};

} // namespace rengine::lsg
