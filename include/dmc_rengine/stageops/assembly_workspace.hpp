#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/gdspaces/stage_bundle.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops {

enum class StageAssemblyStatus {
    invalid,
    partial,
    product_materialized,
    game_ready_equivalent,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageAssemblyStatus status) noexcept {
    switch (status) {
    case StageAssemblyStatus::invalid: return "invalid";
    case StageAssemblyStatus::partial: return "partial";
    case StageAssemblyStatus::product_materialized: return "product-materialized";
    case StageAssemblyStatus::game_ready_equivalent: return "game-ready-equivalent";
    }
    return "invalid";
}

enum class StageAssemblyMembershipKind {
    descriptor_root,
    nested_container_child,
};

struct StageAssemblyIdentity final {
    gdspaces::StageIdentity stage;
    std::string catalog_entry_id;
    std::uint32_t global_catalog_row{};
    std::string source_table_id;
    std::uint32_t source_row_index{};
    std::optional<std::uint16_t> numeric_stage_id;
    std::optional<std::string> semantic_stage_id;

    [[nodiscard]] bool valid() const noexcept {
        return stage.valid() && !catalog_entry_id.empty() &&
            !source_table_id.empty();
    }
};

// One canonical resource identity inside the assembled stage workspace.
// Relationships/roles are stored separately so shared resources are never
// duplicated merely because more than one parent/role references them.
struct StageAssemblyResource final {
    gdspaces::ResourceRef resource;
    bool materialized{false};
    bool descriptor_root{false};
    bool nested_container_child{false};
    bool container_expansion_attempted{false};
    bool container_expansion_complete{false};
    bool game_ready{false};

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid();
    }
};

// A resource may participate in more than one relationship while preserving one
// ResourceId. descriptor_root has no parent; nested children retain the exact
// parent ResourceId and container slot selected by GDSpaces/container expansion.
struct StageAssemblyMembership final {
    std::string resource_id;
    gdspaces::StageResourceCategory category{
        gdspaces::StageResourceCategory::unknown};
    std::string role;
    StageAssemblyMembershipKind kind{
        StageAssemblyMembershipKind::descriptor_root};
    std::optional<std::string> parent_resource_id;
    std::optional<std::uint32_t> container_slot;

    [[nodiscard]] bool valid() const noexcept {
        if (resource_id.empty() || role.empty()) {
            return false;
        }
        if (kind == StageAssemblyMembershipKind::nested_container_child) {
            return parent_resource_id.has_value() &&
                !parent_resource_id->empty() && container_slot.has_value();
        }
        return !parent_resource_id.has_value() && !container_slot.has_value();
    }
};

class StageAssemblyWorkspace final {
public:
    StageAssemblyIdentity identity;
    std::vector<StageAssemblyResource> resources;
    std::vector<StageAssemblyMembership> memberships;
    std::vector<gdspaces::Diagnostic> diagnostics;
    bool product_materialization_complete{false};
    bool game_ready_equivalent{false};

    [[nodiscard]] StageAssemblyStatus status() const noexcept;
    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool product_ready() const noexcept;
    [[nodiscard]] bool original_game_ready() const noexcept;

    [[nodiscard]] const StageAssemblyResource* find_resource(
        std::string_view canonical_resource_id) const noexcept;

    [[nodiscard]] std::vector<const StageAssemblyMembership*> by_category(
        gdspaces::StageResourceCategory category) const;

    [[nodiscard]] std::vector<const StageAssemblyMembership*> memberships_for(
        std::string_view canonical_resource_id) const;

    [[nodiscard]] std::size_t descriptor_root_count() const noexcept;
    [[nodiscard]] std::size_t nested_resource_count() const noexcept;
};

} // namespace dmc::rengine::stageops
