#pragma once

#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/gdspaces/stage_bundle.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops {

struct StageResourceView final {
    gdspaces::StageResourceCategory category{
        gdspaces::StageResourceCategory::unknown};
    std::string role;
    gdspaces::ResourceRef resource;
    integration::WorkspaceStatus workspace_status{
        integration::WorkspaceStatus::invalid};
    bool binary_document{false};
    std::uint64_t binary_coverage_bytes{};
    std::size_t evidence_record_count{};
    std::size_t diagnostic_count{};
    std::uint64_t working_copy_revision{};
    bool dirty{false};

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid() && !role.empty();
    }
};

struct StageWorkspaceView final {
    gdspaces::StageIdentity identity;
    std::vector<StageResourceView> resources;
    std::size_t error_count{};
    std::size_t warning_count{};
    std::size_t dirty_resource_count{};
    std::size_t validation_request_count{};

    [[nodiscard]] bool valid() const noexcept {
        return identity.valid() && !resources.empty();
    }

    [[nodiscard]] std::vector<const StageResourceView*> by_category(
        gdspaces::StageResourceCategory category) const;
};

[[nodiscard]] StageWorkspaceView build_workspace_view(
    const integration::ProjectWorkspace& project,
    std::string_view stage_id);

} // namespace dmc::rengine::stageops
