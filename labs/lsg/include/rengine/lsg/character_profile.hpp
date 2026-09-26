#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace rengine::lsg {

enum class CarrierId : std::uint8_t {
  male_base = 0,
  female_base = 1,
};

inline constexpr std::size_t kBuiltinCarrierCount = 2;
inline constexpr std::uint32_t kBuiltinProfileCount = 3;
inline constexpr std::uint32_t kAdaProfileIndex = 2;

struct CarrierDefinition {
  CarrierId id{};
  std::string_view key{};
  std::string_view display_name{};
  std::string_view body_asset_path{};
  std::string_view eye_asset_path{};
};

struct CharacterProfileDefinition {
  std::uint32_t index{};
  std::string_view key{};
  std::string_view display_name{};
  CarrierId carrier{CarrierId::male_base};
};

[[nodiscard]] std::span<const CarrierDefinition> builtin_carriers() noexcept;
[[nodiscard]] std::span<const CharacterProfileDefinition> builtin_character_profiles() noexcept;
[[nodiscard]] const CarrierDefinition& carrier_definition(CarrierId id) noexcept;
[[nodiscard]] const CharacterProfileDefinition& character_profile_definition(std::uint32_t index) noexcept;

[[nodiscard]] constexpr std::size_t carrier_slot(CarrierId id) noexcept {
  return id == CarrierId::female_base ? 1u : 0u;
}

[[nodiscard]] constexpr std::uint32_t normalize_character_profile_index(std::uint32_t index) noexcept {
  return index % kBuiltinProfileCount;
}

} // namespace rengine::lsg
