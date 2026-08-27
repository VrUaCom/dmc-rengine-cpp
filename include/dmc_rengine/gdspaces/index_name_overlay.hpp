#pragma once

#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

struct IndexProfileDisplaySemantic final {
    std::string canonical_extension;
    std::string semantic_format;
};

using IndexProfileDisplayResolver = std::optional<IndexProfileDisplaySemantic> (*)(
    const ResourcePayload& child,
    const IndexSlotNameAuthority& authority);

enum class IndexDisplayEvidenceKind : unsigned char {
    index_source_extension,
    magic_confirmed_format,
    profile_structural_format,
    embedded_name_list_format,
};

class IndexNameOverlayEntry final {
public:
    [[nodiscard]] std::uint32_t slot_index() const noexcept;
    [[nodiscard]] const ResourceId& child_resource() const noexcept;
    [[nodiscard]] std::string_view display_name() const noexcept;
    [[nodiscard]] std::string_view raw_index_label() const noexcept;
    [[nodiscard]] std::string_view index_name() const noexcept;
    [[nodiscard]] std::size_t manifest_line() const noexcept;
    [[nodiscard]] std::string_view semantic_format() const noexcept;
    [[nodiscard]] IndexDisplayEvidenceKind evidence_kind() const noexcept;

private:
    friend class IndexNameOverlayBuilder;

    IndexNameOverlayEntry(
        std::uint32_t slot_index,
        ResourceId child_resource,
        std::string display_name,
        std::string raw_index_label,
        std::string index_name,
        std::size_t manifest_line,
        std::string semantic_format,
        IndexDisplayEvidenceKind evidence_kind);

    std::uint32_t slot_index_{};
    ResourceId child_resource_;
    std::string display_name_;
    std::string raw_index_label_;
    std::string index_name_;
    std::size_t manifest_line_{};
    std::string semantic_format_;
    IndexDisplayEvidenceKind evidence_kind_{
        IndexDisplayEvidenceKind::index_source_extension};
};

class IndexNameOverlay final {
public:
    [[nodiscard]] const ResourceId& parent_resource() const noexcept;
    [[nodiscard]] const ResourceId& manifest_resource() const noexcept;
    [[nodiscard]] std::string_view manifest_sha256() const noexcept;
    [[nodiscard]] IndexSlotMappingMode mapping_mode() const noexcept;
    [[nodiscard]] const std::vector<IndexNameOverlayEntry>& entries() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class IndexNameOverlayBuilder;

    IndexNameOverlay(
        ResourceId parent_resource,
        ResourceId manifest_resource,
        std::string manifest_sha256,
        IndexSlotMappingMode mapping_mode,
        std::vector<IndexNameOverlayEntry> entries);

    ResourceId parent_resource_;
    ResourceId manifest_resource_;
    std::string manifest_sha256_;
    IndexSlotMappingMode mapping_mode_{IndexSlotMappingMode::physical_position};
    std::vector<IndexNameOverlayEntry> entries_;
};

struct IndexNameOverlayBuildResult final {
    std::optional<IndexNameOverlay> overlay;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct IndexNameOverlayApplyResult final {
    bool applied{false};
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class IndexNameOverlayBuilder final {
public:
    [[nodiscard]] static IndexNameOverlayBuildResult build(
        const ContainerExpansion& expansion,
        const IndexSlotBindingResult& binding,
        IndexProfileDisplayResolver profile_resolver = nullptr);

    [[nodiscard]] static IndexNameOverlayApplyResult apply(
        ContainerExpansion& expansion,
        const IndexNameOverlay& overlay);
};

} // namespace dmc::rengine::gdspaces
