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

// One exact reference occurrence observed in the executable descriptor bank.
// This is structural evidence only; it does not assert semantic stage ownership.
struct StageCatalogReferenceUse final {
    std::uint32_t row_index{};
    std::uint32_t column_index{};
    StageResourceRole role{StageResourceRole::script};
};

// Exact logical paths repeated by more than one observed cell. Equality here is
// deliberately byte/string equality from the EXE observation, not GDSpaces
// runtime-normalized identity and not a claim that game objects are shared.
struct StageCatalogRepeatedReference final {
    std::string logical_path;
    std::vector<StageCatalogReferenceUse> uses;

    [[nodiscard]] bool repeated() const noexcept {
        return uses.size() > 1U;
    }
};

struct StageCatalogEntry final {
    // Stable identity of one observed descriptor-bank row in this catalog
    // coverage. This is not a gameplay stage id and must never be inferred from
    // an stNNN filename.
    std::string catalog_entry_id;
    std::uint32_t row_index{};
    std::string evidence_id;
    StageResourceTableRowObservation observation;

    // Optional semantic identity may be attached only by separate evidence.
    // The StageCatalog builder never derives it from resource filenames.
    std::optional<std::string> semantic_stage_id;

    [[nodiscard]] bool complete() const noexcept;

    // Compatibility bridge to the existing four-role resolver contract. The
    // StageResourceRowPlan::stage_id field carries catalog_entry_id here; it is
    // not promoted to semantic stage identity. Wave-2 kind16 observations are
    // preserved into each role reference where available.
    [[nodiscard]] StageResourceRowPlan resource_plan() const;
};

struct StageCatalog final {
    // Current production builder covers corrected Wave-2 Bank A only. The enum
    // prevents a complete 110-row Bank-A read from being misrepresented as the
    // full 189-descriptor + selector/group Stage universe.
    StageCatalogCoverage coverage{StageCatalogCoverage::wave2_bank_a_compatibility};
    std::string table_id;
    std::string evidence_id;
    std::vector<StageCatalogEntry> entries;
    std::vector<StageCatalogRepeatedReference> repeated_references;
    std::vector<gdspaces::Diagnostic> diagnostics;

    // Complete for this catalog's advertised coverage, not necessarily for the
    // full DMC3 Stage descriptor universe.
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
    // Build a deterministic Wave-2 Bank-A compatibility catalog from raw
    // executable observations. Full selector/group-derived coverage is a
    // separate reconciliation step and is never inferred from filenames.
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
    // Production gate for the canonical Wave-2-corrected Bank-A compatibility
    // catalog. This is not yet the full selector-derived DMC3 Stage universe.
    // The supplied bytes are SHA-gated before observations are promoted.
    [[nodiscard]] static StageCatalogLoadResult load_canonical(
        std::span<const std::byte> executable_bytes,
        const exe::PeImage& image,
        std::size_t max_path_bytes = 260U);
};

} // namespace dmc::rengine::profiles::dmc3
