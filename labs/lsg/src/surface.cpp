#include "rengine/lsg/surface.hpp"
#include "rengine/lsg/deterministic_hash.hpp"
#include <algorithm>
#include <cmath>
namespace rengine::lsg {
namespace {
float region_pore_multiplier(BodyRegion r) noexcept {
  switch(r){case BodyRegion::head:return 1.35f;case BodyRegion::neck:return 1.05f;case BodyRegion::chest:return 0.85f;case BodyRegion::back:return 0.9f;case BodyRegion::hand:return 0.55f;default:return 0.75f;}
}
}
SurfaceSample sample_surface(const CharacterGenomeV0& g, BodyRegion region, Vec3 p, float mm, const PhysiologyState& ps) noexcept {
  const DetailBand band = select_detail_band(mm);
  const float cell_m = std::max(0.00008f, 0.00018f + static_cast<float>(g.skin.pore_scale) / 255.0f * 0.00022f);
  const auto qx = static_cast<std::int32_t>(std::floor(p.x / cell_m));
  const auto qy = static_cast<std::int32_t>(std::floor(p.y / cell_m));
  const auto qz = static_cast<std::int32_t>(std::floor(p.z / cell_m));
  const float rnd = hash01(hash5(g.surface_seed, static_cast<std::uint32_t>(region), qx, qy, qz));
  const float density = static_cast<float>(g.skin.pore_density) / 255.0f * region_pore_multiplier(region);
  const bool pore_visible = band >= DetailBand::micro && rnd < std::min(0.96f, density);
  const float pore_depth = static_cast<float>(g.skin.pore_depth) / 255.0f;
  const float meso = band >= DetailBand::meso ? (hash01(pcg_hash(hash5(g.surface_seed ^ 0xA511E9B3u, static_cast<std::uint32_t>(region), qx>>4, qy>>4, qz>>4))) - 0.5f) : 0.0f;
  const float oil = static_cast<float>(g.skin.oiliness) / 255.0f;
  const float rough_bias = static_cast<float>(g.skin.roughness_bias) / 255.0f;
  SurfaceSample s{};
  s.pore_height = pore_visible ? -pore_depth * (0.35f + 0.65f * rnd) : 0.0f;
  s.meso_variation = meso * (static_cast<float>(g.micro.meso_strength) / 255.0f);
  s.roughness = std::clamp(0.48f + (rough_bias - 0.5f) * 0.32f - oil * 0.12f - ps.sweat * 0.16f + s.meso_variation * 0.05f, 0.18f, 0.88f);
  s.redness = std::clamp((static_cast<float>(g.skin.haemoglobin) / 255.0f) * 0.35f + ps.perfusion * 0.35f + s.meso_variation * 0.02f, 0.0f, 1.0f);
  s.specular_scale = 0.82f + oil * 0.18f + ps.sweat * 0.45f;
  return s;
}
}
