#pragma once

#include "dmc_rengine/formats/relative_slot_container.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::rengine::formats {

using PnstParseError = RelativeSlotParseError;
using PnstParseResult = RelativeSlotParseResult;

class PnstParser final {
public:
    // Product-side denial-of-service bound shared with PAC because both use
    // the currently evidenced relative-slot envelope. Not a game ABI limit.
    static constexpr std::uint32_t k_max_slot_count =
        k_relative_slot_max_slot_count;

    [[nodiscard]] static PnstParseResult parse(
        std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats
