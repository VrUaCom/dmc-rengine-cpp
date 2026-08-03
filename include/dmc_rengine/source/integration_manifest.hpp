#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"

#include <string>
#include <string_view>
#include <tuple>

namespace dmc::rengine::source {

[[nodiscard]] std::string integration_project_manifest_json(
    const integration::ProjectWorkspace& workspace,
    std::string_view project_id);

} // namespace dmc::rengine::source
