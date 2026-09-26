#include "rengine/lsg/anatomical_field.hpp"
#include "rengine/lsg/character_profile.hpp"
#include "rengine/lsg/camera.hpp"
#include "rengine/lsg/derived_character.hpp"
#include "rengine/lsg/derived_eye.hpp"
#include "rengine/lsg/eye_runtime.hpp"
#include "rengine/lsg/face_field.hpp"
#include "rengine/lsg/lighting_runtime.hpp"
#include "rengine/lsg/polarization_approx.hpp"
#include "rengine/lsg/deterministic_hash.hpp"
#include "rengine/lsg/detail_scheduler.hpp"
#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/physiology.hpp"
#include "rengine/lsg/projection.hpp"
#include "rengine/lsg/rmesh.hpp"
#include "rengine/lsg/surface.hpp"
#include "rengine/lsg/shadow_quality.hpp"
#include "rengine/lsg/skin_material.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace rengine::lsg;

namespace {
float point_distance(AnatomicalPoint a, AnatomicalPoint b) {
  const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}
}

int main() {
  static_assert(kBuiltinCarrierCount == 2u);
  static_assert(kBuiltinProfileCount == 3u);
  static_assert(kAdaProfileIndex == 2u);
  const auto carriers = builtin_carriers();
  const auto profiles = builtin_character_profiles();
  assert(carriers.size() == kBuiltinCarrierCount);
  assert(profiles.size() == kBuiltinProfileCount);
  assert(character_profile_definition(0).carrier == CarrierId::male_base);
  assert(character_profile_definition(1).carrier == CarrierId::female_base);
  assert(character_profile_definition(kAdaProfileIndex).carrier == CarrierId::female_base);
  assert(carrier_slot(character_profile_definition(kAdaProfileIndex).carrier) ==
         carrier_slot(CarrierId::female_base));
  assert(carrier_definition(CarrierId::male_base).body_asset_path !=
         carrier_definition(CarrierId::female_base).body_asset_path);
  assert(character_profile_definition(kBuiltinProfileCount + kAdaProfileIndex).index ==
         kAdaProfileIndex);
  const auto g0 = builtin_profile(0), g1 = builtin_profile(1), g2 = builtin_profile(kAdaProfileIndex);
  const auto bytes = encode_genome(g0);
  assert(!bytes.empty()); assert(bytes.size() < 512); assert(bytes.size() <= kGenomeHardLimit);
  DecodedGenome d{}; std::string err;
  assert(decode_genome(bytes, d, err)); assert(d.generator_revision == kGeneratorRevision); assert(d.value.surface_seed == g0.surface_seed);
  const auto ada_bytes = encode_genome(g2); assert(!ada_bytes.empty());
  DecodedGenome ada_decoded{}; assert(decode_genome(ada_bytes, ada_decoded, err));
  assert(ada_decoded.value.identity_seed == g2.identity_seed);
  assert(ada_decoded.value.face.eye_tilt == g2.face.eye_tilt);
  assert(g2.face.cheekbone_width > 0 && g2.face.jaw_width < 0);
  assert(g2.face.upper_lip_fullness > 0 && g2.face.lower_lip_fullness > g2.face.upper_lip_fullness);
  assert(g0.geometry.jaw == 0 && g0.geometry.facial_softness == 0);
  assert(g1.geometry.jaw == 0 && g1.geometry.facial_softness == 0);
  assert(g2.geometry.jaw == 0 && g2.geometry.facial_softness == 0);
  assert(g2.identity_seed != g1.identity_seed); assert(g2.surface_seed != g1.surface_seed); assert(g2.eye_seed != g1.eye_seed);

  // Universal FaceField contract: all profiles use the same math. Neutral base profiles
  // are identity transforms; Ada and synthetic profiles are parameter-only variations.
  const auto neutral_face = derive_face_field_parameters(g0.face);
  const FacePoint raw_face{0.035f, 0.735f, 0.080f};
  const FacePoint shaped_face{0.035f, 0.735f, 0.080f};
  const auto neutral_result = deform_face_field(shaped_face, raw_face, 1.0f, neutral_face);
  assert(std::abs(neutral_result.x - shaped_face.x) < 1e-7f);
  assert(std::abs(neutral_result.y - shaped_face.y) < 1e-7f);
  assert(std::abs(neutral_result.z - shaped_face.z) < 1e-7f);

  const auto ada_face = derive_face_field_parameters(g2.face);
  const auto ada_result = deform_face_field(shaped_face, raw_face, 1.0f, ada_face);
  assert(std::isfinite(ada_result.x) && std::isfinite(ada_result.y) && std::isfinite(ada_result.z));
  const auto inactive = deform_face_field(shaped_face, raw_face, 0.0f, ada_face);
  assert(inactive.x == shaped_face.x && inactive.y == shaped_face.y && inactive.z == shaped_face.z);

  FaceGenomeV0 synthetic_face{};
  synthetic_face.skull_width = 16000;
  synthetic_face.skull_height = -12000;
  synthetic_face.cheekbone_width = 20000;
  synthetic_face.jaw_width = -18000;
  synthetic_face.chin_width = 14000;
  synthetic_face.nose_projection = 17000;
  synthetic_face.mouth_width = 9000;
  const auto synthetic = derive_face_field_parameters(synthetic_face);
  const FacePoint left_raw{-0.040f, 0.680f, 0.085f};
  const FacePoint right_raw{0.040f, 0.680f, 0.085f};
  const auto left = deform_face_field(left_raw, left_raw, 1.0f, synthetic);
  const auto right = deform_face_field(right_raw, right_raw, 1.0f, synthetic);
  assert(std::isfinite(left.x) && std::isfinite(right.x));
  assert(std::abs(left.x + right.x) < 1e-5f);
  assert(std::abs(left.y - right.y) < 1e-5f);
  assert(std::abs(left.z - right.z) < 1e-5f);

  const FacePoint left_eye{-0.032f, 0.752f, 0.080f};
  const FacePoint right_eye{0.032f, 0.752f, 0.080f};
  const auto eye_left = deform_eye_socket_field(left_eye, left_eye, 1.0f, ada_face);
  const auto eye_right = deform_eye_socket_field(right_eye, right_eye, 1.0f, ada_face);
  assert(std::isfinite(eye_left.x) && std::isfinite(eye_right.x));
  assert(std::abs(eye_left.x + eye_right.x) < 1e-5f);

  constexpr auto face_max = std::numeric_limits<std::int16_t>::max();
  constexpr auto face_min = std::numeric_limits<std::int16_t>::min();
  static_assert(sizeof(FaceGenomeV0) == 20u * sizeof(std::int16_t));
  const FaceGenomeV0 extreme_face{
      face_max, face_min, face_max, face_min,
      face_max, face_min, face_max, face_min,
      face_max, face_min, face_max, face_min,
      face_max, face_min, face_max, face_min,
      face_max, face_min, face_max, face_min};
  const auto extreme = derive_face_field_parameters(extreme_face);
  float previous_activation_x = raw_face.x;
  for (int step = 0; step <= 25; ++step) {
    const float head_weight = static_cast<float>(step) / 100.0f;
    const auto point = deform_face_field(shaped_face, raw_face, head_weight, extreme);
    assert(std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z));
    assert(std::abs(point.x - shaped_face.x) < 0.20f);
    assert(std::abs(point.y - shaped_face.y) < 0.20f);
    assert(std::abs(point.z - shaped_face.z) < 0.20f);
    if (step > 0) assert(std::abs(point.x - previous_activation_x) < 0.02f);
    previous_activation_x = point.x;
  }

  for (float x : {-0.09f, -0.045f, 0.0f, 0.045f, 0.09f}) {
    for (float y : {0.53f, 0.60f, 0.69f, 0.76f, 0.86f}) {
      for (float z : {0.0f, 0.04f, 0.08f, 0.12f}) {
        const FacePoint raw{x, y, z};
        const auto point = deform_face_field(raw, raw, 1.0f, extreme);
        assert(std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z));
      }
    }
  }

  auto corrupt = bytes; corrupt.back() ^= std::byte{1}; assert(!decode_genome(corrupt, d, err));
  const auto legacy_v2 = encode_genome(g0, kLegacyGeneratorRevision); assert(!legacy_v2.empty());
  DecodedGenome migrated_v2{}; assert(decode_genome(legacy_v2, migrated_v2, err));
  assert(migrated_v2.generator_revision == kLegacyGeneratorRevision);
  assert(migrated_v2.value.face.skull_width == 0 && migrated_v2.value.face.nose_projection == 0);
  const auto legacy_revision = encode_genome(g0, 1u); assert(!legacy_revision.empty());
  assert(!decode_genome(legacy_revision, d, err)); assert(err.find("generator revision") != std::string::npos);

  constexpr auto h1 = hash5(123,2,10,20,30); constexpr auto h2 = hash5(123,2,10,20,30); constexpr auto h3 = hash5(124,2,10,20,30);
  static_assert(h1 == h2); static_assert(h1 != h3);
  static_assert(hash5(0x1122334455667788ull, 4, -2, 7, 11) ==
                hash5_32(fold_seed64(0x1122334455667788ull), 4, -2, 7, 11));
  constexpr std::uint64_t cell_seed = 0x1122334455667788ull;
  constexpr std::uint32_t cell_key = fold_seed64(cell_seed);
  constexpr std::uint32_t cell_manual =
      pcg_hash(pcg_hash(pcg_hash(pcg_hash(cell_key) ^ static_cast<std::uint32_t>(-2)) ^
                                static_cast<std::uint32_t>(7)) ^
                         static_cast<std::uint32_t>(11));
  static_assert(hash_cell3(cell_seed, -2, 7, 11) == cell_manual);
  static_assert(hash_cell3_32(cell_key, -2, 7, 11) == cell_manual);
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

  auto warmth = [](const std::array<float, 3>& rgb) {
    return rgb[0] - rgb[2];
  };
  assert(warmth(morning_clear.sun_tint) > warmth(noon_clear.sun_tint));
  assert(warmth(evening_clear.sun_tint) > warmth(noon_clear.sun_tint));
  for (const auto* state : {&morning_clear, &noon_clear, &evening_clear, &night_clear}) {
    for (float v : state->sun_tint) assert(v >= 0.0f && v <= 1.0f);
    for (float v : state->sky_zenith_tint) assert(v >= 0.0f && v <= 1.0f);
    for (float v : state->sky_horizon_tint) assert(v >= 0.0f && v <= 1.0f);
  }

  assert(noon_tinted.filter_transmission < noon_clear.filter_transmission);
  assert(noon_polarized.filter_transmission <= noon_clear.filter_transmission);
  assert(noon_tinted.effective_eye_luminance < noon_clear.effective_eye_luminance);
  assert(noon_polarized.polarization_strength > 0.0f);

  // Pass 3 optical-filter matrix: all four times x three filters.
  for (const auto time : {LightingPreset::morning, LightingPreset::noon,
                          LightingPreset::evening, LightingPreset::night}) {
    const auto clear = lighting_for(time, OpticalFilterPreset::clear);
    const auto polarized = lighting_for(time, OpticalFilterPreset::polarized_approx);
    const auto tinted = lighting_for(time, OpticalFilterPreset::tinted);
    for (const auto* state : {&clear, &polarized, &tinted}) {
      assert(valid_lighting_state(*state));
      assert(state->filter_transmission > 0.0f && state->filter_transmission <= 1.0f);
      assert(state->polarization_strength >= 0.0f && state->polarization_strength <= 1.0f);
      assert(state->effective_eye_luminance >= 0.0f);
      for (float value : state->filter_tint) assert(value >= 0.0f && value <= 1.0f);
    }
    assert(clear.effective_eye_luminance > polarized.effective_eye_luminance);
    assert(polarized.effective_eye_luminance > tinted.effective_eye_luminance);

    const float clear_target =
        pupil_target_from_luminance(clear.effective_eye_luminance, eye0.pupil_bias);
    const float polarized_target =
        pupil_target_from_luminance(polarized.effective_eye_luminance, eye0.pupil_bias);
    const float tinted_target =
        pupil_target_from_luminance(tinted.effective_eye_luminance, eye0.pupil_bias);
    for (const float target : {clear_target, polarized_target, tinted_target}) {
      assert(std::isfinite(target));
      assert(target >= kPupilRadiusMin && target <= kPupilRadiusMax);
    }
    assert(clear_target <= polarized_target + 1e-6f);
    assert(polarized_target <= tinted_target + 1e-6f);
  }

  assert(std::abs(rayleigh_dolp_from_mu(0.0f) - 1.0f) < 1e-6f);
  assert(std::abs(rayleigh_dolp_from_mu(1.0f)) < 1e-6f);
  assert(std::abs(rayleigh_dolp_from_mu(-1.0f)) < 1e-6f);
  assert(std::abs(rayleigh_dolp_from_mu(0.5f) - rayleigh_dolp_from_mu(-0.5f)) < 1e-6f);
  const float dolp_half = rayleigh_dolp_from_mu(0.5f);
  assert(dolp_half > 0.0f && dolp_half < 1.0f);

  assert(std::abs(polarized_attenuation(1.0f, 0.0f, 0.0f) - 1.0f) < 1e-6f);
  assert(std::abs(polarized_attenuation(0.0f, 0.0f, 1.0f) - 1.0f) < 1e-6f);
  assert(std::abs(polarized_attenuation(1.0f, 1.0f, 1.0f) - 1.0f) < 1e-6f);
  const float cross_attenuation = polarized_attenuation(1.0f, 0.0f, 1.0f);
  assert(cross_attenuation < 1.0f);
  assert(cross_attenuation >= 0.45f);
  for (float d : {0.0f, 0.25f, 0.5f, 1.0f}) {
    for (float a : {0.0f, 0.4f, 1.0f}) {
      for (float strength : {0.0f, 0.68f, 1.0f}) {
        const float attenuation = polarized_attenuation(d, a, strength);
        assert(std::isfinite(attenuation));
        assert(attenuation >= 0.45f && attenuation <= 1.0f);
      }
    }
  }

  EyeRuntimeState eye_state{};
  assert(std::abs(eye_state.pupil_radius - kPupilInitialRadius) < 1e-6f);
  assert(std::abs(eye_state.target_pupil_radius - kPupilInitialRadius) < 1e-6f);
  const float dark_target = pupil_target_from_luminance(0.01f, eye0.pupil_bias);
  const float bright_target = pupil_target_from_luminance(4.0f, eye0.pupil_bias);
  assert(dark_target > bright_target);
  assert(dark_target >= kPupilRadiusMin && dark_target <= kPupilRadiusMax);
  assert(bright_target >= kPupilRadiusMin && bright_target <= kPupilRadiusMax);
  const float darkest_endpoint = pupil_target_from_luminance(0.0f, 0.5f);
  const float brightest_endpoint = pupil_target_from_luminance(1.0e6f, 0.5f);
  assert(std::abs(darkest_endpoint - kPupilDarkRadius) < 0.0001f);
  assert(std::abs(brightest_endpoint - kPupilBrightRadius) < 0.0001f);
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
  assert(eye_state.pupil_radius >= kPupilRadiusMin &&
         eye_state.pupil_radius <= kPupilRadiusMax);

  const auto p0 = derive_character_parameters(g0), p1 = derive_character_parameters(g1), p2 = derive_character_parameters(g2);
  assert(p0.shoulder_scale != p1.shoulder_scale); assert(p0.pelvis_scale != p1.pelvis_scale);
  assert(p0.melanin >= 0.0f && p0.melanin <= 1.0f); assert(p1.pore_density >= 0.0f && p1.pore_density <= 1.0f);
  assert(p0.height_scale >= 0.80f && p0.height_scale <= 1.20f); assert(p1.head_scale >= 0.80f && p1.head_scale <= 1.20f);
  assert(p0.surface_seed_low == fold_seed64(g0.surface_seed));
  assert(p1.surface_seed_low == fold_seed64(g1.surface_seed));
  assert(p0.surface_seed_low != p1.surface_seed_low);
  assert(p2.surface_seed_low == fold_seed64(g2.surface_seed)); assert(p2.surface_seed_low != p1.surface_seed_low);
  assert(p2.pelvis_scale != p1.pelvis_scale || p2.chest_depth_scale != p1.chest_depth_scale);

  const auto skin0 = derive_skin_phenotype(g0, physiology_for(PhysiologyPreset::normal));
  const auto skin1 = derive_skin_phenotype(g1, physiology_for(PhysiologyPreset::normal));
  const auto skin2 = derive_skin_phenotype(g2, physiology_for(PhysiologyPreset::normal));
  static_assert(sizeof(SkinMaterialGpuV0) == 80u);
  for (const auto* skin : {&skin0, &skin1, &skin2}) {
    assert(skin->melanin >= 0.0f && skin->melanin <= 1.0f);
    assert(skin->carotene >= 0.0f && skin->carotene <= 1.0f);
    assert(skin->follicle_density >= 0.0f && skin->follicle_density <= 1.0f);
    assert(skin->freckle_density >= 0.0f && skin->freckle_density <= 1.0f);
    assert(skin->micro_strength >= 0.0f && skin->micro_strength <= 1.0f);
    assert(skin->wrinkle_bias >= 0.0f && skin->wrinkle_bias <= 1.0f);
    assert(skin->coat_strength >= 0.05f && skin->coat_strength <= 1.0f);
    assert(skin->subsurface_strength >= 0.12f && skin->subsurface_strength <= 0.66f);
    const auto rgb = skin_base_reflectance(*skin);
    assert(std::isfinite(rgb.r) && std::isfinite(rgb.g) && std::isfinite(rgb.b));
    assert(rgb.r > 0.0f && rgb.g > 0.0f && rgb.b > 0.0f);
    assert(skin_base_roughness(*skin) >= 0.24f && skin_base_roughness(*skin) <= 0.90f);
    const auto gpu_skin = pack_skin_material_gpu(*skin);
    assert(gpu_skin.pigments[0] == skin->melanin);
    assert(gpu_skin.pores[3] == skin->follicle_density);
    assert(gpu_skin.features[2] == skin->micro_strength);
    assert(gpu_skin.physiology[3] == skin->subsurface_strength);
  }

  auto light_skin_genome = g0;
  auto dark_skin_genome = g0;
  light_skin_genome.skin.melanin = 8;
  dark_skin_genome.skin.melanin = 245;
  const auto light_skin = derive_skin_phenotype(light_skin_genome, physiology_for(PhysiologyPreset::normal));
  const auto dark_skin = derive_skin_phenotype(dark_skin_genome, physiology_for(PhysiologyPreset::normal));
  const auto light_rgb = skin_base_reflectance(light_skin);
  const auto dark_rgb = skin_base_reflectance(dark_skin);
  assert((dark_rgb.r + dark_rgb.g + dark_rgb.b) < (light_rgb.r + light_rgb.g + light_rgb.b));

  auto carotene_low_genome = g0;
  auto carotene_high_genome = g0;
  carotene_low_genome.skin.carotene = 0;
  carotene_high_genome.skin.carotene = 255;
  const auto carotene_low = skin_base_reflectance(
      derive_skin_phenotype(carotene_low_genome, physiology_for(PhysiologyPreset::normal)));
  const auto carotene_high = skin_base_reflectance(
      derive_skin_phenotype(carotene_high_genome, physiology_for(PhysiologyPreset::normal)));
  assert(carotene_high.r + carotene_high.g > carotene_low.r + carotene_low.g);

  const auto cold_skin = derive_skin_phenotype(g0, physiology_for(PhysiologyPreset::cold));
  const auto hot_skin = derive_skin_phenotype(g0, physiology_for(PhysiologyPreset::hot));
  assert(hot_skin.sweat > cold_skin.sweat);
  assert(hot_skin.perfusion > cold_skin.perfusion);

  // Semantic BodyRegion labels must not phase-shift the procedural skin field.
  const Vec3 seam_probe{0.21f, 0.48f, 0.09f};
  const auto seam_a = sample_surface(g0, BodyRegion::chest, seam_probe, 0.05f,
                                     physiology_for(PhysiologyPreset::normal));
  const auto seam_b = sample_surface(g0, BodyRegion::upper_arm, seam_probe, 0.05f,
                                     physiology_for(PhysiologyPreset::normal));
  assert(std::abs(seam_a.pore_height - seam_b.pore_height) < 1e-9f);
  assert(std::abs(seam_a.meso_variation - seam_b.meso_variation) < 1e-9f);
  assert(std::abs(seam_a.freckle_mask - seam_b.freckle_mask) < 1e-9f);

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

  const auto shadow_full =
      close_shadow_config(CameraPreset::full_body, full_distance);
  const auto shadow_portrait =
      close_shadow_config(CameraPreset::portrait, portrait_distance);
  const auto shadow_close =
      close_shadow_config(CameraPreset::extreme_close_up, close_distance);
  assert(shadow_full.level == CloseShadowLevel::baseline);
  assert(shadow_portrait.level == CloseShadowLevel::portrait);
  assert(shadow_close.level == CloseShadowLevel::cinematic);
  assert(shadow_close.half_extent_m < shadow_portrait.half_extent_m);
  assert(shadow_portrait.half_extent_m < shadow_full.half_extent_m);
  assert(shadow_close.texel_size_m < shadow_portrait.texel_size_m);
  assert(shadow_portrait.texel_size_m < shadow_full.texel_size_m);
  assert(std::isfinite(shadow_close.texel_size_m));
  assert(shadow_close.texel_size_m > 0.0f);
  assert(select_close_shadow_level(CameraPreset::full_body, 1.20f) ==
         CloseShadowLevel::cinematic);
  assert(select_close_shadow_level(CameraPreset::full_body, 2.00f) ==
         CloseShadowLevel::portrait);
  assert(select_close_shadow_level(CameraPreset::full_body, 3.00f) ==
         CloseShadowLevel::baseline);

  const float snap_texel = shadow_close.texel_size_m;
  const float snap_base = 10.25f * snap_texel;
  const float snapped_a = snap_shadow_axis(snap_base, snap_texel);
  const float snapped_b =
      snap_shadow_axis(snap_base + 0.10f * snap_texel, snap_texel);
  assert(snapped_a == snapped_b);

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
