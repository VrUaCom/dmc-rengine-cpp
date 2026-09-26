#include "rengine/lsg/genome.hpp"

#include <nlohmann/json.hpp>

#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

namespace {
using json = nlohmann::json;
using rengine::lsg::CharacterGenomeV0;

[[noreturn]] void fail(const std::string& message) { throw std::runtime_error(message); }

void require_keys_only(const json& object, std::initializer_list<std::string_view> allowed,
                       std::string_view path) {
  if (!object.is_object()) fail(std::string(path) + " must be an object");
  for (const auto& [key, value] : object.items()) {
    (void)value;
    bool known = false;
    for (const auto candidate : allowed) if (key == candidate) { known = true; break; }
    if (!known) fail(std::string(path) + ": unknown field '" + key + "'");
  }
}

template <class T>
T required_integer(const json& object, const char* key, std::int64_t minimum, std::uint64_t maximum,
                   std::string_view path) {
  if (!object.contains(key)) fail(std::string(path) + ": missing field '" + key + "'");
  const auto& value = object.at(key);
  if (value.is_number_unsigned()) {
    const auto v = value.get<std::uint64_t>();
    if (v > maximum) fail(std::string(path) + "." + key + " out of range");
    return static_cast<T>(v);
  }
  if (value.is_number_integer()) {
    const auto v = value.get<std::int64_t>();
    if (v < minimum || (v >= 0 && static_cast<std::uint64_t>(v) > maximum))
      fail(std::string(path) + "." + key + " out of range");
    return static_cast<T>(v);
  }
  fail(std::string(path) + "." + key + " must be an integer");
}

std::uint64_t parse_seed(const json& root, const char* key) {
  if (!root.contains(key)) fail(std::string("root: missing field '") + key + "'");
  const auto& value = root.at(key);
  if (value.is_number_unsigned()) return value.get<std::uint64_t>();
  if (value.is_number_integer()) {
    const auto signed_value = value.get<std::int64_t>();
    if (signed_value < 0) fail(std::string(key) + " must be non-negative");
    return static_cast<std::uint64_t>(signed_value);
  }
  if (!value.is_string()) fail(std::string(key) + " must be an integer or numeric string");
  std::string text = value.get<std::string>();
  int base = 10;
  std::string_view digits{text};
  if (digits.size() >= 2 && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X')) {
    base = 16; digits.remove_prefix(2);
  }
  if (digits.empty()) fail(std::string(key) + " has an empty numeric string");
  std::uint64_t result{};
  const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), result, base);
  if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size())
    fail(std::string(key) + " is not a valid uint64 value");
  return result;
}

