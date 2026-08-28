#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_set>
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

void add_error(
    IndexSlotBindingBuildResult& result,
    const ResourceId& resource,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

} // namespace

IndexSlotNameAuthority::IndexSlotNameAuthority(
    std::uint32_t slot_index,
    std::size_t extracted_ordinal,
    ResourceId child_resource,
    std::string raw_index_label,
    std::string index_name,
    std::string stem,
    std::optional<std::string> source_extension,
    bool is_folder,
    std::size_t manifest_line)
    : slot_index_(slot_index),
      extracted_ordinal_(extracted_ordinal),
      child_resource_(std::move(child_resource)),
      raw_index_label_(std::move(raw_index_label)),
      index_name_(std::move(index_name)),
      stem_(std::move(stem)),
      source_extension_(std::move(source_extension)),
      is_folder_(is_folder),
      manifest_line_(manifest_line) {}

std::uint32_t IndexSlotNameAuthority::slot_index() const noexcept {
    return slot_index_;
}

std::size_t IndexSlotNameAuthority::extracted_ordinal() const noexcept {
    return extracted_ordinal_;
}

const ResourceId& IndexSlotNameAuthority::child_resource() const noexcept {
    return child_resource_;
}

std::string_view IndexSlotNameAuthority::raw_index_label() const noexcept {
    return raw_index_label_;
}

std::string_view IndexSlotNameAuthority::index_name() const noexcept {
    return index_name_;
}

std::string_view IndexSlotNameAuthority::stem() const noexcept {
    return stem_;
}

const std::optional<std::string>& IndexSlotNameAuthority::source_extension() const noexcept {
    return source_extension_;
}

bool IndexSlotNameAuthority::is_folder() const noexcept {
    return is_folder_;
}

std::size_t IndexSlotNameAuthority::manifest_line() const noexcept {
    return manifest_line_;
}

IndexSlotBindingResult::IndexSlotBindingResult(
    ResourceId parent_resource,
    ResourceId manifest_resource,
    std::string manifest_sha256,
    IndexSlotMappingMode mapping_mode,
    std::vector<IndexSlotNameAuthority> authorities,
    std::vector<IndexSlotBindingDiagnostic> diagnostics)
    : parent_resource_(std::move(parent_resource)),
      manifest_resource_(std::move(manifest_resource)),
      manifest_sha256_(std::move(manifest_sha256)),
      mapping_mode_(mapping_mode),
      authorities_(std::move(authorities)),
      diagnostics_(std::move(diagnostics)) {}

const ResourceId& IndexSlotBindingResult::parent_resource() const noexcept {
    return parent_resource_;
}

const ResourceId& IndexSlotBindingResult::manifest_resource() const noexcept {
    return manifest_resource_;
}

std::string_view IndexSlotBindingResult::manifest_sha256() const noexcept {
    return manifest_sha256_;
}

IndexSlotMappingMode IndexSlotBindingResult::mapping_mode() const noexcept {
    return mapping_mode_;
}

const std::vector<IndexSlotNameAuthority>& IndexSlotBindingResult::authorities() const noexcept {
    return authorities_;
}

const std::vector<IndexSlotBindingDiagnostic>& IndexSlotBindingResult::diagnostics() const noexcept {
    return diagnostics_;
}

bool IndexSlotBindingResult::valid() const noexcept {
    if (!parent_resource_.valid() || !manifest_resource_.valid() ||
        !valid_digest(manifest_sha256_) || authorities_.empty() ||
        !diagnostics_.empty()) {
        return false;
    }

    std::unordered_set<std::uint32_t> seen_slots;
    seen_slots.reserve(authorities_.size());
    for (std::size_t index = 0U; index < authorities_.size(); ++index) {
        const auto& authority = authorities_[index];
        if (!authority.child_resource().valid() ||
            authority.index_name().empty() || authority.stem().empty() ||
            authority.manifest_line() == 0U ||
            authority.extracted_ordinal() != index ||
            !seen_slots.insert(authority.slot_index()).second) {
            return false;
        }
    }
    return true;
}

bool IndexSlotBindingBuildResult::ok() const noexcept {
    if (!binding.has_value() || !binding->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

IndexSlotBindingBuildResult IndexSlotNameBinder::bind(
    const ContainerExpansion& expansion,
    const IndexManifest& manifest) {
    IndexSlotBindingBuildResult result;
    if (!expansion.usable()) {
        add_error(
            result,
            expansion.parent.id,
            "gdspaces.index-binding.invalid-expansion",
            "Index name authority requires a usable physical container expansion.");
        return result;
    }
    if (!manifest.valid()) {
        add_error(
            result,
            manifest.source(),
            "gdspaces.index-binding.invalid-manifest",
            "Index name authority requires a sealed valid manifest observation.");
        return result;
    }

    std::unordered_set<std::uint32_t> physical_slots;
    physical_slots.reserve(expansion.children.size());
    for (const auto& child : expansion.children) {
        if (!physical_slots.insert(child.entry.slot_index).second) {
            add_error(
                result,
                child.payload.resource.id,
                "gdspaces.index-binding.duplicate-physical-slot",
                "Container expansion contains duplicate physical slot indices; extracted-ordinal authority cannot be bound safely.");
            return result;
        }
    }

    std::vector<IndexSlotNameAuthority> authorities;
    std::vector<IndexSlotBindingDiagnostic> binding_diagnostics;

    // Recovered DMC3 HDC extraction invariant: .index entry N names the
    // N-th populated payload. Empty physical slots never consume an ordinal.
    // Dense containers only make physical_slot_index == extracted_ordinal by
    // coincidence; physical position is never the naming authority.
    constexpr auto mode = IndexSlotMappingMode::populated_slot_sequence;
    authorities.reserve(manifest.entries().size());

    std::size_t extracted_ordinal = 0U;
    for (const auto& child : expansion.children) {
        if (!child.entry.populated) {
            continue;
        }
        if (extracted_ordinal < manifest.entries().size()) {
            const auto& entry = manifest.entries()[extracted_ordinal];
            authorities.push_back(IndexSlotNameAuthority(
                child.entry.slot_index,
                extracted_ordinal,
                child.payload.resource.id,
                entry.raw,
                entry.name,
                entry.stem,
                entry.extension,
                entry.is_folder,
                entry.line_number));
            ++extracted_ordinal;
        } else {
            binding_diagnostics.push_back(IndexSlotBindingDiagnostic{
                .issue = IndexSlotBindingIssue::slot_without_manifest_entry,
                .slot_index = child.entry.slot_index,
                .manifest_line = std::nullopt,
            });
        }
    }

    for (; extracted_ordinal < manifest.entries().size(); ++extracted_ordinal) {
        binding_diagnostics.push_back(IndexSlotBindingDiagnostic{
            .issue = IndexSlotBindingIssue::manifest_entry_without_slot,
            .slot_index = std::nullopt,
            .manifest_line = manifest.entries()[extracted_ordinal].line_number,
        });
    }

    result.binding = IndexSlotBindingResult(
        expansion.parent.id,
        manifest.source(),
        std::string{manifest.observed_sha256()},
        mode,
        std::move(authorities),
        std::move(binding_diagnostics));
    return result;
}

} // namespace dmc::rengine::gdspaces
