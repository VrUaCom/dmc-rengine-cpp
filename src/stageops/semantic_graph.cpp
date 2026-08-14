#include "dmc_rengine/stageops/semantic_graph.hpp"

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>

namespace dmc::rengine::stageops::semantic {
namespace {

[[nodiscard]] std::string bool_text(bool value) {
    return value ? "true" : "false";
}

[[nodiscard]] std::string stage_node_id(
    const StageAssemblyIdentity& identity) {
    return "stage-assembly:" + identity.catalog_entry_id;
}

[[nodiscard]] std::string requirement_node_id(
    const StageAssemblyIdentity& identity,
    const StageAssemblyRequirement& requirement) {
    return "stage-requirement:" + identity.catalog_entry_id + ":" +
        requirement.requirement_id;
}

[[nodiscard]] std::string resource_node_id(std::string_view canonical_id) {
    return "resource:" + std::string{canonical_id};
}

[[nodiscard]] std::string runtime_node_id(std::string_view runtime_id) {
    return "recovered-runtime:" + std::string{runtime_id};
}

[[nodiscard]] std::string edge_id(
    EdgeKind kind,
    std::string_view from,
    std::string_view to,
    std::string_view role,
    const std::optional<std::uint32_t>& slot = std::nullopt) {
    std::string result = "stage-edge:" + std::string{to_string(kind)} + ":" +
        std::string{from} + "->" + std::string{to} + ":" +
        std::string{role};
    if (slot.has_value()) {
        result += ":slot/" + std::to_string(*slot);
    }
    return result;
}

[[nodiscard]] bool same_identity(
    const StageAssemblyIdentity& left,
    const StageAssemblyIdentity& right) noexcept {
    return left.catalog_entry_id == right.catalog_entry_id &&
        left.global_catalog_row == right.global_catalog_row &&
        left.source_table_id == right.source_table_id &&
        left.source_row_index == right.source_row_index &&
        left.stage.profile == right.stage.profile &&
        left.stage.resource_set_key() == right.stage.resource_set_key() &&
        left.stage.numeric_stage_id == right.stage.numeric_stage_id &&
        left.stage.semantic_stage_id == right.stage.semantic_stage_id &&
        left.stage.exe_evidence_id == right.stage.exe_evidence_id;
}

[[nodiscard]] gdspaces::StageResourceCategory domain_category(
    StageDomainKind kind) noexcept {
    switch (kind) {
    case StageDomainKind::hits_collision:
        return gdspaces::StageResourceCategory::collision;
    case StageDomainKind::lighting_records:
        return gdspaces::StageResourceCategory::lighting;
    case StageDomainKind::stage_script_tokens:
    case StageDomainKind::stage_set_directive_token:
    case StageDomainKind::stage_set_value_token:
    case StageDomainKind::door_token:
    case StageDomainKind::box_in_token:
    case StageDomainKind::next_room_token:
        return gdspaces::StageResourceCategory::scripts;
    case StageDomainKind::dca_records:
        return gdspaces::StageResourceCategory::unknown;
    }
    return gdspaces::StageResourceCategory::unknown;
}

[[nodiscard]] const StageDomainObject* find_domain(
    const StageDomainWorkspace& domains,
    std::string_view id) noexcept {
    const auto iterator = std::find_if(
        domains.objects.begin(), domains.objects.end(),
        [id](const StageDomainObject& object) {
            return object.id == id;
        });
    return iterator == domains.objects.end() ? nullptr : &*iterator;
}

[[nodiscard]] Authority runtime_authority(
    StageRuntimeLinkAuthority authority) noexcept {
    switch (authority) {
    case StageRuntimeLinkAuthority::direct_reconstructed:
        return Authority::recovered_runtime_fact;
    case StageRuntimeLinkAuthority::disassembly_complete_corpus_pending:
        return Authority::recovered_runtime_partial;
    case StageRuntimeLinkAuthority::executable_candidate:
        return Authority::recovered_runtime_candidate;
    }
    return Authority::recovered_runtime_candidate;
}

[[nodiscard]] int runtime_authority_strength(Authority authority) noexcept {
    switch (authority) {
    case Authority::recovered_runtime_fact: return 2;
    case Authority::recovered_runtime_partial: return 1;
    case Authority::recovered_runtime_candidate: return 0;
    case Authority::structural_product_fact:
    case Authority::semantic_confirmed:
    case Authority::semantic_inferred:
    case Authority::unresolved:
        return -1;
    }
    return -1;
}

void add_stage_node(
    StageSemanticGraph& graph,
    const StageAssemblyWorkspace& assembly) {
    const auto& identity = assembly.identity;
    Node node{
        .id = stage_node_id(identity),
        .kind = NodeKind::stage_assembly,
        .authority = Authority::structural_product_fact,
        .label = identity.stage.display_name.empty()
            ? identity.catalog_entry_id
            : identity.stage.display_name,
        .attributes = {
            {"catalog_entry_id", identity.catalog_entry_id},
            {"profile", identity.stage.profile},
            {"resource_set_id", std::string{identity.stage.resource_set_key()}},
            {"global_catalog_row", std::to_string(identity.global_catalog_row)},
            {"source_table_id", identity.source_table_id},
            {"source_row_index", std::to_string(identity.source_row_index)},
            {"assembly_status", std::string{stageops::to_string(assembly.status())}},
            {"game_readiness", std::string{stageops::to_string(assembly.game_readiness())}},
        },
        .evidence_ids = {},
    };

    if (identity.stage.numeric_stage_id.has_value()) {
        node.attributes.emplace(
            "numeric_stage_id",
            std::to_string(*identity.stage.numeric_stage_id));
    }
    if (!identity.stage.semantic_stage_id.empty()) {
        node.attributes.emplace(
            "semantic_stage_id",
            identity.stage.semantic_stage_id);
    }
    if (!identity.stage.exe_evidence_id.empty()) {
        node.evidence_ids.push_back(identity.stage.exe_evidence_id);
    }
    graph.nodes.push_back(std::move(node));
}

void add_requirement_nodes_and_edges(
    StageSemanticGraph& graph,
    const StageAssemblyWorkspace& assembly) {
    const auto root = stage_node_id(assembly.identity);
    for (const auto& requirement : assembly.requirements) {
        const auto id = requirement_node_id(assembly.identity, requirement);
        graph.nodes.push_back(Node{
            .id = id,
            .kind = NodeKind::requirement,
            .authority = requirement.resource_id.has_value()
                ? Authority::structural_product_fact
                : Authority::unresolved,
            .label = requirement.role,
            .attributes = {
                {"requirement_id", requirement.requirement_id},
                {"role", requirement.role},
                {"category", std::string{gdspaces::to_string(requirement.category)}},
                {"requested_logical_path", requirement.requested_logical_path},
                {"resolved", bool_text(requirement.resource_id.has_value())},
                {"materialized", bool_text(requirement.materialized)},
            },
            .evidence_ids = {},
        });

        graph.edges.push_back(Edge{
            .id = edge_id(
                EdgeKind::requires_requirement,
                root,
                id,
                requirement.role),
            .from = root,
            .to = id,
            .kind = EdgeKind::requires_requirement,
            .authority = requirement.resource_id.has_value()
                ? Authority::structural_product_fact
                : Authority::unresolved,
            .role = requirement.role,
            .category = requirement.category,
            .attributes = {},
            .evidence_ids = {},
        });

        if (requirement.resource_id.has_value()) {
            const auto target = resource_node_id(*requirement.resource_id);
            graph.edges.push_back(Edge{
                .id = edge_id(
                    EdgeKind::resolves_to,
                    id,
                    target,
                    requirement.role),
                .from = id,
                .to = target,
                .kind = EdgeKind::resolves_to,
                .authority = Authority::structural_product_fact,
                .role = requirement.role,
                .category = requirement.category,
                .attributes = {
                    {"materialized", bool_text(requirement.materialized)},
                },
                .evidence_ids = {},
            });
        }
    }
}

void add_resource_nodes(
    StageSemanticGraph& graph,
    const StageAssemblyWorkspace& assembly) {
    for (const auto& resource : assembly.resources) {
        graph.nodes.push_back(Node{
            .id = resource_node_id(resource.resource.id.canonical()),
            .kind = NodeKind::resource,
            .authority = Authority::structural_product_fact,
            .label = resource.resource.display_name,
            .attributes = {
                {"resource_id", resource.resource.id.canonical()},
                {"logical_path", resource.resource.id.logical_path},
                {"format", resource.resource.format},
                {"profile", resource.resource.profile},
                {"materialized", bool_text(resource.materialized)},
                {"descriptor_root", bool_text(resource.descriptor_root)},
                {"nested_container_child", bool_text(resource.nested_container_child)},
                {"container_expansion_observed", bool_text(resource.container_expansion_observed)},
                {"byte_provenance", bool_text(
                    resource.byte_provenance.has_value() &&
                    resource.byte_provenance->valid())},
            },
            .evidence_ids = {},
        });
    }
}

void add_membership_edges(
    StageSemanticGraph& graph,
    const StageAssemblyWorkspace& assembly) {
    const auto root = stage_node_id(assembly.identity);
    for (const auto& membership : assembly.memberships) {
        const auto child = resource_node_id(membership.resource_id);
        if (membership.kind == StageAssemblyMembershipKind::descriptor_root) {
            graph.edges.push_back(Edge{
                .id = edge_id(
                    EdgeKind::stage_member,
                    root,
                    child,
                    membership.role),
                .from = root,
                .to = child,
                .kind = EdgeKind::stage_member,
                .authority = Authority::structural_product_fact,
                .role = membership.role,
                .category = membership.category,
                .attributes = {},
                .evidence_ids = {},
            });
            continue;
        }

        if (!membership.parent_resource_id.has_value()) {
            continue;
        }
        const auto parent = resource_node_id(*membership.parent_resource_id);
        graph.edges.push_back(Edge{
            .id = edge_id(
                EdgeKind::contains,
                parent,
                child,
                membership.role,
                membership.container_slot),
            .from = parent,
            .to = child,
            .kind = EdgeKind::contains,
            .authority = Authority::structural_product_fact,
            .role = membership.role,
            .category = membership.category,
            .attributes = membership.container_slot.has_value()
                ? std::map<std::string, std::string, std::less<>>{
                    {"container_slot", std::to_string(*membership.container_slot)}}
                : std::map<std::string, std::string, std::less<>>{},
            .evidence_ids = {},
        });
    }
}

void add_domain_nodes_and_edges(
    StageSemanticGraph& graph,
    const StageDomainWorkspace& domains) {
    for (const auto& object : domains.objects) {
        auto attributes = object.attributes;
        attributes.emplace("domain_kind", std::string{stageops::to_string(object.kind)});
        attributes.emplace("parser_id", object.parser_id);
        attributes.emplace(
            "byte_source",
            std::string{integration::to_string(object.byte_source)});
        attributes.emplace("byte_revision", std::to_string(object.byte_revision));
        attributes.emplace(
            "current_for_active_bytes",
            bool_text(object.current_for_active_bytes));

        graph.nodes.push_back(Node{
            .id = object.id,
            .kind = NodeKind::domain_object,
            .authority = Authority::structural_product_fact,
            .label = std::string{stageops::to_string(object.kind)},
            .attributes = std::move(attributes),
            .evidence_ids = {},
        });

        const auto resource = resource_node_id(object.resource_id);
        graph.edges.push_back(Edge{
            .id = edge_id(
                EdgeKind::projects_domain,
                resource,
                object.id,
                stageops::to_string(object.kind)),
            .from = resource,
            .to = object.id,
            .kind = EdgeKind::projects_domain,
            .authority = Authority::structural_product_fact,
            .role = std::string{stageops::to_string(object.kind)},
            .category = domain_category(object.kind),
            .attributes = {
                {"current_for_active_bytes", bool_text(
                    object.current_for_active_bytes)},
            },
            .evidence_ids = {},
        });
    }
}

void add_domain_relation_edges(
    StageSemanticGraph& graph,
    const StageDomainKnowledgeWorkspace& knowledge) {
    for (const auto& relation : knowledge.relations.relations) {
        const auto* source = find_domain(
            knowledge.domains, relation.from_domain_id);
        const auto category = source == nullptr
            ? gdspaces::StageResourceCategory::unknown
            : domain_category(source->kind);
        graph.edges.push_back(Edge{
            .id = "stage-edge:domain-relation:" + relation.id,
            .from = relation.from_domain_id,
            .to = relation.to_domain_id,
            .kind = EdgeKind::domain_relation,
            .authority = Authority::structural_product_fact,
            .role = std::string{stageops::to_string(relation.kind)},
            .category = category,
            .attributes = {
                {"relation_kind", std::string{stageops::to_string(relation.kind)}},
                {"relation_authority", std::string{
                    stageops::to_string(relation.authority)}},
                {"resource_id", relation.resource_id},
            },
            .evidence_ids = {},
        });
    }
}

[[nodiscard]] bool add_runtime_nodes_and_edges(
    StageSemanticGraph& graph,
    const StageDomainKnowledgeWorkspace& knowledge) {
    struct RuntimeProjection final {
        const RecoveredRuntimeIdentity* runtime{nullptr};
        Authority authority{Authority::recovered_runtime_fact};
    };

    std::map<std::string, RuntimeProjection, std::less<>> runtimes;
    for (const auto& link : knowledge.runtime_links.links) {
        const auto link_authority = runtime_authority(link.authority);
        const auto [iterator, inserted] = runtimes.emplace(
            link.runtime.id,
            RuntimeProjection{
                .runtime = &link.runtime,
                .authority = link_authority,
            });
        if (!inserted) {
            const auto* existing = iterator->second.runtime;
            if (existing == nullptr ||
                existing->source_tree_path != link.runtime.source_tree_path ||
                existing->symbol != link.runtime.symbol ||
                existing->executable_artifact_id !=
                    link.runtime.executable_artifact_id ||
                existing->evidence_ids != link.runtime.evidence_ids) {
                // One stable recovered-runtime identity cannot describe two
                // incompatible artifacts/contracts in one graph revision.
                return false;
            }
            if (runtime_authority_strength(link_authority) <
                runtime_authority_strength(iterator->second.authority)) {
                iterator->second.authority = link_authority;
            }
        }
    }

    for (const auto& [id, projection] : runtimes) {
        const auto* runtime = projection.runtime;
        if (runtime == nullptr) {
            return false;
        }
        graph.nodes.push_back(Node{
            .id = runtime_node_id(id),
            .kind = NodeKind::recovered_runtime,
            .authority = projection.authority,
            .label = runtime->symbol,
            .attributes = {
                {"runtime_id", runtime->id},
                {"source_tree_path", runtime->source_tree_path},
                {"symbol", runtime->symbol},
                {"executable_artifact_id", runtime->executable_artifact_id},
            },
            .evidence_ids = runtime->evidence_ids,
        });
    }

    for (const auto& link : knowledge.runtime_links.links) {
        const auto* domain = find_domain(knowledge.domains, link.domain_object_id);
        if (domain == nullptr) {
            return false;
        }
        graph.edges.push_back(Edge{
            .id = "stage-edge:runtime-link:" + link.id,
            .from = link.domain_object_id,
            .to = runtime_node_id(link.runtime.id),
            .kind = EdgeKind::runtime_link,
            .authority = runtime_authority(link.authority),
            .role = std::string{stageops::to_string(link.kind)},
            .category = domain_category(domain->kind),
            .attributes = {
                {"claim_id", link.claim_id},
                {"runtime_link_kind", std::string{
                    stageops::to_string(link.kind)}},
                {"runtime_link_authority", std::string{
                    stageops::to_string(link.authority)}},
            },
            .evidence_ids = link.runtime.evidence_ids,
        });
    }
    return true;
}

void finalize(StageSemanticGraph& graph) {
    std::sort(
        graph.nodes.begin(), graph.nodes.end(),
        [](const Node& left, const Node& right) {
            return left.id < right.id;
        });
    std::sort(
        graph.edges.begin(), graph.edges.end(),
        [](const Edge& left, const Edge& right) {
            return left.id < right.id;
        });
}

} // namespace

bool StageSemanticGraph::valid() const noexcept {
    if (!source_identity.valid() ||
        assembly_status == StageAssemblyStatus::invalid ||
        nodes.empty()) {
        return false;
    }

    std::set<std::string, std::less<>> node_ids;
    for (const auto& node : nodes) {
        if (!node.valid() || !node_ids.insert(node.id).second) {
            return false;
        }
    }

    std::set<std::string, std::less<>> edge_ids;
    for (const auto& edge : edges) {
        if (!edge.valid() || !edge_ids.insert(edge.id).second ||
            !node_ids.contains(edge.from) || !node_ids.contains(edge.to)) {
            return false;
        }
    }
    return true;
}

const Node* StageSemanticGraph::find_node(std::string_view id) const noexcept {
    const auto iterator = std::find_if(
        nodes.begin(), nodes.end(),
        [id](const Node& node) {
            return node.id == id;
        });
    return iterator == nodes.end() ? nullptr : &*iterator;
}

std::vector<const Node*> StageSemanticGraph::by_kind(NodeKind kind) const {
    std::vector<const Node*> result;
    for (const auto& node : nodes) {
        if (node.kind == kind) {
            result.push_back(&node);
        }
    }
    return result;
}

std::vector<const Edge*> StageSemanticGraph::outgoing(
    std::string_view node_id,
    EdgeKind kind) const {
    std::vector<const Edge*> result;
    for (const auto& edge : edges) {
        if (edge.from == node_id && edge.kind == kind) {
            result.push_back(&edge);
        }
    }
    return result;
}

StageSemanticGraph StageSemanticGraphBuilder::build(
    const StageAssemblyWorkspace& assembly,
    std::uint64_t source_stage_revision) {
    StageSemanticGraph graph;
    if (!assembly.valid()) {
        return graph;
    }

    graph.source_identity = assembly.identity;
    graph.assembly_status = assembly.status();
    graph.game_readiness = assembly.game_readiness();
    graph.source_stage_revision = source_stage_revision;

    add_stage_node(graph, assembly);
    add_resource_nodes(graph, assembly);
    add_requirement_nodes_and_edges(graph, assembly);
    add_membership_edges(graph, assembly);
    finalize(graph);

    if (!graph.valid()) {
        return StageSemanticGraph{};
    }
    return graph;
}

StageSemanticGraph StageSemanticGraphBuilder::build(
    const StageAssemblyWorkspace& assembly,
    const StageDomainWorkspace& domains,
    std::uint64_t source_stage_revision) {
    if (!assembly.valid() || !domains.valid() ||
        !same_identity(assembly.identity, domains.identity) ||
        domains.source_stage_revision != source_stage_revision) {
        return StageSemanticGraph{};
    }

    auto graph = build(assembly, source_stage_revision);
    if (!graph.valid()) {
        return StageSemanticGraph{};
    }

    add_domain_nodes_and_edges(graph, domains);
    finalize(graph);
    return graph.valid() ? graph : StageSemanticGraph{};
}

StageSemanticGraph StageSemanticGraphBuilder::build(
    const StageAssemblyWorkspace& assembly,
    const StageDomainKnowledgeWorkspace& knowledge,
    std::uint64_t source_stage_revision) {
    if (!assembly.valid() || !knowledge.valid() ||
        !same_identity(assembly.identity, knowledge.identity()) ||
        knowledge.source_stage_revision() != source_stage_revision) {
        return StageSemanticGraph{};
    }

    auto graph = build(assembly, knowledge.domains, source_stage_revision);
    if (!graph.valid()) {
        return StageSemanticGraph{};
    }

    add_domain_relation_edges(graph, knowledge);
    if (!add_runtime_nodes_and_edges(graph, knowledge)) {
        return StageSemanticGraph{};
    }
    finalize(graph);
    return graph.valid() ? graph : StageSemanticGraph{};
}

} // namespace dmc::rengine::stageops::semantic
