#pragma once

#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/gdspaces/stage_bundle.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::modviz {

enum class VisualResourceKind {
    model,
    texture,
    animation,
    camera,
    light,
    position,
    effect,
    collision,
};

[[nodiscard]] constexpr std::string_view to_string(
    VisualResourceKind kind) noexcept {
    switch (kind) {
    case VisualResourceKind::model: return "model";
    case VisualResourceKind::texture: return "texture";
    case VisualResourceKind::animation: return "animation";
    case VisualResourceKind::camera: return "camera";
    case VisualResourceKind::light: return "light";
    case VisualResourceKind::position: return "position";
    case VisualResourceKind::effect: return "effect";
    case VisualResourceKind::collision: return "collision";
    }
    return "model";
}

struct SceneResourceView final {
    VisualResourceKind kind{VisualResourceKind::model};
    gdspaces::StageResourceCategory stage_category{
        gdspaces::StageResourceCategory::unknown};
    std::string role;
    gdspaces::ResourceRef resource;
    bool editable{false};
    bool dirty{false};
    std::uint64_t working_copy_revision{};
    bool binary_document{false};
    std::uint64_t binary_coverage_bytes{};
    std::size_t evidence_record_count{};

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid() && !role.empty();
    }
};

struct SceneWorkspaceView final {
    gdspaces::StageIdentity identity;
    std::vector<SceneResourceView> resources;
    std::size_t dirty_resource_count{};
    std::size_t collision_resource_count{};

    [[nodiscard]] bool valid() const noexcept {
        return identity.valid();
    }

    [[nodiscard]] std::vector<const SceneResourceView*> by_kind(
        VisualResourceKind kind) const;
};

[[nodiscard]] SceneWorkspaceView build_scene_workspace_view(
    const integration::ProjectWorkspace& project,
    std::string_view stage_id);

} // namespace dmc::rengine::modviz
