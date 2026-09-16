#include "rengine/lsg/surface.hpp"
#include "rengine/lsg/deterministic_hash.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rengine::lsg {
namespace {

float region_pore_multiplier(BodyRegion region) noexcept {
  switch (region) {
    case BodyRegion::head: return 1.18f;
    case BodyRegion::hand: return 1.08f;
    case BodyRegion::neck:
    case BodyRegion::chest: return 0.98f;
    case BodyRegion::lower_leg:
    case BodyRegion::foot: return 0.78f;
    default: return 0.88f;
  }
}

float smooth01(float x) noexcept { return x * x * (3.0f - 2.0f * x); }
float lerp(float a, float b, float t) noexcept { return a + (b - a) * t; }

float value_noise(std::uint64_t seed, BodyRegion region, Vec3 p) noexcept {
  const auto x0 = static_cast<std::int32_t>(std::floor(p.x));
  const auto y0 = static_cast<std::int32_t>(std::floor(p.y));
  const auto z0 = static_cast<std::int32_t>(std::floor(p.z));
  const float fx = smooth01(p.x - std::floor(p.x));
  const float fy = smooth01(p.y - std::floor(p.y));
  const float fz = smooth01(p.z - std::floor(p.z));
  const auto region_id = static_cast<std::uint32_t>(region);
  const auto sample = [&](std::int32_t dx, std::int32_t dy, std::int32_t dz) noexcept {
    return hash01(hash5(seed, region_id, x0 + dx, y0 + dy, z0 + dz));
  };

  const float v000 = sample(0, 0, 0), v100 = sample(1, 0, 0);
  const float v010 = sample(0, 1, 0), v110 = sample(1, 1, 0);
  const float v001 = sample(0, 0, 1), v101 = sample(1, 0, 1);
  const float v011 = sample(0, 1, 1), v111 = sample(1, 1, 1);
  const float x00 = lerp(v000, v100, fx), x10 = lerp(v010, v110, fx);
  const float x01 = lerp(v001, v101, fx), x11 = lerp(v011, v111, fx);
  return lerp(lerp(x00, x10, fy), lerp(x01, x11, fy), fz);
}

struct PoreFieldSample {
  float height_m{};
  float influence{};
};

PoreFieldSample sample_pore_field(const CharacterGenomeV0& genome, BodyRegion region, Vec3 p,
                                  float cell_m, float density, float depth_m) noexcept {
  const auto bx = static_cast<std::int32_t>(std::floor(p.x / cell_m));
  const auto by = static_cast<std::int32_t>(std::floor(p.y / cell_m));
  const auto bz = static_cast<std::int32_t>(std::floor(p.z / cell_m));
  const auto region_id = static_cast<std::uint32_t>(region);
  PoreFieldSample result{};

  for (std::int32_t dz = -1; dz <= 1; ++dz) {
    for (std::int32_t dy = -1; dy <= 1; ++dy) {
      for (std::int32_t dx = -1; dx <= 1; ++dx) {
        const auto cx = bx + dx, cy = by + dy, cz = bz + dz;
        const std::uint32_t h = hash5(genome.surface_seed ^ 0xB5297A4Dull, region_id, cx, cy, cz);
        if (hash01(pcg_hash(h ^ 0xD1B54A35u)) >= density) continue;

        const float jx = 0.15f + 0.70f * hash01(pcg_hash(h ^ 0x68E31DA4u));
        const float jy = 0.15f + 0.70f * hash01(pcg_hash(h ^ 0xB5297A4Du));
        const float jz = 0.15f + 0.70f * hash01(pcg_hash(h ^ 0x1B56C4E9u));
        const Vec3 center{(static_cast<float>(cx) + jx) * cell_m,
                          (static_cast<float>(cy) + jy) * cell_m,
                          (static_cast<float>(cz) + jz) * cell_m};

        float ax = hash01(pcg_hash(h ^ 0x9E3779B9u)) * 2.0f - 1.0f;
        float ay = hash01(pcg_hash(h ^ 0x85EBCA6Bu)) * 2.0f - 1.0f;
        float az = hash01(pcg_hash(h ^ 0xC2B2AE35u)) * 2.0f - 1.0f;
        const float axis_length = std::sqrt(ax * ax + ay * ay + az * az);
        if (axis_length > 1.0e-6f) {
          ax /= axis_length; ay /= axis_length; az /= axis_length;
        } else {
          ax = 0.0f; ay = 1.0f; az = 0.0f;
        }

        const float vx = p.x - center.x, vy = p.y - center.y, vz = p.z - center.z;
        const float axial = vx * ax + vy * ay + vz * az;
        const float tx = vx - ax * axial, ty = vy - ay * axial, tz = vz - az * axial;
        const float axial_scale = 0.85f + 0.30f * hash01(pcg_hash(h ^ 0x27D4EB2Du));
        const float scaled_axial = axial / axial_scale;
        const float distance = std::sqrt(tx * tx + ty * ty + tz * tz + scaled_axial * scaled_axial);
        const float radius = cell_m * lerp(0.16f, 0.34f, hash01(pcg_hash(h ^ 0x165667B1u)));
        if (distance >= radius) continue;

        const float t = std::clamp(1.0f - distance / radius, 0.0f, 1.0f);
        const float shape = smooth01(t);
        const float local_depth = depth_m * lerp(0.55f, 1.0f, hash01(pcg_hash(h ^ 0xA511E9B3u)));
        result.height_m = std::min(result.height_m, -local_depth * shape);
        result.influence = std::max(result.influence, shape);
      }
    }
  }
  return result;
}

} // namespace

