#pragma once

#include "derived_character.hpp"

namespace rengine::lsg {

struct AnatomicalPoint {
  float x{};
  float y{};
  float z{};
};

struct AnatomicalWeights {
  float shoulder{};
  float chest{};
  float waist{};
  float pelvis{};
  float head{};
};

[[nodiscard]] AnatomicalWeights sample_anatomical_weights(float body_y01) noexcept;
[[nodiscard]] AnatomicalPoint deform_anatomy_rest(
    AnatomicalPoint centered_position_m,
    const DerivedCharacterParameters& parameters) noexcept;

} // namespace rengine::lsg
