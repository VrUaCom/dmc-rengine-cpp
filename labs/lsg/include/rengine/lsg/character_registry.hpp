#pragma once

#include "character_profile.hpp"
#include "genome.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace rengine::lsg {

using RuntimeCarrierId = std::uint32_t;
using RuntimeProfileId = std::uint32_t;
inline constexpr RuntimeCarrierId kInvalidRuntimeCarrierId = 0xFFFFFFFFu;
inline constexpr RuntimeProfileId kInvalidRuntimeProfileId = 0xFFFFFFFFu;

struct RuntimeCarrierDefinition {
  RuntimeCarrierId id{kInvalidRuntimeCarrierId};
  std::string key{};
  std::string display_name{};
  std::string body_asset_path{};
  std::string eye_asset_path{};
  CarrierFaceFieldMetadataV0 face_field{};
};

struct RuntimeProfileDefinition {
  RuntimeProfileId id{kInvalidRuntimeProfileId};
  std::string key{};
  std::string display_name{};
  RuntimeCarrierId carrier_id{kInvalidRuntimeCarrierId};
  std::string genome_asset_path{};
};

struct RegisteredCharacterProfile {
  RuntimeProfileDefinition definition{};
  CharacterGenomeV0 genome{};
  bool genome_loaded{};
};

class CharacterRegistry {
public:
  [[nodiscard]] bool parse_manifest(std::string_view text, std::string& error);
  [[nodiscard]] bool attach_genome(RuntimeProfileId profile_id,
                                   std::span<const std::byte> bytes,
                                   std::string& error);

  [[nodiscard]] std::size_t carrier_count() const noexcept { return carriers_.size(); }
  [[nodiscard]] std::size_t profile_count() const noexcept { return profiles_.size(); }

  [[nodiscard]] const RuntimeCarrierDefinition* carrier_by_id(RuntimeCarrierId id) const noexcept;
  [[nodiscard]] const RuntimeCarrierDefinition* carrier_by_ordinal(std::size_t ordinal) const noexcept;
  [[nodiscard]] std::size_t carrier_ordinal(RuntimeCarrierId id) const noexcept;

  [[nodiscard]] const RegisteredCharacterProfile* profile_by_id(RuntimeProfileId id) const noexcept;
  [[nodiscard]] const RegisteredCharacterProfile* profile_by_ordinal(std::size_t ordinal) const noexcept;
  [[nodiscard]] std::size_t profile_ordinal(RuntimeProfileId id) const noexcept;

  [[nodiscard]] bool complete() const noexcept;

private:
  std::vector<RuntimeCarrierDefinition> carriers_;
  std::vector<RegisteredCharacterProfile> profiles_;
};

} // namespace rengine::lsg
