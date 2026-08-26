#pragma once

#include "dmc_rengine/formats/container.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"

#include <vector>

namespace dmc::rengine::gdspaces {

struct ContainerChild final {
    formats::ContainerEntry entry;
    ResourcePayload payload;
    // How the shown name was arrived at, and — where the container carries its
    // own name list — what that list calls this slot.
    //
    // The attributed name never replaces the identity or the display name. A
    // manifest line is a claim the container makes, corroborated at most by
    // the payload agreeing with the extension, and printing it as though it
    // were recovered would be the exact mistake this field exists to avoid.
    SlotNameAttribution name_attribution;
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
