#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/validation/item_runtime_plan.hpp"

#include <string>

namespace dmc::rengine::validation {

[[nodiscard]] std::string item_runtime_plan_manifest_json(
    const integration::ProjectWorkspace& project,
    const ItemRuntimeValidationPlan& plan,
    const item::RuntimeChangeRequest& request);

} // namespace dmc::rengine::validation
