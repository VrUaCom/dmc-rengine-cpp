#pragma once

#include "dmc_rengine/formats/container_parser_registry.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ContainerTreeExpansionLimits final {
    std::size_t max_nested_depth{8U};
    std::size_t max_expanded_containers{1024U};
    std::size_t max_graph_nodes{65536U};
    std::uint64_t max_parsed_container_bytes{256ULL * 1024ULL * 1024ULL};

    [[nodiscard]] bool valid() const noexcept;
};

struct ContainerTreeExpansion final {
    ResourceGraph graph;
    std::vector<ContainerExpansion> expansions;
    std::vector<Diagnostic> diagnostics;

    // Number of container resource identities expanded. Alias slots are
    // counted separately even when they share one parsed byte span.
    std::size_t expanded_container_count{};

    // Unique parser executions. Reused parse results do not increment this.
    std::size_t unique_parse_count{};
    std::size_t reused_parse_count{};

    // Sum of materialized bytes consumed by unique parser executions only.
    std::uint64_t parsed_container_bytes{};
    bool fully_expanded{true};

    [[nodiscard]] bool complete() const noexcept;
};

class ContainerTreeExpander final {
public:
    [[nodiscard]] static ContainerTreeExpansion expand(
        const ResourcePayload& root,
        const formats::ContainerParserRegistry& registry,
        ContainerTreeExpansionLimits limits = {});
};

} // namespace dmc::rengine::gdspaces
