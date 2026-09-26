#pragma once
#include "eye_runtime.hpp"
#include "physiology.hpp"
#include <cstdint>

namespace rengine::lsg {

struct CharacterInstanceState {
  std::uint32_t profile_index{};
  EyeRuntimeState eye{};
  PhysiologyPreset physiology{PhysiologyPreset::normal};
};

inline void set_instance_profile(CharacterInstanceState& instance,
                                 std::uint32_t profile_index) noexcept {
  if (instance.profile_index == profile_index) return;
  instance.profile_index = profile_index;
  instance.eye = EyeRuntimeState{};
}

} // namespace rengine::lsg
