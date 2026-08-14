#include "dmc_rengine/stageops/semantic_graph.hpp"

#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace dmc::rengine;

[[nodiscard]] gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    bool container = false) {
    return gdspaces::ResourceRef{
        .id = gdspaces::ResourceId{
            .source_id = "stage-semantic-graph-test",
            .logical_path = std::move(path),
            .container_chain = {},
            .offset = offset,
            .size = 1U,
        },
        .display_name = "semantic graph resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = container,
    };
}

[[nodiscard]] gdspaces::ByteProvenance provenance(std::uint64_t offset) {
    return gdspaces::ByteProvenance{
        .kind = gdspaces::ByteOriginKind::direct_source_span,
        .authority_id = "stage-semantic-graph-source",
        .offset = offset,
        .stored_size = 1U,
        .materialized_size = 1U,
        .transform = gdspaces::ByteTransform::none,
        .crc32 = std::nullopt,
    };
}

[[nodiscard]] stageops::StageAssemblyWorkspace make_assembly() {
    const auto root = resource("scr/st308.pac", "pac", 0x1000U, true);
    auto child = resource(
        "scr/st308.pac::PAC/slot-0003/collision.hits",
        "hits",
        0x1010U);
    child.id.container_chain = "PAC[3]";

    stageops::StageAssemblyWorkspace assembly;
    assembly.identity = stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "dmc3-stage-row-110",
            .display_name = "Stage resource set dmc3-stage-row-110",
            .exe_evidence_id = "dmc3-stage-wave2",
            .resource_set_id = "dmc3-stage-row-110",
            .semantic_stage_id = {},
            .numeric_stage_id = 308U,
        },
        .catalog_entry_id = "dmc3-stage-row-110",
        .global_catalog_row = 110U,
        .source_table_id = "dmc3-stage-wave2-bank-b",
        .source_row_index = 0U,
    };

    assembly.requirements = {
        stageops::StageAssemblyRequirement{
            .requirement_id = "descriptor-root/script",
            .category = gdspaces::StageResourceCategory::scripts,
            .role = "script",
            .requested_logical_path = root.id.logical_path,
            .resource_id = root.id.canonical(),
            .materialized = true,
        },
        stageops::StageAssemblyRequirement{
            .requirement_id = "runtime/enemy-dependency-unresolved",
            .category = gdspaces::StageResourceCategory::unknown,
            .role = "enemy-dependency",
            .requested_logical_path = "<unresolved-runtime-dependency>",
            .resource_id = std::nullopt,
            .materialized = false,
        },
    };

    assembly.resources = {
        stageops::StageAssemblyResource{
            .resource = root,
            .byte_provenance = provenance(root.id.offset),
            .materialized = true,
            .descriptor_root = true,
            .nested_container_child = false,
            .container_expansion_observed = true,
        },
        stageops::StageAssemblyResource{
            .resource = child,
            .byte_provenance = provenance(child.id.offset),
            .materialized = true,
            .descriptor_root = false,
            .nested_container_child = true,
            .container_expansion_observed = false,
        },
    };

    assembly.memberships = {
        stageops::StageAssemblyMembership{
            .resource_id = root.id.canonical(),
            .category = gdspaces::StageResourceCategory::scripts,
            .role = "script",
            .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
            .parent_resource_id = std::nullopt,
            .container_slot = std::nullopt,
        },
        stageops::StageAssemblyMembership{
            .resource_id = child.id.canonical(),
            .category = gdspaces::StageResourceCategory::collision,
            .role = "container-slot/3",
            .kind = stageops::StageAssemblyMembershipKind::nested_container_child,
            .parent_resource_id = root.id.canonical(),
            .container_slot = 3U,
        },
    };
    assembly.product_materialization_complete = false;
    assembly.game_ready_equivalent = false;
    return assembly;
}

} // namespace

int main() {
    using namespace dmc::rengine;
    using namespace dmc::rengine::stageops::semantic;

    const auto assembly = make_assembly();
    assert(assembly.valid());
    assert(assembly.status() == stageops::StageAssemblyStatus::partial);

    const auto graph = StageSemanticGraphBuilder::build(assembly, 7U);
    assert(graph.valid());
    assert(graph.source_stage_revision == 7U);
    assert(graph.assembly_status == stageops::StageAssemblyStatus::partial);
    assert(graph.game_readiness == stageops::StageGameReadiness::unproven);
    assert(graph.nodes.size() == 5U);
    assert(graph.edges.size() == 5U);
    assert(graph.by_kind(NodeKind::stage_assembly).size() == 1U);
    assert(graph.by_kind(NodeKind::requirement).size() == 2U);
    assert(graph.by_kind(NodeKind::resource).size() == 2U);

    const auto stage_id =
        std::string{"stage-assembly:dmc3-stage-row-110"};
    const auto unresolved_id =
        std::string{"stage-requirement:dmc3-stage-row-110:"}
        + "runtime/enemy-dependency-unresolved";
    const auto* unresolved = graph.find_node(unresolved_id);
    assert(unresolved != nullptr);
    assert(unresolved->kind == NodeKind::requirement);
    assert(unresolved->authority == Authority::unresolved);
    assert(unresolved->attributes.at("resolved") == "false");
    assert(unresolved->attributes.at("materialized") == "false");

    const auto stage_requirements = graph.outgoing(
        stage_id,
        EdgeKind::requires_requirement);
    assert(stage_requirements.size() == 2U);
    const auto stage_members = graph.outgoing(stage_id, EdgeKind::stage_member);
    assert(stage_members.size() == 1U);

    const auto& root = assembly.resources[0].resource;
    const auto& child = assembly.resources[1].resource;
    const auto root_node_id = "resource:" + root.id.canonical();
    const auto child_node_id = "resource:" + child.id.canonical();
    assert(graph.find_node(root_node_id) != nullptr);
    assert(graph.find_node(child_node_id) != nullptr);

    const auto containment = graph.outgoing(root_node_id, EdgeKind::contains);
    assert(containment.size() == 1U);
    assert(containment[0]->to == child_node_id);
    assert(containment[0]->category == gdspaces::StageResourceCategory::collision);
    assert(containment[0]->attributes.at("container_slot") == "3");

    // Rebuilding from the same Stage Ops state must produce the same stable node
    // and edge identity/order. The graph does not perform a second lookup path.
    const auto rebuilt = StageSemanticGraphBuilder::build(assembly, 7U);
    assert(rebuilt.valid());
    assert(rebuilt.nodes.size() == graph.nodes.size());
    assert(rebuilt.edges.size() == graph.edges.size());
    for (std::size_t index = 0U; index < graph.nodes.size(); ++index) {
        assert(rebuilt.nodes[index].id == graph.nodes[index].id);
    }
    for (std::size_t index = 0U; index < graph.edges.size(); ++index) {
        assert(rebuilt.edges[index].id == graph.edges[index].id);
    }

    // Stage operation revision is source metadata; it must not rewrite semantic
    // graph node/resource identity.
    const auto later_revision = StageSemanticGraphBuilder::build(assembly, 8U);
    assert(later_revision.valid());
    assert(later_revision.source_stage_revision == 8U);
    for (std::size_t index = 0U; index < graph.nodes.size(); ++index) {
        assert(later_revision.nodes[index].id == graph.nodes[index].id);
    }

    return 0;
}
