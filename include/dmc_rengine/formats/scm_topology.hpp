#pragma once

#include "dmc_rengine/formats/scm_layout.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::scm {

inline constexpr std::uint8_t triangle_break_bit = 0x02U;

[[nodiscard]] std::vector<std::uint16_t> generate_triangle_strip_indices(
    std::span<const std::uint8_t> topology_flags);

} // namespace dmc::rengine::formats::scm
