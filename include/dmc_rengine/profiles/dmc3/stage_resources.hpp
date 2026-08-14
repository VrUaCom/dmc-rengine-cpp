#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include <array>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct StageResourceReference final {
    StageResourceRole role{StageResourceRole::script};
    std::string logical_path;

    [[nodiscard]] bool valid() const noexcept {
        return !logical_path.empty();
    }
};

struct StageResourceRowPlan final {
    // Legacy technical key retained for source compatibility. New catalog code
    // mirrors resource_set_id here; it is not gameplay semantics.
    std::string stage_id;
    std::string evidence_id;
    std::array<StageResourceReference, 4> resources;

    // Stable technical identity of the exact executable-derived resource set.
    // For StageCatalog-driven plans this is StageCatalogEntry::catalog_entry_id.
    std::string resource_set_id;

    // Optional evidence-backed gameplay-stage identity. It remains empty when
    // only the executable row/resource set is known.
    std::string semantic_stage_id;

    [[nodiscard]] std::string_view resource_set_key() const noexcept {
        return resource_set_id.empty()
            ? std::string_view{stage_id}
            : std::string_view{resource_set_id};
    }

    [[nodiscard]] bool semantic_stage_known() const noexcept {
        return !semantic_stage_id.empty();
    }

    [[nodiscard]] bool valid() const noexcept;
};

struct StageResourceMatch final {
    StageResourceReference reference;
    std::vector<gdspaces::ResourceRef> matches;
};

struct StageResourceMatchReport final {
    StageResourceRowPlan plan;
    std::vector<StageResourceMatch> roles;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;
    [[nodiscard]] std::vector<gdspaces::StageMemberCandidate>
    unique_candidates() const;
};

// Build a resource-set plan from one recovered four-column EXE stage-table row.
// resource_set_id is technical table/catalog identity, not gameplay semantics.
// The supplied paths are authoritative row values; no stXXX naming pattern is
// generated or inferred here.
[[nodiscard]] StageResourceRowPlan make_stage_resource_plan_from_table_row(
    std::string resource_set_id,
    std::array<std::string, 4> logical_paths,
    std::string evidence_id);

// Compatibility fixture for historical tests. Production stage handling must
// use recovered StageCatalog entries rather than this st001-specific helper.
[[nodiscard]] const StageResourceRowPlan& phase12_st001_resource_plan() noexcept;

class StageResourceMatcher final {
public:
    [[nodiscard]] static StageResourceMatchReport match(
        StageResourceRowPlan plan,
        std::span<const gdspaces::ResourceRef> resources);
};

} // namespace dmc::rengine::profiles::dmc3
