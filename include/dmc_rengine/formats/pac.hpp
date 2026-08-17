#pragma once

#include "dmc_rengine/formats/container.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace dmc::rengine::formats {

enum class PacParseError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    slot_count_limit,
    truncated_offset_table,
    slot_offset_before_payload,
    slot_offset_out_of_bounds,
    invalid_document,
};

struct PacParseResult final {
    std::optional<ContainerDocument> document;
    PacParseError error{PacParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == PacParseError::none;
    }
};

class PacParser final {
public:
    // Product-side denial-of-service bound, not an original DMC3 ABI limit.
    static constexpr std::uint32_t k_max_slot_count = 1U << 20U;

    [[nodiscard]] static PacParseResult parse(
        std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats
