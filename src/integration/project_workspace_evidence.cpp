#include "dmc_rengine/integration/project_workspace.hpp"

#include <string_view>

namespace dmc::rengine::integration {

std::size_t ProjectWorkspace::link_artifact_evidence(
    const gdspaces::ResourceId& resource,
    std::string_view sha256) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr) {
        return 0U;
    }

    std::size_t linked = 0U;
    for (const auto* artifact : artifacts_.by_sha256(sha256)) {
        if (artifact == nullptr) {
            continue;
        }
        for (const auto* record : evidence_.by_artifact(artifact->id)) {
            if (record != nullptr &&
                session->link_evidence_record(evidence_, record->id)) {
                ++linked;
            }
        }
    }

    if (linked != 0U) {
        static_cast<void>(sync(*session));
    }
    return linked;
}

} // namespace dmc::rengine::integration
