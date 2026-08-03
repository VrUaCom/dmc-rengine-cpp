#pragma once

#include "dmc_rengine/integration/resource_workspace.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::integration {

enum class ProjectNodeKind {
    resource,
    format,
    parser,
    tool,
    evidence_packet,
    artifact,
    evidence_record,
    executable_analysis,
    runtime_request,
    validation_plan,
    validation_requirement,
    patch_plan,
    stage,
    binary_document,
    working_copy,
    manifest,
    event,
    specification,
};

[[nodiscard]] constexpr std::string_view to_string(
    ProjectNodeKind kind) noexcept {
    switch (kind) {
    case ProjectNodeKind::resource: return "resource";
    case ProjectNodeKind::format: return "format";
    case ProjectNodeKind::parser: return "parser";
    case ProjectNodeKind::tool: return "tool";
    case ProjectNodeKind::evidence_packet: return "evidence-packet";
    case ProjectNodeKind::artifact: return "artifact";
    case ProjectNodeKind::evidence_record: return "evidence-record";
    case ProjectNodeKind::executable_analysis: return "executable-analysis";
    case ProjectNodeKind::runtime_request: return "runtime-request";
    case ProjectNodeKind::validation_plan: return "validation-plan";
    case ProjectNodeKind::validation_requirement: return "validation-requirement";
    case ProjectNodeKind::patch_plan: return "patch-plan";
    case ProjectNodeKind::stage: return "stage";
    case ProjectNodeKind::binary_document: return "binary-document";
    case ProjectNodeKind::working_copy: return "working-copy";
    case ProjectNodeKind::manifest: return "manifest";
    case ProjectNodeKind::event: return "event";
    case ProjectNodeKind::specification: return "specification";
    }
    return "resource";
}

enum class ProjectEdgeKind {
    contains,
    depends_on,
    declares,
    references_artifact,
    requests_change,
    requires_requirement,
    blocks,
    compiled_from,
    targets,
    classified_as,
    parsed_by,
    opens_with,
    evidence_for,
    stage_member,
    derived_from,
    produced_by,
    delivered_to,
    records_event,
    validates,
    related_to,
};

[[nodiscard]] constexpr std::string_view to_string(
    ProjectEdgeKind kind) noexcept {
    switch (kind) {
    case ProjectEdgeKind::contains: return "contains";
    case ProjectEdgeKind::depends_on: return "depends-on";
    case ProjectEdgeKind::declares: return "declares";
    case ProjectEdgeKind::references_artifact: return "references-artifact";
    case ProjectEdgeKind::requests_change: return "requests-change";
    case ProjectEdgeKind::requires_requirement: return "requires";
    case ProjectEdgeKind::blocks: return "blocks";
    case ProjectEdgeKind::compiled_from: return "compiled-from";
    case ProjectEdgeKind::targets: return "targets";
    case ProjectEdgeKind::classified_as: return "classified-as";
    case ProjectEdgeKind::parsed_by: return "parsed-by";
    case ProjectEdgeKind::opens_with: return "opens-with";
    case ProjectEdgeKind::evidence_for: return "evidence-for";
    case ProjectEdgeKind::stage_member: return "stage-member";
    case ProjectEdgeKind::derived_from: return "derived-from";
    case ProjectEdgeKind::produced_by: return "produced-by";
    case ProjectEdgeKind::delivered_to: return "delivered-to";
    case ProjectEdgeKind::records_event: return "records-event";
    case ProjectEdgeKind::validates: return "validates";
    case ProjectEdgeKind::related_to: return "related-to";
    }
    return "related-to";
}

struct ProjectNode final {
    std::string id;
    ProjectNodeKind kind{ProjectNodeKind::resource};
    std::string label;
    std::map<std::string, std::string, std::less<>> attributes;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !label.empty();
    }
};

struct ProjectEdge final {
    std::string from;
    std::string to;
    ProjectEdgeKind kind{ProjectEdgeKind::related_to};
    std::string label;

    [[nodiscard]] bool valid() const noexcept {
        return !from.empty() && !to.empty() && from != to;
    }

    friend bool operator==(const ProjectEdge&, const ProjectEdge&) = default;
};

class ProjectGraph final {
public:
    [[nodiscard]] bool upsert(ProjectNode node);
    [[nodiscard]] bool connect(ProjectEdge edge);
    [[nodiscard]] const ProjectNode* find(std::string_view id) const noexcept;
    [[nodiscard]] std::vector<const ProjectNode*> nodes(
        ProjectNodeKind kind) const;
    [[nodiscard]] std::vector<const ProjectEdge*> outgoing(
        std::string_view node_id) const;
    [[nodiscard]] std::vector<const ProjectEdge*> incoming(
        std::string_view node_id) const;

    [[nodiscard]] bool ingest_workspace(
        const ResourceWorkspaceSession& workspace,
        const ToolRegistry& tools);

    [[nodiscard]] const std::map<
        std::string, ProjectNode, std::less<>>& all_nodes() const noexcept;
    [[nodiscard]] const std::vector<ProjectEdge>& all_edges() const noexcept;
    [[nodiscard]] std::size_t node_count() const noexcept;
    [[nodiscard]] std::size_t edge_count() const noexcept;

private:
    std::map<std::string, ProjectNode, std::less<>> nodes_;
    std::vector<ProjectEdge> edges_;
};

} // namespace dmc::rengine::integration
