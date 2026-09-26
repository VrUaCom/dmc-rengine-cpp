#include "rengine/lsg/skin_material.hpp"

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

  out.perfusion = std::clamp(physiology.perfusion * 0.75f + genome_perfusion * 0.25f,
                             0.0f, 1.0f);
  out.sweat = std::clamp(
      physiology.sweat * lerp(0.70f, 1.30f, genome_sweat), 0.0f, 1.0f);
  out.temperature_norm = std::clamp(
      (physiology.temperature - 35.5f) / 2.2f + (genome_temperature - 0.5f) * 0.18f,
      0.0f, 1.0f);
  out.fatigue = std::clamp(physiology.fatigue, 0.0f, 1.0f);

  out.haemoglobin = std::clamp(
      genome_haemoglobin + (out.perfusion - 0.45f) * 0.36f,
      0.0f, 1.0f);
  out.coat_strength = std::clamp(
      0.10f + out.oiliness * 0.34f + out.sweat * 0.50f + out.hydration * 0.06f,
      0.05f, 1.0f);
  out.subsurface_strength = std::clamp(
      0.18f + out.hydration * 0.22f + (1.0f - out.melanin) * 0.18f +
          out.haemoglobin * 0.08f,
      0.12f, 0.66f);
  return out;
}

SkinRgb skin_base_reflectance(const SkinPhenotype& skin) noexcept {
  const float melanin_mix = std::pow(std::clamp(skin.melanin, 0.0f, 1.0f), 0.82f) * 0.90f;
  SkinRgb out{
      lerp(0.66f, 0.12f, melanin_mix),
      lerp(0.39f, 0.050f, melanin_mix),
      lerp(0.29f, 0.030f, melanin_mix),
  };

  const float blood = skin.haemoglobin - 0.45f;
  out.r += 0.080f * blood;
  out.g += 0.010f * blood;
  out.b += 0.005f * blood;

  const float carotene = skin.carotene - 0.35f;
  out.r += 0.040f * carotene;
  out.g += 0.028f * carotene;
  out.b -= 0.012f * carotene;

  const float temperature = skin.temperature_norm - 0.50f;
  out.r += 0.026f * temperature;
  out.g += 0.004f * temperature;
  out.b -= 0.018f * temperature;

  out.r = std::clamp(out.r, 0.015f, 0.95f);
  out.g = std::clamp(out.g, 0.010f, 0.90f);
  out.b = std::clamp(out.b, 0.008f, 0.85f);
  return out;
}

float skin_base_roughness(const SkinPhenotype& skin) noexcept {
  return std::clamp(
      0.62f + (skin.roughness_bias - 0.5f) * 0.26f -
          skin.oiliness * 0.16f - skin.hydration * 0.05f -
          skin.sweat * 0.10f + skin.age_profile * 0.035f,
      0.24f, 0.90f);
}

} // namespace rengine::lsg
