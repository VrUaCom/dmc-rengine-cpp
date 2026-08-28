#include "dmc_rengine/gdspaces/resource_naming_identity.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/format_identity.hpp"

#include <algorithm>
#include <cctype>
#include <span>
#include <string_view>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] bool has_error(const std::vector<Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

void add_error(
    std::vector<Diagnostic>& diagnostics,
    const ResourceId& resource,
    std::string code,
    std::string message) {
    diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

[[nodiscard]] std::string_view trim_ascii(std::string_view value) noexcept {
    while (!value.empty() &&
           std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1U);
    }
    while (!value.empty() &&
           std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1U);
    }
    return value;
}

[[nodiscard]] bool is_exact_folder_marker(
    const ResourceNameEvidence& evidence) noexcept {
    if (evidence.kind() != ResourceNameEvidenceKind::external_index) {
        return false;
    }

    constexpr std::string_view marker = "folder";
    const auto raw = trim_ascii(evidence.raw_label());
    if (raw.size() <= marker.size() ||
        raw.substr(raw.size() - marker.size()) != marker) {
        return false;
    }
    const auto marker_start = raw.size() - marker.size();
    if (std::isspace(static_cast<unsigned char>(raw[marker_start - 1U])) == 0) {
        return false;
    }
    const auto prefix = trim_ascii(raw.substr(0U, marker_start));
    return prefix == evidence.normalized_name();
}

} // namespace

bool ResourceNamingIdentity::valid() const noexcept {
    if (!resource_id.valid() || canonical_display_name.empty() ||
        semantic_format.empty()) {
        return false;
    }
    if (external_index_name.has_value() != extracted_ordinal.has_value() ||
        external_index_raw_label.has_value() != external_index_name.has_value()) {
        return false;
    }
    if (external_index_folder && !external_index_name.has_value()) {
        return false;
    }
    return true;
}

bool ResourceNamingIdentityBuildResult::ok() const noexcept {
    return identity.has_value() && identity->valid() && !has_error(diagnostics);
}

bool ContainerNamingIdentitySnapshot::ok() const noexcept {
    return parent_resource.valid() && !has_error(diagnostics) &&
        std::all_of(
            children.begin(), children.end(),
            [](const ResourceNamingIdentity& identity) {
                return identity.valid();
            });
}

