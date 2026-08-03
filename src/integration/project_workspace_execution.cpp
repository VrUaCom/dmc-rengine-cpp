#include "dmc_rengine/integration/project_workspace.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/patch/copy_executor.hpp"

#include <span>
#include <string>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string execution_tool_node_id(
    gdspaces::ToolTarget target) {
    return "tool:" + std::string(gdspaces::to_string(target));
}

} // namespace

bool ProjectWorkspace::register_patch_copy_artifact(
    const patch::PatchCopyArtifact& artifact) {
    if (!artifact.valid() ||
        graph_.find(artifact.patch_plan_id) == nullptr ||
        graph_.find(artifact.id) != nullptr ||
        graph_.find(artifact.rollback_plan_id) != nullptr) {
        return false;
    }

    auto* source_session = find_session_mutable(artifact.source_resource);
    const auto* target_artifact = artifacts_.find(
        artifact.target_artifact_id);
    if (source_session == nullptr || target_artifact == nullptr ||
        source_session->executable_context() == nullptr ||
        source_session->executable_context()->sha256 !=
            artifact.source_sha256 ||
        target_artifact->sha256 != artifact.source_sha256 ||
        artifact.output_bytes.size() !=
            source_session->source_payload().bytes.size()) {
        return false;
    }

    const auto actual_output_sha = core::Sha256::compute(
        std::span<const std::byte>{artifact.output_bytes}).hex();
    if (actual_output_sha != artifact.output_sha256) {
        return false;
    }

    const auto ensure_tool = [this](gdspaces::ToolTarget target) {
        const auto id = execution_tool_node_id(target);
        if (graph_.find(id) != nullptr) {
            return id;
        }
        const auto* descriptor = tools_.find(target);
        static_cast<void>(graph_.upsert(ProjectNode{
            .id = id,
            .kind = ProjectNodeKind::tool,
            .label = descriptor == nullptr
                ? std::string(gdspaces::to_string(target))
                : descriptor->product_name,
            .attributes = {
                {"target", std::string(gdspaces::to_string(target))},
            },
        }));
        return id;
    };

    if (!graph_.upsert(ProjectNode{
            .id = artifact.id,
            .kind = ProjectNodeKind::patch_execution,
            .label = artifact.id,
            .attributes = {
                {"patch_plan_id", artifact.patch_plan_id},
                {"rollback_plan_id", artifact.rollback_plan_id},
                {"target_artifact_id", artifact.target_artifact_id},
                {"source_sha256", artifact.source_sha256},
                {"output_sha256", artifact.output_sha256},
                {"output_size", std::to_string(
                    artifact.output_bytes.size())},
                {"execution_mode", "copy-buffer-only"},
                {"original_file_write_performed", "false"},
            },
        })) {
        return false;
    }
    if (!graph_.upsert(ProjectNode{
            .id = artifact.rollback_plan_id,
            .kind = ProjectNodeKind::rollback_plan,
            .label = artifact.rollback_plan_id,
            .attributes = {
                {"target_sha256", artifact.output_sha256},
                {"restores_sha256", artifact.source_sha256},
                {"patch_count", std::to_string(
                    artifact.rollback_plan.patches().size())},
                {"verified", "true"},
            },
        })) {
        return false;
    }

    static_cast<void>(graph_.connect(ProjectEdge{
        .from = artifact.id,
        .to = artifact.patch_plan_id,
        .kind = ProjectEdgeKind::executed_from,
        .label = "guarded plan executed on copied bytes",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = artifact.id,
        .to = "resource:" + artifact.source_resource.canonical(),
        .kind = ProjectEdgeKind::targets,
        .label = "immutable source identity only",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = artifact.id,
        .to = "artifact:" + artifact.target_artifact_id,
        .kind = ProjectEdgeKind::references_artifact,
        .label = "source artifact provenance",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = artifact.id,
        .to = artifact.rollback_plan_id,
        .kind = ProjectEdgeKind::produces,
        .label = "verified rollback plan",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = artifact.rollback_plan_id,
        .to = artifact.id,
        .kind = ProjectEdgeKind::rollback_for,
        .label = "restores source SHA-256",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = artifact.id,
        .to = ensure_tool(gdspaces::ToolTarget::build_test_lab),
        .kind = ProjectEdgeKind::produced_by,
        .label = "Trial Chamber copy execution",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = artifact.id,
        .to = ensure_tool(gdspaces::ToolTarget::spider_hub),
        .kind = ProjectEdgeKind::delivered_to,
        .label = "execution provenance only",
    }));

    if (!source_session->record_patch_copy_executed(
            artifact.id,
            artifact.output_sha256,
            artifact.rollback_plan_id)) {
        return false;
    }
    return sync(*source_session);
}

} // namespace dmc::rengine::integration
