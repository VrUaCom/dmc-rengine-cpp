#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/integration/stage_view_consistency.hpp"
#include "dmc_rengine/modviz/scene_workspace_view.hpp"
#include "dmc_rengine/stageops/workspace_view.hpp"

#include "hits_test_fixture.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
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

} // namespace

int main() {
    using dmc::rengine::gdspaces::EditOperation;
    using dmc::rengine::gdspaces::ResourcePayload;
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
    const auto hits = resource(
        "room/st001cfg_006.hits", "hits", 4096U, hits_bytes.size());
    const auto txt = resource(
        "room/st001cfg_004.txt", "txt", 8192U, txt_bytes.size());

    ProjectWorkspace project;
    const WorkspaceContext context{
        .stage_context = true,
        .menu_context = false,
        .evidence_context = true,
    };
    assert(project.create_session(ResourcePayload{
        .resource = hits,
        .bytes = hits_bytes,
        .diagnostics = {},
    }, context));
    assert(project.create_session(ResourcePayload{
        .resource = txt,
        .bytes = txt_bytes,
        .diagnostics = {},
    }, context));
    assert(ResourceAnalyzer::analyze(project, hits.id).ok());

    StageBundle stage(StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = "st001",
        .display_name = "Stage 001",
        .exe_evidence_id = "ev-dmc3-stage-resource-table",
    });
    assert(stage.add(StageMember{
        .category = StageResourceCategory::collision,
        .resource = hits,
        .role = "hits-source-1-member-6",
    }));
    assert(stage.add(StageMember{
        .category = StageResourceCategory::scripts,
        .resource = txt,
        .role = "room-config-script",
    }));
    assert(project.attach_stage_bundle(stage) == 2U);

    auto stage_ops = dmc::rengine::stageops::build_workspace_view(
        project, "st001");
    auto modviz = dmc::rengine::modviz::build_scene_workspace_view(
        project, "st001");
    assert(stage_ops.valid());
    assert(modviz.valid());
    assert(stage_ops.by_category(StageResourceCategory::collision).size() == 1U);
    assert(modviz.by_kind(VisualResourceKind::collision).size() == 1U);
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

    stage_ops = dmc::rengine::stageops::build_workspace_view(
        project, "st001");
    modviz = dmc::rengine::modviz::build_scene_workspace_view(
        project, "st001");
    assert(stage_ops.dirty_resource_count == 1U);
    assert(modviz.dirty_resource_count == 1U);
    assert(stage_ops.by_category(
        StageResourceCategory::collision)[0]->working_copy_revision == 1U);
    assert(modviz.by_kind(
        VisualResourceKind::collision)[0]->working_copy_revision == 1U);
    assert(validate_stage_views(stage_ops, modviz).consistent());

    auto role_mismatch = modviz;
    role_mismatch.resources[0].role = "different-role";
    assert(!validate_stage_views(stage_ops, role_mismatch).consistent());

    auto missing_visual = modviz;
    missing_visual.resources.clear();
    assert(!validate_stage_views(stage_ops, missing_visual).consistent());
    return 0;
}
