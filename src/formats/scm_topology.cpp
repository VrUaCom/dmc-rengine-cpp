#include "dmc_rengine/formats/scm_topology.hpp"

namespace dmc::rengine::formats::scm {

std::vector<std::uint16_t> generate_triangle_strip_indices(
    std::span<const std::uint8_t> topology_flags) {
    std::vector<std::uint16_t> indices;
    if (topology_flags.size() < 3U || topology_flags.size() > 65535U) {
        return indices;
    }

    indices.reserve(topology_flags.size() * 3U);
    std::int32_t previous = -1;
    bool inside_run = false;

    for (std::size_t index = 0; index < topology_flags.size(); ++index) {
        if (index <= 1U ||
            (topology_flags[index] & triangle_break_bit) != 0U) {
            inside_run = false;
            continue;
        }

        if (!inside_run) {
            inside_run = true;
            if (previous >= 0) {
                indices.push_back(static_cast<std::uint16_t>(previous));
                indices.push_back(static_cast<std::uint16_t>(index - 2U));
            }
            indices.push_back(static_cast<std::uint16_t>(index - 2U));
            indices.push_back(static_cast<std::uint16_t>(index - 1U));
        }

        indices.push_back(static_cast<std::uint16_t>(index));
        previous = static_cast<std::int32_t>(index);
    }
    return indices;
}

} // namespace dmc::rengine::formats::scm
