#include "dmc_rengine/formats/pac.hpp"

#include <array>

namespace dmc::rengine::formats {
namespace {

constexpr RelativeSlotContainerSpec pac_spec{
    .magic = {
        std::byte{'P'},
        std::byte{'A'},
        std::byte{'C'},
        std::byte{0},
    },
    .document_format = "PAC",
    .max_slot_count = PacParser::k_max_slot_count,
};

} // namespace

PacParseResult PacParser::parse(std::span<const std::byte> bytes) {
    return parse_relative_slot_container(bytes, pac_spec);
}

} // namespace dmc::rengine::formats
