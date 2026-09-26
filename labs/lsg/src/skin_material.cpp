#include "rengine/lsg/skin_material.hpp"
#include "rengine/lsg/skin_contract.inc"

#include <algorithm>
#include <cmath>

namespace rengine::lsg {
namespace {

float byte01(std::uint8_t value) noexcept {
  return static_cast<float>(value) / 255.0f;
}

float lerp(float a, float b, float t) noexcept {
  return a + (b - a) * t;
}

} // namespace

SkinMaterialGpuV0 pack_skin_material_gpu(const SkinPhenotype& skin) noexcept {
  SkinMaterialGpuV0 gpu{};
  gpu.pigments[0] = skin.melanin;
  gpu.pigments[1] = skin.haemoglobin;
  gpu.pigments[2] = skin.carotene;
  gpu.pigments[3] = skin.age_profile;

  gpu.surface[0] = skin.oiliness;
  gpu.surface[1] = skin.hydration;
  gpu.surface[2] = skin.roughness_bias;
  gpu.surface[3] = skin.coat_strength;

  gpu.pores[0] = skin.pore_density;
  gpu.pores[1] = skin.pore_scale;
  gpu.pores[2] = skin.pore_depth;
  gpu.pores[3] = skin.follicle_density;

  gpu.features[0] = skin.freckle_density;
  gpu.features[1] = skin.meso_strength;
  gpu.features[2] = skin.micro_strength;
  gpu.features[3] = skin.wrinkle_bias;

  gpu.physiology[0] = skin.perfusion;
  gpu.physiology[1] = skin.sweat;
  gpu.physiology[2] = skin.temperature_norm;
  gpu.physiology[3] = skin.subsurface_strength;
  return gpu;
}

SkinPhenotype derive_skin_phenotype(const CharacterGenomeV0& genome,
                                    const PhysiologyState& physiology) noexcept {
  SkinPhenotype out{};
  out.melanin = byte01(genome.skin.melanin);
  const float genome_haemoglobin = byte01(genome.skin.haemoglobin);
  out.carotene = byte01(genome.skin.carotene);
  out.oiliness = byte01(genome.skin.oiliness);
  out.hydration = byte01(genome.skin.hydration);
  out.roughness_bias = byte01(genome.skin.roughness_bias);
  out.pore_density = byte01(genome.skin.pore_density);
  out.pore_scale = byte01(genome.skin.pore_scale);
  out.pore_depth = byte01(genome.skin.pore_depth);
  out.follicle_density = byte01(genome.skin.follicle_density);
  out.freckle_density = byte01(genome.skin.freckle_density);
  out.age_profile = byte01(genome.skin.age_profile);
  out.meso_strength = byte01(genome.micro.meso_strength);
  out.micro_strength = byte01(genome.micro.micro_strength);
  out.wrinkle_bias = byte01(genome.micro.wrinkle_bias);

  const float genome_perfusion = byte01(genome.physiology.perfusion);
  const float genome_sweat = byte01(genome.physiology.sweat_bias);
  const float genome_temperature = byte01(genome.physiology.temperature_bias);

  out.perfusion = std::clamp(physiology.perfusion * RENGINE_SKIN_PHYS_RUNTIME_WEIGHT + genome_perfusion * RENGINE_SKIN_PHYS_GENOME_WEIGHT,
                             0.0f, 1.0f);
  out.sweat = std::clamp(
      physiology.sweat * lerp(RENGINE_SKIN_SWEAT_MIN_SCALE, RENGINE_SKIN_SWEAT_MAX_SCALE, genome_sweat), 0.0f, 1.0f);
  out.temperature_norm = std::clamp(
      (physiology.temperature - RENGINE_SKIN_TEMP_BASE_C) / RENGINE_SKIN_TEMP_RANGE_C + (genome_temperature - 0.5f) * RENGINE_SKIN_TEMP_GENOME_BIAS,
      0.0f, 1.0f);
  out.fatigue = std::clamp(physiology.fatigue, 0.0f, 1.0f);

  out.haemoglobin = std::clamp(
      genome_haemoglobin + (out.perfusion - RENGINE_SKIN_HAEM_BASELINE) * RENGINE_SKIN_HAEM_PERFUSION_GAIN,
      0.0f, 1.0f);
  out.coat_strength = std::clamp(
      RENGINE_SKIN_COAT_BASE + out.oiliness * RENGINE_SKIN_COAT_OIL_GAIN + out.sweat * RENGINE_SKIN_COAT_SWEAT_GAIN + out.hydration * RENGINE_SKIN_COAT_HYDRATION_GAIN,
      RENGINE_SKIN_COAT_MIN, RENGINE_SKIN_COAT_MAX);
  out.subsurface_strength = std::clamp(
      RENGINE_SKIN_SSS_BASE + out.hydration * RENGINE_SKIN_SSS_HYDRATION_GAIN +
          (1.0f - out.melanin) * RENGINE_SKIN_SSS_LOW_MELANIN_GAIN +
          out.haemoglobin * RENGINE_SKIN_SSS_HAEM_GAIN,
      RENGINE_SKIN_SSS_MIN, RENGINE_SKIN_SSS_MAX);
  return out;
}

SkinRgb skin_base_reflectance(const SkinPhenotype& skin) noexcept {
  const float melanin_mix = std::pow(std::clamp(skin.melanin, 0.0f, 1.0f), RENGINE_SKIN_MELANIN_EXPONENT) * RENGINE_SKIN_MELANIN_MIX_SCALE;
  SkinRgb out{
      lerp(RENGINE_SKIN_LIGHT_R, RENGINE_SKIN_DARK_R, melanin_mix),
      lerp(RENGINE_SKIN_LIGHT_G, RENGINE_SKIN_DARK_G, melanin_mix),
      lerp(RENGINE_SKIN_LIGHT_B, RENGINE_SKIN_DARK_B, melanin_mix),
  };

  const float blood = skin.haemoglobin - RENGINE_SKIN_HAEM_BASELINE;
  out.r += RENGINE_SKIN_BLOOD_R * blood;
  out.g += RENGINE_SKIN_BLOOD_G * blood;
  out.b += RENGINE_SKIN_BLOOD_B * blood;

  const float carotene = skin.carotene - RENGINE_SKIN_CAROTENE_BASELINE;
  out.r += RENGINE_SKIN_CAROTENE_R * carotene;
  out.g += RENGINE_SKIN_CAROTENE_G * carotene;
  out.b -= (-RENGINE_SKIN_CAROTENE_B) * carotene;

  const float temperature = skin.temperature_norm - RENGINE_SKIN_TEMP_BASELINE;
  out.r += RENGINE_SKIN_TEMP_R * temperature;
  out.g += RENGINE_SKIN_TEMP_G * temperature;
  out.b -= (-RENGINE_SKIN_TEMP_B) * temperature;

  out.r = std::clamp(out.r, RENGINE_SKIN_CLAMP_MIN_R, RENGINE_SKIN_CLAMP_MAX_R);
  out.g = std::clamp(out.g, RENGINE_SKIN_CLAMP_MIN_G, RENGINE_SKIN_CLAMP_MAX_G);
  out.b = std::clamp(out.b, RENGINE_SKIN_CLAMP_MIN_B, RENGINE_SKIN_CLAMP_MAX_B);
  return out;
}

float skin_base_roughness(const SkinPhenotype& skin) noexcept {
  return std::clamp(
      RENGINE_SKIN_ROUGH_BASE + (skin.roughness_bias - 0.5f) * RENGINE_SKIN_ROUGH_BIAS_GAIN -
          skin.oiliness * RENGINE_SKIN_ROUGH_OIL_GAIN - skin.hydration * RENGINE_SKIN_ROUGH_HYDRATION_GAIN -
          skin.sweat * RENGINE_SKIN_ROUGH_SWEAT_GAIN + skin.age_profile * RENGINE_SKIN_ROUGH_AGE_GAIN,
      RENGINE_SKIN_ROUGH_MIN, RENGINE_SKIN_ROUGH_MAX);
}

} // namespace rengine::lsg
