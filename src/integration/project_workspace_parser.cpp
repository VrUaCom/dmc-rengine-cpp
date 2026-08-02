#include "dmc_rengine/integration/project_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ProjectWorkspace::record_parser_completed(
    const gdspaces::ResourceId& resource,
    std::string parser_id,
    bool recognized,
    gdspaces::ToolTarget consumer) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr || !session->record_parser_completed(
            std::move(parser_id), recognized, consumer)) {
        return false;
    }
    return sync(*session);
}

} // namespace dmc::rengine::integration