ResourceNamingIdentityBuildResult ResourceNamingIdentityBuilder::build(
    const ContainerChild& child) {
    ResourceNamingIdentityBuildResult result;
    if (!child.payload.resource.id.valid()) {
        add_error(
            result.diagnostics,
            child.payload.resource.id,
            "gdspaces.naming-identity.invalid-resource",
            "Naming identity requires a valid physical child ResourceId.");
        return result;
    }

    const ResourceNameEvidence* external_index = nullptr;
    const ResourceNameEvidence* embedded_alias = nullptr;
    for (const auto& evidence : child.payload.name_evidence) {
        if (!evidence.valid()) {
            add_error(
                result.diagnostics,
                child.payload.resource.id,
                "gdspaces.naming-identity.invalid-name-evidence",
                "A materialized child carries invalid naming evidence.");
            return result;
        }
        if (evidence.physical_slot_index() != child.entry.slot_index) {
            add_error(
                result.diagnostics,
                child.payload.resource.id,
                "gdspaces.naming-identity.slot-mismatch",
                "Naming evidence physical slot differs from the materialized child slot.");
            return result;
        }

        if (evidence.kind() == ResourceNameEvidenceKind::external_index) {
            if (external_index != nullptr) {
                add_error(
                    result.diagnostics,
                    child.payload.resource.id,
                    "gdspaces.naming-identity.multiple-external-index",
                    "More than one active external .index authority is attached to one physical child.");
                return result;
            }
            if (evidence.mapping_mode() !=
                ResourceNameMappingMode::populated_slot_sequence) {
                add_error(
                    result.diagnostics,
                    child.payload.resource.id,
                    "gdspaces.naming-identity.superseded-positional-mapping",
                    "External .index evidence must use the recovered extracted-ordinal/populated-payload sequence.");
                return result;
            }
            external_index = &evidence;
        } else if (evidence.kind() == ResourceNameEvidenceKind::embedded_alias) {
            if (embedded_alias != nullptr) {
                add_error(
                    result.diagnostics,
                    child.payload.resource.id,
                    "gdspaces.naming-identity.multiple-embedded-alias",
                    "More than one active embedded alias is attached to one physical child.");
                return result;
            }
            embedded_alias = &evidence;
        }
    }

    std::string semantic_format = child.payload.resource.format.empty()
        ? std::string{"unknown"}
        : child.payload.resource.format;
    auto canonical_extension = ResourceFormatIdentity::canonical_extension(
        semantic_format);

    if (!child.payload.semantic_evidence.empty()) {
        const auto bytes = std::span<const std::byte>{
            child.payload.bytes.data(), child.payload.bytes.size()};
        const auto digest = core::Sha256::compute(bytes).hex();
        const ResourceSemanticEvidence* active = nullptr;
        for (const auto& evidence : child.payload.semantic_evidence) {
            if (!evidence.valid() ||
                evidence.authority_resource() != child.payload.resource.id ||
                evidence.authority_sha256() != digest ||
                evidence.physical_slot_index() != child.entry.slot_index) {
                add_error(
                    result.diagnostics,
                    child.payload.resource.id,
                    "gdspaces.naming-identity.stale-semantic-evidence",
                    "Semantic naming evidence does not bind to the current physical child bytes and slot.");
                return result;
            }
            if (active != nullptr &&
                (active->semantic_format() != evidence.semantic_format() ||
                 active->canonical_extension() != evidence.canonical_extension())) {
                add_error(
                    result.diagnostics,
                    child.payload.resource.id,
                    "gdspaces.naming-identity.semantic-conflict",
                    "Conflicting active semantic naming authorities are attached to one child.");
                return result;
            }
            active = &evidence;
        }
        if (active != nullptr) {
            semantic_format = std::string{active->semantic_format()};
            canonical_extension = std::string{active->canonical_extension()};
        }
    }

    ResourceNamingIdentity identity{
        .resource_id = child.payload.resource.id,
        .physical_slot_index = child.entry.slot_index,
        .populated = child.entry.populated,
        .extracted_ordinal = std::nullopt,
        .external_index_raw_label = std::nullopt,
        .external_index_name = std::nullopt,
        .external_index_folder = false,
        .embedded_alias = std::nullopt,
        .semantic_format = std::move(semantic_format),
        .canonical_extension = std::move(canonical_extension),
        .canonical_display_name = child.payload.resource.display_name,
    };

    if (external_index != nullptr) {
        identity.extracted_ordinal = external_index->extracted_ordinal();
        identity.external_index_raw_label =
            std::string{external_index->raw_label()};
        identity.external_index_name =
            std::string{external_index->normalized_name()};
        identity.external_index_folder = is_exact_folder_marker(*external_index);
    }
    if (embedded_alias != nullptr) {
        identity.embedded_alias =
            std::string{embedded_alias->normalized_name()};
    }

    if (!identity.valid()) {
        add_error(
            result.diagnostics,
            child.payload.resource.id,
            "gdspaces.naming-identity.invalid-derived-identity",
            "The reconciled naming domains could not form a valid unified naming identity.");
        return result;
    }

    result.identity = std::move(identity);
    return result;
}

ContainerNamingIdentitySnapshot ResourceNamingIdentityBuilder::build(
    const ContainerExpansion& expansion) {
    ContainerNamingIdentitySnapshot result;
    result.parent_resource = expansion.parent.id;
    if (!expansion.usable()) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.naming-identity.invalid-expansion",
            "Container naming snapshot requires a usable physical expansion.");
        return result;
    }

    result.children.reserve(expansion.children.size());
    for (const auto& child : expansion.children) {
        auto built = build(child);
        result.diagnostics.insert(
            result.diagnostics.end(),
            built.diagnostics.begin(), built.diagnostics.end());
        if (!built.ok()) {
            return result;
        }
        result.children.push_back(std::move(*built.identity));
    }
    return result;
}

} // namespace dmc::rengine::gdspaces
