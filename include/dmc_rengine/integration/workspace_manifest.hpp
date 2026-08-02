#pragma once

#include "dmc_rengine/integration/resource_workspace.hpp"

#include <string>

namespace dmc::rengine::integration {

[[nodiscard]] std::string workspace_manifest_json(
    const ResourceWorkspaceSession& workspace);

} // namespace dmc::rengine::integration
