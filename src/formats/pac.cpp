#include "dmc_rengine/formats/pac.hpp"

#include <string_view>
#include <utility>

namespace dmc::rengine::formats {
namespace {

[[nodiscard]] PacParseError to_pac_error(
    RelativeSlotParseError error) noexcept {
    switch (error) {
    case RelativeSlotParseError::none: return PacParseError::none;
    case RelativeSlotParseError::truncated_header:
        return PacParseError::truncated_header;
    case RelativeSlotParseError::invalid_magic:
        return PacParseError::invalid_magic;
    case RelativeSlotParseError::slot_count_limit:
        return PacParseError::slot_count_limit;
    case RelativeSlotParseError::truncated_offset_table:
        return PacParseError::truncated_offset_table;
    case RelativeSlotParseError::slot_offset_before_payload:
        return PacParseError::slot_offset_before_payload;
    case RelativeSlotParseError::slot_offset_out_of_bounds:
        return PacParseError::slot_offset_out_of_bounds;
    case RelativeSlotParseError::invalid_document:
        return PacParseError::invalid_document;
    }
    return PacParseError::invalid_document;
}

constexpr RelativeSlotContainerSpec pac_spec{
    .format = "PAC",
    .magic = std::string_view{"PAC\0", 4U},
    .display_label = "PAC",
};

} // namespace

PacParseResult PacParser::parse(std::span<const std::byte> bytes) {
    auto result = parse_relative_slot_container(bytes, pac_spec);
    return PacParseResult{
        .document = std::move(result.document),
        .error = to_pac_error(result.error),
        .message = std::move(result.message),
    };
}

} // namespace dmc::rengine::formats
