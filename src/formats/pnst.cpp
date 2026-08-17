#include "dmc_rengine/formats/pnst.hpp"

#include <string_view>

namespace dmc::rengine::formats {
namespace {

constexpr RelativeSlotContainerSpec pnst_spec{
    .format = "PNST",
    .magic = std::string_view{"PNST", 4U},
    .display_label = "PNST",
};

} // namespace

PnstParseResult PnstParser::parse(std::span<const std::byte> bytes) {
    return parse_relative_slot_container(bytes, pnst_spec);
}

} // namespace dmc::rengine::formats
