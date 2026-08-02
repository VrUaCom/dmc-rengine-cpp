#pragma once

#include "dmc_rengine/evidence/record.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::item {

enum class RuntimeEvidenceKind {
    registry_slot_guard,
    game_verified_item_ids,
    inventory_limit_patch_family,
};

[[nodiscard]] constexpr std::string_view to_string(
    RuntimeEvidenceKind kind) noexcept {
    switch (kind) {
    case RuntimeEvidenceKind::registry_slot_guard:
        return "registry-slot-guard";
    case RuntimeEvidenceKind::game_verified_item_ids:
        return "game-verified-item-ids";
    case RuntimeEvidenceKind::inventory_limit_patch_family:
        return "inventory-limit-patch-family";
    }
    return "registry-slot-guard";
}

struct RuntimeEvidenceView final {
    RuntimeEvidenceKind kind{RuntimeEvidenceKind::registry_slot_guard};
    std::string record_id;
    std::string claim_id;
    evidence::Confidence confidence{evidence::Confidence::hypothesis};
    std::string summary;
    bool location_complete{false};

    [[nodiscard]] bool valid() const noexcept {
        return !record_id.empty() && !claim_id.empty() && !summary.empty();
    }
};

struct ItemWorkspaceView final {
    gdspaces::ResourceRef resource;
    integration::WorkspaceStatus workspace_status{
        integration::WorkspaceStatus::invalid};
    integration::IntegrationMaturity format_maturity{
        integration::IntegrationMaturity::recognized};
    integration::ResourceWritePolicy write_policy{
        integration::ResourceWritePolicy::read_only};
    std::uint64_t working_copy_revision{};
    bool dirty{false};
    bool binary_document{false};
    std::uint64_t binary_coverage_bytes{};
    std::size_t diagnostic_count{};
    std::vector<std::string> routes;
    std::vector<RuntimeEvidenceView> runtime_evidence;

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid() && resource.format == "itm";
    }

    [[nodiscard]] const RuntimeEvidenceView* evidence(
        RuntimeEvidenceKind kind) const noexcept;
    [[nodiscard]] bool has_route(std::string_view route) const noexcept;
};

[[nodiscard]] ItemWorkspaceView build_workspace_view(
    const integration::ProjectWorkspace& project,
    const gdspaces::ResourceId& item_resource);

} // namespace dmc::rengine::item
