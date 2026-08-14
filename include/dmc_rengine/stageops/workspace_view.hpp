#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_workspace.hpp"
#include "dmc_rengine/stageops/assembly_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops {

struct StageResourceView final {
    gdspaces::ResourceRef resource;
    std::vector<StageAssemblyMembership> memberships;
    bool materialized{false};
    bool byte_provenance_valid{false};

    // Per-resource operational substrate projected from ProjectWorkspace when a
    // matching session exists. These fields do not define scene membership.
    bool project_session_attached{false};
    integration::WorkspaceStatus workspace_status{
        integration::WorkspaceStatus::invalid};
    bool binary_document{false};
    std::uint64_t binary_coverage_bytes{};
    std::size_t evidence_record_count{};
    std::size_t diagnostic_count{};
    std::uint64_t working_copy_revision{};
    bool dirty{false};

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid() && !memberships.empty();
    }

    [[nodiscard]] bool has_category(
        gdspaces::StageResourceCategory category) const noexcept;
};

struct StageWorkspaceView final {
    StageAssemblyIdentity identity;
    StageAssemblyStatus assembly_status{StageAssemblyStatus::invalid};
    StageGameReadiness game_readiness{StageGameReadiness::unproven};
    std::vector<StageAssemblyRequirement> requirements;
    std::vector<StageResourceView> resources;
    std::size_t unresolved_requirement_count{};
    std::size_t error_count{};
    std::size_t warning_count{};
    std::size_t dirty_resource_count{};
    std::size_t validation_request_count{};

    [[nodiscard]] bool valid() const noexcept {
        return identity.valid() && assembly_status != StageAssemblyStatus::invalid;
    }

    [[nodiscard]] std::vector<const StageResourceView*> by_category(
        gdspaces::StageResourceCategory category) const;
};

// Canonical projection. Scene membership/identity comes only from the Stage Ops
// aggregate. ProjectWorkspace contributes per-resource parser/evidence/edit
// state when supplied; it is not queried to discover the scene.
[[nodiscard]] StageWorkspaceView build_workspace_view(
    const StageAssemblyWorkspace& assembly,
    const integration::ProjectWorkspace* project = nullptr);

// Legacy compatibility adapter for pre-ADR-0003 callers. New code must supply a
// StageAssemblyWorkspace. This path is intentionally marked as partial/unproven
// because project stage tags alone are not the canonical scene assembly.
[[nodiscard]] StageWorkspaceView build_workspace_view(
    const integration::ProjectWorkspace& project,
    std::string_view resource_set_id);

} // namespace dmc::rengine::stageops
