#include "dmc_rengine/integration/project_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ProjectWorkspace::attach_parsed_resource(
    const gdspaces::ResourceId& resource,
    ParsedResourceResult parsed) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr ||
        !session->attach_parsed_resource(std::move(parsed))) {
        return false;
    }
    return sync(*session);
}

} // namespace dmc::rengine::integration
