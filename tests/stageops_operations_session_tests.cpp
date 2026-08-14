#include "dmc_rengine/stageops/operations_session.hpp"

#include "hits_test_fixture.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace dmc::rengine;

[[nodiscard]] gdspaces::ResourceRef resource(
    std::string path,
    std::uint64_t offset,
    std::uint64_t size) {
    return gdspaces::ResourceRef{
        .id = gdspaces::ResourceId{
            .source_id = "stageops-operations-test",
            .logical_path = std::move(path),
            .container_chain = "PAC[0]",
            .offset = offset,
            .size = size,
        },
        .display_name = "stage resource",
        .format = "hits",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] gdspaces::ByteProvenance provenance(
    std::uint64_t offset,
    std::uint64_t size) {
    return gdspaces::ByteProvenance{
        .kind = gdspaces::ByteOriginKind::direct_source_span,
        .authority_id = "stageops-operations-source",
        .offset = offset,
        .stored_size = size,
        .materialized_size = size,
        .transform = gdspaces::ByteTransform::none,
        .crc32 = std::nullopt,
    };
}

[[nodiscard]] stageops::StageAssemblyWorkspace make_assembly(
    const gdspaces::ResourceRef& editable,
    const gdspaces::ResourceRef& materialized_without_session) {
    stageops::StageAssemblyWorkspace assembly;
    assembly.identity = stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "dmc3-stage-row-test",
            .display_name = "Stage resource set dmc3-stage-row-test",
            .exe_evidence_id = "stageops-operations-evidence",
            .resource_set_id = "dmc3-stage-row-test",
            .semantic_stage_id = {},
            .numeric_stage_id = 308U,
        },
        .catalog_entry_id = "dmc3-stage-row-test",
        .global_catalog_row = 110U,
        .source_table_id = "dmc3-stage-wave2-bank-b",
        .source_row_index = 0U,
    };

    assembly.requirements = {
        stageops::StageAssemblyRequirement{
            .requirement_id = "fixture/editable-collision",
            .category = gdspaces::StageResourceCategory::collision,
            .role = "collision",
            .requested_logical_path = editable.id.logical_path,
            .resource_id = editable.id.canonical(),
            .materialized = true,
        },
        stageops::StageAssemblyRequirement{
            .requirement_id = "fixture/no-session-collision",
            .category = gdspaces::StageResourceCategory::collision,
            .role = "collision-shared",
            .requested_logical_path = materialized_without_session.id.logical_path,
            .resource_id = materialized_without_session.id.canonical(),
            .materialized = true,
        },
    };

    assembly.resources = {
        stageops::StageAssemblyResource{
            .resource = editable,
            .byte_provenance = provenance(editable.id.offset, editable.id.size),
            .materialized = true,
            .descriptor_root = true,
            .nested_container_child = false,
            .container_expansion_observed = false,
        },
        stageops::StageAssemblyResource{
            .resource = materialized_without_session,
            .byte_provenance = provenance(
                materialized_without_session.id.offset,
                materialized_without_session.id.size),
            .materialized = true,
            .descriptor_root = true,
            .nested_container_child = false,
            .container_expansion_observed = false,
        },
    };

    assembly.memberships = {
        stageops::StageAssemblyMembership{
            .resource_id = editable.id.canonical(),
            .category = gdspaces::StageResourceCategory::collision,
            .role = "collision",
            .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
            .parent_resource_id = std::nullopt,
            .container_slot = std::nullopt,
        },
        stageops::StageAssemblyMembership{
            .resource_id = materialized_without_session.id.canonical(),
            .category = gdspaces::StageResourceCategory::collision,
            .role = "collision-shared",
            .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
            .parent_resource_id = std::nullopt,
            .container_slot = std::nullopt,
        },
    };
    assembly.product_materialization_complete = true;
    assembly.game_ready_equivalent = false;
    return assembly;
}

} // namespace

