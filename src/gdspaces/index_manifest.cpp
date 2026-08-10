#include "dmc_rengine/gdspaces/index_manifest.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] std::string trim_copy(std::string_view value) {
    std::size_t begin = 0U;
    while (begin < value.size() &&
           std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
        ++begin;
    }

    std::size_t end = value.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(value[end - 1U])) != 0) {
        --end;
    }
    return std::string{value.substr(begin, end - begin)};
}

[[nodiscard]] std::string lower_copy(std::string_view value) {
    std::string result{value};
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return result;
}

void add_diagnostic(
    std::vector<Diagnostic>& diagnostics,
    DiagnosticSeverity severity,
    std::string code,
    std::string message,
    std::optional<ResourceId> resource = std::nullopt) {
    diagnostics.push_back(Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::move(resource),
    });
}

} // namespace

IndexManifest parse_index_manifest(std::string_view text) {
    IndexManifest manifest;
    std::istringstream input{std::string{text}};
    std::string line;
    std::size_t line_number = 0U;

    while (std::getline(input, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        const auto trimmed = trim_copy(line);
        if (trimmed.empty()) {
            continue;
        }

        if (lower_copy(trimmed) == "pnst") {
            manifest.pnst_directive = true;
            continue;
        }

        std::istringstream tokens{trimmed};
        std::string label;
        tokens >> label;
        if (label.empty()) {
            continue;
        }

        bool folder = false;
        std::string token;
        std::size_t trailing_tokens = 0U;
        while (tokens >> token) {
            ++trailing_tokens;
            if (lower_copy(token) == "folder") {
                folder = true;
            } else {
                add_diagnostic(
                    manifest.diagnostics,
                    DiagnosticSeverity::warning,
                    "gdspaces.index.unknown-token",
                    "Index manifest line " + std::to_string(line_number) +
                        " contains an unrecognized token; the raw line is preserved.");
            }
        }

        if (trailing_tokens > 1U) {
            add_diagnostic(
                manifest.diagnostics,
                DiagnosticSeverity::warning,
                "gdspaces.index.extra-tokens",
                "Index manifest line " + std::to_string(line_number) +
                    " contains multiple trailing tokens; only recognized markers are interpreted.");
        }

        manifest.entries.push_back(IndexManifestEntry{
            .ordinal = manifest.entries.size(),
            .label = std::move(label),
            .folder = folder,
            .raw_line = trimmed,
        });
    }

    return manifest;
}

ContainerIndexLinkReport link_index_manifest(
    const formats::ContainerDocument& container,
    const IndexManifest& manifest,
    std::optional<ResourceId> resource) {
    ContainerIndexLinkReport report;
    report.diagnostics = manifest.diagnostics;

    if (!container.valid()) {
        add_diagnostic(
            report.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.index.invalid-container",
            "Index metadata cannot be linked to an invalid container document.",
            std::move(resource));
        return report;
    }

    const auto has_empty_slot = std::any_of(
        container.entries.begin(), container.entries.end(),
        [](const formats::ContainerEntry& entry) {
            return !entry.populated;
        });

    if (has_empty_slot) {
        add_diagnostic(
            report.diagnostics,
            DiagnosticSeverity::warning,
            "gdspaces.index.sparse-slot-mapping-unresolved",
            "The binary container has empty slots, so sequential index-manifest entries are not mapped onto physical slot identities without stronger positional evidence.",
            std::move(resource));
        return report;
    }

    if (manifest.entries.size() != container.entries.size()) {
        add_diagnostic(
            report.diagnostics,
            DiagnosticSeverity::warning,
            "gdspaces.index.entry-count-mismatch",
            "The index-manifest entry count does not match the dense binary slot count; positional metadata linkage is not applied.",
            std::move(resource));
        return report;
    }

    std::vector<const formats::ContainerEntry*> ordered;
    ordered.reserve(container.entries.size());
    for (const auto& entry : container.entries) {
        ordered.push_back(&entry);
    }
    std::sort(
        ordered.begin(), ordered.end(),
        [](const auto* left, const auto* right) {
            return left->slot_index < right->slot_index;
        });

    for (std::size_t index = 0U; index < ordered.size(); ++index) {
        if (ordered[index]->slot_index != index) {
            add_diagnostic(
                report.diagnostics,
                DiagnosticSeverity::warning,
                "gdspaces.index.noncontiguous-dense-slots",
                "The dense container entries are not an explicit contiguous 0..N-1 slot sequence; positional metadata linkage is not applied.",
                std::move(resource));
            return report;
        }
    }

    report.links.reserve(ordered.size());
    for (std::size_t index = 0U; index < ordered.size(); ++index) {
        const auto& manifest_entry = manifest.entries[index];
        report.links.push_back(ContainerIndexLink{
            .slot_index = ordered[index]->slot_index,
            .manifest_ordinal = manifest_entry.ordinal,
            .label = manifest_entry.label,
            .folder = manifest_entry.folder,
        });
    }
    report.safe_positional_mapping = true;
    return report;
}

} // namespace dmc::rengine::gdspaces
