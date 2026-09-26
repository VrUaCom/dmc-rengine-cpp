#include "rengine/lsg/surface.hpp"
#include "rengine/lsg/deterministic_hash.hpp"
#include "rengine/lsg/skin_material.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rengine::lsg {
namespace {

float smooth_range(float value, float lo, float hi) noexcept {
  const float x = std::clamp((value - lo) / std::max(hi - lo, 1.0e-6f), 0.0f, 1.0f);
  return x * x * (3.0f - 2.0f * x);
}

float smooth_band(float value, float rise0, float rise1, float fall0, float fall1) noexcept {
  return smooth_range(value, rise0, rise1) *
         (1.0f - smooth_range(value, fall0, fall1));
}

float anatomical_pore_density_scale(Vec3 p) noexcept {
  const float y01 = std::clamp(p.y / 1.75f + 0.5f, 0.0f, 1.0f);
  const float lateral = std::clamp(std::abs(p.x) / 0.52f, 0.0f, 1.0f);
  const float head = smooth_range(y01, 0.80f, 0.90f);
  const float neck = smooth_band(y01, 0.75f, 0.81f, 0.86f, 0.91f);
  const float upper_torso = smooth_band(y01, 0.56f, 0.64f, 0.76f, 0.83f);
  const float arms = smooth_band(y01, 0.47f, 0.57f, 0.76f, 0.85f) *
                     smooth_range(lateral, 0.44f, 0.76f);
  const float lower_leg = smooth_band(y01, 0.06f, 0.12f, 0.27f, 0.34f);
  const float foot = 1.0f - smooth_range(y01, 0.05f, 0.11f);
  return std::clamp(0.88f + head * 0.30f + neck * 0.08f +
                        upper_torso * 0.05f - arms * 0.05f -
                        lower_leg * 0.07f - foot * 0.10f,
                    0.76f, 1.20f);
}

float smooth01(float x) noexcept { return x * x * (3.0f - 2.0f * x); }
float lerp(float a, float b, float t) noexcept { return a + (b - a) * t; }

float value_noise(std::uint64_t seed, Vec3 p) noexcept {
  const auto x0 = static_cast<std::int32_t>(std::floor(p.x));
  const auto y0 = static_cast<std::int32_t>(std::floor(p.y));
  const auto z0 = static_cast<std::int32_t>(std::floor(p.z));
  const float fx = smooth01(p.x - std::floor(p.x));
  const float fy = smooth01(p.y - std::floor(p.y));
  const float fz = smooth01(p.z - std::floor(p.z));
  const auto sample = [&](std::int32_t dx, std::int32_t dy, std::int32_t dz) noexcept {
    return hash01(hash_cell3(seed, x0 + dx, y0 + dy, z0 + dz));
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

PoreFieldSample sample_pore_field(const CharacterGenomeV0& genome, Vec3 p,
                                  float cell_m, float density, float depth_m) noexcept {
  const auto bx = static_cast<std::int32_t>(std::floor(p.x / cell_m));
  const auto by = static_cast<std::int32_t>(std::floor(p.y / cell_m));
  const auto bz = static_cast<std::int32_t>(std::floor(p.z / cell_m));
  PoreFieldSample result{};

  for (std::int32_t dz = -1; dz <= 1; ++dz) {
    for (std::int32_t dy = -1; dy <= 1; ++dy) {
      for (std::int32_t dx = -1; dx <= 1; ++dx) {
        const auto cx = bx + dx, cy = by + dy, cz = bz + dz;
        const std::uint32_t h = hash_cell3(genome.surface_seed ^ 0xB5297A4Dull, cx, cy, cz);
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

SurfaceSample sample_surface(const CharacterGenomeV0& genome, BodyRegion region, Vec3 p,
                             float mm_per_pixel,
                             const PhysiologyState& physiology) noexcept {
  (void)region; // semantic labels must not change procedural phase or create seams.
  const DetailBand band = select_detail_band(mm_per_pixel);
  const SkinPhenotype skin = derive_skin_phenotype(genome, physiology);
  const float cell_m = lerp(0.00052f, 0.00024f, skin.pore_scale);
  const float density = std::clamp(
      skin.pore_density * anatomical_pore_density_scale(p), 0.05f, 0.95f);
  const float depth_m = lerp(0.000010f, 0.000050f, skin.pore_depth);

  const float meso = band >= DetailBand::meso
      ? value_noise(genome.surface_seed ^ 0xA511E9B3ull,
                    {p.x / 0.0065f, p.y / 0.0065f, p.z / 0.0065f}) - 0.5f
      : 0.0f;
  const PoreFieldSample pore = band >= DetailBand::micro
      ? sample_pore_field(genome, p, cell_m, density, depth_m)
      : PoreFieldSample{};

  const float freckle_noise = band >= DetailBand::meso
      ? value_noise(genome.surface_seed ^ 0xF1357AEAull,
                    {p.x / 0.0045f, p.y / 0.0045f, p.z / 0.0045f})
      : 0.0f;
  const float freckle_threshold = lerp(0.97f, 0.70f, skin.freckle_density);
  const float freckle_mask = band >= DetailBand::meso
      ? smooth_range(freckle_noise, freckle_threshold, 1.0f) : 0.0f;

  const float follicle_noise = band >= DetailBand::micro
      ? value_noise(genome.surface_seed ^ 0x7F4A7C15ull,
                    {p.x / 0.0012f, p.y / 0.0012f, p.z / 0.0012f})
      : 0.0f;
  const float follicle_threshold = lerp(0.985f, 0.72f, skin.follicle_density);
  const float follicle_influence = band >= DetailBand::micro
      ? smooth_range(follicle_noise, follicle_threshold, 1.0f) : 0.0f;

  const float wrinkle_noise = band >= DetailBand::micro_high
      ? value_noise(genome.surface_seed ^ 0x91E10DA5ull,
                    {p.x / 0.0018f, p.y / 0.00072f, p.z / 0.0018f}) - 0.5f
      : 0.0f;
  const float wrinkle_height = band >= DetailBand::micro_high
      ? wrinkle_noise * skin.wrinkle_bias * skin.micro_strength * 0.000018f
      : 0.0f;

  SurfaceSample sample{};
  sample.pore_height = pore.height_m;
  sample.meso_variation = meso * skin.meso_strength;
  sample.follicle_influence = follicle_influence;
  sample.freckle_mask = freckle_mask;
  sample.wrinkle_height = wrinkle_height;
  sample.roughness = std::clamp(
      skin_base_roughness(skin) + sample.meso_variation * 0.07f +
          pore.influence * 0.10f + follicle_influence * 0.045f +
          std::abs(wrinkle_noise) * skin.wrinkle_bias * 0.025f,
      0.24f, 0.95f);
  sample.redness = std::clamp(
      skin.haemoglobin * 0.35f + skin.perfusion * 0.35f +
          sample.meso_variation * 0.02f - freckle_mask * 0.035f,
      0.0f, 1.0f);
  sample.specular_scale = 0.80f + skin.coat_strength * 0.65f;
  return sample;
}

} // namespace rengine::lsg
