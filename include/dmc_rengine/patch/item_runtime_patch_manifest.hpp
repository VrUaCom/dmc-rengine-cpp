#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/patch/item_runtime_compiler.hpp"

#include <string>

namespace dmc::rengine::patch {

[[nodiscard]] std::string item_runtime_patch_plan_manifest_json(
    const integration::ProjectWorkspace& project,
    const CompiledItemRuntimePatchPlan& compiled);

} // namespace dmc::rengine::patch
