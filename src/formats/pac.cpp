#include "dmc_rengine/formats/pac.hpp"

#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

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
    // The recovered walk compares three bytes and never looks at the stored
    // NUL. The fourth byte stays in the literal so the writer keeps emitting
    // it; only the comparison narrows.
    .magic_bytes = profiles::dmc3::RelativeSlotWalkContract::pac_magic_bytes,
    .document_format = "PAC",
    .max_slot_count = PacParser::k_max_slot_count,
};

} // namespace

PacParseResult PacParser::parse(std::span<const std::byte> bytes) {
    return parse_relative_slot_container(bytes, pac_spec);
}

} // namespace dmc::rengine::formats
