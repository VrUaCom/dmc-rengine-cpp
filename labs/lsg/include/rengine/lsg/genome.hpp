#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace rengine::lsg {
inline constexpr std::uint16_t kGenomeVersion = 0;
inline constexpr std::uint32_t kGeneratorRevision = 3;
inline constexpr std::uint32_t kLegacyGeneratorRevision = 2;
inline constexpr std::size_t kGenomeHardLimit = 4096;

struct GeometryGenome {
  std::int16_t height{}; std::int16_t shoulder_width{}; std::int16_t pelvis_width{};
  std::int16_t chest_volume{}; std::int16_t waist{}; std::int16_t limb_length{};
  std::int16_t muscle{}; std::int16_t body_fat{}; std::int16_t neck{};
  std::int16_t head_scale{};
  // Revision-3 ownership: FaceGenomeV0 owns facial identity. These two slots remain only
  // for revision-2 binary migration and MUST be zero in current authoring.
  std::int16_t jaw{}; std::int16_t facial_softness{};
};
struct FaceGenomeV0 {
  std::int16_t skull_width{}, skull_height{}, face_length{}, forehead_height{};
  std::int16_t brow_depth{}, eye_spacing{}, eye_size{}, eye_tilt{};
  std::int16_t nose_length{}, nose_width{}, nose_projection{}, cheekbone_width{};
  std::int16_t cheek_fullness{}, jaw_width{}, chin_width{}, chin_projection{};
  std::int16_t mouth_width{}, upper_lip_fullness{}, lower_lip_fullness{}, lip_projection{};
};
struct SkinGenome {
  std::uint8_t melanin{}, haemoglobin{}, carotene{}, oiliness{}, hydration{}, roughness_bias{};
  std::uint8_t pore_density{}, pore_scale{}, pore_depth{}, follicle_density{}, freckle_density{}, age_profile{};
};
struct EyeGenome {
  std::uint8_t iris_r{}, iris_g{}, iris_b{}, iris2_r{}, iris2_g{}, iris2_b{};
  std::uint8_t pupil_bias{}, sclera_tint{}, vascularity{}, reserved{};
};
struct MicroDetailGenome { std::uint8_t meso_strength{}, micro_strength{}, wrinkle_bias{}, reserved{}; };
struct PhysiologyGenome { std::uint8_t resting_pulse{}, perfusion{}, sweat_bias{}, temperature_bias{}; };

struct CharacterGenomeV0 {
  GeometryGenome geometry{};
  FaceGenomeV0 face{};
  SkinGenome skin{};
  EyeGenome eyes{};
  MicroDetailGenome micro{};
  PhysiologyGenome physiology{};
  std::uint64_t identity_seed{};
  std::uint64_t surface_seed{};
  std::uint64_t eye_seed{};
};

struct DecodedGenome { CharacterGenomeV0 value{}; std::uint32_t generator_revision{}; std::uint32_t flags{}; };

[[nodiscard]] std::vector<std::byte> encode_genome(const CharacterGenomeV0& genome,
                                                   std::uint32_t generator_revision = kGeneratorRevision,
                                                   std::uint32_t flags = 0);
[[nodiscard]] bool decode_genome(std::span<const std::byte> bytes, DecodedGenome& out, std::string& error);
[[nodiscard]] constexpr float decode_snorm16(std::int16_t v) noexcept {
  return v < 0 ? static_cast<float>(v) / 32768.0f : static_cast<float>(v) / 32767.0f;
}
}
