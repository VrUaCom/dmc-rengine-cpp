#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

class EmbeddedNameEvidenceBuilder;

class EmbeddedNameAlias final {
public:
    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::uint64_t source_offset() const noexcept;

private:
    friend class EmbeddedNameEvidenceBuilder;

    EmbeddedNameAlias(std::string name, std::uint64_t source_offset);

    std::string name_;
    std::uint64_t source_offset_{};
};

// Sealed observation of the legacy DMC3 embedded-name-list convention used by
// the retained GDSpaces v6 implementation: physical slot 0 carries a compact
// text payload whose accepted filename tokens name physical slots 1..N.
//
// This is naming evidence only. It never becomes ResourceId/write authority.
class EmbeddedNameListObservation final {
public:
    [[nodiscard]] const ResourceId& parent_resource() const noexcept;
    [[nodiscard]] const ResourceId& authority_resource() const noexcept;
    [[nodiscard]] std::string_view authority_sha256() const noexcept;
    [[nodiscard]] const std::vector<EmbeddedNameAlias>& aliases() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class EmbeddedNameEvidenceBuilder;

    EmbeddedNameListObservation(
        ResourceId parent_resource,
        ResourceId authority_resource,
        std::string authority_sha256,
        std::vector<EmbeddedNameAlias> aliases);

    ResourceId parent_resource_;
    ResourceId authority_resource_;
    std::string authority_sha256_;
    std::vector<EmbeddedNameAlias> aliases_;
};

struct EmbeddedNameObserveResult final {
    std::optional<EmbeddedNameListObservation> observation;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct EmbeddedNameApplyResult final {
    bool applied{false};
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class EmbeddedNameEvidenceBuilder final {
public:
    // Reproduce the retained GDSpaces v6 GDContainerNameHints contract:
    // - only populated physical slot 0 is inspected;
    // - accepted payload size is 1..4096 bytes;
    // - UTF-8 is decoded with replacement and NUL becomes newline;
    // - printable ratio must be >= 0.75;
    // - tokens split on CR/LF/TAB/SPACE and are trim-filtered;
    // - only the exact retained filename-extension vocabulary is accepted;
    // - accepted names map sequentially to physical slots 1..N.
    [[nodiscard]] static EmbeddedNameObserveResult observe(
        const ContainerExpansion& expansion);

    // Attach sealed embedded_alias ResourceNameEvidence to physical children.
    // External .index evidence remains the stronger display-name authority.
    [[nodiscard]] static EmbeddedNameApplyResult apply(
        ContainerExpansion& expansion,
        const EmbeddedNameListObservation& observation);
};

} // namespace dmc::rengine::gdspaces
