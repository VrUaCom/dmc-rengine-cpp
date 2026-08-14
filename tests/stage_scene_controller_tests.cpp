#include "dmc_rengine/stageops/domain_queries.hpp"
#include "dmc_rengine/stageops/scene_controller.hpp"

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

[[nodiscard]] stageops::StageAssemblyWorkspace make_assembly(
    const gdspaces::ResourceRef& txt,
    const gdspaces::ByteProvenance& provenance) {
    stageops::StageAssemblyWorkspace assembly;
    assembly.identity = stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "stage-scene-controller-fixture",
            .display_name = "Stage scene controller fixture",
            .exe_evidence_id = "stage-scene-controller-evidence",
            .resource_set_id = "stage-scene-controller-fixture",
            .semantic_stage_id = {},
            .numeric_stage_id = 1U,
        },
        .catalog_entry_id = "stage-scene-controller-fixture",
        .global_catalog_row = 0U,
        .source_table_id = "stage-scene-controller-table",
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

[[nodiscard]] stageops::StageRuntimeLink runtime_link(
    std::string id,
    std::string domain_object_id) {
    return stageops::StageRuntimeLink{
        .id = std::move(id),
        .domain_object_id = std::move(domain_object_id),
        .runtime = stageops::RecoveredRuntimeIdentity{
            .id = "fixture/runtime/stage-set-classifier",
            .source_tree_path =
                "recovered-game/runtime/stage/stage_set_classifier.cpp",
            .symbol = "StageSetTokenClassifier",
            .executable_artifact_id = "fixture-dmc3-hd-executable",
            .evidence_ids = {"fixture-stage-set-classifier-evidence"},
        },
        .kind = stageops::StageRuntimeLinkKind::classified_by_runtime_function,
        .authority =
            stageops::StageRuntimeLinkAuthority::disassembly_complete_corpus_pending,
        .claim_id = "fixture-stage-set-classifier-claim",
    };
}

} // namespace

