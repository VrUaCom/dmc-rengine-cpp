#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/item/runtime_request.hpp"

#include <span>
#include <string>

namespace dmc::rengine::item {

[[nodiscard]] std::string workspace_manifest_json(
    const integration::ProjectWorkspace& project,
    const gdspaces::ResourceId& item_resource,
    std::span<const RuntimeChangeRequest> requests = {});

} // namespace dmc::rengine::item
