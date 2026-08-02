#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"
#include "dmc_rengine/item/workspace_view.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::item {

enum class RuntimeChangeKind {
    register_item_slot,
    set_inventory_limit,
};

[[nodiscard]] constexpr std::string_view to_string(
    RuntimeChangeKind kind) noexcept {
    switch (kind) {
    case RuntimeChangeKind::register_item_slot:
        return "register-item-slot";
    case RuntimeChangeKind::set_inventory_limit:
        return "set-inventory-limit";
    }
    return "register-item-slot";
}

enum class RuntimeRequestStatus {
    ready_for_patch_planning,
    research_required,
    rejected,
};

[[nodiscard]] constexpr std::string_view to_string(
    RuntimeRequestStatus status) noexcept {
    switch (status) {
    case RuntimeRequestStatus::ready_for_patch_planning:
        return "ready-for-patch-planning";
    case RuntimeRequestStatus::research_required:
        return "research-required";
    case RuntimeRequestStatus::rejected:
        return "rejected";
    }
    return "rejected";
}

struct RuntimeRequestDiagnostic final {
    std::string code;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

struct RuntimeChangeRequest final {
    std::string id;
    RuntimeChangeKind kind{RuntimeChangeKind::register_item_slot};
    RuntimeRequestStatus status{RuntimeRequestStatus::rejected};
    gdspaces::ResourceId item_resource;
    std::uint64_t item_revision{};
    std::string target_artifact_id;
    std::uint32_t requested_value{};
    std::vector<std::string> evidence_record_ids;
    std::vector<std::string> required_guards;
    std::vector<RuntimeRequestDiagnostic> diagnostics;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && item_resource.valid() &&
               !target_artifact_id.empty() && !evidence_record_ids.empty();
    }
};

[[nodiscard]] RuntimeChangeRequest make_slot_registration_request(
    const ItemWorkspaceView& workspace,
    std::uint32_t slot,
    std::string target_artifact_id);

[[nodiscard]] RuntimeChangeRequest make_inventory_limit_request(
    const ItemWorkspaceView& workspace,
    std::uint32_t limit,
    std::string target_artifact_id);

} // namespace dmc::rengine::item
