#include "dmc_rengine/modviz/scene_workspace_view.hpp"

#include "dmc_rengine/integration/format_registry.hpp"

#include <algorithm>
#include <optional>

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

} // namespace

std::vector<const SceneResourceView*> SceneWorkspaceView::by_kind(
    VisualResourceKind kind) const {
    std::vector<const SceneResourceView*> result;
    for (const auto& resource : resources) {
        if (resource.kind == kind) {
            result.push_back(&resource);
        }
    }
    return result;
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
        if (!view.identity.valid()) {
            view.identity = stage->identity;
        }
        if (stage->identity.resource_set_key() != view.identity.resource_set_key() ||
            stage->identity.profile != view.identity.profile) {
            return SceneWorkspaceView{};
        }

        const auto kind = visual_kind(stage->category);
        if (!kind.has_value()) {
            continue;
        }

        const auto* format = session->format();
        const auto* binary = session->binary_document();
        const auto* working = session->working_copy();
        const auto editable = format != nullptr &&
            format->write_policy != integration::ResourceWritePolicy::read_only;
        const auto dirty = working != nullptr && working->dirty();
        view.resources.push_back(SceneResourceView{
            .kind = *kind,
            .stage_category = stage->category,
            .role = stage->role,
            .resource = session->resource(),
            .editable = editable,
            .dirty = dirty,
            .working_copy_revision = working == nullptr
                ? 0U
                : working->revision(),
            .binary_document = binary != nullptr,
            .binary_coverage_bytes = binary == nullptr
                ? 0U
                : binary->coverage_bytes(),
            .evidence_record_count = session->evidence_record_ids().size(),
        });
        if (dirty) {
            ++view.dirty_resource_count;
        }
        if (*kind == VisualResourceKind::collision) {
            ++view.collision_resource_count;
        }
    }
    return view;
}

} // namespace dmc::rengine::modviz
