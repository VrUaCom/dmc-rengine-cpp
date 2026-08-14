#pragma once

#include "dmc_rengine/profiles/dmc3/stage_runtime_loader.hpp"
#include "dmc_rengine/stageops/assembly_workspace.hpp"

namespace dmc::rengine::stageops {

// DMC3 profile adapter that turns the existing GDSpaces-backed materialization
// report into the canonical Stage Ops operational workspace. It does not perform
// resource lookup itself and does not claim original-game readiness beyond the
// explicit gate carried by StageRuntimeLoadReport.
class DMC3StageAssembler final {
public:
    [[nodiscard]] static StageAssemblyWorkspace assemble(
        const profiles::dmc3::StageRuntimeLoadReport& report);
};

} // namespace dmc::rengine::stageops
