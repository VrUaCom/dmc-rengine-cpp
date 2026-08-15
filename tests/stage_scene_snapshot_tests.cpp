#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/stageops/domain_queries.hpp"
#include "dmc_rengine/stageops/scene_snapshot.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
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

[[nodiscard]] stageops::StageAssemblyWorkspace make_assembly(
    const gdspaces::ResourceRef& txt,
    const gdspaces::ByteProvenance& provenance) {
    stageops::StageAssemblyWorkspace assembly;
    assembly.identity = stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "stage-scene-snapshot-fixture",
            .display_name = "Stage scene snapshot fixture",
            .exe_evidence_id = "stage-scene-snapshot-evidence",
            .resource_set_id = "stage-scene-snapshot-fixture",
            .semantic_stage_id = {},
            .numeric_stage_id = 1U,
        },
        .catalog_entry_id = "stage-scene-snapshot-fixture",
        .global_catalog_row = 0U,
        .source_table_id = "stage-scene-snapshot-table",
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
    return assembly;
}

} // namespace

int main() {
    using namespace dmc::rengine;

    const auto bytes = text_bytes(
        "#SET STAY\n"
        "DOOR BoxIn NextRoom \"st002\"\n");
    const gdspaces::ResourceRef txt{
        .id = gdspaces::ResourceId{
            .source_id = "stage-scene-snapshot-test",
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
        .authority_id = "stage-scene-snapshot-source",
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

    stageops::StageOperationsSession session(
        make_assembly(txt, provenance), project);
    assert(session.valid());

    auto source_snapshot = stageops::StageSceneSnapshotBuilder::build(session);
    assert(source_snapshot.valid());
    assert(source_snapshot.stage_revision() == 0U);
    assert(source_snapshot.current_for_active_bytes());
    assert(source_snapshot.knowledge.valid());
    assert(source_snapshot.relations().by_kind(
        stageops::StageDomainRelationKind::contains_marker).size() == 5U);
    assert(source_snapshot.relations().by_kind(
        stageops::StageDomainRelationKind::lexical_next_marker).size() == 4U);
    assert(source_snapshot.runtime_links().links.empty());

    const auto source_values = stageops::domain_objects_by_kind_source_order(
        source_snapshot.domains(),
        stageops::StageDomainKind::stage_set_value_token);
    assert(source_values.size() == 1U);
    assert(source_values[0]->attributes.at("value") == "STAY");
    assert(source_values[0]->current_for_active_bytes);

    assert(session.enable_editing(txt.id.canonical()));
    const auto edit = session.apply_edit(
        txt.id.canonical(),
        gdspaces::EditOperation{
            .id = "stage-scene-snapshot-stay-to-seal",
            .base_revision = 0U,
            .offset = 5U,
            .expected = {
                std::byte{'S'}, std::byte{'T'}, std::byte{'A'}, std::byte{'Y'}},
            .replacement = {
                std::byte{'S'}, std::byte{'E'}, std::byte{'A'}, std::byte{'L'}},
            .description = "Change one valid StageSet lexical value in-place.",
        });
    assert(edit.applied);
    assert(edit.stage_revision == 1U);
    assert(session.derived_state_stale());

    // Before canonical re-analysis the old source parse remains visible only as
    // stale diagnostic knowledge; it must not be presented as current scene data.
    const auto stale_snapshot = stageops::StageSceneSnapshotBuilder::build(session);
    assert(stale_snapshot.valid());
    assert(stale_snapshot.stage_revision() == 1U);
    assert(!stale_snapshot.current_for_active_bytes());
    assert(stale_snapshot.requires_derived_refresh());
    const auto stale_values = stageops::domain_objects_by_kind_source_order(
        stale_snapshot.domains(),
        stageops::StageDomainKind::stage_set_value_token);
    assert(stale_values.size() == 1U);
    assert(stale_values[0]->attributes.at("value") == "STAY");
    assert(!stale_values[0]->current_for_active_bytes);

    const auto analysis = session.refresh_resource_analysis();
    assert(analysis.stage_revision == 1U);
    assert(analysis.attempted == 1U);
    assert(analysis.succeeded == 1U);
    assert(analysis.complete_for_attempted());
    assert(analysis.reports.size() == 1U);
    assert(analysis.reports[0].byte_source ==
        integration::ParsedByteSource::working_copy);
    assert(analysis.reports[0].byte_revision == 1U);

    // Parser/domain knowledge now matches the active WorkingCopy, but the Stage
    // Ops derived-state stale gate remains set until the exact revision commits.
    const auto provisional = stageops::StageSceneSnapshotBuilder::build(session);
    assert(provisional.valid());
    assert(!provisional.current_for_active_bytes());
    assert(provisional.knowledge.current_for_active_bytes());
    const auto provisional_values = stageops::domain_objects_by_kind_source_order(
        provisional.domains(),
        stageops::StageDomainKind::stage_set_value_token);
    assert(provisional_values.size() == 1U);
    assert(provisional_values[0]->attributes.at("value") == "SEAL");
    assert(provisional_values[0]->current_for_active_bytes);

    const auto seal_domain_id = provisional_values[0]->id;
    std::vector<stageops::StageRuntimeLink> runtime_links;
    runtime_links.push_back(stageops::StageRuntimeLink{
        .id = "fixture/runtime-link/seal-classifier",
        .domain_object_id = seal_domain_id,
        .runtime = stageops::RecoveredRuntimeIdentity{
            .id = "fixture/runtime/stage-set-token-classifier",
            .source_tree_path =
                "recovered-game/runtime/stage/stage_set_classifier.cpp",
            .symbol = "StageSetTokenClassifier",
            .executable_artifact_id = "fixture-dmc3-hd-executable",
            .evidence_ids = {"fixture-stage-set-classifier-evidence"},
        },
        .kind = stageops::StageRuntimeLinkKind::classified_by_runtime_function,
        .authority =
            stageops::StageRuntimeLinkAuthority::disassembly_complete_corpus_pending,
        .claim_id = "fixture-claim-stage-set-seal-classification",
    });

    assert(session.commit_derived_refresh(1U));
    assert(!session.derived_state_stale());

    const auto final_snapshot = stageops::StageSceneSnapshotBuilder::build(
        session, std::move(runtime_links));
    assert(final_snapshot.valid());
    assert(final_snapshot.stage_revision() == 1U);
    assert(final_snapshot.current_for_active_bytes());
    assert(final_snapshot.runtime_links().links.size() == 1U);
    assert(final_snapshot.semantic_graph.by_kind(
        stageops::semantic::NodeKind::recovered_runtime).size() == 1U);
    const auto runtime_edges = final_snapshot.semantic_graph.outgoing(
        seal_domain_id,
        stageops::semantic::EdgeKind::runtime_link);
    assert(runtime_edges.size() == 1U);
    assert(runtime_edges[0]->authority ==
        stageops::semantic::Authority::recovered_runtime_partial);
    assert(runtime_edges[0]->attributes.at("runtime_link_authority") ==
        "disassembly-complete-corpus-pending");

    return 0;
}
