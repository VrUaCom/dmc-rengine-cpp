#include "dmc_rengine/gdspaces/container_expander.hpp"

#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <optional>
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

[[nodiscard]] std::optional<ByteProvenance> child_byte_provenance(
    const ResourcePayload& parent,
    const formats::ContainerEntry& entry) {
    if (!entry.populated) {
        return std::nullopt;
    }

    if (parent.byte_provenance.has_value()) {
        // A present-but-invalid lineage is an evidence failure. Do not launder
        // it into a fresh materialized-parent provenance record.
        if (!parent.byte_provenance->valid()) {
            return std::nullopt;
        }

        if (parent.byte_provenance->direct_byte_mapping()) {
            if (parent.byte_provenance->offset >
                std::numeric_limits<std::uint64_t>::max() - entry.offset) {
                return std::nullopt;
            }
            return ByteProvenance{
                .kind = ByteOriginKind::direct_source_span,
                .authority_id = parent.byte_provenance->authority_id,
                .offset = parent.byte_provenance->offset + entry.offset,
                .stored_size = entry.size,
                .materialized_size = entry.size,
                .transform = ByteTransform::none,
                .crc32 = std::nullopt,
            };
        }
    }

    // No source-direct mapping is available. The child is addressed only
    // within the already materialized parent byte domain. This is valid when
    // the parent lineage is absent or valid-but-transformed/materialized.
    return ByteProvenance{
        .kind = ByteOriginKind::materialized_parent_span,
        .authority_id = parent.resource.id.canonical(),
        .offset = entry.offset,
        .stored_size = entry.size,
        .materialized_size = entry.size,
        .transform = ByteTransform::none,
        .crc32 = std::nullopt,
    };
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

    // A relative-slot container may carry its own name list in slot 0. Read it
    // once, before the children, so every slot can be attributed against it.
    std::vector<std::string> manifest;
    if (!parsed.document.entries.empty()) {
        const auto& first = parsed.document.entries.front();
        if (first.slot_index == SlotNameManifest::k_manifest_slot &&
            first.populated) {
            const auto parent_size =
                static_cast<std::uint64_t>(parent.bytes.size());
            if (first.offset <= parent_size &&
                first.size <= parent_size - first.offset) {
                manifest = SlotNameManifest::parse(std::span<const std::byte>{
                    parent.bytes.data() + static_cast<std::size_t>(first.offset),
                    static_cast<std::size_t>(first.size)});
            }
        }
    }

    expansion.children.reserve(parsed.document.entries.size());
    for (const auto& entry : parsed.document.entries) {
        if (parent.resource.id.offset >
            std::numeric_limits<std::uint64_t>::max() - entry.offset) {
            expansion.diagnostics.push_back(Diagnostic{
                .severity = DiagnosticSeverity::error,
                .code = "gdspaces.container.child_offset_overflow",
                .message = "A child materialized offset overflows the resource identity range.",
                .resource = parent.resource.id,
            });
            continue;
        }

        const auto name = safe_component(entry.logical_name, entry.slot_index);
        const auto logical_path = parent.resource.id.logical_path + "::" +
            parsed.document.format + "/" + slot_component(entry.slot_index) +
            "/" + name;

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
            .format = "unknown",
            .profile = parent.resource.profile,
            .synthetic_name = entry.synthetic_name,
            .container = false,
        };

        SlotNameAttribution attribution{
            .slot_index = entry.slot_index,
            .name = name,
            .origin = SlotNameOrigin::parser_placeholder,
            .corroborated_by_payload = false,
        };

        std::vector<std::byte> child_bytes;
        std::vector<Diagnostic> child_diagnostics;
        auto provenance = child_byte_provenance(parent, entry);
        if (entry.populated) {
            if (!provenance.has_value() || !provenance->valid()) {
                const Diagnostic diagnostic{
                    .severity = DiagnosticSeverity::error,
                    .code = "gdspaces.container.child_provenance_invalid",
                    .message = "A populated child could not be assigned safe byte provenance.",
                    .resource = child_ref.id,
                };
                expansion.diagnostics.push_back(diagnostic);
                child_diagnostics.push_back(diagnostic);
            }

            const auto parent_size = static_cast<std::uint64_t>(parent.bytes.size());
            const auto range_valid = entry.offset <= parent_size &&
                entry.size <= parent_size - entry.offset;
            if (!range_valid) {
                const Diagnostic diagnostic{
                    .severity = DiagnosticSeverity::error,
                    .code = "gdspaces.container.child_range_changed",
                    .message = "A parsed child range is outside the current parent payload.",
                    .resource = child_ref.id,
                };
                expansion.diagnostics.push_back(diagnostic);
                child_diagnostics.push_back(diagnostic);
            } else {
                const auto offset = static_cast<std::size_t>(entry.offset);
                const auto size = static_cast<std::size_t>(entry.size);
                child_bytes.assign(
                    parent.bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                    parent.bytes.begin() + static_cast<std::ptrdiff_t>(offset + size));
                const auto classification = ResourceClassifier::classify(
                    entry.logical_name,
                    std::span<const std::byte>{child_bytes},
                    !entry.synthetic_name);
                child_ref.format = classification.format;
                child_ref.container = classification.container;

                // A relative-slot container stores no names, so the parser
                // supplies one. Where the payload itself decides its type —
                // a magic signature, a four-byte record tag, or a body that is
                // wholly text — that decision names the type better than a
                // generic suffix does, and it comes from the bytes rather than
                // from a guess, which is the only reason it is allowed to
                // appear here at all.
                // The identity is untouched: only what the operator reads
                // changes.
                if (entry.synthetic_name && classification.byte_derived) {
                    // Keep the parser's own spelling of the slot and replace
                    // only the suffix, so one slot never appears under two
                    // names.
                    auto named = entry.logical_name;
                    const auto dot = named.rfind('.');
                    if (dot != std::string::npos) {
                        named.erase(dot);
                    }
                    child_ref.display_name = named + "." + classification.format;
                    attribution.name = child_ref.display_name;
                    attribution.origin = SlotNameOrigin::byte_derived_suffix;
                }

                // A manifest line, where one exists for this slot. It replaces
                // the attributed name and nothing else: the identity and the
                // display name stay as they were, so an operator sees both what
                // the container calls this slot and what the tool does.
                if (entry.slot_index > SlotNameManifest::k_manifest_slot) {
                    const auto line = static_cast<std::size_t>(
                        entry.slot_index - SlotNameManifest::k_manifest_slot - 1U);
                    if (line < manifest.size()) {
                        attribution.name = manifest[line];
                        attribution.origin = SlotNameOrigin::container_manifest;
                        // The one check available: does the type the payload
                        // declares for itself agree with the extension the
                        // manifest line carries? Agreement does not prove the
                        // mapping, and disagreement is worth seeing.
                        attribution.corroborated_by_payload =
                            SlotNameManifest::extension_of(attribution.name) ==
                            classification.format;
                    }
                }
            }
        } else {
            child_ref.format = "empty-slot";
            provenance.reset();
        }

        expansion.children.push_back(ContainerChild{
            .entry = entry,
            .payload = ResourcePayload{
                .resource = std::move(child_ref),
                .bytes = std::move(child_bytes),
                .diagnostics = std::move(child_diagnostics),
                .byte_provenance = std::move(provenance),
            },
            .name_attribution = std::move(attribution),
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
