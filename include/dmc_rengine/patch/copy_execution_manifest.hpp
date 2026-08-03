#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/patch/copy_executor.hpp"

#include <string>

namespace dmc::rengine::patch {

[[nodiscard]] std::string copy_execution_manifest_json(
    const integration::ProjectWorkspace& project,
    const PatchCopyArtifact& artifact);

} // namespace dmc::rengine::patch
