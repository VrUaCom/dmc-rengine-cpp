#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/stageops/domain_workspace.hpp"

#include "hits_test_fixture.hpp"

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

[[nodiscard]] gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    std::uint64_t size) {
    return gdspaces::ResourceRef{
        .id = gdspaces::ResourceId{
            .source_id = "stage-domain-workspace-test",
            .logical_path = std::move(path),
            .container_chain = "PAC[0]",
            .offset = offset,
            .size = size,
        },
        .display_name = "stage domain resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] gdspaces::ByteProvenance provenance(
    const gdspaces::ResourceRef& resource) {
    return gdspaces::ByteProvenance{
        .kind = gdspaces::ByteOriginKind::direct_source_span,
        .authority_id = "stage-domain-workspace-source",
        .offset = resource.id.offset,
        .stored_size = resource.id.size,
        .materialized_size = resource.id.size,
        .transform = gdspaces::ByteTransform::none,
        .crc32 = std::nullopt,
    };
}

[[nodiscard]] stageops::StageAssemblyWorkspace make_assembly(
    const gdspaces::ResourceRef& hits,
    const gdspaces::ResourceRef& txt) {
    stageops::StageAssemblyWorkspace assembly;
    assembly.identity = stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "stage-domain-fixture",
            .display_name = "Stage domain fixture",
            .exe_evidence_id = "stage-domain-fixture-evidence",
            .resource_set_id = "stage-domain-fixture",
            .semantic_stage_id = {},
            .numeric_stage_id = 1U,
        },
        .catalog_entry_id = "stage-domain-fixture",
        .global_catalog_row = 0U,
        .source_table_id = "stage-domain-fixture-table",
        .source_row_index = 0U,
    };

    assembly.requirements = {
        stageops::StageAssemblyRequirement{
            .requirement_id = "fixture/hits",
            .category = gdspaces::StageResourceCategory::collision,
            .role = "collision",
            .requested_logical_path = hits.id.logical_path,
            .resource_id = hits.id.canonical(),
            .materialized = true,
        },
        stageops::StageAssemblyRequirement{
            .requirement_id = "fixture/txt",
            .category = gdspaces::StageResourceCategory::scripts,
            .role = "stage-script",
            .requested_logical_path = txt.id.logical_path,
            .resource_id = txt.id.canonical(),
            .materialized = true,
        },
    };

    assembly.resources = {
        stageops::StageAssemblyResource{
            .resource = hits,
            .byte_provenance = provenance(hits),
            .materialized = true,
            .descriptor_root = true,
            .nested_container_child = false,
            .container_expansion_observed = false,
        },
        stageops::StageAssemblyResource{
            .resource = txt,
            .byte_provenance = provenance(txt),
            .materialized = true,
            .descriptor_root = true,
            .nested_container_child = false,
            .container_expansion_observed = false,
        },
    };

    assembly.memberships = {
        stageops::StageAssemblyMembership{
            .resource_id = hits.id.canonical(),
            .category = gdspaces::StageResourceCategory::collision,
            .role = "collision",
            .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
            .parent_resource_id = std::nullopt,
            .container_slot = std::nullopt,
        },
        stageops::StageAssemblyMembership{
            .resource_id = txt.id.canonical(),
            .category = gdspaces::StageResourceCategory::scripts,
            .role = "stage-script",
            .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
            .parent_resource_id = std::nullopt,
            .container_slot = std::nullopt,
        },
    };
    assembly.product_materialization_complete = true;
    return assembly;
}

} // namespace

