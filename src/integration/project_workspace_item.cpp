#include "dmc_rengine/integration/project_workspace.hpp"

#include "dmc_rengine/item/runtime_request.hpp"

#include <algorithm>
#include <string>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string tool_node_id(gdspaces::ToolTarget target) {
    return "tool:" + std::string(gdspaces::to_string(target));
}

} // namespace

std::vector<const ResourceWorkspaceSession*>
ProjectWorkspace::executable_sessions_for_artifact(
    std::string_view artifact_id) const {
    std::vector<const ResourceWorkspaceSession*> result;
    const auto* artifact = artifacts_.find(artifact_id);
    if (artifact == nullptr) {
        return result;
    }

    for (const auto& [canonical_id, session] : sessions_) {
        static_cast<void>(canonical_id);
        const auto* executable = session.executable_context();
        if (executable != nullptr && executable->sha256 == artifact->sha256) {
            result.push_back(&session);
        }
    }
    return result;
}

bool ProjectWorkspace::register_item_runtime_request(
    const item::RuntimeChangeRequest& request) {
    if (!request.valid() ||
        request.status == item::RuntimeRequestStatus::rejected) {
        return false;
    }

    const auto node_id = "runtime-request:" + request.id;
    if (graph_.find(node_id) != nullptr) {
        return false;
    }

    auto* session = find_session_mutable(request.item_resource);
    const auto* artifact = artifacts_.find(request.target_artifact_id);
    if (session == nullptr || artifact == nullptr ||
        session->resource().format != "itm") {
        return false;
    }
    const auto* working = session->working_copy();
    if (working == nullptr || working->revision() != request.item_revision) {
        return false;
    }
    if (request.producer != gdspaces::ToolTarget::item_editor ||
        request.consumer != gdspaces::ToolTarget::exe_editor) {
        return false;
    }

    for (const auto& record_id : request.evidence_record_ids) {
        const auto* record = evidence_.find(record_id);
        if (record == nullptr) {
            return false;
        }
        const auto targets_artifact = std::any_of(
            record->locations.begin(), record->locations.end(),
            [&request](const evidence::EvidenceLocation& location) {
                return location.artifact_id == request.target_artifact_id;
            });
        if (!targets_artifact) {
            return false;
        }
    }

    const auto ensure_tool = [this](gdspaces::ToolTarget target) {
        const auto id = tool_node_id(target);
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
            .id = node_id,
            .kind = ProjectNodeKind::runtime_request,
            .label = request.id,
            .attributes = {
                {"kind", std::string(item::to_string(request.kind))},
                {"status", std::string(item::to_string(request.status))},
                {"item_revision", std::to_string(request.item_revision)},
                {"requested_value", std::to_string(request.requested_value)},
                {"target_artifact_id", request.target_artifact_id},
            },
        })) {
        return false;
    }

    const auto resource_id = "resource:" + request.item_resource.canonical();
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = node_id,
        .to = resource_id,
        .kind = ProjectEdgeKind::derived_from,
        .label = "ITM working-copy revision " +
            std::to_string(request.item_revision),
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = node_id,
        .to = "artifact:" + request.target_artifact_id,
        .kind = ProjectEdgeKind::requests_change,
        .label = "guarded executable change request",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = node_id,
        .to = ensure_tool(request.producer),
        .kind = ProjectEdgeKind::produced_by,
        .label = "Item Editor request",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = node_id,
        .to = ensure_tool(request.consumer),
        .kind = ProjectEdgeKind::delivered_to,
        .label = "EXE Editor patch planning",
    }));

    for (const auto& record_id : request.evidence_record_ids) {
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = node_id,
            .to = "evidence:" + record_id,
            .kind = ProjectEdgeKind::evidence_for,
            .label = "runtime request evidence",
        }));
    }
    for (const auto validator : request.validators) {
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = node_id,
            .to = ensure_tool(validator),
            .kind = ProjectEdgeKind::validates,
            .label = "mandatory request validator",
        }));
    }

    if (!session->record_runtime_change_requested(
            request.id,
            request.consumer,
            "Item Editor submitted an evidence-backed runtime change request.")) {
        return false;
    }
    if (!session->request_validation(
            request.producer,
            request.id,
            "Validate the runtime request through Evidence Registry and Build & Test Lab.")) {
        return false;
    }
    return sync(*session);
}

} // namespace dmc::rengine::integration
