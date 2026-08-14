#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/stageops/domain_workspace.hpp"
#include "dmc_rengine/stageops/semantic_graph.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace dmc::rengine;

[[nodiscard]] std::vector<std::byte> text_bytes(std::string_view text) {
    std::vector<std::byte> result;
    result.reserve(text.size());
    for (const auto character : text) {
        result.push_back(static_cast<std::byte>(character));
    }
    return result;
}

} // namespace

int main() {
    using namespace dmc::rengine;

    const auto bytes = text_bytes(
        "#SET STAY\n"
        "DOOR BoxIn NextRoom \"st002\"\n"
        "ORBREAK\n");
    const gdspaces::ResourceRef txt{
        .id = gdspaces::ResourceId{
            .source_id = "stage-txt-domain-semantic-test",
            .logical_path = "room/st001cfg_004.txt",
            .container_chain = "PAC[4]",
            .offset = 0x2000U,
            .size = static_cast<std::uint64_t>(bytes.size()),
        },
        .display_name = "Stage TXT fixture",
        .format = "txt",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
    const gdspaces::ByteProvenance provenance{
        .kind = gdspaces::ByteOriginKind::direct_source_span,
        .authority_id = "stage-txt-domain-semantic-source",
        .offset = txt.id.offset,
        .stored_size = static_cast<std::uint64_t>(bytes.size()),
        .materialized_size = static_cast<std::uint64_t>(bytes.size()),
        .transform = gdspaces::ByteTransform::none,
        .crc32 = std::nullopt,
    };

    integration::ProjectWorkspace project;
    assert(project.create_session(
        gdspaces::ResourcePayload{
            .resource = txt,
            .bytes = bytes,
            .diagnostics = {},
            .byte_provenance = provenance,
        },
        integration::WorkspaceContext{
            .stage_context = true,
            .menu_context = false,
            .evidence_context = true,
        }));
    assert(integration::ResourceAnalyzer::analyze(project, txt.id).ok());

    stageops::StageAssemblyWorkspace assembly;
    assembly.identity = stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "stage-txt-semantic-fixture",
            .display_name = "Stage TXT semantic fixture",
            .exe_evidence_id = "stage-txt-semantic-evidence",
            .resource_set_id = "stage-txt-semantic-fixture",
            .semantic_stage_id = {},
            .numeric_stage_id = 1U,
        },
        .catalog_entry_id = "stage-txt-semantic-fixture",
        .global_catalog_row = 0U,
        .source_table_id = "stage-txt-semantic-fixture-table",
        .source_row_index = 0U,
    };
    assembly.requirements.push_back(stageops::StageAssemblyRequirement{
        .requirement_id = "fixture/txt",
        .category = gdspaces::StageResourceCategory::scripts,
        .role = "stage-script",
        .requested_logical_path = txt.id.logical_path,
        .resource_id = txt.id.canonical(),
        .materialized = true,
    });
    assembly.resources.push_back(stageops::StageAssemblyResource{
        .resource = txt,
        .byte_provenance = provenance,
        .materialized = true,
        .descriptor_root = true,
        .nested_container_child = false,
        .container_expansion_observed = false,
    });
    assembly.memberships.push_back(stageops::StageAssemblyMembership{
        .resource_id = txt.id.canonical(),
        .category = gdspaces::StageResourceCategory::scripts,
        .role = "stage-script",
        .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
        .parent_resource_id = std::nullopt,
        .container_slot = std::nullopt,
    });
    assembly.product_materialization_complete = true;
    assert(assembly.valid());

    const auto domains = stageops::StageDomainAssembler::assemble(
        assembly, project, 4U);
    assert(domains.valid());
    assert(domains.objects.size() == 7U);
    assert(domains.by_kind(stageops::StageDomainKind::stage_script_tokens).size() == 1U);
    assert(domains.by_kind(stageops::StageDomainKind::stage_set_directive_token).size() == 1U);
    assert(domains.by_kind(stageops::StageDomainKind::stage_set_value_token).size() == 2U);
    assert(domains.by_kind(stageops::StageDomainKind::door_token).size() == 1U);
    assert(domains.by_kind(stageops::StageDomainKind::box_in_token).size() == 1U);
    assert(domains.by_kind(stageops::StageDomainKind::next_room_token).size() == 1U);

    const auto graph = stageops::semantic::StageSemanticGraphBuilder::build(
        assembly, domains, 4U);
    assert(graph.valid());
    assert(graph.by_kind(stageops::semantic::NodeKind::domain_object).size() == 7U);

    const auto resource_node_id = "resource:" + txt.id.canonical();
    const auto projected = graph.outgoing(
        resource_node_id,
        stageops::semantic::EdgeKind::projects_domain);
    assert(projected.size() == 7U);
    for (const auto* edge : projected) {
        assert(edge->authority ==
            stageops::semantic::Authority::structural_product_fact);
        assert(edge->category == gdspaces::StageResourceCategory::scripts);
    }

    const auto doors = domains.by_kind(stageops::StageDomainKind::door_token);
    assert(doors.size() == 1U);
    const auto* door_node = graph.find_node(doors[0]->id);
    assert(door_node != nullptr);
    assert(door_node->authority ==
        stageops::semantic::Authority::structural_product_fact);
    assert(door_node->attributes.at("value") == "DOOR");
    assert(door_node->attributes.at("offset") == "10");
    assert(door_node->attributes.at("runtime_object_claim") == "false");

    const auto values = domains.by_kind(
        stageops::StageDomainKind::stage_set_value_token);
    assert(values.size() == 2U);
    const auto* stay_node = graph.find_node(values[0]->id);
    assert(stay_node != nullptr);
    assert(stay_node->attributes.at("value") == "STAY");
    assert(stay_node->attributes.at("runtime_object_claim") == "false");

    return 0;
}
