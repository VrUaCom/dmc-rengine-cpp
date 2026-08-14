#pragma once

#include "dmc_rengine/stageops/assembly_workspace.hpp"
#include "dmc_rengine/stageops/domain_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops::semantic {

enum class NodeKind {
    stage_assembly,
    requirement,
    resource,
    domain_object,
};

[[nodiscard]] constexpr std::string_view to_string(NodeKind kind) noexcept {
    switch (kind) {
    case NodeKind::stage_assembly: return "stage-assembly";
    case NodeKind::requirement: return "requirement";
    case NodeKind::resource: return "resource";
    case NodeKind::domain_object: return "domain-object";
    }
    return "resource";
}

enum class EdgeKind {
    requires_requirement,
    resolves_to,
    stage_member,
    contains,
    projects_domain,
};

[[nodiscard]] constexpr std::string_view to_string(EdgeKind kind) noexcept {
    switch (kind) {
    case EdgeKind::requires_requirement: return "requires";
    case EdgeKind::resolves_to: return "resolves-to";
    case EdgeKind::stage_member: return "stage-member";
    case EdgeKind::contains: return "contains";
    case EdgeKind::projects_domain: return "projects-domain";
    }
    return "stage-member";
}

// Semantic/evidence authority is explicit even in the first structural slice so
// later runtime or inferred edges cannot silently look equivalent to facts
// emitted directly from Stage Ops assembly state.
enum class Authority {
    structural_product_fact,
    recovered_runtime_fact,
    semantic_confirmed,
    semantic_inferred,
    unresolved,
};

[[nodiscard]] constexpr std::string_view to_string(Authority authority) noexcept {
    switch (authority) {
    case Authority::structural_product_fact: return "structural-product-fact";
    case Authority::recovered_runtime_fact: return "recovered-runtime-fact";
    case Authority::semantic_confirmed: return "semantic-confirmed";
    case Authority::semantic_inferred: return "semantic-inferred";
    case Authority::unresolved: return "unresolved";
    }
    return "unresolved";
}

struct Node final {
    std::string id;
    NodeKind kind{NodeKind::resource};
    Authority authority{Authority::structural_product_fact};
    std::string label;
    std::map<std::string, std::string, std::less<>> attributes;
    std::vector<std::string> evidence_ids;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !label.empty();
    }
};

struct Edge final {
    std::string id;
    std::string from;
    std::string to;
    EdgeKind kind{EdgeKind::stage_member};
    Authority authority{Authority::structural_product_fact};
    std::string role;
    gdspaces::StageResourceCategory category{
        gdspaces::StageResourceCategory::unknown};
    std::map<std::string, std::string, std::less<>> attributes;
    std::vector<std::string> evidence_ids;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !from.empty() && !to.empty() && from != to;
    }
};

class StageSemanticGraph final {
public:
    StageAssemblyIdentity source_identity;
    StageAssemblyStatus assembly_status{StageAssemblyStatus::invalid};
    StageGameReadiness game_readiness{StageGameReadiness::unproven};
    std::uint64_t source_stage_revision{};
    std::vector<Node> nodes;
    std::vector<Edge> edges;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] const Node* find_node(std::string_view id) const noexcept;
    [[nodiscard]] std::vector<const Node*> by_kind(NodeKind kind) const;
    [[nodiscard]] std::vector<const Edge*> outgoing(
        std::string_view node_id,
        EdgeKind kind) const;
};

// Deterministic, resolution-free projection. This builder consumes only the
// already assembled Stage Ops state and never calls GDSpaces lookup, source
// mounting, archive traversal or ProjectWorkspace scene discovery.
class StageSemanticGraphBuilder final {
public:
    [[nodiscard]] static StageSemanticGraph build(
        const StageAssemblyWorkspace& assembly,
        std::uint64_t source_stage_revision = 0U);

    // Adds Stage Ops typed domain projections without transferring their
    // ownership to the graph. Domain identity/resource links remain derived and
    // rebuildable from Stage Ops state.
    [[nodiscard]] static StageSemanticGraph build(
        const StageAssemblyWorkspace& assembly,
        const StageDomainWorkspace& domains,
        std::uint64_t source_stage_revision);
};

} // namespace dmc::rengine::stageops::semantic
