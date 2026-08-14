#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"

#include <string>
#include <string_view>

namespace dmc::rengine::integration {

// Schema v2 exports resource_set_id separately from optional semantic_stage_id.
[[nodiscard]] std::string stage_workspace_manifest_json(
    const ProjectWorkspace& project,
    std::string_view resource_set_id);

} // namespace dmc::rengine::integration
