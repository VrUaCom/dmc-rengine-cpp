#pragma once

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/stage_catalog.hpp"
#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table_reader.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct StageRuntimeResourceResolution final {
    StageResourceReference reference;
    RuntimeResolutionReport runtime;

    // Wave 2: kind16 == 0 performs a second lookup after rewriting the original
    // extension to .lst when the primary path is absent. We preserve that probe
    // separately because list parsing/recursion/ownership is not yet fully
    // reconstructed and therefore cannot be promoted to a normal resolved root.
    std::optional<RuntimeResolutionReport> list_fallback;

    [[nodiscard]] bool resolved() const noexcept {
        return reference.valid() && runtime.ok();
    }

    [[nodiscard]] bool list_fallback_required() const noexcept {
        return !runtime.ok() && reference.kind16 == 0U &&
            list_fallback.has_value() && list_fallback->ok();
    }
};

struct StageRuntimeResolutionReport final {
    // Stable executable descriptor-row identity inside this catalog coverage.
    // It is separate from any later evidence-backed gameplay-stage identity.
    std::string catalog_entry_id;
    std::uint32_t table_row_index{};
    StageResourceRowPlan plan;
    std::array<StageRuntimeResourceResolution, 4> resources{};
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;

    // Produce the four resolved StageBundle candidates only when every role is
    // uniquely resolved through the primary path. A located .lst fallback is
    // intentionally not treated as an equivalent root until list semantics are
    // reconstructed beyond lookup existence.
    [[nodiscard]] std::vector<gdspaces::StageMemberCandidate>
    resolved_candidates() const;
};

class StageRuntimeResolver final {
public:
    // Current production entry point begins with one exact entry from the
    // Wave-2-corrected Bank-A compatibility catalog. Full selector/group-derived
    // Stage-universe mapping remains a separate reconciliation step.
    [[nodiscard]] static StageRuntimeResolutionReport resolve_entry(
        const StageCatalogEntry& entry,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);

    // Compatibility bridge for callers that already hold one observed Bank-A
    // row. catalog_entry_id is technical resource-set identity only.
    [[nodiscard]] static StageRuntimeResolutionReport resolve_row(
        std::string catalog_entry_id,
        std::string evidence_id,
        const StageResourceTableRowObservation& row,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);
};

} // namespace dmc::rengine::profiles::dmc3
