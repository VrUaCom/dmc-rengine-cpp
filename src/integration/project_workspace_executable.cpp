#include "dmc_rengine/integration/project_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ProjectWorkspace::attach_executable_context(
    const gdspaces::ResourceId& resource,
    ExecutableResourceContext context) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr ||
        !session->attach_executable_context(std::move(context))) {
        return false;
    }
    return sync(*session);
}

} // namespace dmc::rengine::integration
