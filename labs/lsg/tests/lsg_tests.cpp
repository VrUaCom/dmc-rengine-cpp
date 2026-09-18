#include "rengine/lsg/anatomical_field.hpp"
#include "rengine/lsg/camera.hpp"
#include "rengine/lsg/derived_character.hpp"
#include "rengine/lsg/derived_eye.hpp"
#include "rengine/lsg/eye_runtime.hpp"
#include "rengine/lsg/lighting_runtime.hpp"
#include "rengine/lsg/deterministic_hash.hpp"
#include "rengine/lsg/detail_scheduler.hpp"
#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/physiology.hpp"
#include "rengine/lsg/projection.hpp"
#include "rengine/lsg/rmesh.hpp"
#include "rengine/lsg/surface.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace rengine::lsg;

namespace {
float point_distance(AnatomicalPoint a, AnatomicalPoint b) {
  const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}
}

int main() {
  const auto g0 = builtin_profile(0), g1 = builtin_profile(1);
  const auto bytes = encode_genome(g0);
  assert(!bytes.empty()); assert(bytes.size() < 512); assert(bytes.size() <= kGenomeHardLimit);
  DecodedGenome d{}; std::string err;
  assert(decode_genome(bytes, d, err)); assert(d.generator_revision == kGeneratorRevision); assert(d.value.surface_seed == g0.surface_seed);
  auto corrupt = bytes; corrupt.back() ^= std::byte{1}; assert(!decode_genome(corrupt, d, err));
  const auto legacy_revision = encode_genome(g0, 1u); assert(!legacy_revision.empty());
  assert(!decode_genome(legacy_revision, d, err)); assert(err.find("generator revision") != std::string::npos);

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

  const auto eye0 = derive_eye_parameters(g0), eye1 = derive_eye_parameters(g1);
  assert(eye0.eye_seed_low == fold_seed64(g0.eye_seed));
  assert(eye1.eye_seed_low == fold_seed64(g1.eye_seed));
  assert(eye0.eye_seed_low != eye1.eye_seed_low);
  for (float c : eye0.iris_primary) assert(c >= 0.0f && c <= 1.0f);
  for (float c : eye1.iris_secondary) assert(c >= 0.0f && c <= 1.0f);
  const float iris_a = sample_iris_variation(eye0.eye_seed_low, 0u, 17, 3);
  const float iris_b = sample_iris_variation(eye0.eye_seed_low, 0u, 17, 3);
  const float iris_other_side = sample_iris_variation(eye0.eye_seed_low, 1u, 17, 3);
  const float iris_other_seed = sample_iris_variation(eye1.eye_seed_low, 0u, 17, 3);
  assert(iris_a == iris_b);
  assert(iris_a >= 0.0f && iris_a < 1.0f);
  assert(iris_a != iris_other_side);
  assert(iris_a != iris_other_seed);

  const auto morning_clear = lighting_for(LightingPreset::morning, OpticalFilterPreset::clear);
  const auto noon_clear = lighting_for(LightingPreset::noon, OpticalFilterPreset::clear);
  const auto evening_clear = lighting_for(LightingPreset::evening, OpticalFilterPreset::clear);
  const auto night_clear = lighting_for(LightingPreset::night, OpticalFilterPreset::clear);
  const auto noon_tinted = lighting_for(LightingPreset::noon, OpticalFilterPreset::tinted);
  const auto noon_polarized = lighting_for(LightingPreset::noon, OpticalFilterPreset::polarized_approx);

  for (const auto* state : {&morning_clear, &noon_clear, &evening_clear, &night_clear,
                            &noon_tinted, &noon_polarized}) {
    assert(valid_lighting_state(*state));
    const float sun_len = std::sqrt(
        state->sun_direction[0] * state->sun_direction[0] +
        state->sun_direction[1] * state->sun_direction[1] +
        state->sun_direction[2] * state->sun_direction[2]);
    assert(std::abs(sun_len - 1.0f) < 0.0005f);
  }

  assert(noon_clear.effective_eye_luminance > morning_clear.effective_eye_luminance);
  assert(noon_clear.effective_eye_luminance > evening_clear.effective_eye_luminance);
  assert(morning_clear.effective_eye_luminance > night_clear.effective_eye_luminance);
  assert(evening_clear.effective_eye_luminance > night_clear.effective_eye_luminance);
  assert(night_clear.direct_sun_intensity < 0.05f);

  assert(noon_tinted.filter_transmission < noon_clear.filter_transmission);
  assert(noon_polarized.filter_transmission <= noon_clear.filter_transmission);
  assert(noon_tinted.effective_eye_luminance < noon_clear.effective_eye_luminance);
  assert(noon_polarized.polarization_strength > 0.0f);

  EyeRuntimeState eye_state{};
  const float dark_target = pupil_target_from_luminance(0.01f, eye0.pupil_bias);
  const float bright_target = pupil_target_from_luminance(4.0f, eye0.pupil_bias);
  assert(dark_target > bright_target);
  const float noon_clear_target = pupil_target_from_luminance(noon_clear.effective_eye_luminance, eye0.pupil_bias);
  const float noon_tinted_target = pupil_target_from_luminance(noon_tinted.effective_eye_luminance, eye0.pupil_bias);
  const float night_target = pupil_target_from_luminance(night_clear.effective_eye_luminance, eye0.pupil_bias);
  assert(noon_tinted_target >= noon_clear_target);
  assert(night_target > noon_clear_target);
  float previous_radius = eye_state.pupil_radius;
  for (int i = 0; i < 120; ++i) update_eye_runtime(eye_state, 4.0f, eye0.pupil_bias, 1.0f / 60.0f);
  assert(std::isfinite(eye_state.pupil_radius));
  assert(eye_state.pupil_radius < previous_radius);
  assert(std::abs(eye_state.pupil_radius - eye_state.target_pupil_radius) < 0.002f);
  const float bright_radius = eye_state.pupil_radius;
  for (int i = 0; i < 120; ++i) update_eye_runtime(eye_state, 0.01f, eye0.pupil_bias, 1.0f / 60.0f);
  assert(eye_state.pupil_radius > bright_radius);
  assert(eye_state.pupil_radius >= 0.070f && eye_state.pupil_radius <= 0.155f);

  const auto p0 = derive_character_parameters(g0), p1 = derive_character_parameters(g1);
  assert(p0.shoulder_scale != p1.shoulder_scale); assert(p0.pelvis_scale != p1.pelvis_scale);
  assert(p0.melanin >= 0.0f && p0.melanin <= 1.0f); assert(p1.pore_density >= 0.0f && p1.pore_density <= 1.0f);
  assert(p0.height_scale >= 0.80f && p0.height_scale <= 1.20f); assert(p1.head_scale >= 0.80f && p1.head_scale <= 1.20f);
  assert(p0.surface_seed_low == fold_seed64(g0.surface_seed));
  assert(p1.surface_seed_low == fold_seed64(g1.surface_seed));
  assert(p0.surface_seed_low != p1.surface_seed_low);

  // Continuous anatomy must never create the old region-boundary discontinuity. Probe the
  // entire body height with a short representative mesh edge at an off-axis position.
  float worst_local_stretch = 1.0f;
  for (int i = 0; i < 1750; ++i) {
    const float y0 = -0.875f + static_cast<float>(i) * 0.001f;
    const AnatomicalPoint raw0{0.28f, y0, 0.12f};
    const AnatomicalPoint raw1{0.28f, y0 + 0.001f, 0.12f};
    const float raw_length = point_distance(raw0, raw1);
    for (const auto* parameters : {&p0, &p1}) {
      const auto shaped0 = deform_anatomy_rest(raw0, *parameters);
      const auto shaped1 = deform_anatomy_rest(raw1, *parameters);
      const float stretch = point_distance(shaped0, shaped1) / raw_length;
      assert(std::isfinite(stretch));
      worst_local_stretch = std::max(worst_local_stretch, stretch);
    }
    const auto weights = sample_anatomical_weights(y0 / 1.75f + 0.5f);
    assert(weights.shoulder >= 0.0f && weights.shoulder <= 1.0f);
    assert(weights.chest >= 0.0f && weights.chest <= 1.0f);
    assert(weights.waist >= 0.0f && weights.waist <= 1.0f);
    assert(weights.pelvis >= 0.0f && weights.pelvis <= 1.0f);
    assert(weights.head >= 0.0f && weights.head <= 1.0f);
  }
  assert(worst_local_stretch < 1.25f);

  // The visible viewport aspect must not be inverted by Vulkan pre-rotation. The Samsung
  // landscape device evidence showed a 2340x1080 viewport with ROTATE_90; camera projection
  // must stay landscape while clip-space alone is pre-rotated for presentation.
  constexpr Extent2u landscape_viewport{2340u, 1080u};
  constexpr auto landscape_rotate90 = logical_extent_for_surface_rotation(landscape_viewport, 1u);
  constexpr auto landscape_rotate270 = logical_extent_for_surface_rotation(landscape_viewport, 3u);
  static_assert(landscape_rotate90.width == 2340u && landscape_rotate90.height == 1080u);
  static_assert(landscape_rotate270.width == 2340u && landscape_rotate270.height == 1080u);
  assert(std::abs(extent_aspect(landscape_rotate90) - (2340.0f / 1080.0f)) < 0.0001f);

  constexpr Extent2u portrait_viewport{1080u, 2340u};
  constexpr auto portrait_rotate90 = logical_extent_for_surface_rotation(portrait_viewport, 1u);
  static_assert(portrait_rotate90.width == 1080u && portrait_rotate90.height == 2340u);
  assert(std::abs(extent_aspect(portrait_rotate90) - (1080.0f / 2340.0f)) < 0.0001f);

  CameraController camera; camera.set_subject_height(1.75f);
  const float full_distance = camera.state().distance_m;
  assert(camera.state().preset == CameraPreset::full_body); assert(full_distance > 3.0f && full_distance < 8.0f);
  camera.set_preset(CameraPreset::portrait); const float portrait_distance = camera.state().distance_m;
  camera.set_preset(CameraPreset::extreme_close_up); const float close_distance = camera.state().distance_m;
  assert(close_distance < portrait_distance && portrait_distance < full_distance);
  const float yaw_before_drag = camera.state().yaw_radians;
  camera.orbit(0.25f, 100.0f);
  assert(camera.state().yaw_radians < yaw_before_drag);
  assert(camera.state().pitch_radians <= 1.10f);
  camera.orbit(-0.25f, -100.0f); assert(camera.state().pitch_radians >= -1.10f);
  camera.reset_view();
  assert(std::abs(camera.state().yaw_radians) < 0.0001f);
  assert(std::abs(camera.state().pitch_radians) < 0.0001f);
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

  std::cout << "LSG tests PASS; generator revision=" << kGeneratorRevision << "; genome bytes=" << bytes.size()
            << "; rmesh bytes=" << rbytes.size() << "; camera full=" << full_distance
            << "m portrait=" << portrait_distance << "m close=" << close_distance
            << "m; anatomy worst local stretch=" << worst_local_stretch << "\n";
  return 0;
}
