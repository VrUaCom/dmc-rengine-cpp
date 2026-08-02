#include "dmc_rengine/integration/resource_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ResourceWorkspaceSession::record_parser_completed(
    std::string parser_id,
    bool recognized,
    gdspaces::ToolTarget consumer) {
    if (!valid() || parser_id.empty() || !has_tool_route(consumer)) {
        return false;
    }

    return events_.record(
        WorkspaceEventType::parser_completed,
        payload_.resource.id,
        gdspaces::ToolTarget::gdspaces,
        consumer,
        working_copy_.has_value() ? working_copy_->revision() : 0U,
        std::move(parser_id),
        recognized
            ? "The registered parser recognized and completed analysis of the canonical resource."
            : "The registered parser completed without recognizing the canonical resource.") != 0U;
}

} // namespace dmc::rengine::integration
