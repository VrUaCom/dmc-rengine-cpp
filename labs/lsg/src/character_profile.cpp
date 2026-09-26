#include "rengine/lsg/character_profile.hpp"

#include <array>

namespace rengine::lsg {
namespace {

constexpr std::array<CarrierDefinition, kBuiltinCarrierCount> kCarriers{{
    {CarrierId::male_base, "male-base", "Male Base",
     "meshes/human_carrier_male.rmesh", "meshes/eye_carrier_male.rmesh"},
    {CarrierId::female_base, "female-base", "Female Base",
     "meshes/human_carrier_female.rmesh", "meshes/eye_carrier_female.rmesh"},
}};

constexpr std::array<CharacterProfileDefinition, kBuiltinProfileCount> kProfiles{{
    {0u, "male-base", "Male Base", CarrierId::male_base},
    {1u, "female-base", "Female Base", CarrierId::female_base},
    {kAdaProfileIndex, "ada-reference", "Ada", CarrierId::female_base},
}};

static_assert(kProfiles[0].index == 0u);
static_assert(kProfiles[1].index == 1u);
static_assert(kProfiles[2].index == kAdaProfileIndex);
static_assert(kProfiles[2].carrier == CarrierId::female_base);

} // namespace

std::span<const CarrierDefinition> builtin_carriers() noexcept {
  return kCarriers;
}

std::span<const CharacterProfileDefinition> builtin_character_profiles() noexcept {
  return kProfiles;
}

const CarrierDefinition& carrier_definition(CarrierId id) noexcept {
  return kCarriers[carrier_slot(id)];
}

const CharacterProfileDefinition& character_profile_definition(std::uint32_t index) noexcept {
  return kProfiles[normalize_character_profile_index(index)];
}

} // namespace rengine::lsg
