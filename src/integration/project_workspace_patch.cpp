#include "dmc_rengine/integration/project_workspace.hpp"

#include "dmc_rengine/patch/item_runtime_compiler.hpp"

#include <string>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string patch_tool_node_id(gdspaces::ToolTarget target) {
    return "tool:" + std::string(gdspaces::to_string(target));
}

} // namespace

bool ProjectWorkspace::register_compiled_item_runtime_patch_plan(
    const patch::CompiledItemRuntimePatchPlan& compiled) {
    if (!compiled.valid()) {
        return false;
    }
    const auto& metadata = compiled.metadata;
    if (graph_.find(metadata.id) != nullptr ||
        graph_.find("runtime-request:" + metadata.runtime_request_id) == nullptr ||
        graph_.find(metadata.validation_plan_id) == nullptr) {
        return false;
    }

    auto* item_session = find_session_mutable(metadata.item_resource);
    const auto* executable_session = find_session(metadata.executable_resource);
    const auto* artifact = artifacts_.find(metadata.target_artifact_id);
    const auto* record = evidence_.find(metadata.evidence_record_id);
    if (item_session == nullptr || executable_session == nullptr ||
        artifact == nullptr || record == nullptr ||
        item_session->resource().format != "itm" ||
        executable_session->executable_context() == nullptr ||
        executable_session->executable_context()->sha256 != artifact->sha256 ||
        metadata.source_sha256 != artifact->sha256) {
        return false;
    }

    const auto& patches = compiled.guarded_plan.patches();
    if (patches.size() != 1U ||
        patches.front().offset != metadata.file_offset ||
        patches.front().expected.size() != metadata.size ||
        patches.front().replacement.size() != metadata.size) {
        return false;
    }

    const auto ensure_tool = [this](gdspaces::ToolTarget target) {
        const auto id = patch_tool_node_id(target);
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
            .id = metadata.id,
            .kind = ProjectNodeKind::patch_plan,
            .label = metadata.id,
            .attributes = {
                {"runtime_request_id", metadata.runtime_request_id},
                {"validation_plan_id", metadata.validation_plan_id},
                {"target_artifact_id", metadata.target_artifact_id},
                {"evidence_record_id", metadata.evidence_record_id},
                {"file_offset", std::to_string(metadata.file_offset)},
                {"rva", std::to_string(metadata.rva)},
                {"va", std::to_string(metadata.va)},
                {"size", std::to_string(metadata.size)},
                {"source_sha256", metadata.source_sha256},
                {"expected_hex", metadata.expected_hex},
                {"replacement_hex", metadata.replacement_hex},
                {"description", metadata.description},
                {"applied", "false"},
            },
        })) {
        return false;
    }

    static_cast<void>(graph_.connect(ProjectEdge{
        .from = metadata.id,
        .to = metadata.validation_plan_id,
        .kind = ProjectEdgeKind::compiled_from,
        .label = "compiled from green Trial Chamber preflight",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = metadata.id,
        .to = "runtime-request:" + metadata.runtime_request_id,
        .kind = ProjectEdgeKind::derived_from,
        .label = "runtime change request",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = metadata.id,
        .to = "resource:" + metadata.item_resource.canonical(),
        .kind = ProjectEdgeKind::derived_from,
        .label = "ITM working-copy source",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = metadata.id,
        .to = "resource:" + metadata.executable_resource.canonical(),
        .kind = ProjectEdgeKind::targets,
        .label = "immutable executable source payload",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = metadata.id,
        .to = "artifact:" + metadata.target_artifact_id,
        .kind = ProjectEdgeKind::requests_change,
        .label = "guarded target artifact",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = metadata.id,
        .to = "evidence:" + metadata.evidence_record_id,
        .kind = ProjectEdgeKind::evidence_for,
        .label = "resolved patch location evidence",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = metadata.id,
        .to = ensure_tool(gdspaces::ToolTarget::exe_editor),
        .kind = ProjectEdgeKind::produced_by,
        .label = "EXE Editor Patch Plan Compiler",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = metadata.id,
        .to = ensure_tool(gdspaces::ToolTarget::build_test_lab),
        .kind = ProjectEdgeKind::delivered_to,
        .label = "Trial Chamber execution and validation",
    }));

    if (!item_session->record_patch_plan_compiled(
            metadata.id,
            "EXE Editor compiled a guarded in-memory patch plan; no source file was modified.")) {
        return false;
    }
    return sync(*item_session);
}

} // namespace dmc::rengine::integration
