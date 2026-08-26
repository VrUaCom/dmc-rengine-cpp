#include "dmc_rengine/formats/pnst.hpp"

#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

#include <array>

namespace dmc::rengine::formats {
namespace {

constexpr RelativeSlotContainerSpec pnst_spec{
    .magic = {
        std::byte{'P'},
        std::byte{'N'},
        std::byte{'S'},
        std::byte{'T'},
    },
    .magic_bytes = profiles::dmc3::RelativeSlotWalkContract::pnst_magic_bytes,
    .document_format = "PNST",
    .max_slot_count = PnstParser::k_max_slot_count,
};

} // namespace

PnstParseResult PnstParser::parse(std::span<const std::byte> bytes) {
    return parse_relative_slot_container(bytes, pnst_spec);
}

} // namespace dmc::rengine::formats
