#pragma once

#include "dmc_rengine/formats/relative_slot_container.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::rengine::formats {

using PacParseError = RelativeSlotParseError;
using PacParseResult = RelativeSlotParseResult;

class PacParser final {
public:
    // Product-side denial-of-service bound, not an original DMC3 ABI limit.
    static constexpr std::uint32_t k_max_slot_count = 1U << 20U;

    [[nodiscard]] static PacParseResult parse(
        std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats
