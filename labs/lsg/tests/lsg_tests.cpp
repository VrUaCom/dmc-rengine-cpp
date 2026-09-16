#include "rengine/lsg/camera.hpp"
#include "rengine/lsg/derived_character.hpp"
#include "rengine/lsg/deterministic_hash.hpp"
#include "rengine/lsg/detail_scheduler.hpp"
#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/physiology.hpp"
#include "rengine/lsg/rmesh.hpp"
#include "rengine/lsg/surface.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace rengine::lsg;

int main() {
  const auto g0 = builtin_profile(0), g1 = builtin_profile(1);
  const auto bytes = encode_genome(g0);
  assert(!bytes.empty()); assert(bytes.size() < 512); assert(bytes.size() <= kGenomeHardLimit);
  DecodedGenome d{}; std::string err;
  assert(decode_genome(bytes, d, err)); assert(d.generator_revision == kGeneratorRevision); assert(d.value.surface_seed == g0.surface_seed);
  auto corrupt = bytes; corrupt.back() ^= std::byte{1}; assert(!decode_genome(corrupt, d, err));

  constexpr auto h1 = hash5(123,2,10,20,30); constexpr auto h2 = hash5(123,2,10,20,30); constexpr auto h3 = hash5(124,2,10,20,30);
  static_assert(h1 == h2); static_assert(h1 != h3);
  static_assert(hash5(0x1122334455667788ull, 4, -2, 7, 11) ==
                hash5_32(fold_seed64(0x1122334455667788ull), 4, -2, 7, 11));
  assert(select_detail_band(4.0f) == DetailBand::macro_only); assert(select_detail_band(2.0f) == DetailBand::meso);
  assert(select_detail_band(0.5f) == DetailBand::micro); assert(select_detail_band(0.05f) == DetailBand::micro_high);

  const auto a = sample_surface(g0, BodyRegion::head, {0.1f,1.7f,0.05f}, 0.05f, physiology_for(PhysiologyPreset::normal));
  const auto b = sample_surface(g0, BodyRegion::head, {0.1f,1.7f,0.05f}, 0.05f, physiology_for(PhysiologyPreset::normal));
  assert(a.pore_height == b.pore_height); assert(std::isfinite(a.roughness));
  const auto macro = sample_surface(g0, BodyRegion::head, {0.1f,1.7f,0.05f}, 4.0f, physiology_for(PhysiologyPreset::normal));
  assert(macro.pore_height == 0.0f && macro.meso_variation == 0.0f);
  const auto ex = sample_surface(g1, BodyRegion::head, {0.2f,1.6f,0.03f}, 0.05f, physiology_for(PhysiologyPreset::exercise));
  const auto normal = sample_surface(g1, BodyRegion::head, {0.2f,1.6f,0.03f}, 0.05f, physiology_for(PhysiologyPreset::normal));
  assert(ex.redness > normal.redness); assert(ex.roughness < normal.roughness);

  auto dense = g0; dense.skin.pore_density = 255; dense.skin.pore_depth = 255; dense.skin.pore_scale = 128;
  const float cell_m = 0.00052f + (0.00024f - 0.00052f) * (128.0f / 255.0f);
  bool found_active_cell = false;
  for (std::int32_t x = 0; x < 64 && !found_active_cell; ++x) {
    const std::uint32_t h = hash5(dense.surface_seed ^ 0xB5297A4Dull, static_cast<std::uint32_t>(BodyRegion::head), x, 0, 0);
    if (hash01(pcg_hash(h ^ 0xD1B54A35u)) >= 0.95f) continue;
    const float jx = 0.15f + 0.70f * hash01(pcg_hash(h ^ 0x68E31DA4u));
    const float jy = 0.15f + 0.70f * hash01(pcg_hash(h ^ 0xB5297A4Du));
    const float jz = 0.15f + 0.70f * hash01(pcg_hash(h ^ 0x1B56C4E9u));
    const Vec3 center{(static_cast<float>(x) + jx) * cell_m, jy * cell_m, jz * cell_m};
    const auto pore_center = sample_surface(dense, BodyRegion::head, center, 0.05f, physiology_for(PhysiologyPreset::normal));
    assert(pore_center.pore_height < 0.0f); assert(pore_center.pore_height >= -0.0000501f);
    found_active_cell = true;
  }
  assert(found_active_cell);

  const auto p0 = derive_character_parameters(g0), p1 = derive_character_parameters(g1);
  assert(p0.shoulder_scale != p1.shoulder_scale); assert(p0.pelvis_scale != p1.pelvis_scale);
  assert(p0.melanin >= 0.0f && p0.melanin <= 1.0f); assert(p1.pore_density >= 0.0f && p1.pore_density <= 1.0f);
  assert(p0.height_scale >= 0.80f && p0.height_scale <= 1.20f); assert(p1.head_scale >= 0.80f && p1.head_scale <= 1.20f);
  assert(p0.surface_seed_low == fold_seed64(g0.surface_seed));
  assert(p1.surface_seed_low == fold_seed64(g1.surface_seed));
  assert(p0.surface_seed_low != p1.surface_seed_low);

  CameraController camera; camera.set_subject_height(1.75f);
  const float full_distance = camera.state().distance_m;
  assert(camera.state().preset == CameraPreset::full_body); assert(full_distance > 3.0f && full_distance < 8.0f);
  camera.set_preset(CameraPreset::portrait); const float portrait_distance = camera.state().distance_m;
  camera.set_preset(CameraPreset::extreme_close_up); const float close_distance = camera.state().distance_m;
  assert(close_distance < portrait_distance && portrait_distance < full_distance);
  camera.orbit(0.25f, 100.0f); assert(camera.state().pitch_radians <= 1.10f);
  camera.orbit(-0.25f, -100.0f); assert(camera.state().pitch_radians >= -1.10f);
  camera.zoom(1000.0f); assert(camera.state().distance_m >= 0.32f);
  camera.zoom(0.0001f); assert(camera.state().distance_m <= 12.0f);

  RMeshV0 mesh{}; mesh.flags = rmesh_has_uv | rmesh_has_tangents | rmesh_has_regions; mesh.vertices.resize(3);
  mesh.vertices[0].position = {-1.0f,0.0f,0.0f}; mesh.vertices[1].position = {1.0f,0.0f,0.0f}; mesh.vertices[2].position = {0.0f,1.0f,0.0f};
  mesh.vertices[0].uv = {0.0f,0.0f}; mesh.vertices[1].uv = {1.0f,0.0f}; mesh.vertices[2].uv = {0.5f,1.0f};
  for (auto& v : mesh.vertices) { v.normal = {0.0f,0.0f,1.0f}; v.tangent = {1.0f,0.0f,0.0f,1.0f}; v.region_id = static_cast<std::uint8_t>(BodyRegion::chest); }
  mesh.indices = {0,1,2};
  assert(validate_rmesh(mesh, err));
  const auto rbytes = encode_rmesh(mesh); assert(rbytes.size() == kRMeshHeaderSize + 3u * kRMeshVertexStrideV0 + 3u * 4u);
  RMeshV0 decoded{}; assert(decode_rmesh(rbytes, decoded, err)); assert(decoded.vertices.size() == 3); assert(decoded.indices == mesh.indices);
  auto bad_rmesh = rbytes; bad_rmesh.back() ^= std::byte{1}; assert(!decode_rmesh(bad_rmesh, decoded, err));

  std::cout << "LSG tests PASS; genome bytes=" << bytes.size() << "; rmesh bytes=" << rbytes.size()
            << "; camera full=" << full_distance << "m portrait=" << portrait_distance << "m close=" << close_distance << "m\n";
  return 0;
}
