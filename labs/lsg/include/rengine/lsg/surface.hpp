#pragma once
#include "body_regions.hpp"
#include "detail_scheduler.hpp"
#include "genome.hpp"
#include "physiology.hpp"
namespace rengine::lsg {
struct Vec3 { float x{}, y{}, z{}; };
struct SurfaceSample {
  float roughness{};
  float pore_height{};
  float meso_variation{};
  float redness{};
  float specular_scale{};
};
[[nodiscard]] SurfaceSample sample_surface(const CharacterGenomeV0& genome, BodyRegion region,
                                           Vec3 rest_position_m, float mm_per_pixel,
                                           const PhysiologyState& physiology) noexcept;
}
