#pragma once

#include "dmc_rengine/formats/container.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::formats {

// Product-side denial-of-service bound shared by the currently evidenced
// PAC/PNST relative-slot envelope. This is not an original DMC3 ABI limit.
inline constexpr std::uint32_t k_relative_slot_max_slot_count = 1U << 20U;

enum class RelativeSlotParseError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    slot_count_limit,
    truncated_offset_table,
    slot_offset_before_payload,
    slot_offset_out_of_bounds,
    invalid_document,
};

struct RelativeSlotContainerSpec final {
    std::string_view format;
    std::string_view magic;
    std::string_view display_label;
};

struct RelativeSlotParseResult final {
    std::optional<ContainerDocument> document;
    RelativeSlotParseError error{RelativeSlotParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == RelativeSlotParseError::none;
    }
};

[[nodiscard]] RelativeSlotParseResult parse_relative_slot_container(
    std::span<const std::byte> bytes,
    const RelativeSlotContainerSpec& spec);

} // namespace dmc::rengine::formats