int main() {
    using namespace dmc::rengine;

    const auto bytes = text_bytes(
        "#SET STAY\n"
        "DOOR BoxIn NextRoom \"st002\"\n");
    const gdspaces::ResourceRef txt{
        .id = gdspaces::ResourceId{
            .source_id = "stage-scene-controller-test",
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
        .authority_id = "stage-scene-controller-source",
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

    stageops::StageOperationsSession session(
        make_assembly(txt, provenance), project);
    assert(session.valid());

    auto initial = stageops::StageSceneController::refresh(session);
    assert(initial.refreshed());
    assert(initial.status == stageops::StageSceneRefreshStatus::refreshed);
    assert(initial.target_stage_revision == 0U);
    assert(initial.final_stage_revision == 0U);
    assert(initial.analysis.attempted == 1U);
    assert(initial.analysis.succeeded == 1U);
    assert(!initial.external_change_detected);

    const auto initial_values = stageops::domain_objects_by_kind_source_order(
        initial.snapshot.domains(),
        stageops::StageDomainKind::stage_set_value_token);
    assert(initial_values.size() == 1U);
    assert(initial_values[0]->attributes.at("value") == "STAY");
    const auto stable_value_domain_id = initial_values[0]->id;

    assert(session.enable_editing(txt.id.canonical()));
    const auto edit = session.apply_edit(
        txt.id.canonical(),
        gdspaces::EditOperation{
            .id = "controller-stay-to-seal",
            .base_revision = 0U,
            .offset = 5U,
            .expected = {
                std::byte{'S'}, std::byte{'T'}, std::byte{'A'}, std::byte{'Y'}},
            .replacement = {
                std::byte{'S'}, std::byte{'E'}, std::byte{'A'}, std::byte{'L'}},
            .description = "Valid Stage TXT edit for controller refresh.",
        });
    assert(edit.applied && edit.stage_revision == 1U);

    std::vector<stageops::StageRuntimeLink> links;
    links.push_back(runtime_link(
        "fixture/runtime-link/seal-classifier",
        stable_value_domain_id));
    auto refreshed = stageops::StageSceneController::refresh(
        session, std::move(links));
    assert(refreshed.refreshed());
    assert(refreshed.target_stage_revision == 1U);
    assert(refreshed.final_stage_revision == 1U);
    assert(refreshed.analysis.reports.size() == 1U);
    assert(refreshed.analysis.reports[0].byte_source ==
        integration::ParsedByteSource::working_copy);
    assert(refreshed.analysis.reports[0].byte_revision == 1U);
    assert(refreshed.runtime_link_validation.valid());
    assert(refreshed.runtime_link_validation.accepted_link_count == 1U);
    const auto seal_values = stageops::domain_objects_by_kind_source_order(
        refreshed.snapshot.domains(),
        stageops::StageDomainKind::stage_set_value_token);
    assert(seal_values.size() == 1U);
    assert(seal_values[0]->attributes.at("value") == "SEAL");
    assert(refreshed.snapshot.runtime_links().links.size() == 1U);

    const auto edit_back = session.apply_edit(
        txt.id.canonical(),
        gdspaces::EditOperation{
            .id = "controller-seal-to-stay",
            .base_revision = 1U,
            .offset = 5U,
            .expected = {
                std::byte{'S'}, std::byte{'E'}, std::byte{'A'}, std::byte{'L'}},
            .replacement = {
                std::byte{'S'}, std::byte{'T'}, std::byte{'A'}, std::byte{'Y'}},
            .description = "Create another valid scene revision.",
        });
    assert(edit_back.applied && edit_back.stage_revision == 2U);

    std::vector<stageops::StageRuntimeLink> bad_links;
    bad_links.push_back(runtime_link(
        "fixture/runtime-link/missing-domain",
        "stage-domain:missing-domain-object"));
    const auto rejected = stageops::StageSceneController::refresh(
        session, std::move(bad_links));
    assert(!rejected.refreshed());
    assert(rejected.status ==
        stageops::StageSceneRefreshStatus::runtime_links_invalid);
    assert(!rejected.runtime_link_validation.valid());
    assert(rejected.runtime_link_validation.accepted_link_count == 0U);
    assert(session.stage_revision() == 2U);
    assert(session.derived_state_stale());

    const auto recovered = stageops::StageSceneController::refresh(session);
    assert(recovered.refreshed());
    assert(recovered.target_stage_revision == 2U);
    assert(recovered.final_stage_revision == 2U);
    assert(!session.derived_state_stale());
    const auto recovered_values = stageops::domain_objects_by_kind_source_order(
        recovered.snapshot.domains(),
        stageops::StageDomainKind::stage_set_value_token);
    assert(recovered_values.size() == 1U);
    assert(recovered_values[0]->attributes.at("value") == "STAY");

    // A structurally invalid TXT WorkingCopy must fail at the canonical analyzer
    // gate. No derived refresh commit is allowed and the stale flag survives.
    const auto bad_edit = session.apply_edit(
        txt.id.canonical(),
        gdspaces::EditOperation{
            .id = "controller-invalid-txt",
            .base_revision = 2U,
            .offset = 0U,
            .expected = {std::byte{'#'}},
            .replacement = {std::byte{0}},
            .description = "Introduce an invalid NUL byte for parser-failure regression.",
        });
    assert(bad_edit.applied && bad_edit.stage_revision == 3U);
    const auto parser_failed = stageops::StageSceneController::refresh(session);
    assert(!parser_failed.refreshed());
    assert(parser_failed.status ==
        stageops::StageSceneRefreshStatus::analysis_failed);
    assert(parser_failed.analysis.attempted == 1U);
    assert(parser_failed.analysis.succeeded == 0U);
    assert(session.stage_revision() == 3U);
    assert(session.derived_state_stale());

    return 0;
}