SurfaceSample sample_surface(const CharacterGenomeV0& genome, BodyRegion region, Vec3 p, float mm_per_pixel,
                             const PhysiologyState& physiology) noexcept {
  const DetailBand band = select_detail_band(mm_per_pixel);
  const float pore_scale = static_cast<float>(genome.skin.pore_scale) / 255.0f;
  const float cell_m = lerp(0.00052f, 0.00024f, pore_scale);
  const float density = std::clamp(static_cast<float>(genome.skin.pore_density) / 255.0f *
                                       region_pore_multiplier(region),
                                   0.05f, 0.95f);
  const float depth_m = lerp(0.000010f, 0.000050f,
                             static_cast<float>(genome.skin.pore_depth) / 255.0f);

  const float meso = band >= DetailBand::meso
      ? value_noise(genome.surface_seed ^ 0xA511E9B3ull, region,
                    {p.x / 0.0065f, p.y / 0.0065f, p.z / 0.0065f}) - 0.5f
      : 0.0f;
  const PoreFieldSample pore = band >= DetailBand::micro
      ? sample_pore_field(genome, region, p, cell_m, density, depth_m)
      : PoreFieldSample{};

  const float oil = static_cast<float>(genome.skin.oiliness) / 255.0f;
  const float hydration = static_cast<float>(genome.skin.hydration) / 255.0f;
  const float rough_bias = static_cast<float>(genome.skin.roughness_bias) / 255.0f;
  const float meso_strength = static_cast<float>(genome.micro.meso_strength) / 255.0f;

  SurfaceSample sample{};
  sample.pore_height = pore.height_m;
  sample.meso_variation = meso * meso_strength;
  sample.roughness = std::clamp(0.62f + (rough_bias - 0.5f) * 0.26f - oil * 0.16f - hydration * 0.05f -
                                    physiology.sweat * 0.12f + sample.meso_variation * 0.07f +
                                    pore.influence * 0.10f,
                                0.24f, 0.95f);
  sample.redness = std::clamp((static_cast<float>(genome.skin.haemoglobin) / 255.0f) * 0.35f +
                                  physiology.perfusion * 0.35f + sample.meso_variation * 0.02f,
                              0.0f, 1.0f);
  sample.specular_scale = 0.82f + oil * 0.18f + physiology.sweat * 0.45f;
  return sample;
}

} // namespace rengine::lsg
