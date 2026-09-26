#include "rengine/lsg/character_registry.hpp"

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
#include <system_error>
#include <vector>

namespace rengine::lsg {
namespace {

std::vector<std::string_view> split(std::string_view text, char delimiter) {
  std::vector<std::string_view> out;
  std::size_t start = 0;
  while (start <= text.size()) {
    const auto end = text.find(delimiter, start);
    if (end == std::string_view::npos) {
      out.push_back(text.substr(start));
      break;
    }
    out.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  return out;
}

bool parse_u32(std::string_view text, std::uint32_t& value) noexcept {
  if (text.empty()) return false;
  const char* begin = text.data();
  const char* end = begin + text.size();
  const auto result = std::from_chars(begin, end, value);
  return result.ec == std::errc{} && result.ptr == end;
}

bool parse_float(std::string_view text, float& value) {
  if (text.empty()) return false;
  std::string owned{text};
  char* end = nullptr;
  const float parsed = std::strtof(owned.c_str(), &end);
  if (end != owned.c_str() + owned.size() || !std::isfinite(parsed)) return false;
  value = parsed;
  return true;
}

bool valid_face_metadata(const CarrierFaceFieldMetadataV0& m) noexcept {
  return m.version == 1u &&
         std::isfinite(m.head_pivot_y) &&
         m.face_half_width_m > 0.0f &&
         m.face_half_height_m > 0.0f &&
         m.face_depth_m > 0.0f &&
         m.eye_center_x_abs_m > 0.0f &&
         std::isfinite(m.eye_center_y_m);
}

} // namespace

bool CharacterRegistry::parse_manifest(std::string_view text, std::string& error) {
  carriers_.clear();
  profiles_.clear();

  bool saw_header = false;
  std::size_t line_number = 0;
  while (!text.empty()) {
    const auto newline = text.find('\n');
    std::string_view line = newline == std::string_view::npos ? text : text.substr(0, newline);
    text = newline == std::string_view::npos ? std::string_view{} : text.substr(newline + 1);
    ++line_number;

    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
    if (line.empty() || line.front() == '#') continue;

    const auto fields = split(line, '|');
    if (!saw_header) {
      if (fields.size() != 2u || fields[0] != "LSGR" || fields[1] != "1") {
        error = "registry header must be LSGR|1";
        return false;
      }
      saw_header = true;
      continue;
    }

    if (fields.empty()) continue;
    if (fields[0] == "C") {
      if (fields.size() != 14u) {
        error = "carrier row has wrong field count at line " + std::to_string(line_number);
        return false;
      }
      RuntimeCarrierDefinition carrier{};
      if (!parse_u32(fields[1], carrier.id)) {
        error = "invalid carrier id at line " + std::to_string(line_number);
        return false;
      }
      carrier.key = std::string{fields[2]};
      carrier.display_name = std::string{fields[3]};
      carrier.body_asset_path = std::string{fields[4]};
      carrier.eye_asset_path = std::string{fields[5]};
      if (!parse_u32(fields[6], carrier.face_field.version) ||
          !parse_float(fields[7], carrier.face_field.head_pivot_y) ||
          !parse_float(fields[8], carrier.face_field.face_half_width_m) ||
          !parse_float(fields[9], carrier.face_field.face_half_height_m) ||
          !parse_float(fields[10], carrier.face_field.face_depth_m) ||
          !parse_float(fields[11], carrier.face_field.eye_center_x_abs_m) ||
          !parse_float(fields[12], carrier.face_field.eye_center_y_m)) {
        error = "invalid carrier face metadata at line " + std::to_string(line_number);
        return false;
      }
      std::uint32_t flags{};
      if (!parse_u32(fields[13], flags) || flags != 0u) {
        error = "unsupported carrier flags at line " + std::to_string(line_number);
        return false;
      }
      if (carrier.key.empty() || carrier.body_asset_path.empty() ||
          carrier.eye_asset_path.empty() || !valid_face_metadata(carrier.face_field)) {
        error = "invalid carrier definition at line " + std::to_string(line_number);
        return false;
      }
      if (carrier_by_id(carrier.id) != nullptr) {
        error = "duplicate carrier id at line " + std::to_string(line_number);
        return false;
      }
      carriers_.push_back(std::move(carrier));
      continue;
    }

    if (fields[0] == "P") {
      if (fields.size() != 7u) {
        error = "profile row has wrong field count at line " + std::to_string(line_number);
        return false;
      }
      RegisteredCharacterProfile profile{};
      if (!parse_u32(fields[1], profile.definition.id) ||
          !parse_u32(fields[4], profile.definition.carrier_id)) {
        error = "invalid profile/carrier id at line " + std::to_string(line_number);
        return false;
      }
      profile.definition.key = std::string{fields[2]};
      profile.definition.display_name = std::string{fields[3]};
      profile.definition.genome_asset_path = std::string{fields[5]};
      std::uint32_t flags{};
      if (!parse_u32(fields[6], flags) || flags != 0u) {
        error = "unsupported profile flags at line " + std::to_string(line_number);
        return false;
      }
      if (profile.definition.key.empty() || profile.definition.genome_asset_path.empty()) {
        error = "invalid profile definition at line " + std::to_string(line_number);
        return false;
      }
      if (profile_by_id(profile.definition.id) != nullptr) {
        error = "duplicate profile id at line " + std::to_string(line_number);
        return false;
      }
      profiles_.push_back(std::move(profile));
      continue;
    }

    error = "unknown registry row type at line " + std::to_string(line_number);
    return false;
  }

  if (!saw_header || carriers_.empty() || profiles_.empty()) {
    error = "registry has no carriers or profiles";
    return false;
  }
  for (const auto& profile : profiles_) {
    if (carrier_by_id(profile.definition.carrier_id) == nullptr) {
      error = "profile references missing carrier id " +
              std::to_string(profile.definition.carrier_id);
      return false;
    }
  }
  error.clear();
  return true;
}

bool CharacterRegistry::attach_genome(RuntimeProfileId profile_id,
                                      std::span<const std::byte> bytes,
                                      std::string& error) {
  for (auto& profile : profiles_) {
    if (profile.definition.id != profile_id) continue;
    DecodedGenome decoded{};
    if (!decode_genome(bytes, decoded, error)) return false;
    if (decoded.generator_revision != kGeneratorRevision) {
      error = "runtime registry requires current generator revision";
      return false;
    }
    profile.genome = decoded.value;
    profile.genome_loaded = true;
    error.clear();
    return true;
  }
  error = "unknown profile id";
  return false;
}

const RuntimeCarrierDefinition* CharacterRegistry::carrier_by_id(RuntimeCarrierId id) const noexcept {
  for (const auto& carrier : carriers_) if (carrier.id == id) return &carrier;
  return nullptr;
}

const RuntimeCarrierDefinition* CharacterRegistry::carrier_by_ordinal(std::size_t ordinal) const noexcept {
  return ordinal < carriers_.size() ? &carriers_[ordinal] : nullptr;
}

std::size_t CharacterRegistry::carrier_ordinal(RuntimeCarrierId id) const noexcept {
  for (std::size_t i = 0; i < carriers_.size(); ++i)
    if (carriers_[i].id == id) return i;
  return std::numeric_limits<std::size_t>::max();
}

const RegisteredCharacterProfile* CharacterRegistry::profile_by_id(RuntimeProfileId id) const noexcept {
  for (const auto& profile : profiles_) if (profile.definition.id == id) return &profile;
  return nullptr;
}

const RegisteredCharacterProfile* CharacterRegistry::profile_by_ordinal(std::size_t ordinal) const noexcept {
  return ordinal < profiles_.size() ? &profiles_[ordinal] : nullptr;
}

std::size_t CharacterRegistry::profile_ordinal(RuntimeProfileId id) const noexcept {
  for (std::size_t i = 0; i < profiles_.size(); ++i)
    if (profiles_[i].definition.id == id) return i;
  return std::numeric_limits<std::size_t>::max();
}

bool CharacterRegistry::complete() const noexcept {
  if (carriers_.empty() || profiles_.empty()) return false;
  for (const auto& profile : profiles_) if (!profile.genome_loaded) return false;
  return true;
}

} // namespace rengine::lsg
