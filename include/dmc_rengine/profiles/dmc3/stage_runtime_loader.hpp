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
    [[nodiscard]] bool complete() const noexcept;
};

struct StageRuntimeLoadReport final {
    StageRuntimeResolutionReport resolution;
    std::array<StageRuntimeLoadedResource, 4> resources{};
    std::optional<gdspaces::StageBundle> bundle;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete() const noexcept;
};

class StageRuntimeLoader final {
public:
    // Level-C composition only. This consumes the already-evidenced resolver,
    // source read/materialization, byte provenance and container parser stack.
    // It does not claim cache/lifetime/unload equivalence.
    [[nodiscard]] static StageRuntimeLoadReport load_entry(
        const StageCatalogEntry& entry,
        const VolumeBootstrapPlan& bootstrap,
        const RuntimeSourceBindings& bindings,
        const gdspaces::SourceRegistry& sources,
        const formats::ContainerParserRegistry& parsers,
        gdspaces::ContainerTreeExpansionLimits limits = {});
};

} // namespace dmc::rengine::profiles::dmc3
