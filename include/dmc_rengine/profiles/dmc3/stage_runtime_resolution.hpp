#pragma once

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/stage_catalog.hpp"
#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table_reader.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct StageRuntimeResourceResolution final {
    StageResourceReference reference;
    RuntimeResolutionReport runtime;

    [[nodiscard]] bool resolved() const noexcept {
        return reference.valid() && runtime.ok();
    }
};

struct StageRuntimeResolutionReport final {
    // Stable executable table-row identity. It is separate from any later
    // evidence-backed semantic gameplay-stage identity.
    std::string catalog_entry_id;
    std::uint32_t table_row_index{};
    StageResourceRowPlan plan;
    std::array<StageRuntimeResourceResolution, 4> resources{};
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;

    // Produce the four resolved StageBundle candidates only when every role is
    // uniquely resolved. This does not read bytes or claim load-path success.
    [[nodiscard]] std::vector<gdspaces::StageMemberCandidate>
    resolved_candidates() const;
};

class StageRuntimeResolver final {
public:
    // Canonical production entry point: stage resolution begins with one exact
    // StageCatalog entry, not an inferred stNNN identifier or filename family.
    [[nodiscard]] static StageRuntimeResolutionReport resolve_entry(
        const StageCatalogEntry& entry,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);

    // Compatibility bridge for callers that already hold one raw table row.
    // catalog_entry_id is row identity only and must not be interpreted as a
    // semantic gameplay stage id.
    [[nodiscard]] static StageRuntimeResolutionReport resolve_row(
        std::string catalog_entry_id,
        std::string evidence_id,
        const StageResourceTableRowObservation& row,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);
};

} // namespace dmc::rengine::profiles::dmc3
