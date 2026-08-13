#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include <array>
#include <span>
#include <string>
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
    std::string stage_id;
    std::string evidence_id;
    std::array<StageResourceReference, 4> resources;

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

// Build a stage resource plan from one recovered four-column EXE stage-table row.
// The supplied paths are authoritative row values; no stXXX naming pattern is
// generated or inferred here.
[[nodiscard]] StageResourceRowPlan make_stage_resource_plan_from_table_row(
    std::string stage_id,
    std::array<std::string, 4> logical_paths,
    std::string evidence_id);

// Compatibility fixture for the first evidence-backed integration slice.
// Production stage handling must use recovered table rows rather than this
// st001-specific helper.
[[nodiscard]] const StageResourceRowPlan& phase12_st001_resource_plan() noexcept;

class StageResourceMatcher final {
public:
    [[nodiscard]] static StageResourceMatchReport match(
        StageResourceRowPlan plan,
        std::span<const gdspaces::ResourceRef> resources);
};

} // namespace dmc::rengine::profiles::dmc3
