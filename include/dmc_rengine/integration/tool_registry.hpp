#pragma once

#include "dmc_rengine/gdspaces/open_router.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::integration {

enum class ToolCapability {
    browse_resource,
    inspect_binary,
    inspect_executable,
    inspect_evidence,
    stage_operations,
    scene_editing,
    menu_editing,
    item_editing,
    create_working_copy,
    create_patch_plan,
    validate_and_test,
    export_manifest,
};

[[nodiscard]] constexpr std::string_view to_string(
    ToolCapability capability) noexcept {
    switch (capability) {
    case ToolCapability::browse_resource: return "browse-resource";
    case ToolCapability::inspect_binary: return "inspect-binary";
    case ToolCapability::inspect_executable: return "inspect-executable";
    case ToolCapability::inspect_evidence: return "inspect-evidence";
    case ToolCapability::stage_operations: return "stage-operations";
    case ToolCapability::scene_editing: return "scene-editing";
    case ToolCapability::menu_editing: return "menu-editing";
    case ToolCapability::item_editing: return "item-editing";
    case ToolCapability::create_working_copy: return "create-working-copy";
    case ToolCapability::create_patch_plan: return "create-patch-plan";
    case ToolCapability::validate_and_test: return "validate-and-test";
    case ToolCapability::export_manifest: return "export-manifest";
    }
    return "browse-resource";
}

enum class ToolRouteRole {
    primary,
    companion,
    evidence,
    validation,
};

[[nodiscard]] constexpr std::string_view to_string(
    ToolRouteRole role) noexcept {
    switch (role) {
    case ToolRouteRole::primary: return "primary";
    case ToolRouteRole::companion: return "companion";
    case ToolRouteRole::evidence: return "evidence";
    case ToolRouteRole::validation: return "validation";
    }
    return "companion";
}

struct ToolDescriptor final {
    gdspaces::ToolTarget target{gdspaces::ToolTarget::binary_inspector};
    std::string product_name;
    std::string lore_name;
    std::vector<ToolCapability> capabilities;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool supports(ToolCapability capability) const noexcept;
};

struct ToolRoute final {
    gdspaces::ToolTarget target{gdspaces::ToolTarget::binary_inspector};
    ToolRouteRole role{ToolRouteRole::companion};
    std::string reason;

    [[nodiscard]] bool valid() const noexcept {
        return !reason.empty();
    }
};

class ToolRegistry final {
public:
    ToolRegistry();

    [[nodiscard]] const ToolDescriptor* find(
        gdspaces::ToolTarget target) const noexcept;
    [[nodiscard]] const std::vector<ToolDescriptor>& tools() const noexcept;

    [[nodiscard]] std::vector<ToolRoute> routes_for(
        const gdspaces::ResourceRef& resource,
        bool stage_context,
        bool menu_context,
        bool evidence_context = true) const;

private:
    std::vector<ToolDescriptor> tools_;
    gdspaces::OpenRouter primary_router_;
};

} // namespace dmc::rengine::integration
