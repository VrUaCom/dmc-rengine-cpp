#include "dmc_rengine/integration/tool_registry.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string normalized_format(std::string format) {
    std::transform(
        format.begin(), format.end(), format.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return format;
}

[[nodiscard]] int role_rank(ToolRouteRole role) noexcept {
    switch (role) {
    case ToolRouteRole::primary: return 0;
    case ToolRouteRole::companion: return 1;
    case ToolRouteRole::evidence: return 2;
    case ToolRouteRole::validation: return 3;
    }
    return 4;
}

void add_route(
    std::vector<ToolRoute>& routes,
    gdspaces::ToolTarget target,
    ToolRouteRole role,
    std::string reason) {
    const auto existing = std::find_if(
        routes.begin(), routes.end(),
        [target](const ToolRoute& route) {
            return route.target == target;
        });

    if (existing == routes.end()) {
        routes.push_back(ToolRoute{
            .target = target,
            .role = role,
            .reason = std::move(reason),
        });
        return;
    }

    if (role_rank(role) < role_rank(existing->role)) {
        existing->role = role;
        existing->reason = std::move(reason);
    }
}

[[nodiscard]] bool is_scene_format(std::string_view format) noexcept {
    return format == "scm" || format == "mod" || format == "dds" ||
           format == "ptx" || format == "cam" || format == "dca" ||
           format == "lig" || format == "lig2" || format == "hits";
}

[[nodiscard]] bool is_stage_format(std::string_view format) noexcept {
    return format == "cam" || format == "dca" || format == "lig" ||
           format == "lig2" || format == "hits" || format == "txt" ||
           format == "scm" || format == "mod" || format == "dds" ||
           format == "ptx";
}

[[nodiscard]] bool is_executable_format(std::string_view format) noexcept {
    return format == "pe" || format == "exe" || format == "dll";
}

[[nodiscard]] bool is_editable_domain_format(std::string_view format) noexcept {
    return format == "itm" || is_scene_format(format) || format == "txt";
}

} // namespace

bool ToolDescriptor::valid() const noexcept {
    return !product_name.empty() && !lore_name.empty() &&
           !capabilities.empty();
}

bool ToolDescriptor::supports(ToolCapability capability) const noexcept {
    return std::find(capabilities.begin(), capabilities.end(), capability) !=
           capabilities.end();
}

