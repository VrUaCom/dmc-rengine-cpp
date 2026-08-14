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
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

// One exact reference occurrence observed in the executable table. This is
// structural table evidence only; it does not assert semantic stage ownership.
struct StageCatalogReferenceUse final {
    std::uint32_t row_index{};
    std::uint32_t column_index{};
    StageResourceRole role{StageResourceRole::script};
};

// Exact logical paths repeated by more than one table cell. Equality here is
// deliberately byte/string equality from the EXE observation, not GDSpaces
// runtime-normalized identity and not a claim that the game objects are shared.
struct StageCatalogRepeatedReference final {
    std::string logical_path;
    std::vector<StageCatalogReferenceUse> uses;

    [[nodiscard]] bool repeated() const noexcept {
        return uses.size() > 1U;
    }
};

struct StageCatalogEntry final {
    // Stable identity of the observed executable table row. This is not a
    // gameplay stage id and must never be inferred from an stNNN filename.
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
    // not promoted to semantic stage identity.
    [[nodiscard]] StageResourceRowPlan resource_plan() const;
};

struct StageCatalog final {
    std::string table_id;
    std::string evidence_id;
    std::vector<StageCatalogEntry> entries;
    std::vector<StageCatalogRepeatedReference> repeated_references;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete(
        const StageResourceTableDescriptor& descriptor) const noexcept;

    [[nodiscard]] const StageCatalogEntry* find(
        std::uint32_t row_index) const noexcept;
};

class StageCatalogBuilder final {
public:
    // Build a deterministic catalog from raw executable table observations.
    // No filename templates, variant labels, aliases, or gameplay-stage
    // semantics are invented here.
    [[nodiscard]] static StageCatalog build(
        const StageResourceTableReadResult& table,
        const StageResourceTableDescriptor& descriptor);
};

struct StageCatalogLoadResult final {
    std::string artifact_sha256;
    bool canonical_artifact{};
    StageCatalog catalog;

    [[nodiscard]] bool complete() const noexcept;
};

class StageCatalogLoader final {
public:
    // Production gate for the canonical DMC3 catalog. Unlike the structural
    // reader used by synthetic tests, this path computes SHA-256 over the
    // supplied executable bytes and refuses non-canonical artifacts before any
    // table observation can be promoted into the catalog.
    [[nodiscard]] static StageCatalogLoadResult load_canonical(
        std::span<const std::byte> executable_bytes,
        const exe::PeImage& image,
        std::size_t max_path_bytes = 260U);
};

} // namespace dmc::rengine::profiles::dmc3
