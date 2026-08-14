#pragma once

#include "dmc_rengine/integration/resource_workspace.hpp"
#include "dmc_rengine/stageops/operations_session.hpp"
#include "dmc_rengine/stageops/scene_snapshot.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace dmc::rengine::modviz {

struct StageSceneResourceProjection final {
    gdspaces::ResourceRef resource;
    std::vector<stageops::StageAssemblyMembership> memberships;
    bool materialized{false};

    // Per-resource editor capability/state is read through the Stage Ops-owned
    // operations session. It does not define scene membership.
    bool project_session_present{false};
    integration::WorkspaceStatus workspace_status{
        integration::WorkspaceStatus::invalid};
    bool dirty{false};
    std::uint64_t working_copy_revision{};

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid() && !memberships.empty();
    }
};

struct StageSceneDomainProjection final {
    std::string domain_object_id;
    stageops::StageDomainKind kind{stageops::StageDomainKind::hits_collision};
    std::string resource_id;
    bool current_for_active_bytes{false};

    [[nodiscard]] bool valid() const noexcept {
        return !domain_object_id.empty() && !resource_id.empty();
    }
};

// New ModViz scene contract: a pure projection over one Stage Ops operations
// session and a revision-coherent StageSceneSnapshot. It carries no independent
// resolver, parser, scene graph or byte ownership.
struct StageSceneProjection final {
    stageops::StageAssemblyIdentity identity;
    std::uint64_t stage_revision{};
    bool current_for_active_bytes{false};
    std::size_t unresolved_requirement_count{};
    std::vector<StageSceneResourceProjection> resources;
    std::vector<StageSceneDomainProjection> visual_domains;

    [[nodiscard]] bool valid() const noexcept {
        return identity.valid();
    }
};

class StageSceneProjectionBuilder final {
public:
    [[nodiscard]] static StageSceneProjection build(
        const stageops::StageOperationsSession& session,
        const stageops::StageSceneSnapshot& snapshot);
};

} // namespace dmc::rengine::modviz
