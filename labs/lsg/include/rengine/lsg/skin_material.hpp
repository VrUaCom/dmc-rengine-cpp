#pragma once

#include "genome.hpp"
#include "physiology.hpp"

namespace rengine::lsg {

struct SkinRgb {
  float r{}, g{}, b{};
};

struct alignas(16) SkinMaterialGpuV0 {
  float pigments[4]{};
  float surface[4]{};
  float pores[4]{};
  float features[4]{};
  float physiology[4]{};
};
static_assert(sizeof(SkinMaterialGpuV0) == 80);

struct SkinPhenotype {
  float melanin{};
  float haemoglobin{};
  float carotene{};
  float oiliness{};
  float hydration{};
  float roughness_bias{};
  float pore_density{};
  float pore_scale{};
  float pore_depth{};
  float follicle_density{};
  float freckle_density{};
  float age_profile{};
  float meso_strength{};
  float micro_strength{};
  float wrinkle_bias{};

  float perfusion{};
  float sweat{};
  float temperature_norm{};
  float fatigue{};

  float coat_strength{};
  float subsurface_strength{};
};

[[nodiscard]] SkinMaterialGpuV0 pack_skin_material_gpu(const SkinPhenotype& skin) noexcept;
[[nodiscard]] SkinPhenotype derive_skin_phenotype(const CharacterGenomeV0& genome,
                                                  const PhysiologyState& physiology) noexcept;
[[nodiscard]] SkinRgb skin_base_reflectance(const SkinPhenotype& skin) noexcept;
[[nodiscard]] float skin_base_roughness(const SkinPhenotype& skin) noexcept;

} // namespace rengine::lsg
