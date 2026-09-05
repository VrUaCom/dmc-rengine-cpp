#include "dmc_rengine/profiles/dmc3/legacy_extraction_naming.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::string leaf_name(std::string_view value) {
    const auto separator = value.find_last_of("/\\");
    const auto leaf = separator == std::string_view::npos
        ? value
        : value.substr(separator + 1U);
    return std::string{leaf};
}

[[nodiscard]] std::optional<std::string> safe_relative_export_path(
    std::string_view value) {
    if (value.empty() || value.front() == '/' || value.front() == '\\' ||
        value.find(':') != std::string_view::npos) {
        return std::nullopt;
    }

    std::string normalized{value};
    for (auto& character : normalized) {
        if (character == '\\') {
            character = '/';
        }
    }

    std::size_t at = 0U;
    while (at <= normalized.size()) {
        const auto end = normalized.find('/', at);
        const auto component = end == std::string::npos
            ? std::string_view{normalized}.substr(at)
            : std::string_view{normalized}.substr(at, end - at);
        if (component.empty() || component == "." || component == "..") {
            return std::nullopt;
        }
        if (end == std::string::npos) {
            break;
        }
        at = end + 1U;
    }

    return normalized;
}

} // namespace

bool LegacyExtractionNamingPlan::valid() const noexcept {
    if (!resource_id.valid() || canonical_display_name.empty()) {
        return false;
    }
    if (export_safe != (export_name.has_value() && export_path.has_value())) {
        return false;
    }
    if (export_safe && (export_name->empty() || export_path->empty())) {
        return false;
    }

    switch (representation) {
    case LegacyExtractionRepresentation::unavailable:
        return !exact_from_external_index &&
            !manifest_entry_raw.has_value() &&
            !extraction_name.has_value() &&
            !nested_index_name.has_value() && !export_safe;
    case LegacyExtractionRepresentation::file:
        return exact_from_external_index && extracted_ordinal.has_value() &&
            manifest_entry_raw.has_value() && extraction_name.has_value() &&
            !extraction_name->empty() && !nested_index_name.has_value();
    case LegacyExtractionRepresentation::expanded_directory:
        return exact_from_external_index && extracted_ordinal.has_value() &&
            manifest_entry_raw.has_value() && extraction_name.has_value() &&
            !extraction_name->empty() && nested_index_name.has_value() &&
            !nested_index_name->empty();
    }
    return false;
}

LegacyExtractionNamingPlan LegacyExtractionNamingPlanner::build(
    const gdspaces::ResourceNamingIdentity& identity) {
    LegacyExtractionNamingPlan result{
        .resource_id = identity.resource_id,
        .physical_slot_index = identity.physical_slot_index,
        .extracted_ordinal = identity.extracted_ordinal,
        .representation = LegacyExtractionRepresentation::unavailable,
        .manifest_entry_raw = std::nullopt,
        .extraction_name = std::nullopt,
        .nested_index_name = std::nullopt,
        .embedded_semantic_alias = identity.embedded_alias,
        .export_safe = false,
        .export_name = std::nullopt,
        .export_path = std::nullopt,
        .canonical_display_name = identity.canonical_display_name,
        .exact_from_external_index = false,
    };

    if (!identity.external_index_normalized_name().has_value() ||
        !identity.external_index_raw_label.has_value() ||
        !identity.extracted_ordinal.has_value()) {
        return result;
    }

    result.manifest_entry_raw = identity.external_index_raw_label;
    result.extraction_name = identity.external_index_normalized_name();
    result.exact_from_external_index = true;

    if (identity.external_index_folder_marker()) {
        result.representation =
            LegacyExtractionRepresentation::expanded_directory;
        auto nested = leaf_name(*identity.external_index_normalized_name());
        nested.append(".index");
        result.nested_index_name = std::move(nested);
    } else {
        result.representation = LegacyExtractionRepresentation::file;
    }

    const auto export_path = safe_relative_export_path(
        *identity.external_index_normalized_name());
    if (export_path.has_value()) {
        result.export_path = *export_path;
        result.export_name = leaf_name(*export_path);
        result.export_safe = true;
    }

    return result;
}

} // namespace dmc::rengine::profiles::dmc3
