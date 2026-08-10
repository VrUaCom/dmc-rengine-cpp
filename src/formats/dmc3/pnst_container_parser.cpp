#include "dmc_rengine/formats/dmc3/pnst_container_parser.hpp"

#include "dmc_rengine/formats/dmc3/relative_slot_container.hpp"

namespace dmc::rengine::formats::dmc3 {
namespace {

constexpr std::string_view pnst_magic{"PNST", 4U};
constexpr RelativeSlotContainerSpec pnst_spec{
    .format = "pnst",
    .magic = pnst_magic,
    .diagnostic_prefix = "dmc3.pnst",
    .display_label = "PNST",
};

} // namespace

std::string_view PnstContainerParser::id() const noexcept {
    return "dmc3-pnst-runtime-v1";
}

std::string_view PnstContainerParser::format() const noexcept {
    return "pnst";
}

int PnstContainerParser::probe(
    std::span<const std::byte> bytes,
    std::string_view) const noexcept {
    return probe_relative_slot_container(bytes, pnst_spec);
}

ContainerParseResult PnstContainerParser::parse(
    std::span<const std::byte> bytes,
    std::string_view) const {
    return parse_relative_slot_container(bytes, pnst_spec);
}

} // namespace dmc::rengine::formats::dmc3
