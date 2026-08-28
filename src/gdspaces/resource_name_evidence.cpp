#include "dmc_rengine/gdspaces/resource_name_evidence.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] bool valid_digest(std::string_view digest) noexcept {
    if (digest.size() != 64U) {
        return false;
    }
    return std::all_of(
        digest.begin(), digest.end(),
        [](unsigned char character) {
            return std::isxdigit(character) != 0;
        });
}

} // namespace

ResourceNameEvidence::ResourceNameEvidence(
    ResourceNameEvidenceKind kind,
    ResourceNameMappingMode mapping_mode,
    ResourceId authority_resource,
    std::string authority_sha256,
    std::string raw_label,
    std::string normalized_name,
    std::uint32_t physical_slot_index,
    std::optional<std::size_t> source_line,
    std::optional<std::uint64_t> source_offset,
    std::optional<std::size_t> extracted_ordinal)
    : kind_(kind),
      mapping_mode_(mapping_mode),
      authority_resource_(std::move(authority_resource)),
      authority_sha256_(std::move(authority_sha256)),
      raw_label_(std::move(raw_label)),
      normalized_name_(std::move(normalized_name)),
      physical_slot_index_(physical_slot_index),
      source_line_(source_line),
      source_offset_(source_offset),
      extracted_ordinal_(extracted_ordinal) {}

ResourceNameEvidenceKind ResourceNameEvidence::kind() const noexcept {
    return kind_;
}

ResourceNameMappingMode ResourceNameEvidence::mapping_mode() const noexcept {
    return mapping_mode_;
}

const ResourceId& ResourceNameEvidence::authority_resource() const noexcept {
    return authority_resource_;
}

std::string_view ResourceNameEvidence::authority_sha256() const noexcept {
    return authority_sha256_;
}

std::string_view ResourceNameEvidence::raw_label() const noexcept {
    return raw_label_;
}

std::string_view ResourceNameEvidence::normalized_name() const noexcept {
    return normalized_name_;
}

std::uint32_t ResourceNameEvidence::physical_slot_index() const noexcept {
    return physical_slot_index_;
}

const std::optional<std::size_t>& ResourceNameEvidence::source_line() const noexcept {
    return source_line_;
}

const std::optional<std::uint64_t>& ResourceNameEvidence::source_offset() const noexcept {
    return source_offset_;
}

const std::optional<std::size_t>& ResourceNameEvidence::extracted_ordinal() const noexcept {
    return extracted_ordinal_;
}

bool ResourceNameEvidence::valid() const noexcept {
    if (!authority_resource_.valid() || !valid_digest(authority_sha256_) ||
        raw_label_.empty() || normalized_name_.empty()) {
        return false;
    }

    switch (kind_) {
    case ResourceNameEvidenceKind::external_index:
        return source_line_.has_value() && *source_line_ > 0U &&
               !source_offset_.has_value() && extracted_ordinal_.has_value() &&
               (mapping_mode_ == ResourceNameMappingMode::physical_position ||
                mapping_mode_ == ResourceNameMappingMode::populated_slot_sequence);
    case ResourceNameEvidenceKind::embedded_alias:
        return physical_slot_index_ > 0U &&
               !source_line_.has_value() && source_offset_.has_value() &&
               !extracted_ordinal_.has_value() &&
               *source_offset_ < authority_resource_.size &&
               mapping_mode_ == ResourceNameMappingMode::embedded_alias_sequence;
    }
    return false;
}

} // namespace dmc::rengine::gdspaces
