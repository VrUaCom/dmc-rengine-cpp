#include "dmc_rengine/integration/project_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ProjectWorkspace::attach_executable_context(
    const gdspaces::ResourceId& resource,
    ExecutableResourceContext context) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr ||
        !session->attach_executable_context(std::move(context)) ||
        !sync(*session)) {
        return false;
    }

    const auto* executable = session->executable_context();
    if (executable == nullptr) {
        return false;
    }

    const auto resource_node_id = "resource:" + resource.canonical();
    const auto analysis_node_id = "executable-analysis:" + resource.canonical();
    if (!graph_.upsert(ProjectNode{
            .id = analysis_node_id,
            .kind = ProjectNodeKind::executable_analysis,
            .label = session->resource().display_name + " PE analysis",
            .attributes = {
                {"sha256", executable->sha256},
                {"pe_kind", std::string(exe::to_string(executable->image.kind))},
                {"machine", std::string(exe::to_string(executable->image.machine))},
                {"image_base", std::to_string(executable->image.image_base)},
                {"entry_point_rva", std::to_string(executable->image.entry_point_rva)},
                {"section_count", std::to_string(executable->image.section_count)},
                {"known_target_id", executable->known_target_id},
                {"known_target_name", executable->known_target_name},
                {"known_target_hash_match",
                 executable->known_target_hash_match ? "true" : "false"},
                {"known_target_metadata_match",
                 executable->known_target_metadata_match ? "true" : "false"},
            },
        })) {
        return false;
    }

    static_cast<void>(graph_.connect(ProjectEdge{
        .from = analysis_node_id,
        .to = resource_node_id,
        .kind = ProjectEdgeKind::derived_from,
        .label = "PE metadata and target identification",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = analysis_node_id,
        .to = "tool:exe-editor",
        .kind = ProjectEdgeKind::produced_by,
        .label = "EXE Editor analysis",
    }));

    for (const auto* artifact : artifacts_.by_sha256(executable->sha256)) {
        if (artifact == nullptr) {
            continue;
        }
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = analysis_node_id,
            .to = "artifact:" + artifact->id,
            .kind = ProjectEdgeKind::references_artifact,
            .label = "SHA-256 identity match",
        }));
    }
    return true;
}

} // namespace dmc::rengine::integration
