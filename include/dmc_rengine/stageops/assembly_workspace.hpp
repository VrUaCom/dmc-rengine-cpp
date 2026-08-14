#pragma once

#include "dmc_rengine/gdspaces/byte_provenance.hpp"
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

// Product-side Stage Ops assembly progress. Original-game readiness is an
// independent evidence gate and must not be collapsed into this status axis.
enum class StageAssemblyStatus {
    invalid,
    partial,
    product_materialized,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageAssemblyStatus status) noexcept {
    switch (status) {
    case StageAssemblyStatus::invalid: return "invalid";
    case StageAssemblyStatus::partial: return "partial";
    case StageAssemblyStatus::product_materialized:
        return "product-materialized";
    }
    return "invalid";
}

// This is deliberately binary and evidence-conservative: `unproven` means
// Stage Ops must not present the assembled product workspace as equivalent to a
// vanilla ready scene. It does not assert that equivalence is impossible.
enum class StageGameReadiness {
    unproven,
    equivalent,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageGameReadiness readiness) noexcept {
    switch (readiness) {
    case StageGameReadiness::unproven: return "unproven";
    case StageGameReadiness::equivalent: return "equivalent";
    }
    return "unproven";
}

enum class StageAssemblyMembershipKind {
    descriptor_root,
    nested_container_child,
};

struct StageAssemblyIdentity final {
    // Canonical stage/resource-set identity. Numeric and semantic identities
    // live here once only; Stage Ops must not keep shadow copies that can drift.
    gdspaces::StageIdentity stage;

    // Stable executable/catalog resource-set identity used to enter Stage Ops.
    // It must match stage.resource_set_key().
    std::string catalog_entry_id;

    // Exact source provenance for the selected descriptor/catalog entry. These
    // are structural identities, not gameplay semantics.
    std::uint32_t global_catalog_row{};
    std::string source_table_id;
    std::uint32_t source_row_index{};

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] std::optional<std::uint16_t> numeric_stage_id() const noexcept {
        return stage.numeric_stage_id;
    }

    [[nodiscard]] std::optional<std::string_view> semantic_stage_id() const noexcept {
        if (stage.semantic_stage_id.empty()) {
            return std::nullopt;
        }
        return std::string_view{stage.semantic_stage_id};
    }
};

// One canonical ResourceId inside the assembled stage workspace. Relationships
// and roles are kept separately so shared resources are represented once even
// when several parents/roles reference them.
struct StageAssemblyResource final {
    gdspaces::ResourceRef resource;

    // Present only when bytes were actually materialized. Stage Ops preserves
    // GDSpaces byte authority rather than copying or reconstructing provenance.
    std::optional<gdspaces::ByteProvenance> byte_provenance;

    bool materialized{false};
    bool descriptor_root{false};
    bool nested_container_child{false};

    // Structural fact only: this ResourceId appeared as a parent in at least one
    // bounded container-expansion record. Whole-tree completeness stays a
    // workspace/load-report gate and is not guessed per nested resource.
    bool container_expansion_observed{false};

    [[nodiscard]] bool valid() const noexcept;
};

// A resource may participate in more than one relationship while preserving one
// ResourceId. Descriptor roots have no parent; nested children retain the exact
// parent ResourceId and container slot emitted by GDSpaces expansion.
struct StageAssemblyMembership final {
    std::string resource_id;
    gdspaces::StageResourceCategory category{
        gdspaces::StageResourceCategory::unknown};
    std::string role;
    StageAssemblyMembershipKind kind{
        StageAssemblyMembershipKind::descriptor_root};
    std::optional<std::string> parent_resource_id;
    std::optional<std::uint32_t> container_slot;

    [[nodiscard]] bool valid() const noexcept;
};

class StageAssemblyWorkspace final {
public:
    StageAssemblyIdentity identity;
    std::vector<StageAssemblyResource> resources;
    std::vector<StageAssemblyMembership> memberships;
    std::vector<gdspaces::Diagnostic> diagnostics;

    // Product boundary: all resources required by the supplying materialization
    // report crossed its validated read/provenance/container-expansion gates.
    bool product_materialization_complete{false};

    // Original-game boundary: may become true only from recovered-game evidence
    // proving the relevant typed post-load/factory/lifecycle path. Stage Ops does
    // not infer this from successful product materialization.
    bool game_ready_equivalent{false};

    [[nodiscard]] StageAssemblyStatus status() const noexcept;
    [[nodiscard]] StageGameReadiness game_readiness() const noexcept;
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
