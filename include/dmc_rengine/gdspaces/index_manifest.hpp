#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

enum class IndexContainerDirective : unsigned char {
    none,
    pnst_non_empty_slots,
};

struct IndexManifestEntry final {
    std::string raw;
    std::string name;
    std::string stem;
    std::optional<std::string> extension;
    bool is_folder{false};
    std::size_t line_number{};
};

class IndexManifest final {
public:
    [[nodiscard]] const ResourceId& source() const noexcept;
    [[nodiscard]] std::string_view observed_sha256() const noexcept;
    [[nodiscard]] IndexContainerDirective directive() const noexcept;
    [[nodiscard]] const std::vector<IndexManifestEntry>& entries() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class IndexManifestParser;

    IndexManifest(
        ResourceId source,
        std::string observed_sha256,
        IndexContainerDirective directive,
        std::vector<IndexManifestEntry> entries);

    ResourceId source_;
    std::string observed_sha256_;
    IndexContainerDirective directive_{IndexContainerDirective::none};
    std::vector<IndexManifestEntry> entries_;
};

struct IndexManifestParseResult final {
    std::optional<IndexManifest> manifest;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class IndexManifestParser final {
public:
    [[nodiscard]] static IndexManifestParseResult parse(
        const ResourcePayload& index_payload);
};

} // namespace dmc::rengine::gdspaces
