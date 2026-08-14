#pragma once

#include "dmc_rengine/formats/container_parser_registry.hpp"
#include "dmc_rengine/gdspaces/container_tree_expander.hpp"
#include "dmc_rengine/gdspaces/stage_bundle.hpp"
#include "dmc_rengine/profiles/dmc3/stage_runtime_resolution.hpp"

#include <array>
#include <optional>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct StageRuntimeLoadedResource final {
    StageRuntimeResourceResolution resolution;
    std::optional<gdspaces::ResourcePayload> payload;
    std::optional<gdspaces::ContainerTreeExpansion> expansion;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool payload_valid() const noexcept;

    // Complete for the current product-side materialization/expansion boundary.
    // It does not mean the original game state-2 -> state-3 typed post-load path
    // has executed.
    [[nodiscard]] bool complete() const noexcept;
};

struct StageRuntimeLoadReport final {
    StageRuntimeResolutionReport resolution;
    std::array<StageRuntimeLoadedResource, 4> resources{};
    std::optional<gdspaces::StageBundle> bundle;
    std::vector<gdspaces::Diagnostic> diagnostics;

    // The strongest behavior the current loader actually proves: primary Stage
    // references resolved, bytes/provenance validated, required containers fully
    // expanded, and a valid product StageBundle assembled.
    [[nodiscard]] bool materialization_complete() const noexcept;

    // Source-compatibility alias. Callers that need game-runtime equivalence must
    // use game_ready_equivalent() rather than interpreting complete() as Level C.
    [[nodiscard]] bool complete() const noexcept {
        return materialization_complete();
    }

    // Wave 2 confirms that state 2 performs typed in-place post-load
    // normalization before state 3 ready. That phase is not yet reconstructed in
    // this product loader, so the answer must remain false until a later recovered
    // game-runtime integration explicitly proves otherwise.
    [[nodiscard]] bool game_ready_equivalent() const noexcept {
        return false;
    }
};

class StageRuntimeLoader final {
public:
    // Product-side materialization composition. It consumes the evidenced
    // resolver, source read/materialization, ByteProvenance and container parser
    // stack. It deliberately stops before the recovered game's typed post-load,
    // factory, cache/lifetime and teardown phases.
    [[nodiscard]] static StageRuntimeLoadReport load_entry(
        const StageCatalogEntry& entry,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources,
        const formats::ContainerParserRegistry& parsers,
        gdspaces::ContainerTreeExpansionLimits limits = {});
};

} // namespace dmc::rengine::profiles::dmc3
