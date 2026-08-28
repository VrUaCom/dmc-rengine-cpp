#pragma once

#include "dmc_rengine/formats/container.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ContainerChild final {
    formats::ContainerEntry entry;
    ResourcePayload payload;
};

// Parent-level external .index observation retained after reconciliation.
// `directive` is presentation/extraction metadata (currently "" or "PNST");
// it never controls physical slot identity or write targeting.
struct ContainerIndexNamingEvidence final {
    ResourceId manifest_resource;
    std::string manifest_sha256;
    std::string directive;
    std::size_t entry_count{};
};

struct ContainerExpansion final {
    ResourceRef parent;
    std::string parser_format;
    std::vector<ContainerChild> children;
    std::vector<Diagnostic> diagnostics;
    std::optional<ContainerIndexNamingEvidence> external_index_evidence;

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
