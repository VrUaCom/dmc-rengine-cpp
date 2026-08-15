#include "dmc_rengine/modviz/scene_workspace_view.hpp"

#include "dmc_rengine/integration/format_registry.hpp"

#include <algorithm>
#include <optional>
#include <string>

namespace dmc::rengine::modviz {
namespace {

[[nodiscard]] std::optional<VisualResourceKind> visual_kind(
    gdspaces::StageResourceCategory category) noexcept {
    switch (category) {
    case gdspaces::StageResourceCategory::models:
        return VisualResourceKind::model;
    case gdspaces::StageResourceCategory::textures:
        return VisualResourceKind::texture;
    case gdspaces::StageResourceCategory::animations:
        return VisualResourceKind::animation;
    case gdspaces::StageResourceCategory::cameras:
        return VisualResourceKind::camera;
    case gdspaces::StageResourceCategory::lighting:
        return VisualResourceKind::light;
    case gdspaces::StageResourceCategory::positions:
        return VisualResourceKind::position;
    case gdspaces::StageResourceCategory::effects:
        return VisualResourceKind::effect;
    case gdspaces::StageResourceCategory::collision:
        return VisualResourceKind::collision;
    case gdspaces::StageResourceCategory::scripts:
    case gdspaces::StageResourceCategory::events:
    case gdspaces::StageResourceCategory::sounds:
    case gdspaces::StageResourceCategory::unknown:
        return std::nullopt;
    }
    return std::nullopt;
}

void attach_project_state(
    SceneResourceView& resource,
    const integration::ResourceWorkspaceSession* session) {
    if (session == nullptr) {
        return;
    }

    resource.project_session_attached = true;
    const auto* format = session->format();
    const auto* binary = session->binary_document();
    const auto* working = session->working_copy();
    resource.editable = format != nullptr &&
        format->write_policy != integration::ResourceWritePolicy::read_only;
    resource.dirty = working != nullptr && working->dirty();
    resource.working_copy_revision = working == nullptr
        ? 0U
        : working->revision();
    resource.binary_document = binary != nullptr;
    resource.binary_coverage_bytes = binary == nullptr
        ? 0U
        : binary->coverage_bytes();
    resource.evidence_record_count = session->evidence_record_ids().size();
}

} // namespace

bool SceneResourceView::has_kind(VisualResourceKind kind) const noexcept {
    return std::any_of(
        memberships.begin(), memberships.end(),
        [kind](const SceneMembershipView& membership) {
            return membership.kind == kind;
        });
}

std::vector<const SceneResourceView*> SceneWorkspaceView::by_kind(
    VisualResourceKind kind) const {
    std::vector<const SceneResourceView*> result;
    for (const auto& resource : resources) {
        if (resource.has_kind(kind)) {
            result.push_back(&resource);
        }
    }
    return result;
}

SceneWorkspaceView build_scene_workspace_view(
    const stageops::StageAssemblyWorkspace& assembly,
    const integration::ProjectWorkspace* project) {
    SceneWorkspaceView view;
    if (!assembly.valid()) {
        return view;
    }

    view.identity = assembly.identity;
    view.assembly_status = assembly.status();
    view.game_readiness = assembly.game_readiness();
    view.unresolved_requirement_count = assembly.unresolved_requirement_count();

    for (const auto& assembled_resource : assembly.resources) {
        const auto canonical = assembled_resource.resource.id.canonical();
        const auto memberships = assembly.memberships_for(canonical);

        SceneResourceView resource;
        resource.resource = assembled_resource.resource;
        resource.materialized = assembled_resource.materialized;
        for (const auto* membership : memberships) {
            const auto kind = visual_kind(membership->category);
            if (!kind.has_value()) {
                continue;
            }
            resource.memberships.push_back(SceneMembershipView{
                .kind = *kind,
                .stage_category = membership->category,
                .role = membership->role,
                .source_kind = membership->kind,
                .parent_resource_id = membership->parent_resource_id,
                .container_slot = membership->container_slot,
            });
        }
        if (resource.memberships.empty()) {
            continue;
        }

        const auto* session = project == nullptr
            ? nullptr
            : project->find_session(assembled_resource.resource.id);
        attach_project_state(resource, session);
        if (resource.dirty) {
            ++view.dirty_resource_count;
        }
        if (resource.has_kind(VisualResourceKind::collision)) {
            ++view.collision_resource_count;
        }
        view.resources.push_back(std::move(resource));
    }
    return view;
}

SceneWorkspaceView build_scene_workspace_view(
    const integration::ProjectWorkspace& project,
    std::string_view resource_set_id) {
    SceneWorkspaceView view;
    auto sessions = project.sessions_for_stage_resource_set(resource_set_id);
    std::sort(
        sessions.begin(), sessions.end(),
        [](const integration::ResourceWorkspaceSession* left,
           const integration::ResourceWorkspaceSession* right) {
            return left->resource().id.canonical() <
                   right->resource().id.canonical();
        });

    for (const auto* session : sessions) {
        const auto* stage = session->stage();
        if (stage == nullptr) {
            continue;
        }
        if (!view.identity.stage.valid()) {
            view.identity = stageops::StageAssemblyIdentity{
                .stage = stage->identity,
                .catalog_entry_id = std::string{resource_set_id},
                .global_catalog_row = 0U,
                .source_table_id = "legacy-project-stage-context",
                .source_row_index = 0U,
            };
            view.assembly_status = stageops::StageAssemblyStatus::partial;
            view.game_readiness = stageops::StageGameReadiness::unproven;
        }
        if (stage->identity.resource_set_key() !=
                view.identity.stage.resource_set_key() ||
            stage->identity.profile != view.identity.stage.profile) {
            return SceneWorkspaceView{};
        }

        const auto kind = visual_kind(stage->category);
        if (!kind.has_value()) {
            continue;
        }

        SceneResourceView resource;
        resource.resource = session->resource();
        resource.materialized = session->source_payload().readable();
        resource.memberships.push_back(SceneMembershipView{
            .kind = *kind,
            .stage_category = stage->category,
            .role = stage->role,
            .source_kind = stageops::StageAssemblyMembershipKind::descriptor_root,
            .parent_resource_id = std::nullopt,
            .container_slot = std::nullopt,
        });
        attach_project_state(resource, session);
        if (resource.dirty) {
            ++view.dirty_resource_count;
        }
        if (resource.has_kind(VisualResourceKind::collision)) {
            ++view.collision_resource_count;
        }
        view.resources.push_back(std::move(resource));
    }
    return view;
}

} // namespace dmc::rengine::modviz
