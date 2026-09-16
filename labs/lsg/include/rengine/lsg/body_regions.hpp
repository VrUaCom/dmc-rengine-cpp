#pragma once
#include <cstdint>

namespace rengine::lsg {
enum class BodyRegion : std::uint8_t {
  head=0, neck, chest, back, abdomen, upper_arm, forearm, hand,
  thigh, lower_leg, foot, unknown=255
};
}