int main() {
    using namespace dmc::rengine;

    const auto hits_bytes = tests::hits_fixture::make_minimal_hits();
    const auto txt_bytes = text_bytes(
        "#SET STAY\n"
        "DOOR BoxIn NextRoom \"st002\" 12 -3.5\n"
        "ORBREAK\n");

    const auto hits = resource(
        "room/st001cfg_006.hits",
        "hits",
        0x1000U,
        static_cast<std::uint64_t>(hits_bytes.size()));
    const auto txt = resource(
        "room/st001cfg_004.txt",
        "txt",
        0x2000U,
        static_cast<std::uint64_t>(txt_bytes.size()));

    integration::ProjectWorkspace project;
    const integration::WorkspaceContext context{
        .stage_context = true,
        .menu_context = false,
        .evidence_context = true,
    };
    assert(project.create_session(gdspaces::ResourcePayload{
        .resource = hits,
        .bytes = hits_bytes,
        .diagnostics = {},
        .byte_provenance = provenance(hits),
    }, context));
    assert(project.create_session(gdspaces::ResourcePayload{
        .resource = txt,
        .bytes = txt_bytes,
        .diagnostics = {},
        .byte_provenance = provenance(txt),
    }, context));

    assert(integration::ResourceAnalyzer::analyze(project, hits.id).ok());
    assert(integration::ResourceAnalyzer::analyze(project, txt.id).ok());

    const auto assembly = make_assembly(hits, txt);
    assert(assembly.valid());

    auto domains = stageops::StageDomainAssembler::assemble(
        assembly, project, 0U);
    assert(domains.valid());
    assert(domains.source_stage_revision == 0U);
    assert(domains.missing_project_session_count == 0U);
    assert(domains.missing_typed_result_count == 0U);
    assert(domains.unrecognized_typed_result_count == 0U);
    assert(domains.stale_typed_result_count == 0U);
    assert(domains.current_for_active_bytes());

    // 2 aggregate structural domains + 6 exact Stage TXT lexical markers.
    assert(domains.objects.size() == 8U);

    const auto collisions = domains.by_kind(
        stageops::StageDomainKind::hits_collision);
    assert(collisions.size() == 1U);
    assert(collisions[0]->current_for_active_bytes);
    assert(collisions[0]->resource_id == hits.id.canonical());
    assert(collisions[0]->attributes.at("triangle_count") == "1");

    const auto scripts = domains.by_kind(
        stageops::StageDomainKind::stage_script_tokens);
    assert(scripts.size() == 1U);
    assert(scripts[0]->current_for_active_bytes);
    assert(scripts[0]->attributes.at("door_count") == "1");
    assert(scripts[0]->attributes.at("box_in_count") == "1");
    assert(scripts[0]->attributes.at("next_room_count") == "1");
    assert(scripts[0]->attributes.at("stage_set_value_count") == "2");

    const auto directives = domains.by_kind(
        stageops::StageDomainKind::stage_set_directive_token);
    assert(directives.size() == 1U);
    assert(directives[0]->attributes.at("value") == "#SET");
    assert(directives[0]->attributes.at("offset") == "0");
    assert(directives[0]->attributes.at("line") == "1");
    assert(directives[0]->attributes.at("column") == "1");
    assert(directives[0]->attributes.at("runtime_object_claim") == "false");
    assert(directives[0]->id.find(":offset/0") != std::string::npos);

    const auto stage_set_values = domains.by_kind(
        stageops::StageDomainKind::stage_set_value_token);
    assert(stage_set_values.size() == 2U);
    assert(stage_set_values[0]->attributes.at("value") == "STAY");
    assert(stage_set_values[0]->attributes.at("offset") == "5");
    assert(stage_set_values[0]->attributes.at("line") == "1");
    assert(stage_set_values[0]->attributes.at("column") == "6");
    assert(stage_set_values[1]->attributes.at("value") == "ORBREAK");
    assert(stage_set_values[1]->attributes.at("line") == "3");
    assert(stage_set_values[1]->attributes.at("column") == "1");

    const auto doors = domains.by_kind(stageops::StageDomainKind::door_token);
    assert(doors.size() == 1U);
    assert(doors[0]->attributes.at("value") == "DOOR");
    assert(doors[0]->attributes.at("offset") == "10");
    assert(doors[0]->attributes.at("line") == "2");
    assert(doors[0]->attributes.at("column") == "1");

    const auto box_in = domains.by_kind(stageops::StageDomainKind::box_in_token);
    assert(box_in.size() == 1U);
    assert(box_in[0]->attributes.at("value") == "BoxIn");
    assert(box_in[0]->attributes.at("offset") == "15");
    assert(box_in[0]->attributes.at("line") == "2");
    assert(box_in[0]->attributes.at("column") == "6");

    const auto next_room = domains.by_kind(
        stageops::StageDomainKind::next_room_token);
    assert(next_room.size() == 1U);
    assert(next_room[0]->attributes.at("value") == "NextRoom");
    assert(next_room[0]->attributes.at("offset") == "21");
    assert(next_room[0]->attributes.at("line") == "2");
    assert(next_room[0]->attributes.at("column") == "12");

    // Edit collision bytes through the shared WorkingCopy. Stage Ops must not
    // reparse independently and script markers remain current because only HITS
    // changed. Use triangle.flags rather than structural HITS header bytes.
    assert(project.enable_working_copy(hits.id));
    const auto source_flags = hits_bytes[0x50U];
    const auto edit = project.apply_edit(
        hits.id,
        gdspaces::EditOperation{
            .id = "domain-stale-edit",
            .base_revision = 0U,
            .offset = 0x50U,
            .expected = {source_flags},
            .replacement = {std::byte{0x03}},
            .description = "Make only the retained HITS source parse stale.",
        },
        gdspaces::ToolTarget::stage_ops);
    assert(edit.applied);

    domains = stageops::StageDomainAssembler::assemble(
        assembly, project, 1U);
    assert(domains.stale_typed_result_count == 1U);
    assert(!domains.current_for_active_bytes());
    assert(!domains.by_kind(
        stageops::StageDomainKind::hits_collision)[0]
        ->current_for_active_bytes);
    assert(domains.by_kind(
        stageops::StageDomainKind::stage_script_tokens)[0]
        ->current_for_active_bytes);
    assert(domains.by_kind(stageops::StageDomainKind::door_token)[0]
        ->current_for_active_bytes);

    // Reset restores immutable source bytes. The retained source parse becomes
    // byte-current again without a second parser execution.
    assert(project.reset_working_copy(
        hits.id, gdspaces::ToolTarget::stage_ops));
    domains = stageops::StageDomainAssembler::assemble(
        assembly, project, 2U);
    assert(domains.stale_typed_result_count == 0U);
    assert(domains.current_for_active_bytes());
    assert(domains.by_kind(
        stageops::StageDomainKind::hits_collision)[0]
        ->current_for_active_bytes);

    return 0;
}
