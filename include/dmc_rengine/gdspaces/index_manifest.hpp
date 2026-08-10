#pragma once

#include "dmc_rengine/formats/container.hpp"
#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

struct IndexManifestEntry final {
    std::size_t ordinal{};
    std::string label;
    bool folder{false};
    std::string raw_line;
};

struct IndexManifest final {
    bool pnst_directive{false};
    std::vector<IndexManifestEntry> entries;
    std::vector<Diagnostic> diagnostics;
};

struct ContainerIndexLink final {
    std::uint32_t slot_index{};
    std::size_t manifest_ordinal{};
    std::string label;
    bool folder{false};
};

struct ContainerIndexLinkReport final {
    std::vector<ContainerIndexLink> links;
    std::vector<Diagnostic> diagnostics;
    bool safe_positional_mapping{false};
};

[[nodiscard]] IndexManifest parse_index_manifest(std::string_view text);

[[nodiscard]] ContainerIndexLinkReport link_index_manifest(
    const formats::ContainerDocument& container,
    const IndexManifest& manifest,
    std::optional<ResourceId> resource = std::nullopt);

} // namespace dmc::rengine::gdspaces
