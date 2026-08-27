#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/index_manifest.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

enum class IndexSlotMappingMode : unsigned char {
    physical_position,
    populated_slot_sequence,
};

enum class IndexSlotBindingIssue : unsigned char {
    manifest_entry_without_slot,
    slot_without_manifest_entry,
};

struct IndexSlotBindingDiagnostic final {
    IndexSlotBindingIssue issue{IndexSlotBindingIssue::slot_without_manifest_entry};
    std::optional<std::uint32_t> slot_index;
    std::optional<std::size_t> manifest_line;
};

class IndexSlotNameAuthority final {
public:
    [[nodiscard]] std::uint32_t slot_index() const noexcept;
    [[nodiscard]] const ResourceId& child_resource() const noexcept;
    [[nodiscard]] std::string_view raw_index_label() const noexcept;
    [[nodiscard]] std::string_view index_name() const noexcept;
    [[nodiscard]] std::string_view stem() const noexcept;
    [[nodiscard]] const std::optional<std::string>& source_extension() const noexcept;
    [[nodiscard]] bool is_folder() const noexcept;
    [[nodiscard]] std::size_t manifest_line() const noexcept;

private:
    friend class IndexSlotNameBinder;

    IndexSlotNameAuthority(
        std::uint32_t slot_index,
        ResourceId child_resource,
        std::string raw_index_label,
        std::string index_name,
        std::string stem,
        std::optional<std::string> source_extension,
        bool is_folder,
        std::size_t manifest_line);

    std::uint32_t slot_index_{};
    ResourceId child_resource_;
    std::string raw_index_label_;
    std::string index_name_;
    std::string stem_;
    std::optional<std::string> source_extension_;
    bool is_folder_{false};
    std::size_t manifest_line_{};
};

class IndexSlotBindingResult final {
public:
    [[nodiscard]] const ResourceId& parent_resource() const noexcept;
    [[nodiscard]] const ResourceId& manifest_resource() const noexcept;
    [[nodiscard]] std::string_view manifest_sha256() const noexcept;
    [[nodiscard]] IndexSlotMappingMode mapping_mode() const noexcept;
    [[nodiscard]] const std::vector<IndexSlotNameAuthority>& authorities() const noexcept;
    [[nodiscard]] const std::vector<IndexSlotBindingDiagnostic>& diagnostics() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class IndexSlotNameBinder;

    IndexSlotBindingResult(
        ResourceId parent_resource,
        ResourceId manifest_resource,
        std::string manifest_sha256,
        IndexSlotMappingMode mapping_mode,
        std::vector<IndexSlotNameAuthority> authorities,
        std::vector<IndexSlotBindingDiagnostic> diagnostics);

    ResourceId parent_resource_;
    ResourceId manifest_resource_;
    std::string manifest_sha256_;
    IndexSlotMappingMode mapping_mode_{IndexSlotMappingMode::physical_position};
    std::vector<IndexSlotNameAuthority> authorities_;
    std::vector<IndexSlotBindingDiagnostic> diagnostics_;
};

struct IndexSlotBindingBuildResult final {
    std::optional<IndexSlotBindingResult> binding;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class IndexSlotNameBinder final {
public:
    [[nodiscard]] static IndexSlotBindingBuildResult bind(
        const ContainerExpansion& expansion,
        const IndexManifest& manifest);
};

} // namespace dmc::rengine::gdspaces
