#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

namespace dmc::rengine::profiles::dmc3 {

const exe::KnownExecutableTarget& phase12_canonical_target() noexcept {
    static const exe::KnownExecutableTarget target{
        .id = "dmc3-hdc-phase12-canonical-target",
        .display_name = "Devil May Cry 3 HD canonical Phase 12 target",
        .sha256 = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .kind = exe::PeKind::pe32_plus,
        .machine = exe::PeMachine::amd64,
        .image_base = 0x140000000ULL,
        .entry_point_rva = 0x0034615CU,
    };
    return target;
}

} // namespace dmc::rengine::profiles::dmc3
