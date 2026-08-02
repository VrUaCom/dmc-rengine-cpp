#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/integration/stage_view_consistency.hpp"
#include "dmc_rengine/modviz/scene_workspace_view.hpp"
#include "dmc_rengine/stageops/workspace_view.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] std::vector<std::byte> make_hits() {
    std::vector<std::byte> bytes(64U, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    bytes[4] = std::byte{'$'};
    write_u32(bytes, 8U, dmc::rengine::formats::hits::record_marker);
    for (std::size_t index = 0;
         index < dmc::rengine::formats::hits::value_count;
         ++index) {
        write_u32(
            bytes,
            12U + index * 4U,
            std::bit_cast<std::uint32_t>(static_cast<float>(index + 1U)));
    }
    return bytes;
}

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

    const auto hits_bytes = make_hits();
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
        .role = "room-collision-hits",
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
    assert(stage_ops.resources.size() == 2U);
    assert(stage_ops.by_category(StageResourceCategory::collision).size() == 1U);
    assert(stage_ops.by_category(StageResourceCategory::scripts).size() == 1U);
    assert(modviz.resources.size() == 1U);
    assert(modviz.by_kind(VisualResourceKind::collision).size() == 1U);
    assert(modviz.collision_resource_count == 1U);
    assert(validate_stage_views(stage_ops, modviz).consistent());

    assert(project.enable_working_copy(hits.id));
    assert(project.apply_edit(
        hits.id,
        EditOperation{
            .id = "shared-stage-edit",
            .base_revision = 0U,
            .offset = 6U,
            .expected = {std::byte{0}},
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
    const auto role_report = validate_stage_views(stage_ops, role_mismatch);
    assert(!role_report.consistent());
    assert(role_report.issues[0].code == "stage-view.role-mismatch");

    auto missing_visual = modviz;
    missing_visual.resources.clear();
    const auto missing_report = validate_stage_views(stage_ops, missing_visual);
    assert(!missing_report.consistent());
    assert(missing_report.issues[0].code ==
        "stage-view.visual-resource-missing-in-modviz");

    return 0;
}
