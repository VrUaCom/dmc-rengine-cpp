#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table_reader.hpp"

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
    std::uint32_t row_index{};
    std::uint32_t column_index{};
    StageResourceRole role{StageResourceRole::script};
};

struct StageCatalogRepeatedReference final {
    std::string logical_path;
    std::vector<StageCatalogReferenceUse> uses;

    [[nodiscard]] bool repeated() const noexcept {
        return uses.size() > 1U;
    }
};

struct StageCatalogEntry final {
    std::string catalog_entry_id;
    std::uint32_t row_index{};
    std::string evidence_id;
    StageResourceTableRowObservation observation;
    std::optional<std::string> semantic_stage_id;

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

    [[nodiscard]] bool covers_full_stage_universe() const noexcept {
        return coverage == StageCatalogCoverage::full_selector_universe;
    }

    [[nodiscard]] const StageCatalogEntry* find(
        std::uint32_t row_index) const noexcept;
};

class StageCatalogBuilder final {
public:
    // Deterministic Wave-2 Bank-A compatibility builder. No selector mapping,
    // variant semantics, aliases, or gameplay-stage identity are inferred.
    [[nodiscard]] static StageCatalog build(
        const StageResourceTableReadResult& table,
        const StageResourceTableDescriptor& descriptor);
};

struct StageCatalogLoadResult final {
    std::string artifact_sha256;
    bool canonical_artifact{};
    StageCatalog catalog;

    // Compatibility name: complete for advertised catalog coverage.
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
    // Production canonical gate. The PE image is deliberately parsed inside
    // this function from the exact same byte span that is SHA-256 verified, so
    // callers cannot pair canonical bytes with an unrelated injected PeImage.
    // Current coverage remains corrected Wave-2 Bank A, not the full selector
    // universe.
    [[nodiscard]] static StageCatalogLoadResult load_canonical(
        std::span<const std::byte> executable_bytes,
        std::size_t max_path_bytes = 260U);
};

} // namespace dmc::rengine::profiles::dmc3
