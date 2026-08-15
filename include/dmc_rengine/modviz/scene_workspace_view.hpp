#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/stageops/assembly_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
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

// Visual projection of one Stage Ops membership. Several visual relationships
// may point at the same canonical ResourceId without duplicating resource
// ownership in Stage Ops.
struct SceneMembershipView final {
    VisualResourceKind kind{VisualResourceKind::model};
    gdspaces::StageResourceCategory stage_category{
        gdspaces::StageResourceCategory::unknown};
    std::string role;
    stageops::StageAssemblyMembershipKind source_kind{
        stageops::StageAssemblyMembershipKind::descriptor_root};
    std::optional<std::string> parent_resource_id;
    std::optional<std::uint32_t> container_slot;

    [[nodiscard]] bool valid() const noexcept {
        return !role.empty();
    }
};

struct SceneResourceView final {
    gdspaces::ResourceRef resource;
    std::vector<SceneMembershipView> memberships;
    bool materialized{false};

    // Editor capability/state comes from the shared ProjectWorkspace session;
    // it never defines whether the resource belongs to the scene.
    bool project_session_attached{false};
    bool editable{false};
    bool dirty{false};
    std::uint64_t working_copy_revision{};
    bool binary_document{false};
    std::uint64_t binary_coverage_bytes{};
    std::size_t evidence_record_count{};

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid() && !memberships.empty();
    }

    [[nodiscard]] bool has_kind(VisualResourceKind kind) const noexcept;
};

struct SceneWorkspaceView final {
    stageops::StageAssemblyIdentity identity;
    stageops::StageAssemblyStatus assembly_status{
        stageops::StageAssemblyStatus::invalid};
    stageops::StageGameReadiness game_readiness{
        stageops::StageGameReadiness::unproven};
    std::size_t unresolved_requirement_count{};
    std::vector<SceneResourceView> resources;
    std::size_t dirty_resource_count{};
    std::size_t collision_resource_count{};

    [[nodiscard]] bool valid() const noexcept {
        return identity.valid() &&
            assembly_status != stageops::StageAssemblyStatus::invalid;
    }

    [[nodiscard]] std::vector<const SceneResourceView*> by_kind(
        VisualResourceKind kind) const;
};

// Canonical ModViz projection. Stage membership comes from Stage Ops only.
// ProjectWorkspace is optional per-resource editor context.
[[nodiscard]] SceneWorkspaceView build_scene_workspace_view(
    const stageops::StageAssemblyWorkspace& assembly,
    const integration::ProjectWorkspace* project = nullptr);

// Legacy compatibility adapter. New ModViz code must consume the Stage Ops
// assembly overload above rather than rediscovering the scene from project tags.
[[nodiscard]] SceneWorkspaceView build_scene_workspace_view(
    const integration::ProjectWorkspace& project,
    std::string_view resource_set_id);

} // namespace dmc::rengine::modviz