CharacterGenomeV0 parse_profile(const json& root) {
  require_keys_only(root,
                    {"generator_revision", "flags", "geometry", "face", "skin", "eyes", "micro", "physiology",
                     "identity_seed", "surface_seed", "eye_seed"},
                    "root");
  if (!root.contains("generator_revision")) fail("root: missing field 'generator_revision'");
  const auto revision = required_integer<std::uint32_t>(root, "generator_revision", 0,
                                                         std::numeric_limits<std::uint32_t>::max(), "root");
  if (revision != rengine::lsg::kGeneratorRevision &&
      revision != rengine::lsg::kLegacyGeneratorRevision)
    fail("generator_revision is not supported by this lsg_compile build");
  if (root.contains("flags"))
    (void)required_integer<std::uint32_t>(root, "flags", 0, std::numeric_limits<std::uint32_t>::max(), "root");

  const auto& geometry = root.at("geometry");
  require_keys_only(geometry,
                    {"height", "shoulder_width", "pelvis_width", "chest_volume", "waist", "limb_length",
                     "muscle", "body_fat", "neck", "head_scale", "jaw", "facial_softness"},
                    "geometry");
  const auto i16 = [&](const char* key) {
    return required_integer<std::int16_t>(geometry, key, std::numeric_limits<std::int16_t>::min(),
                                          static_cast<std::uint64_t>(std::numeric_limits<std::int16_t>::max()),
                                          "geometry");
  };

  rengine::lsg::FaceGenomeV0 face{};
  if (revision == rengine::lsg::kGeneratorRevision && !root.contains("face"))
    fail("root.face is mandatory for current generator revision");
  if (revision == rengine::lsg::kLegacyGeneratorRevision && root.contains("face"))
    fail("legacy generator revision must not define Face DNA");
  if (root.contains("face")) {
    const auto& face_json = root.at("face");
    require_keys_only(face_json,
      {"skull_width","skull_height","face_length","forehead_height","brow_depth",
       "eye_spacing","eye_size","eye_tilt","nose_length","nose_width","nose_projection",
       "cheekbone_width","cheek_fullness","jaw_width","chin_width","chin_projection",
       "mouth_width","upper_lip_fullness","lower_lip_fullness","lip_projection"}, "face");
    const auto face_i16 = [&](const char* key) {
      return required_integer<std::int16_t>(face_json, key, std::numeric_limits<std::int16_t>::min(),
        static_cast<std::uint64_t>(std::numeric_limits<std::int16_t>::max()), "face");
    };
    face = {face_i16("skull_width"),face_i16("skull_height"),face_i16("face_length"),face_i16("forehead_height"),
            face_i16("brow_depth"),face_i16("eye_spacing"),face_i16("eye_size"),face_i16("eye_tilt"),
            face_i16("nose_length"),face_i16("nose_width"),face_i16("nose_projection"),face_i16("cheekbone_width"),
            face_i16("cheek_fullness"),face_i16("jaw_width"),face_i16("chin_width"),face_i16("chin_projection"),
            face_i16("mouth_width"),face_i16("upper_lip_fullness"),face_i16("lower_lip_fullness"),face_i16("lip_projection")};
  }

  const auto& skin = root.at("skin");
  require_keys_only(skin,
                    {"melanin", "haemoglobin", "carotene", "oiliness", "hydration", "roughness_bias",
                     "pore_density", "pore_scale", "pore_depth", "follicle_density", "freckle_density",
                     "age_profile"},
                    "skin");
  const auto skin_u8 = [&](const char* key) { return required_integer<std::uint8_t>(skin, key, 0, 255, "skin"); };

  const auto& eyes = root.at("eyes");
  require_keys_only(eyes,
                    {"iris_r", "iris_g", "iris_b", "iris2_r", "iris2_g", "iris2_b", "pupil_bias",
                     "sclera_tint", "vascularity", "reserved"},
                    "eyes");
  const auto eye_u8 = [&](const char* key) { return required_integer<std::uint8_t>(eyes, key, 0, 255, "eyes"); };

  const auto& micro = root.at("micro");
  require_keys_only(micro, {"meso_strength", "micro_strength", "wrinkle_bias", "reserved"}, "micro");
  const auto micro_u8 = [&](const char* key) { return required_integer<std::uint8_t>(micro, key, 0, 255, "micro"); };

  const auto& physiology = root.at("physiology");
  require_keys_only(physiology, {"resting_pulse", "perfusion", "sweat_bias", "temperature_bias"}, "physiology");
  const auto phys_u8 = [&](const char* key) { return required_integer<std::uint8_t>(physiology, key, 0, 255, "physiology"); };

  CharacterGenomeV0 genome{};
  genome.geometry = {i16("height"), i16("shoulder_width"), i16("pelvis_width"), i16("chest_volume"),
                     i16("waist"), i16("limb_length"), i16("muscle"), i16("body_fat"), i16("neck"),
                     i16("head_scale"), i16("jaw"), i16("facial_softness")};
  if (revision == rengine::lsg::kGeneratorRevision &&
      (genome.geometry.jaw != 0 || genome.geometry.facial_softness != 0))
    fail("geometry.jaw and geometry.facial_softness are legacy revision-2 slots; use Face DNA in revision 3");
  genome.face = face;
  genome.skin = {skin_u8("melanin"), skin_u8("haemoglobin"), skin_u8("carotene"), skin_u8("oiliness"),
                 skin_u8("hydration"), skin_u8("roughness_bias"), skin_u8("pore_density"),
                 skin_u8("pore_scale"), skin_u8("pore_depth"), skin_u8("follicle_density"),
                 skin_u8("freckle_density"), skin_u8("age_profile")};
  genome.eyes = {eye_u8("iris_r"), eye_u8("iris_g"), eye_u8("iris_b"), eye_u8("iris2_r"),
                 eye_u8("iris2_g"), eye_u8("iris2_b"), eye_u8("pupil_bias"), eye_u8("sclera_tint"),
                 eye_u8("vascularity"), eye_u8("reserved")};
  genome.micro = {micro_u8("meso_strength"), micro_u8("micro_strength"), micro_u8("wrinkle_bias"),
                  micro_u8("reserved")};
  genome.physiology = {phys_u8("resting_pulse"), phys_u8("perfusion"), phys_u8("sweat_bias"),
                       phys_u8("temperature_bias")};
  genome.identity_seed = parse_seed(root, "identity_seed");
  genome.surface_seed = parse_seed(root, "surface_seed");
  genome.eye_seed = parse_seed(root, "eye_seed");
  return genome;
}

std::uint32_t parse_flags(const json& root) {
  if (!root.contains("flags")) return 0;
  return required_integer<std::uint32_t>(root, "flags", 0, std::numeric_limits<std::uint32_t>::max(), "root");
}
} // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "usage: lsg_compile <input.lsg.json> <output.lsg>\n";
    return 2;
  }
  try {
    std::ifstream input(argv[1]);
    if (!input) fail(std::string("cannot open input: ") + argv[1]);
    json root;
    input >> root;
    const auto genome = parse_profile(root);
    const auto flags = parse_flags(root);
    const auto bytes = rengine::lsg::encode_genome(genome, rengine::lsg::kGeneratorRevision, flags);
    if (bytes.empty()) fail("genome encoder rejected the compiled profile");

    rengine::lsg::DecodedGenome verify{};
    std::string error;
    if (!rengine::lsg::decode_genome(bytes, verify, error)) fail("self-verification failed: " + error);

    std::ofstream output(argv[2], std::ios::binary | std::ios::trunc);
    if (!output) fail(std::string("cannot open output: ") + argv[2]);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!output) fail("failed while writing output genome");

    std::cout << "LSG compile PASS: revision=" << verify.generator_revision << " bytes=" << bytes.size()
              << " input=" << argv[1] << " output=" << argv[2] << "\n";
    return 0;
  } catch (const std::exception& exception) {
    std::cerr << "LSG compile FAIL: " << exception.what() << "\n";
    return 1;
  }
}
