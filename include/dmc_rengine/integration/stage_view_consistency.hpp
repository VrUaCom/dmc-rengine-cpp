#pragma once

#include "dmc_rengine/modviz/scene_workspace_view.hpp"
#include "dmc_rengine/stageops/workspace_view.hpp"

#include <string>
#include <vector>

namespace dmc::rengine::integration {

struct StageViewConsistencyIssue final {
    std::string code;
    std::string resource_id;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

struct StageViewConsistencyReport final {
    std::vector<StageViewConsistencyIssue> issues;

    [[nodiscard]] bool consistent() const noexcept {
        return issues.empty();
    }
};

[[nodiscard]] StageViewConsistencyReport validate_stage_views(
    const stageops::StageWorkspaceView& stage_ops,
    const modviz::SceneWorkspaceView& modviz);

} // namespace dmc::rengine::integration
