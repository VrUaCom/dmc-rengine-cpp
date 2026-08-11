#pragma once

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
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
    [[nodiscard]] static StageRuntimeResolutionReport resolve_row(
        std::string stage_id,
        std::string evidence_id,
        const StageResourceTableRowObservation& row,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);
};

} // namespace dmc::rengine::profiles::dmc3
