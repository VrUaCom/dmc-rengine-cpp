#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"

#include <string>

namespace dmc::rengine::integration {

[[nodiscard]] std::string executable_workspace_manifest_json(
    const ProjectWorkspace& project,
    const gdspaces::ResourceId& resource);

} // namespace dmc::rengine::integration
