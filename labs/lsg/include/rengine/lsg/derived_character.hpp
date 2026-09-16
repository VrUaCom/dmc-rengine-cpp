#pragma once

#include "genome.hpp"
#include <cstdint>

namespace rengine::lsg {

struct DerivedCharacterParameters {
  float height_scale{1.0f};
  float shoulder_scale{1.0f};
  float pelvis_scale{1.0f};
  float chest_depth_scale{1.0f};
  float waist_scale{1.0f};
  float muscle_scale{1.0f};
  float body_fat_scale{1.0f};
  float head_scale{1.0f};

  float melanin{};
  float haemoglobin{};
  float oiliness{};
  float hydration{};
  float roughness_bias{};
  float pore_density{};
  float pore_scale{};
  float pore_depth{};
  float meso_strength{};
  std::uint32_t surface_seed_low{};
};

[[nodiscard]] DerivedCharacterParameters derive_character_parameters(const CharacterGenomeV0& genome) noexcept;

} // namespace rengine::lsg
