#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table_reader.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class StageCatalogCoverage {
    wave2_bank_a_compatibility,
    full_selector_universe,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageCatalogCoverage coverage) noexcept {
    switch (coverage) {
    case StageCatalogCoverage::wave2_bank_a_compatibility:
        return "wave2-bank-a-compatibility";
    case StageCatalogCoverage::full_selector_universe:
        return "full-selector-universe";
    }
    return "wave2-bank-a-compatibility";
}

struct StageCatalogReferenceUse final {
    std::uint32_t row_index{}; // global catalog index
    std::uint32_t column_index{};
    StageResourceRole role{StageResourceRole::script};
    std::string source_table_id;
    std::uint32_t source_row_index{};
};

struct StageCatalogRepeatedReference final {
    std::string logical_path;
    std::vector<StageCatalogReferenceUse> uses;

    [[nodiscard]] bool repeated() const noexcept {
        return uses.size() > 1U;
    }
};

struct StageCatalogEntry final {
    // Technical catalog identity, numeric selector-facing identity and gameplay
    // semantics are intentionally separate axes.
    std::string catalog_entry_id;
    std::uint32_t row_index{}; // global catalog index
    std::optional<std::uint16_t> numeric_stage_id;
    std::string evidence_id;
    StageResourceTableRowObservation observation;
    std::optional<std::string> semantic_stage_id;

    // Exact executable provenance. Empty only for legacy manual/synthetic
    // fixtures created before the two-bank model existed.
    std::string source_table_id;
    std::uint32_t source_row_index{};

    [[nodiscard]] bool complete() const noexcept;
    [[nodiscard]] StageResourceRowPlan resource_plan() const;
};

struct StageCatalog final {
    StageCatalogCoverage coverage{StageCatalogCoverage::wave2_bank_a_compatibility};
    std::string table_id;
    std::string evidence_id;
    std::vector<StageCatalogEntry> entries;
    std::vector<StageCatalogRepeatedReference> repeated_references;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete(
        const StageResourceTableDescriptor& descriptor) const noexcept;
    [[nodiscard]] bool complete_full_universe() const noexcept;

    [[nodiscard]] bool covers_full_stage_universe() const noexcept {
        return coverage == StageCatalogCoverage::full_selector_universe;
    }

    [[nodiscard]] const StageCatalogEntry* find(
        std::uint32_t row_index) const noexcept;
    [[nodiscard]] const StageCatalogEntry* find_numeric_stage(
        std::uint16_t numeric_stage_id) const noexcept;
};

class StageCatalogBuilder final {
public:
    [[nodiscard]] static StageCatalog build(
        const StageResourceTableReadResult& table,
        const StageResourceTableDescriptor& descriptor);

    [[nodiscard]] static StageCatalog build_full_universe(
        const std::array<StageResourceTableReadResult, 2>& tables,
        const std::array<StageResourceTableDescriptor, 2>& descriptors);
};

struct StageCatalogLoadResult final {
    std::string artifact_sha256;
    bool canonical_artifact{};
    StageCatalog catalog;

    [[nodiscard]] bool complete() const noexcept;

    [[nodiscard]] bool bank_a_complete() const noexcept {
        return complete() &&
            catalog.coverage == StageCatalogCoverage::wave2_bank_a_compatibility;
    }

    [[nodiscard]] bool full_stage_universe_complete() const noexcept {
        return complete() && catalog.covers_full_stage_universe();
    }
};

class StageCatalogLoader final {
public:
    // Compatibility production gate retained for existing 110-row Bank-A users.
    [[nodiscard]] static StageCatalogLoadResult load_canonical(
        std::span<const std::byte> executable_bytes,
        std::size_t max_path_bytes = 260U);

    // Wave-3 full-universe gate. Reads both exact executable banks and produces
    // 189 deterministic entries while preserving source-bank provenance.
    [[nodiscard]] static StageCatalogLoadResult load_canonical_full_universe(
        std::span<const std::byte> executable_bytes,
        std::size_t max_path_bytes = 260U);
};

} // namespace dmc::rengine::profiles::dmc3
