#include "dmc_rengine/integration/resource_workspace.hpp"

#include <string>
#include <utility>

namespace dmc::rengine::integration {

bool ResourceWorkspaceSession::record_validation_plan_created(
    std::string plan_id,
    bool ready_for_execution,
    std::size_t requirement_count,
    std::size_t blocker_count) {
    if (!valid() || plan_id.empty() ||
        !has_tool_route(gdspaces::ToolTarget::build_test_lab)) {
        return false;
    }

    return events_.record(
        WorkspaceEventType::validation_plan_created,
        payload_.resource.id,
        gdspaces::ToolTarget::build_test_lab,
        gdspaces::ToolTarget::spider_hub,
        working_copy_.has_value() ? working_copy_->revision() : 0U,
        std::move(plan_id),
        std::string("Trial Chamber created a validation plan with ") +
            std::to_string(requirement_count) + " requirements and " +
            std::to_string(blocker_count) + " blockers; execution readiness is " +
            (ready_for_execution ? "true." : "false.")) != 0U;
}

bool ResourceWorkspaceSession::record_patch_plan_compiled(
    std::string patch_plan_id,
    std::string message) {
    if (!valid() || patch_plan_id.empty() || message.empty()) {
        return false;
    }

    return events_.record(
        WorkspaceEventType::patch_plan_compiled,
        payload_.resource.id,
        gdspaces::ToolTarget::exe_editor,
        gdspaces::ToolTarget::build_test_lab,
        working_copy_.has_value() ? working_copy_->revision() : 0U,
        std::move(patch_plan_id),
        std::move(message)) != 0U;
}

} // namespace dmc::rengine::integration