int main() {
    using namespace dmc::rengine;

    const auto bytes = tests::hits_fixture::make_minimal_hits();
    const auto editable = resource(
        "room/st308cfg_006.hits",
        0x1000U,
        static_cast<std::uint64_t>(bytes.size()));
    const auto no_session = resource(
        "room/shared_collision.hits",
        0x2000U,
        static_cast<std::uint64_t>(bytes.size()));

    integration::ProjectWorkspace project;
    assert(project.create_session(
        gdspaces::ResourcePayload{
            .resource = editable,
            .bytes = bytes,
            .diagnostics = {},
            .byte_provenance = provenance(
                editable.id.offset,
                static_cast<std::uint64_t>(bytes.size())),
        },
        integration::WorkspaceContext{
            .stage_context = true,
            .menu_context = false,
            .evidence_context = true,
        }));

    auto assembly = make_assembly(editable, no_session);
    assert(assembly.valid());

    stageops::StageOperationsSession session(std::move(assembly), project);
    assert(session.valid());
    assert(session.stage_revision() == 0U);
    assert(!session.derived_state_stale());

    auto snapshot = session.snapshot();
    assert(snapshot.valid());
    assert(snapshot.assembly_status ==
        stageops::StageAssemblyStatus::product_materialized);
    assert(snapshot.game_readiness == stageops::StageGameReadiness::unproven);
    assert(snapshot.dirty_resource_count == 0U);
    assert(snapshot.missing_project_session_count == 1U);
    assert(snapshot.validation_request_count == 0U);

    // WorkingCopy enablement is operational setup, not a byte mutation.
    assert(session.enable_editing(editable.id.canonical()));
    assert(session.stage_revision() == 0U);
    assert(!session.derived_state_stale());

    const auto source_06 = bytes[0x06U];
    const auto first = session.apply_edit(
        editable.id.canonical(),
        gdspaces::EditOperation{
            .id = "stageops-first-edit",
            .base_revision = 0U,
            .offset = 0x06U,
            .expected = {source_06},
            .replacement = {std::byte{0x09}},
            .description = "Stage Ops collision edit.",
        });
    assert(first.applied);
    assert(first.stage_revision == 1U);
    assert(first.resource_revision == 1U);
    assert(session.derived_state_stale());

    snapshot = session.snapshot();
    assert(snapshot.stage_revision == 1U);
    assert(snapshot.dirty_resource_count == 1U);
    assert(snapshot.derived_state_stale);

    // Stale state cannot be cleared using a revision other than the scene state
    // that was actually rebuilt.
    assert(!session.commit_derived_refresh(0U));
    assert(session.commit_derived_refresh(1U));
    assert(!session.derived_state_stale());

    // One resource lacks a ProjectWorkspace session, so only the editable
    // resource can receive an actual Build & Test Lab request.
    assert(session.request_validation(
        "stageops-validation-1",
        "Validate the current Stage Ops scene revision.") == 1U);
    snapshot = session.snapshot();
    assert(snapshot.validation_request_count == 1U);

    // Change the same WorkingCopy through ProjectWorkspace directly. Stage Ops
    // must detect that its derived scene state is no longer synchronized.
    const auto source_07 = bytes[0x07U];
    const auto external = project.apply_edit(
        editable.id,
        gdspaces::EditOperation{
            .id = "external-stage-edit",
            .base_revision = 1U,
            .offset = 0x07U,
            .expected = {source_07},
            .replacement = {std::byte{0x0A}},
            .description = "External project edit seen by Stage Ops.",
        },
        gdspaces::ToolTarget::stage_ops);
    assert(external.applied);
    assert(session.stage_revision() == 1U);
    assert(session.detect_external_project_changes());
    assert(session.stage_revision() == 2U);
    assert(session.derived_state_stale());

    // The observed baseline advances when the external change is detected, so
    // repeated polling does not manufacture additional revisions.
    assert(!session.detect_external_project_changes());
    assert(session.stage_revision() == 2U);

    const auto undone = session.undo_last_edit(editable.id.canonical());
    assert(undone.applied);
    assert(undone.stage_revision == 3U);
    assert(undone.resource_revision == 3U);

    // Reset removes the remaining first edit and is a real byte-state change.
    const auto reset = session.reset_resource(editable.id.canonical());
    assert(reset.applied);
    assert(reset.stage_revision == 4U);
    assert(reset.resource_revision == 0U);
    assert(session.snapshot().dirty_resource_count == 0U);

    // Resetting an already clean revision-0 WorkingCopy records the resource
    // event but does not invent a new scene revision because bytes did not move.
    const auto noop_reset = session.reset_resource(editable.id.canonical());
    assert(noop_reset.applied);
    assert(noop_reset.stage_revision == 4U);
    assert(noop_reset.resource_revision == 0U);

    assert(session.commit_derived_refresh(4U));
    assert(!session.derived_state_stale());

    // Scene authority boundary: resources outside the aggregate cannot be
    // edited merely because ProjectWorkspace might know about them.
    const auto rejected = session.apply_edit(
        "not-a-stage-resource",
        gdspaces::EditOperation{
            .id = "rejected-edit",
            .base_revision = 0U,
            .offset = 0U,
            .expected = {},
            .replacement = {},
            .description = "Must be rejected before ProjectWorkspace mutation.",
        });
    assert(!rejected.applied);
    assert(rejected.stage_revision == 4U);

    return 0;
}