ToolRegistry::ToolRegistry() {
    tools_ = {
        ToolDescriptor{
            .target = gdspaces::ToolTarget::gdspaces,
            .product_name = "GDSpaces",
            .lore_name = "The Archive",
            .capabilities = {
                ToolCapability::browse_resource,
                ToolCapability::inspect_evidence,
                ToolCapability::create_working_copy,
                ToolCapability::export_manifest,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::spider_hub,
            .product_name = "Spider Hub",
            .lore_name = "The Nexus",
            .capabilities = {
                ToolCapability::browse_resource,
                ToolCapability::inspect_evidence,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::binary_inspector,
            .product_name = "Binary Inspector",
            .lore_name = "The Reliquary",
            .capabilities = {
                ToolCapability::inspect_binary,
                ToolCapability::inspect_evidence,
                ToolCapability::export_manifest,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::exe_editor,
            .product_name = "EXE Editor",
            .lore_name = "The Scriptorium",
            .capabilities = {
                ToolCapability::inspect_binary,
                ToolCapability::inspect_executable,
                ToolCapability::inspect_evidence,
                ToolCapability::create_patch_plan,
                ToolCapability::export_manifest,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::evidence_registry,
            .product_name = "Evidence Registry",
            .lore_name = "The Evidence Registry",
            .capabilities = {
                ToolCapability::inspect_evidence,
                ToolCapability::export_manifest,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::stage_ops,
            .product_name = "Stage Ops",
            .lore_name = "The Theatre",
            .capabilities = {
                ToolCapability::inspect_binary,
                ToolCapability::inspect_evidence,
                ToolCapability::stage_operations,
                ToolCapability::create_working_copy,
                ToolCapability::export_manifest,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::modviz_scene,
            .product_name = "ModViz Scene/Model Editor",
            .lore_name = "The Observatory",
            .capabilities = {
                ToolCapability::inspect_evidence,
                ToolCapability::scene_editing,
                ToolCapability::create_working_copy,
                ToolCapability::export_manifest,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::modviz_menu,
            .product_name = "ModViz Menu Editor",
            .lore_name = "The Observatory",
            .capabilities = {
                ToolCapability::inspect_evidence,
                ToolCapability::menu_editing,
                ToolCapability::create_working_copy,
                ToolCapability::create_patch_plan,
                ToolCapability::export_manifest,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::item_editor,
            .product_name = "Item Editor",
            .lore_name = "The Forge",
            .capabilities = {
                ToolCapability::inspect_evidence,
                ToolCapability::item_editing,
                ToolCapability::create_working_copy,
                ToolCapability::create_patch_plan,
                ToolCapability::export_manifest,
            },
        },
        ToolDescriptor{
            .target = gdspaces::ToolTarget::build_test_lab,
            .product_name = "Build & Test Lab",
            .lore_name = "The Trial Chamber",
            .capabilities = {
                ToolCapability::validate_and_test,
                ToolCapability::export_manifest,
            },
        },
    };
}

const ToolDescriptor* ToolRegistry::find(
    gdspaces::ToolTarget target) const noexcept {
    const auto iterator = std::find_if(
        tools_.begin(), tools_.end(),
        [target](const ToolDescriptor& descriptor) {
            return descriptor.target == target;
        });
    return iterator == tools_.end() ? nullptr : &*iterator;
}

const std::vector<ToolDescriptor>& ToolRegistry::tools() const noexcept {
    return tools_;
}

std::vector<ToolRoute> ToolRegistry::routes_for(
    const gdspaces::ResourceRef& resource,
    bool stage_context,
    bool menu_context,
    bool evidence_context) const {
    std::vector<ToolRoute> routes;
    if (!resource.valid()) {
        return routes;
    }

    const auto format = normalized_format(resource.format);
    const gdspaces::OpenRequest request{
        .resource = resource,
        .preferred_target = std::nullopt,
        .stage_context = stage_context,
        .menu_context = menu_context,
        .evidence_context = evidence_context,
    };
    const auto primary = primary_router_.route(request);

    add_route(
        routes,
        primary,
        ToolRouteRole::primary,
        "Primary route selected by the GDSpaces OpenRouter.");
    add_route(
        routes,
        gdspaces::ToolTarget::gdspaces,
        ToolRouteRole::companion,
        "GDSpaces owns source access, canonical identity, classification, graph relationships, and working-copy policy.");
    add_route(
        routes,
        gdspaces::ToolTarget::spider_hub,
        ToolRouteRole::companion,
        "Spider Hub exposes the resource and its verified relationships.");

    if (primary != gdspaces::ToolTarget::binary_inspector) {
        add_route(
            routes,
            gdspaces::ToolTarget::binary_inspector,
            ToolRouteRole::companion,
            "Binary Inspector preserves shared structure, ownership, unknown regions, and diagnostics.");
    }

    if (evidence_context) {
        add_route(
            routes,
            gdspaces::ToolTarget::evidence_registry,
            ToolRouteRole::evidence,
            "Evidence Registry exposes claims and provenance linked to this resource.");
    }

    if (is_executable_format(format)) {
        add_route(
            routes,
            gdspaces::ToolTarget::build_test_lab,
            ToolRouteRole::validation,
            "Executable findings and patch plans require reproducible validation.");
    }

    if (format == "itm") {
        add_route(
            routes,
            gdspaces::ToolTarget::build_test_lab,
            ToolRouteRole::validation,
            "Item edits and runtime-linked patches require validation and manifest output.");
    }

    if (stage_context || is_stage_format(format)) {
        add_route(
            routes,
            gdspaces::ToolTarget::stage_ops,
            primary == gdspaces::ToolTarget::stage_ops
                ? ToolRouteRole::primary
                : ToolRouteRole::companion,
            "Stage Ops consumes typed stage context without resolving the resource independently.");
    }

    if (is_scene_format(format)) {
        add_route(
            routes,
            gdspaces::ToolTarget::modviz_scene,
            primary == gdspaces::ToolTarget::modviz_scene
                ? ToolRouteRole::primary
                : ToolRouteRole::companion,
            "ModViz consumes the same resource identity for visual reconstruction.");
    }

    if (menu_context) {
        add_route(
            routes,
            gdspaces::ToolTarget::modviz_menu,
            ToolRouteRole::primary,
            "Menu context explicitly selects the ModViz Menu Editor.");
        add_route(
            routes,
            gdspaces::ToolTarget::exe_editor,
            ToolRouteRole::companion,
            "Menu runtime bindings and limits remain evidence-backed EXE Editor concerns.");
    }

    if (is_editable_domain_format(format) || menu_context) {
        add_route(
            routes,
            gdspaces::ToolTarget::build_test_lab,
            ToolRouteRole::validation,
            "Editable resources require WorkingCopy validation before export.");
    }

    std::sort(
        routes.begin(), routes.end(),
        [](const ToolRoute& left, const ToolRoute& right) {
            const auto left_rank = role_rank(left.role);
            const auto right_rank = role_rank(right.role);
            if (left_rank != right_rank) {
                return left_rank < right_rank;
            }
            return static_cast<int>(left.target) <
                   static_cast<int>(right.target);
        });
    return routes;
}

} // namespace dmc::rengine::integration
