#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

class EmbeddedNameEvidenceBuilder;
class IndexNameOverlayBuilder;

enum class ResourceNameEvidenceKind : unsigned char {
    external_index,
    embedded_alias,
};

enum class ResourceNameMappingMode : unsigned char {
    // Legacy/superseded value retained for explicit recognition only. External
    // DMC3 .index evidence using this mode is invalid after extracted-ordinal
    // recovery; physical slots remain locators, not naming sequence authority.
    physical_position,
    populated_slot_sequence,
    embedded_alias_sequence,
};

// Read-only naming evidence attached to a materialized resource. This is not
// part of ResourceId and is never write authority. Construction is deliberately
// sealed so arbitrary callers cannot launder an invented label/hash pair into
// evidence-looking metadata.
class ResourceNameEvidence final {
public:
    [[nodiscard]] ResourceNameEvidenceKind kind() const noexcept;
    [[nodiscard]] ResourceNameMappingMode mapping_mode() const noexcept;
    [[nodiscard]] const ResourceId& authority_resource() const noexcept;
    [[nodiscard]] std::string_view authority_sha256() const noexcept;
    [[nodiscard]] std::string_view raw_label() const noexcept;
    [[nodiscard]] std::string_view normalized_name() const noexcept;
    [[nodiscard]] std::uint32_t physical_slot_index() const noexcept;
    [[nodiscard]] const std::optional<std::size_t>& source_line() const noexcept;
    [[nodiscard]] const std::optional<std::uint64_t>& source_offset() const noexcept;
    [[nodiscard]] const std::optional<std::size_t>& extracted_ordinal() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class EmbeddedNameEvidenceBuilder;
    friend class IndexNameOverlayBuilder;

    ResourceNameEvidence(
        ResourceNameEvidenceKind kind,
        ResourceNameMappingMode mapping_mode,
        ResourceId authority_resource,
        std::string authority_sha256,
        std::string raw_label,
        std::string normalized_name,
        std::uint32_t physical_slot_index,
        std::optional<std::size_t> source_line,
        std::optional<std::uint64_t> source_offset,
        std::optional<std::size_t> extracted_ordinal = std::nullopt);

    ResourceNameEvidenceKind kind_{ResourceNameEvidenceKind::external_index};
    ResourceNameMappingMode mapping_mode_{ResourceNameMappingMode::populated_slot_sequence};
    ResourceId authority_resource_;
    std::string authority_sha256_;
    std::string raw_label_;
    std::string normalized_name_;
    std::uint32_t physical_slot_index_{};
    std::optional<std::size_t> source_line_;
    std::optional<std::uint64_t> source_offset_;
    std::optional<std::size_t> extracted_ordinal_;
};

} // namespace dmc::rengine::gdspaces
