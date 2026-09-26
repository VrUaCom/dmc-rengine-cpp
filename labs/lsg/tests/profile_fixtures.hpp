#pragma once
#include "rengine/lsg/character_profile.hpp"
#include "rengine/lsg/genome.hpp"

#include <cstdint>

namespace rengine::lsg::test_fixture {

inline constexpr std::uint32_t kProfileCount = 3u;
inline constexpr std::uint32_t kCarrierCount = 2u;
inline constexpr std::uint32_t kAdaOrdinal = 2u;

inline CarrierFaceFieldMetadataV0 carrier_face(bool female) {
  (void)female;
  return {1u, 0.690f, 0.100f, 0.180f, 0.120f, 0.032f, 0.752f};
}

inline CharacterGenomeV0 profile(std::uint32_t i) {
  CharacterGenomeV0 g{};
  switch (i) {
    case 0u:
      g.geometry={3200,19000,-9000,8500,-2200,1800,11000,1200,2300,0,0,0};
      g.face={};
      g.skin={82,112,64,118,140,136,152,118,92,126,32,52};
      g.eyes={42,34,25,78,54,31,118,112,64,0};
      g.micro={146,170,48,0}; g.physiology={62,116,26,128};
      g.identity_seed=0xA771E3D52C09ull; g.surface_seed=0xC0FFEE1234ull; g.eye_seed=0x123456789ABCull;
      break;
    case 1u:
      g.geometry={-1200,-6200,13100,1700,-1400,400,1200,4700,-900,-700,0,0};
      g.face={};
      g.skin={104,126,72,86,154,148,132,92,70,98,58,48};
      g.eyes={78,92,74,36,54,44,126,118,52,0};
      g.micro={132,148,40,0}; g.physiology={64,122,22,128};
      g.identity_seed=0xB55D001234ull; g.surface_seed=0xDEADBEEF42ull; g.eye_seed=0xCAFEBABE77ull;
      break;
    default:
      g.geometry={-900,-5200,11800,2300,-1800,500,900,4300,-800,-500,0,0};
      g.face={-3000,4000,2500,1500,1500,-1000,4500,5000,1500,-5500,2500,5000,2000,-5000,-3000,2500,2500,6000,7500,2500};
      g.skin={100,124,74,90,156,146,130,90,68,96,52,44};
      g.eyes={82,92,70,38,52,40,124,118,48,0};
      g.micro={134,150,38,0}; g.physiology={64,122,22,128};
      g.identity_seed=0xADA220260926ull; g.surface_seed=0xA5DAFACE2401ull; g.eye_seed=0xE1EADA260926ull;
      break;
  }
  return g;
}

inline constexpr const char* kRegistryManifest =
    "LSGR|1\n"
    "C|0|male-base|Male Base|meshes/human_carrier_male.rmesh|meshes/eye_carrier_male.rmesh|1|0.690|0.100|0.180|0.120|0.032|0.752|0\n"
    "C|1|female-base|Female Base|meshes/human_carrier_female.rmesh|meshes/eye_carrier_female.rmesh|1|0.690|0.100|0.180|0.120|0.032|0.752|0\n"
    "P|0|male-base|Male Base|0|profiles/character_0.lsg|0\n"
    "P|1|female-base|Female Base|1|profiles/character_1.lsg|0\n"
    "P|2|ada-reference|Ada|1|profiles/character_2.lsg|0\n";

} // namespace rengine::lsg::test_fixture
