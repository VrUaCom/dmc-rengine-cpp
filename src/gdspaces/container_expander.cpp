#include "dmc_rengine/gdspaces/container_expander.hpp"

#include "dmc_rengine/gdspaces/classifier.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <span>
#include <sstream>
#include <string>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] DiagnosticSeverity convert_severity(
    formats::ParseSeverity severity) noexcept {
    switch (severity) {
    case formats::ParseSeverity::info: return DiagnosticSeverity::info;
    case formats::ParseSeverity::warning: return DiagnosticSeverity::warning;
    case formats::ParseSeverity::error: return DiagnosticSeverity::error;
    }
    return DiagnosticSeverity::error;
}

[[nodiscard]] std::string safe_component(
    std::string_view name,
    std::uint32_t slot) {
    std::string result;
    result.reserve(name.size());
    for (const unsigned char character : name) {
        if (std::isalnum(character) != 0 || character == '.' ||
            character == '-' || character == '_') {
            result.push_back(static_cast<char>(character));
        } else {
            result.push_back('_');
        }
    }

    if (!result.empty() && result != "." && result != "..") {
        return result;
    }

    std::ostringstream output;
    output << "slot_" << std::setfill('0') << std::setw(4) << slot << ".bin";
    return output.str();
}

[[nodiscard]] std::string slot_component(std::uint32_t slot) {
    std::ostringstream output;
    output << "slot-" << std::setfill('0') << std::setw(4) << slot;
    return output.str();
}

[[nodiscard]] std::string child_chain(
    const ResourceId& parent,
    std::string_view format,
    std::uint32_t slot) {
    std::ostringstream output;
    if (!parent.container_chain.empty()) {
        output << parent.container_chain << '/';
    }
    output << format << '[' << slot << ']';
    return output.str();
}

} // namespace

bool ContainerExpansion::usable() const noexcept {
    if (!parent.valid() || parser_format.empty()) {
        return false;
    }

    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

ContainerExpansion ContainerExpander::expand(
    const ResourcePayload& parent,
    const formats::ContainerParseResult& parsed) {
    ContainerExpansion expansion{
        .parent = parent.resource,
        .parser_format = parsed.document.format,
        .children = {},
        .diagnostics = {},
    };

    for (const auto& diagnostic : parsed.diagnostics) {
        expansion.diagnostics.push_back(Diagnostic{
            .severity = convert_severity(diagnostic.severity),
            .code = diagnostic.code,
            .message = diagnostic.message,
            .resource = parent.resource.id,
        });
    }

    if (!parent.readable()) {
        expansion.diagnostics.push_back(Diagnostic{
            .severity = DiagnosticSeverity::error,
            .code = "gdspaces.container.parent_unreadable",
            .message = "The parent resource payload is not readable.",
            .resource = parent.resource.id,
        });
        return expansion;
    }

    if (!parsed.recognized || !parsed.document.valid()) {
        expansion.diagnostics.push_back(Diagnostic{
            .severity = DiagnosticSeverity::error,
            .code = "gdspaces.container.invalid_document",
            .message = "The parser did not produce a valid recognized container document.",
            .resource = parent.resource.id,
        });
        return expansion;
    }

    if (parsed.document.container_size != parent.bytes.size()) {
        expansion.diagnostics.push_back(Diagnostic{
            .severity = DiagnosticSeverity::warning,
            .code = "gdspaces.container.size_mismatch",
            .message = "The parsed container size differs from the parent payload size.",
            .resource = parent.resource.id,
        });
    }

    expansion.children.reserve(parsed.document.entries.size());
    for (const auto& entry : parsed.document.entries) {
        if (parent.resource.id.offset >
            std::numeric_limits<std::uint64_t>::max() - entry.offset) {
            expansion.diagnostics.push_back(Diagnostic{
                .severity = DiagnosticSeverity::error,
                .code = "gdspaces.container.child_offset_overflow",
                .message = "A child physical offset overflows the resource identity range.",
                .resource = parent.resource.id,
            });
            continue;
        }

        const auto name = safe_component(entry.logical_name, entry.slot_index);
        const auto logical_path = parent.resource.id.logical_path + "::" +
            parsed.document.format + "/" + slot_component(entry.slot_index) +
            "/" + name;

        std::vector<std::byte> child_bytes;
        ResourceClassification classification;
        if (entry.populated) {
            const auto offset = static_cast<std::size_t>(entry.offset);
            const auto size = static_cast<std::size_t>(entry.size);
            child_bytes.assign(
                parent.bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                parent.bytes.begin() + static_cast<std::ptrdiff_t>(offset + size));
            classification = ResourceClassifier::classify(
                entry.logical_name,
                std::span<const std::byte>{child_bytes});
        } else {
            classification = ResourceClassification{
                .format = "empty-slot",
                .profile = GameProfile::unknown,
                .container = false,
                .magic_confirmed = false,
            };
        }

        ResourceRef child_ref{
            .id = ResourceId{
                .source_id = parent.resource.id.source_id,
                .logical_path = logical_path,
                .container_chain = child_chain(
                    parent.resource.id,
                    parsed.document.format,
                    entry.slot_index),
                .offset = parent.resource.id.offset + entry.offset,
                .size = entry.size,
            },
            .display_name = entry.logical_name,
            .format = classification.format,
            .profile = parent.resource.profile,
            .synthetic_name = entry.synthetic_name,
            .container = classification.container,
        };

        expansion.children.push_back(ContainerChild{
            .entry = entry,
            .payload = ResourcePayload{
                .resource = std::move(child_ref),
                .bytes = std::move(child_bytes),
                .diagnostics = {},
            },
        });
    }

    return expansion;
}

void ContainerExpander::connect_graph(
    const ContainerExpansion& expansion,
    ResourceGraph& graph) {
    if (graph.find(expansion.parent.id) == nullptr) {
        static_cast<void>(graph.add(expansion.parent));
    }

    for (const auto& child : expansion.children) {
        if (graph.find(child.payload.resource.id) == nullptr) {
            static_cast<void>(graph.add(child.payload.resource));
        }
        static_cast<void>(graph.connect(
            expansion.parent.id,
            child.payload.resource.id,
            ResourceRelation::contains));
    }
}

} // namespace dmc::rengine::gdspaces
