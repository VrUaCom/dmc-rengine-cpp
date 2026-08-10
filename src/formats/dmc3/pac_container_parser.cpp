#include "dmc_rengine/formats/dmc3/pac_container_parser.hpp"

#include "dmc_rengine/formats/dmc3/relative_slot_container.hpp"

namespace dmc::rengine::formats::dmc3 {
namespace {

constexpr std::string_view pac_magic{"PAC\0", 4U};
constexpr RelativeSlotContainerSpec pac_spec{
    .format = "pac",
    .magic = pac_magic,
    .diagnostic_prefix = "dmc3.pac",
    .display_label = "PAC\\0",
};

} // namespace

std::string_view PacContainerParser::id() const noexcept {
    return "dmc3-pac-runtime-v1";
}

std::string_view PacContainerParser::format() const noexcept {
    return "pac";
}

int PacContainerParser::probe(
    std::span<const std::byte> bytes,
    std::string_view) const noexcept {
    return probe_relative_slot_container(bytes, pac_spec);
}

ContainerParseResult PacContainerParser::parse(
    std::span<const std::byte> bytes,
    std::string_view) const {
    return parse_relative_slot_container(bytes, pac_spec);
}

} // namespace dmc::rengine::formats::dmc3
