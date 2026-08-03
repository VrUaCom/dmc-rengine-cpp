#include "dmc_rengine/integration/resource_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ResourceWorkspaceSession::record_runtime_change_requested(
    std::string request_id,
    gdspaces::ToolTarget consumer,
    std::string message) {
    if (!valid() || request_id.empty() || message.empty() ||
        !has_tool_route(gdspaces::ToolTarget::item_editor)) {
        return false;
    }

    return events_.record(
        WorkspaceEventType::runtime_change_requested,
        payload_.resource.id,
        gdspaces::ToolTarget::item_editor,
        consumer,
        working_copy_.has_value() ? working_copy_->revision() : 0U,
        std::move(request_id),
        std::move(message)) != 0U;
}

} // namespace dmc::rengine::integration
