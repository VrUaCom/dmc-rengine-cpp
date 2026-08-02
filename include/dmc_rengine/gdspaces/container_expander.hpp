#pragma once

#include "dmc_rengine/formats/container.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"

#include <vector>

namespace dmc::rengine::gdspaces {

struct ContainerChild final {
    formats::ContainerEntry entry;
    ResourcePayload payload;
};

struct ContainerExpansion final {
    ResourceRef parent;
    std::string parser_format;
    std::vector<ContainerChild> children;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool usable() const noexcept;
};

class ContainerExpander final {
public:
    [[nodiscard]] static ContainerExpansion expand(
        const ResourcePayload& parent,
        const formats::ContainerParseResult& parsed);

    static void connect_graph(
        const ContainerExpansion& expansion,
        ResourceGraph& graph);
};

} // namespace dmc::rengine::gdspaces
