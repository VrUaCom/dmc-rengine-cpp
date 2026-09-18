#pragma once
#include "rengine/lsg/genome.hpp"
#include <array>
#include <cstdint>

namespace rengine::lsg {

struct DerivedEyeParameters {
  std::array<float, 3> iris_primary{};
  std::array<float, 3> iris_secondary{};
  float pupil_bias{};
  float sclera_tint{};
  float vascularity{};
  std::uint32_t eye_seed_low{};
};

[[nodiscard]] DerivedEyeParameters derive_eye_parameters(const CharacterGenomeV0& genome) noexcept;

[[nodiscard]] float sample_iris_variation(std::uint32_t eye_seed_low,
                                          std::uint32_t eye_side,
                                          std::int32_t angular_sector,
                                          std::int32_t radial_band) noexcept;

}  // namespace rengine::lsg
