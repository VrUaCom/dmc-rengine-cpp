#include "rengine/lsg/derived_eye.hpp"
#include "rengine/lsg/deterministic_hash.hpp"

namespace rengine::lsg {
namespace {
constexpr float unorm8(std::uint8_t value) noexcept {
  return static_cast<float>(value) * (1.0f / 255.0f);
}
}

DerivedEyeParameters derive_eye_parameters(const CharacterGenomeV0& genome) noexcept {
  DerivedEyeParameters out{};
  out.iris_primary = {
      unorm8(genome.eyes.iris_r),
      unorm8(genome.eyes.iris_g),
      unorm8(genome.eyes.iris_b)};
  out.iris_secondary = {
      unorm8(genome.eyes.iris2_r),
      unorm8(genome.eyes.iris2_g),
      unorm8(genome.eyes.iris2_b)};
  out.pupil_bias = unorm8(genome.eyes.pupil_bias);
  out.sclera_tint = unorm8(genome.eyes.sclera_tint);
  out.vascularity = unorm8(genome.eyes.vascularity);
  out.eye_seed_low = fold_seed64(genome.eye_seed);
  return out;
}

float sample_iris_variation(std::uint32_t eye_seed_low,
                            std::uint32_t eye_side,
                            std::int32_t angular_sector,
                            std::int32_t radial_band) noexcept {
  const std::uint32_t side_key = eye_side == 0u ? 0xA511E9B3u : 0x63D83595u;
  const auto h = hash5_32(eye_seed_low ^ side_key,
                          0x455945u,
                          angular_sector,
                          radial_band,
                          static_cast<std::int32_t>(eye_side));
  return hash01(h);
}

}  // namespace rengine::lsg
