#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/integration/stage_view_consistency.hpp"
#include "dmc_rengine/modviz/scene_workspace_view.hpp"
#include "dmc_rengine/stageops/workspace_view.hpp"

#include "hits_test_fixture.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "stage-tool-view-test",
            .logical_path = std::move(path),
            .container_chain = "NBZ[0]/PAC[4]",
            .offset = offset,
            .size = size,
        },
        .display_name = "resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ByteProvenance provenance(
    std::uint64_t offset,
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ByteProvenance{
        .kind = dmc::rengine::gdspaces::ByteOriginKind::direct_source_span,
        .authority_id = "stage-tool-view-test-source",
        .offset = offset,
        .stored_size = size,
        .materialized_size = size,
        .transform = dmc::rengine::gdspaces::ByteTransform::none,
        .crc32 = std::nullopt,
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload payload(
    dmc::rengine::gdspaces::ResourceRef reference,
    std::vector<std::byte> bytes) {
    const auto source_offset = reference.id.offset;
    const auto byte_count = static_cast<std::uint64_t>(bytes.size());
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = std::move(reference),
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = provenance(source_offset, byte_count),
    };
}

[[nodiscard]] dmc::rengine::stageops::StageAssemblyWorkspace make_assembly(
    const dmc::rengine::gdspaces::ResourceRef& hits,
    const dmc::rengine::gdspaces::ResourceRef& txt,
    std::string resource_set_id,
    std::string semantic_stage_id) {
    using namespace dmc::rengine;

    stageops::StageAssemblyWorkspace assembly;
    assembly.identity = stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = resource_set_id,
            .display_name = semantic_stage_id.empty()
                ? "Stage resource set " + resource_set_id
                : "Stage " + semantic_stage_id,
            .exe_evidence_id = "ev-dmc3-stage-resource-table",
            .resource_set_id = resource_set_id,
            .semantic_stage_id = semantic_stage_id,
            .numeric_stage_id = std::nullopt,
        },
        .catalog_entry_id = resource_set_id,
        .global_catalog_row = 1U,
        .source_table_id = "stage-tool-view-fixture-bank",
        .source_row_index = 1U,
    };

    assembly.requirements = {
        stageops::StageAssemblyRequirement{
            .requirement_id = "fixture/collision",
            .category = gdspaces::StageResourceCategory::collision,
            .role = "hits-source-1-member-6",
            .requested_logical_path = hits.id.logical_path,
            .resource_id = hits.id.canonical(),
            .materialized = true,
        },
        stageops::StageAssemblyRequirement{
            .requirement_id = "fixture/script",
            .category = gdspaces::StageResourceCategory::scripts,
            .role = "room-config-script",
            .requested_logical_path = txt.id.logical_path,
            .resource_id = txt.id.canonical(),
            .materialized = true,
        },
    };

    assembly.resources = {
        stageops::StageAssemblyResource{
            .resource = hits,
            .byte_provenance = provenance(hits.id.offset, hits.id.size),
            .materialized = true,
            .descriptor_root = true,
            .nested_container_child = false,
            .container_expansion_observed = false,
        },
        stageops::StageAssemblyResource{
            .resource = txt,
            .byte_provenance = provenance(txt.id.offset, txt.id.size),
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
            .role = "hits-source-1-member-6",
            .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
            .parent_resource_id = std::nullopt,
            .container_slot = std::nullopt,
        },
        stageops::StageAssemblyMembership{
            .resource_id = txt.id.canonical(),
            .category = gdspaces::StageResourceCategory::scripts,
            .role = "room-config-script",
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
    using dmc::rengine::gdspaces::EditOperation;
    using dmc::rengine::gdspaces::StageBundle;
    using dmc::rengine::gdspaces::StageIdentity;
    using dmc::rengine::gdspaces::StageMember;
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::gdspaces::ToolTarget;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::integration::WorkspaceContext;
    using dmc::rengine::integration::validate_stage_views;
    using dmc::rengine::modviz::VisualResourceKind;

    const auto hits_bytes =
        dmc::rengine::tests::hits_fixture::make_minimal_hits();
    const std::vector<std::byte> txt_bytes{
        std::byte{'#'}, std::byte{'S'}, std::byte{'E'}, std::byte{'T'}};
    const std::vector<std::byte> extra_bytes{std::byte{0x00}};

    const auto hits = resource(
        "room/st001cfg_006.hits", "hits", 4096U, hits_bytes.size());
    const auto txt = resource(
        "room/st001cfg_004.txt", "txt", 8192U, txt_bytes.size());
    const auto extra_model = resource(
        "room/project-only-extra.mod", "mod", 12288U, extra_bytes.size());

    ProjectWorkspace project;
    const WorkspaceContext context{
        .stage_context = true,
        .menu_context = false,
        .evidence_context = true,
    };
    assert(project.create_session(payload(hits, hits_bytes), context));
    assert(project.create_session(payload(txt, txt_bytes), context));
    assert(project.create_session(payload(extra_model, extra_bytes), context));
    assert(ResourceAnalyzer::analyze(project, hits.id).ok());

    // ProjectWorkspace intentionally contains an additional stage-tagged visual
    // resource. Canonical Stage Ops/ModViz projections must NOT rediscover it if
    // the StageAssemblyWorkspace does not contain it.
    StageBundle legacy_stage_tags(StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = "st001",
        .display_name = "Stage 001",
        .exe_evidence_id = "ev-dmc3-stage-resource-table",
        .resource_set_id = "st001",
        .semantic_stage_id = "st001",
        .numeric_stage_id = std::nullopt,
    });
    assert(legacy_stage_tags.add(StageMember{
        .category = StageResourceCategory::collision,
        .resource = hits,
        .role = "hits-source-1-member-6",
    }));
    assert(legacy_stage_tags.add(StageMember{
        .category = StageResourceCategory::scripts,
        .resource = txt,
        .role = "room-config-script",
    }));
    assert(legacy_stage_tags.add(StageMember{
        .category = StageResourceCategory::models,
        .resource = extra_model,
        .role = "project-only-extra",
    }));
    assert(project.attach_stage_bundle(legacy_stage_tags) == 3U);

    const auto assembly = make_assembly(hits, txt, "st001", "st001");
    assert(assembly.valid());

    auto stage_ops = dmc::rengine::stageops::build_workspace_view(
        assembly, &project);
    auto modviz = dmc::rengine::modviz::build_scene_workspace_view(
        assembly, &project);
    assert(stage_ops.valid());
    assert(modviz.valid());
    assert(stage_ops.identity.stage.resource_set_key() == "st001");
    assert(stage_ops.identity.stage.semantic_stage_id == "st001");
    assert(stage_ops.assembly_status ==
        dmc::rengine::stageops::StageAssemblyStatus::product_materialized);
    assert(stage_ops.game_readiness ==
        dmc::rengine::stageops::StageGameReadiness::unproven);
    assert(stage_ops.by_category(StageResourceCategory::collision).size() == 1U);
    assert(stage_ops.resources.size() == 2U);
    assert(modviz.by_kind(VisualResourceKind::collision).size() == 1U);
    assert(modviz.resources.size() == 1U);
    for (const auto& entry : stage_ops.resources) {
        assert(entry.resource.id.canonical() != extra_model.id.canonical());
    }
    for (const auto& entry : modviz.resources) {
        assert(entry.resource.id.canonical() != extra_model.id.canonical());
    }
    assert(validate_stage_views(stage_ops, modviz).consistent());

    assert(project.enable_working_copy(hits.id));
    const auto original = hits_bytes[0x06U];
    assert(project.apply_edit(
        hits.id,
        EditOperation{
            .id = "shared-stage-edit",
            .base_revision = 0U,
            .offset = 0x06U,
            .expected = {original},
            .replacement = {std::byte{9}},
            .description = "Edit shared HITS working copy.",
        },
        ToolTarget::stage_ops).applied);

    stage_ops = dmc::rengine::stageops::build_workspace_view(assembly, &project);
    modviz = dmc::rengine::modviz::build_scene_workspace_view(assembly, &project);
    assert(stage_ops.dirty_resource_count == 1U);
    assert(modviz.dirty_resource_count == 1U);
    assert(stage_ops.by_category(
        StageResourceCategory::collision)[0]->working_copy_revision == 1U);
    assert(modviz.by_kind(
        VisualResourceKind::collision)[0]->working_copy_revision == 1U);
    assert(validate_stage_views(stage_ops, modviz).consistent());

    auto role_mismatch = modviz;
    role_mismatch.resources[0].memberships[0].role = "different-role";
    assert(!validate_stage_views(stage_ops, role_mismatch).consistent());

    auto missing_visual = modviz;
    missing_visual.resources.clear();
    assert(!validate_stage_views(stage_ops, missing_visual).consistent());

    // A technical catalog/resource-set entry remains usable even when semantic
    // gameplay identity is unresolved. The Stage Ops aggregate is still the
    // source for both Stage Ops UI and ModViz.
    const auto catalog_hits = resource(
        "room/common_effects.hits", "hits", 16384U, hits_bytes.size());
    const auto catalog_txt = resource(
        "room/common_effects.txt", "txt", 20480U, txt_bytes.size());
    ProjectWorkspace unresolved_project;
    assert(unresolved_project.create_session(
        payload(catalog_hits, hits_bytes), context));
    assert(unresolved_project.create_session(
        payload(catalog_txt, txt_bytes), context));
    assert(ResourceAnalyzer::analyze(unresolved_project, catalog_hits.id).ok());

    const std::string catalog_row_id = "dmc3-stage-resource-table/row/17";
    const auto unresolved_assembly = make_assembly(
        catalog_hits, catalog_txt, catalog_row_id, {});
    assert(unresolved_assembly.valid());

    const auto unresolved_stage_ops =
        dmc::rengine::stageops::build_workspace_view(
            unresolved_assembly, &unresolved_project);
    const auto unresolved_modviz =
        dmc::rengine::modviz::build_scene_workspace_view(
            unresolved_assembly, &unresolved_project);
    assert(unresolved_stage_ops.valid());
    assert(unresolved_modviz.valid());
    assert(unresolved_stage_ops.identity.stage.resource_set_key() == catalog_row_id);
    assert(unresolved_stage_ops.identity.stage.semantic_stage_id.empty());
    assert(unresolved_modviz.identity.stage.semantic_stage_id.empty());
    assert(validate_stage_views(
        unresolved_stage_ops, unresolved_modviz).consistent());

    // Legacy overloads remain only as a migration bridge and must advertise a
    // partial/unproven scene instead of pretending project stage tags are the
    // canonical assembly authority.
    const auto legacy_stage_ops = dmc::rengine::stageops::build_workspace_view(
        project, "st001");
    const auto legacy_modviz = dmc::rengine::modviz::build_scene_workspace_view(
        project, "st001");
    assert(legacy_stage_ops.valid());
    assert(legacy_modviz.valid());
    assert(legacy_stage_ops.assembly_status ==
        dmc::rengine::stageops::StageAssemblyStatus::partial);
    assert(legacy_modviz.assembly_status ==
        dmc::rengine::stageops::StageAssemblyStatus::partial);

    return 0;
}
