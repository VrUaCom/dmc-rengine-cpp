#pragma once

#include "dmc_rengine/profiles/dmc3/stage_runtime_loader.hpp"
#include "dmc_rengine/stageops/assembly_workspace.hpp"

namespace dmc::rengine::profiles::dmc3 {

// Profile adapter: converts the existing DMC3 GDSpaces-backed materialization
// report into the generic Stage Ops aggregate. Dependency direction is
// profile -> Stage Ops core; generic Stage Ops does not depend on DMC3 runtime
// types or executable coordinates.
class StageOpsAssembler final {
public:
    [[nodiscard]] static stageops::StageAssemblyWorkspace assemble(
        const StageRuntimeLoadReport& report);
};

} // namespace dmc::rengine::profiles::dmc3
