#include "dmc_rengine/integration/resource_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ResourceWorkspaceSession::record_parser_completed(
    std::string parser_id,
    bool recognized,
    gdspaces::ToolTarget consumer,
    std::uint64_t byte_revision) {
    if (!valid() || parser_id.empty() || !has_tool_route(consumer)) {
        return false;
    }

    return events_.record(
        WorkspaceEventType::parser_completed,
        payload_.resource.id,
        gdspaces::ToolTarget::gdspaces,
        consumer,
        byte_revision,
        std::move(parser_id),
        recognized
            ? "The registered parser recognized and completed analysis of the explicitly identified byte revision."
            : "The registered parser completed without recognizing the explicitly identified byte revision.") != 0U;
}

} // namespace dmc::rengine::integration
