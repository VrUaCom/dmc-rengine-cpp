#pragma once

#include "character_profile.hpp"
#include "genome.hpp"
#include "physiology.hpp"
#include "skin_material.hpp"

#include <cstddef>

namespace rengine::lsg {

struct alignas(16) CharacterIdentityGpuV0 {
  float face0[4]{};
  float face1[4]{};
  float face2[4]{};
  float face3[4]{};
  float face4[4]{};
  float basis0[4]{}; // head pivot, half width, half height, depth
  float basis1[4]{}; // eye half spacing, eye Y, field version, reserved
};
static_assert(sizeof(CharacterIdentityGpuV0) == 112);

struct alignas(16) CharacterGpuStateV0 {
  CharacterIdentityGpuV0 identity{};
  SkinMaterialGpuV0 skin{};
};
static_assert(alignof(CharacterGpuStateV0) == 16);
static_assert(offsetof(CharacterGpuStateV0, identity) == 0);
static_assert(offsetof(CharacterGpuStateV0, skin) == 112);
static_assert(sizeof(CharacterGpuStateV0) == 192);

[[nodiscard]] CharacterGpuStateV0 pack_character_gpu_state(
    const CharacterGenomeV0& genome,
    const CarrierFaceFieldMetadataV0& metadata,
    const PhysiologyState& physiology) noexcept;

} // namespace rengine::lsg
