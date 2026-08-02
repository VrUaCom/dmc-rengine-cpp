#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"

#include <string>
#include <string_view>

namespace dmc::rengine::integration {

[[nodiscard]] std::string stage_workspace_manifest_json(
    const ProjectWorkspace& project,
    std::string_view stage_id);

} // namespace dmc::rengine::integration
