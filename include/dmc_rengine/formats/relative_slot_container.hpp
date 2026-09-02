#pragma once

#include "dmc_rengine/formats/container.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::formats {

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

struct RelativeSlotParseResult final {
    std::optional<ContainerDocument> document;
    RelativeSlotParseError error{RelativeSlotParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == RelativeSlotParseError::none;
    }
};

struct RelativeSlotContainerSpec final {
    std::array<std::byte, 4> magic{};
    // How many of those four bytes the original runtime actually compares.
    // It is three for PAC and four for PNST — see RelativeSlotWalkContract.
    // Comparing more than the runtime does makes a reader stricter than the
    // game, which is a real difference, not a harmless extra check.
    std::size_t magic_bytes{4U};
    std::string_view document_format;
    std::uint32_t max_slot_count{1U << 20U};
};

[[nodiscard]] RelativeSlotParseResult parse_relative_slot_container(
    std::span<const std::byte> bytes,
    const RelativeSlotContainerSpec& spec);

} // namespace dmc::rengine::formats
