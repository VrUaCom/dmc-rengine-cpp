#include "dmc_rengine/profiles/dmc3/vanilla_dependency_runtime.hpp"

#include <algorithm>
#include <iterator>

namespace dmc::rengine::profiles::dmc3::vanilla {

bool observed_stage_load_transition(
    StageLoadPhase from,
    StageLoadPhase to) noexcept {
    const auto from_it = std::find(
        observed_stage_load_order.begin(),
        observed_stage_load_order.end(),
        from);
    if (from_it == observed_stage_load_order.end()) {
        return false;
    }

    const auto next = std::next(from_it);
    return next != observed_stage_load_order.end() && *next == to;
}

} // namespace dmc::rengine::profiles::dmc3::vanilla
