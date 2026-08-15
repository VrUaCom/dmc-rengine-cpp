#include "dmc_rengine/bridges/dmc3/recovered_stage_links.hpp"

#include <algorithm>
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

[[nodiscard]] stageops::StageAssemblyIdentity identity() {
    return stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "dmc3-recovered-bridge-fixture",
            .display_name = "DMC3 recovered bridge fixture",
            .exe_evidence_id = "dmc3-door-parser-contract",
            .resource_set_id = "dmc3-recovered-bridge-fixture",
            .semantic_stage_id = {},
            .numeric_stage_id = 1U,
        },
        .catalog_entry_id = "dmc3-recovered-bridge-fixture",
        .global_catalog_row = 0U,
        .source_table_id = "fixture-stage-table",
        .source_row_index = 0U,
    };
}

} // namespace

int main() {
    using namespace dmc::rengine;

    // Unit-level bridge contract: only exact Door/BoxIn marker kinds receive
    // Door parser links. Co-located StageSet markers must remain unrelated.
    stageops::StageDomainWorkspace domains;
    domains.identity = identity();
    domains.source_stage_revision = 7U;

    const auto make_object = [](std::string id, stageops::StageDomainKind kind) {
        return stageops::StageDomainObject{
            .id = std::move(id),
            .kind = kind,
            .resource_id = "fixture-resource-id",
            .parser_id = "formats.stage-txt-lexer",
            .byte_source = integration::ParsedByteSource::immutable_source,
            .byte_revision = 0U,
            .current_for_active_bytes = true,
            .attributes = {},
        };
    };

    domains.objects.push_back(make_object(
        "stage-domain:door-token:fixture:offset/10",
        stageops::StageDomainKind::door_token));
    domains.objects.push_back(make_object(
        "stage-domain:box-in-token:fixture:offset/15",
        stageops::StageDomainKind::box_in_token));
    domains.objects.push_back(make_object(
        "stage-domain:stage-set-value-token:fixture:offset/5",
        stageops::StageDomainKind::stage_set_value_token));
    assert(domains.valid());

    const auto report =
        bridges::dmc3::RecoveredStageLinkProvider::build(domains);
    assert(report.valid());
    assert(report.door_marker_count == 1U);
    assert(report.box_in_marker_count == 1U);
    assert(report.links.size() == 2U);

    for (const auto& link : report.links) {
        assert(link.valid());
        assert(link.kind ==
            stageops::StageRuntimeLinkKind::parsed_by_runtime_consumer);
        assert(link.authority == stageops::StageRuntimeLinkAuthority::
            disassembly_complete_corpus_pending);
        assert(link.runtime.id == "dmc3/runtime/stage/door-txt-parser");
        assert(link.runtime.source_tree_path ==
            "recovered-game/runtime/stage/door_parser_contract.cpp");
        assert(link.runtime.symbol == "door_txt_parser@0x1401A9DE0");
        assert(link.runtime.executable_artifact_id ==
            "dmc3-hdc-exe-e454272e");
        assert(link.runtime.evidence_ids.size() == 4U);
        assert(std::find(
            link.runtime.evidence_ids.begin(),
            link.runtime.evidence_ids.end(),
            "ev-dmc3-door-parser-logical-function") !=
            link.runtime.evidence_ids.end());
    }

    const auto* door_link = &report.links[0];
    const auto* box_in_link = &report.links[1];
    if (door_link->domain_object_id.find("door-token") == std::string::npos) {
        std::swap(door_link, box_in_link);
    }
    assert(door_link->domain_object_id.find("door-token") != std::string::npos);
    assert(door_link->claim_id ==
        "claim-dmc3-door-parser-logical-function");
    assert(box_in_link->domain_object_id.find("box-in-token") !=
        std::string::npos);
    assert(box_in_link->claim_id == "claim-dmc3-door-box-grammar");

    assert(std::none_of(
        report.links.begin(), report.links.end(),
        [](const stageops::StageRuntimeLink& link) {
            return link.domain_object_id.find("stage-set-value-token") !=
                std::string::npos;
        }));

    const auto workspace = stageops::StageRuntimeLinkBuilder::build(
        domains, report.links);
    assert(workspace.valid());
    assert(workspace.source_stage_revision == 7U);
    assert(workspace.links.size() == 2U);

    // End-to-end composition: recovered links are generated only after the
    // controller has parsed the exact active TXT bytes for the target revision.
    const auto bytes = text_bytes("DOOR BoxIn NextRoom\n");
    const gdspaces::ResourceRef txt{
        .id = gdspaces::ResourceId{
            .source_id = "dmc3-recovered-bridge-scene-test",
            .logical_path = "room/st001cfg_000.txt",
            .container_chain = "PAC[0]",
            .offset = 0x3000U,
            .size = static_cast<std::uint64_t>(bytes.size()),
        },
        .display_name = "Door TXT fixture",
        .format = "txt",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
    const gdspaces::ByteProvenance provenance{
        .kind = gdspaces::ByteOriginKind::direct_source_span,
        .authority_id = "dmc3-recovered-bridge-source",
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

    stageops::StageAssemblyWorkspace assembly;
    assembly.identity = identity();
    assembly.requirements.push_back(stageops::StageAssemblyRequirement{
        .requirement_id = "fixture/door-txt",
        .category = gdspaces::StageResourceCategory::scripts,
        .role = "door-script",
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
        .role = "door-script",
        .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
        .parent_resource_id = std::nullopt,
        .container_slot = std::nullopt,
    });
    assembly.product_materialization_complete = true;
    assert(assembly.valid());

    stageops::StageOperationsSession session(std::move(assembly), project);
    const auto refreshed =
        bridges::dmc3::RecoveredStageSceneController::refresh(session);
    assert(refreshed.refreshed());
    assert(refreshed.runtime_link_validation.valid());
    assert(refreshed.runtime_link_validation.accepted_link_count == 2U);
    assert(refreshed.snapshot.runtime_links().links.size() == 2U);
    assert(refreshed.snapshot.semantic_graph.by_kind(
        stageops::semantic::NodeKind::recovered_runtime).size() == 1U);

    const auto door_domains = refreshed.snapshot.domains().by_kind(
        stageops::StageDomainKind::door_token);
    const auto box_in_domains = refreshed.snapshot.domains().by_kind(
        stageops::StageDomainKind::box_in_token);
    const auto next_room_domains = refreshed.snapshot.domains().by_kind(
        stageops::StageDomainKind::next_room_token);
    assert(door_domains.size() == 1U);
    assert(box_in_domains.size() == 1U);
    assert(next_room_domains.size() == 1U);
    assert(door_domains[0]->attributes.at("offset") == "0");
    assert(box_in_domains[0]->attributes.at("offset") == "5");
    assert(next_room_domains[0]->attributes.at("offset") == "11");

    const std::string source_door_domain_id = door_domains[0]->id;
    const std::string source_box_in_domain_id = box_in_domains[0]->id;

    assert(refreshed.snapshot.semantic_graph.outgoing(
        source_door_domain_id,
        stageops::semantic::EdgeKind::runtime_link).size() == 1U);
    assert(refreshed.snapshot.semantic_graph.outgoing(
        source_box_in_domain_id,
        stageops::semantic::EdgeKind::runtime_link).size() == 1U);

    // NextRoom remains a structural lexical marker until its exact parser/runtime
    // ownership is separately promoted. It must not inherit the Door parser link
    // merely because it appears on the same line/resource.
    assert(refreshed.snapshot.semantic_graph.outgoing(
        next_room_domains[0]->id,
        stageops::semantic::EdgeKind::runtime_link).empty());

    // Offset-shifting edit regression. Inserting a lexer comment changes every
    // subsequent token byte offset while preserving the Door/BoxIn token stream.
    // Runtime links must therefore be regenerated from domains@WorkingCopy-1;
    // precomputed revision-0 links would point at stale, nonexistent domain IDs.
    assert(session.enable_editing(txt.id.canonical()));
    const auto insert_comment = session.apply_edit(
        txt.id.canonical(),
        gdspaces::EditOperation{
            .id = "shift-door-token-offsets",
            .base_revision = 0U,
            .offset = 0U,
            .expected = {},
            .replacement = {
                std::byte{'/'}, std::byte{'/'}, std::byte{'x'}, std::byte{'\n'}},
            .description =
                "Insert an ignored lexer comment before Door tokens to shift exact source offsets.",
        });
    assert(insert_comment.applied);
    assert(insert_comment.stage_revision == 1U);
    assert(session.derived_state_stale());

    const auto shifted =
        bridges::dmc3::RecoveredStageSceneController::refresh(session);
    assert(shifted.refreshed());
    assert(shifted.target_stage_revision == 1U);
    assert(shifted.final_stage_revision == 1U);
    assert(shifted.analysis.reports.size() == 1U);
    assert(shifted.analysis.reports[0].byte_source ==
        integration::ParsedByteSource::working_copy);
    assert(shifted.analysis.reports[0].byte_revision == 1U);
    assert(shifted.runtime_link_validation.accepted_link_count == 2U);

    const auto shifted_doors = shifted.snapshot.domains().by_kind(
        stageops::StageDomainKind::door_token);
    const auto shifted_box_in = shifted.snapshot.domains().by_kind(
        stageops::StageDomainKind::box_in_token);
    const auto shifted_next_room = shifted.snapshot.domains().by_kind(
        stageops::StageDomainKind::next_room_token);
    assert(shifted_doors.size() == 1U);
    assert(shifted_box_in.size() == 1U);
    assert(shifted_next_room.size() == 1U);
    assert(shifted_doors[0]->attributes.at("offset") == "4");
    assert(shifted_box_in[0]->attributes.at("offset") == "9");
    assert(shifted_next_room[0]->attributes.at("offset") == "15");
    assert(shifted_doors[0]->id != source_door_domain_id);
    assert(shifted_box_in[0]->id != source_box_in_domain_id);

    assert(shifted.snapshot.semantic_graph.find_node(source_door_domain_id) == nullptr);
    assert(shifted.snapshot.semantic_graph.find_node(source_box_in_domain_id) == nullptr);
    assert(shifted.snapshot.semantic_graph.outgoing(
        shifted_doors[0]->id,
        stageops::semantic::EdgeKind::runtime_link).size() == 1U);
    assert(shifted.snapshot.semantic_graph.outgoing(
        shifted_box_in[0]->id,
        stageops::semantic::EdgeKind::runtime_link).size() == 1U);
    assert(shifted.snapshot.semantic_graph.outgoing(
        shifted_next_room[0]->id,
        stageops::semantic::EdgeKind::runtime_link).empty());

    for (const auto& link : shifted.snapshot.runtime_links().links) {
        assert(link.domain_object_id != source_door_domain_id);
        assert(link.domain_object_id != source_box_in_domain_id);
        assert(link.runtime.evidence_ids.size() == 4U);
    }

    return 0;
}
