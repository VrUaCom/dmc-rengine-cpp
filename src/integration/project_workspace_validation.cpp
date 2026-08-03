#include "dmc_rengine/integration/project_workspace.hpp"

#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/validation/item_runtime_plan.hpp"

#include <algorithm>
#include <set>
#include <string>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string tool_node_id(gdspaces::ToolTarget target) {
    return "tool:" + std::string(gdspaces::to_string(target));
}

[[nodiscard]] std::string requirement_node_id(
    const validation::ItemRuntimeValidationPlan& plan,
    std::string_view requirement_id) {
    return "validation-requirement:" + plan.id + ':' +
        std::string(requirement_id);
}

} // namespace

bool ProjectWorkspace::register_item_runtime_validation_plan(
    const validation::ItemRuntimeValidationPlan& plan,
    const item::RuntimeChangeRequest& request) {
    if (!plan.valid() || !request.valid() ||
        request.status == item::RuntimeRequestStatus::rejected ||
        plan.runtime_request_id != request.id ||
        plan.item_resource != request.item_resource ||
        plan.producer != gdspaces::ToolTarget::build_test_lab) {
        return false;
    }
    if (graph_.find(plan.id) != nullptr ||
        graph_.find("runtime-request:" + request.id) == nullptr) {
        return false;
    }

    auto* session = find_session_mutable(plan.item_resource);
    const auto* artifact = artifacts_.find(request.target_artifact_id);
    if (session == nullptr || artifact == nullptr ||
        session->resource().format != "itm") {
        return false;
    }
    const auto* working = session->working_copy();
    if (working == nullptr || working->revision() != request.item_revision) {
        return false;
    }

    std::set<std::string, std::less<>> requirement_ids;
    for (const auto& requirement : plan.requirements) {
        if (!requirement.valid() ||
            !requirement_ids.insert(requirement.id).second) {
            return false;
        }
        for (const auto& record_id : requirement.evidence_record_ids) {
            if (evidence_.find(record_id) == nullptr) {
                return false;
            }
        }
    }
    for (const auto& blocker : plan.blockers) {
        if (blocker.empty()) {
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
            .id = plan.id,
            .kind = ProjectNodeKind::validation_plan,
            .label = plan.id,
            .attributes = {
                {"runtime_request_id", plan.runtime_request_id},
                {"ready_for_execution",
                 plan.ready_for_execution() ? "true" : "false"},
                {"requirement_count",
                 std::to_string(plan.requirements.size())},
                {"blocker_count", std::to_string(plan.blockers.size())},
                {"target_artifact_id", request.target_artifact_id},
                {"item_revision", std::to_string(request.item_revision)},
            },
        })) {
        return false;
    }

    static_cast<void>(graph_.connect(ProjectEdge{
        .from = plan.id,
        .to = "runtime-request:" + request.id,
        .kind = ProjectEdgeKind::derived_from,
        .label = "Trial Chamber preflight for runtime request",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = plan.id,
        .to = "resource:" + request.item_resource.canonical(),
        .kind = ProjectEdgeKind::validates,
        .label = "ITM working-copy revision " +
            std::to_string(request.item_revision),
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = plan.id,
        .to = "artifact:" + request.target_artifact_id,
        .kind = ProjectEdgeKind::validates,
        .label = "target executable artifact",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = plan.id,
        .to = ensure_tool(gdspaces::ToolTarget::build_test_lab),
        .kind = ProjectEdgeKind::produced_by,
        .label = "Trial Chamber validation plan",
    }));

    const auto executable_sessions =
        executable_sessions_for_artifact(request.target_artifact_id);
    for (const auto* executable_session : executable_sessions) {
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = plan.id,
            .to = "resource:" +
                executable_session->resource().id.canonical(),
            .kind = ProjectEdgeKind::validates,
            .label = "analyzed executable session",
        }));
    }

    std::set<std::string, std::less<>> evidence_ids;
    for (const auto& record_id : request.evidence_record_ids) {
        if (evidence_ids.insert(record_id).second) {
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = plan.id,
                .to = "evidence:" + record_id,
                .kind = ProjectEdgeKind::evidence_for,
                .label = "validation evidence",
            }));
        }
    }

    std::set<std::string, std::less<>> represented_blockers;
    for (const auto& requirement : plan.requirements) {
        const auto node_id = requirement_node_id(plan, requirement.id);
        if (!graph_.upsert(ProjectNode{
                .id = node_id,
                .kind = ProjectNodeKind::validation_requirement,
                .label = requirement.id,
                .attributes = {
                    {"kind", std::string(validation::to_string(
                        requirement.kind))},
                    {"status", std::string(validation::to_string(
                        requirement.status))},
                    {"mandatory", requirement.mandatory ? "true" : "false"},
                    {"detail", requirement.detail},
                    {"external_blocker", "false"},
                    {"evidence_count",
                     std::to_string(requirement.evidence_record_ids.size())},
                },
            })) {
            return false;
        }
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = plan.id,
            .to = node_id,
            .kind = ProjectEdgeKind::requires,
            .label = requirement.detail,
        }));
        if (requirement.status == validation::RequirementStatus::blocked) {
            represented_blockers.insert(requirement.id);
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = node_id,
                .to = plan.id,
                .kind = ProjectEdgeKind::blocks,
                .label = "mandatory validation blocker",
            }));
        }
        for (const auto& record_id : requirement.evidence_record_ids) {
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = node_id,
                .to = "evidence:" + record_id,
                .kind = ProjectEdgeKind::evidence_for,
                .label = "requirement evidence",
            }));
        }
    }

    std::set<std::string, std::less<>> unique_external_blockers;
    for (const auto& blocker : plan.blockers) {
        if (represented_blockers.contains(blocker) ||
            !unique_external_blockers.insert(blocker).second) {
            continue;
        }
        const auto node_id = requirement_node_id(
            plan, "external-blocker-" + blocker);
        if (!graph_.upsert(ProjectNode{
                .id = node_id,
                .kind = ProjectNodeKind::validation_requirement,
                .label = blocker,
                .attributes = {
                    {"kind", "external-blocker"},
                    {"status", "blocked"},
                    {"mandatory", "true"},
                    {"detail", blocker},
                    {"external_blocker", "true"},
                    {"evidence_count", "0"},
                },
            })) {
            return false;
        }
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = plan.id,
            .to = node_id,
            .kind = ProjectEdgeKind::requires,
            .label = blocker,
        }));
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = node_id,
            .to = plan.id,
            .kind = ProjectEdgeKind::blocks,
            .label = "external validation blocker",
        }));
    }

    if (!session->record_validation_plan_created(
            plan.id,
            plan.ready_for_execution(),
            plan.requirements.size(),
            plan.blockers.size())) {
        return false;
    }
    return sync(*session);
}

} // namespace dmc::rengine::integration
