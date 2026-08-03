#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"

#include <string>
#include <string_view>

namespace dmc::rengine::source {

[[nodiscard]] std::string custom_build_manifest_json(
    const integration::ProjectWorkspace& workspace,
    std::string_view custom_build_id);

} // namespace dmc::rengine::source
