#pragma once

#include "dmc_rengine/formats/container.hpp"

#include <cstddef>
#include <span>
#include <string_view>

namespace dmc::rengine::formats::dmc3 {

struct RelativeSlotContainerSpec final {
    std::string_view format;
    std::string_view magic;
    std::string_view diagnostic_prefix;
    std::string_view display_label;
};

[[nodiscard]] int probe_relative_slot_container(
    std::span<const std::byte> bytes,
    const RelativeSlotContainerSpec& spec) noexcept;

[[nodiscard]] ContainerParseResult parse_relative_slot_container(
    std::span<const std::byte> bytes,
    const RelativeSlotContainerSpec& spec);

} // namespace dmc::rengine::formats::dmc3
