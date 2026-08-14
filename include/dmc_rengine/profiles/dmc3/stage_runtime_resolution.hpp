#pragma once

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/stage_catalog.hpp"
#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table_reader.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct StageRuntimeResourceResolution final {
    StageResourceReference reference;
    RuntimeResolutionReport runtime;

    // Wave-2 kind16==0 may probe a sibling .lst when the primary path is
    // absent. The list remains separate until grammar, recursion, ownership and
    // error behavior are fully reconstructed.
    std::optional<RuntimeResolutionReport> list_fallback;

    [[nodiscard]] bool resolved() const noexcept {
        return reference.valid() && runtime.ok();
    }

    [[nodiscard]] bool list_fallback_required() const noexcept {
        return !runtime.ok() && reference.kind16.has_value() &&
            *reference.kind16 == 0U && list_fallback.has_value() &&
            list_fallback->ok();
    }
};

struct StageRuntimeResolutionReport final {
    // Distinct executable/resource identity axes preserved through lookup.
    std::string catalog_entry_id;
    std::uint32_t table_row_index{}; // global catalog index
    std::string source_table_id;
    std::uint32_t source_row_index{};
    std::optional<std::uint16_t> numeric_stage_id;
    std::optional<std::string> semantic_stage_id;
    StageResourceRowPlan plan;
    std::array<StageRuntimeResourceResolution, 4> resources{};
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;

    // Primary-path resources only. A located .lst fallback is not promoted to
    // an equivalent loaded root before list semantics are reconstructed.
    [[nodiscard]] std::vector<gdspaces::StageMemberCandidate>
    resolved_candidates() const;
};

class StageRuntimeResolver final {
public:
    [[nodiscard]] static StageRuntimeResolutionReport resolve_entry(
        const StageCatalogEntry& entry,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);

    // Compatibility bridge for callers holding a raw descriptor row. Source
    // bank provenance and selector-facing numeric identity are unavailable.
    [[nodiscard]] static StageRuntimeResolutionReport resolve_row(
        std::string catalog_entry_id,
        std::string evidence_id,
        const StageResourceTableRowObservation& row,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources);
};

} // namespace dmc::rengine::profiles::dmc3
