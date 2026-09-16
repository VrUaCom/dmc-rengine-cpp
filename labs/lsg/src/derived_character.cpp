#include "rengine/lsg/derived_character.hpp"
#include "rengine/lsg/deterministic_hash.hpp"

#include <algorithm>

namespace rengine::lsg {
namespace {
float byte01(std::uint8_t value) noexcept { return static_cast<float>(value) / 255.0f; }
float bounded_scale(std::int16_t value, float amplitude) noexcept {
  return std::clamp(1.0f + decode_snorm16(value) * amplitude, 0.80f, 1.20f);
}
}

DerivedCharacterParameters derive_character_parameters(const CharacterGenomeV0& genome) noexcept {
  DerivedCharacterParameters out{};
  out.height_scale = bounded_scale(genome.geometry.height, 0.06f);
  out.shoulder_scale = bounded_scale(genome.geometry.shoulder_width, 0.10f);
  out.pelvis_scale = bounded_scale(genome.geometry.pelvis_width, 0.10f);
  out.chest_depth_scale = bounded_scale(genome.geometry.chest_volume, 0.08f);
  out.waist_scale = bounded_scale(genome.geometry.waist, 0.08f);
  out.muscle_scale = bounded_scale(genome.geometry.muscle, 0.06f);
  out.body_fat_scale = bounded_scale(genome.geometry.body_fat, 0.08f);
  out.head_scale = bounded_scale(genome.geometry.head_scale, 0.05f);

  out.melanin = byte01(genome.skin.melanin);
  out.haemoglobin = byte01(genome.skin.haemoglobin);
  out.oiliness = byte01(genome.skin.oiliness);
  out.hydration = byte01(genome.skin.hydration);
  out.roughness_bias = byte01(genome.skin.roughness_bias);
  out.pore_density = byte01(genome.skin.pore_density);
  out.pore_scale = byte01(genome.skin.pore_scale);
  out.pore_depth = byte01(genome.skin.pore_depth);
  out.meso_strength = byte01(genome.micro.meso_strength);
  // The Vulkan push constant budget is kept at the Vulkan 1.2 guaranteed 128-byte minimum.
  // Fold the 64-bit genome seed deterministically instead of silently discarding its high half.
  out.surface_seed_low = fold_seed64(genome.surface_seed);
  return out;
}

} // namespace rengine::lsg
