#include "dmc_rengine/gdspaces/external_index_naming.hpp"

#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/authoring_extension_contract.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_set>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] std::string lower_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

void add_diagnostic(
    ExternalIndexNamingResult& result,
    DiagnosticSeverity severity,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::nullopt,
    });
}

} // namespace

bool ExternalIndexNamingResult::applied() const noexcept {
    return named_slots > 0U &&
        std::none_of(
               diagnostics.begin(), diagnostics.end(), [](const Diagnostic& value) {
                   return value.severity == DiagnosticSeverity::error;
               });
}

ExternalIndexNamingResult ExternalIndexNaming::apply(
    ContainerExpansion& expansion,
    const IndexSidecarManifest::Document& sidecar) {
    ExternalIndexNamingResult result;

    if (!expansion.usable()) {
        add_diagnostic(
            result,
            DiagnosticSeverity::error,
            "gdspaces.external-index.container-unusable",
            "An external index cannot name the slots of a container that did not expand.");
        return result;
    }
    if (sidecar.entries.empty()) {
        add_diagnostic(
            result,
            DiagnosticSeverity::error,
            "gdspaces.external-index.no-names",
            "The external index carries a directive but no slot names.");
        return result;
    }

    // The directive is the sidecar's own claim about which container it
    // describes. A mismatch is not fatal — a correct sidecar for a container
    // this build parses under another format name would trip it — but it is
    // exactly the signal that a sidecar from the wrong folder was picked, so
    // it travels with the result instead of being dropped.
    result.directive_matches_parser =
        lower_ascii(sidecar.container_directive) == lower_ascii(expansion.parser_format);
    if (!result.directive_matches_parser) {
        add_diagnostic(
            result,
            DiagnosticSeverity::warning,
            "gdspaces.external-index.directive-mismatch",
            "The external index says it describes a '" + sidecar.container_directive +
                "' container; this one parsed as '" + expansion.parser_format +
                "'. The names were applied, and this may mean the index belongs to a different folder.");
    }

    std::unordered_set<std::uint32_t> named;
    for (const auto& entry : sidecar.entries) {
        auto child = std::find_if(
            expansion.children.begin(), expansion.children.end(),
            [&entry](const ContainerChild& candidate) {
                return candidate.entry.slot_index == entry.slot_index;
            });
        if (child == expansion.children.end()) {
            ++result.lines_without_a_slot;
            continue;
        }
        if (!child->entry.populated) {
            // A tool naming a slot that carries nothing is naming the
            // numbering, not a resource. Taking the name would turn an intact
            // sparse container into one that looks fully populated.
            ++result.lines_without_a_slot;
            continue;
        }
        if (child->name_attribution.origin == SlotNameOrigin::container_manifest) {
            // A name the container itself stores outranks a name a tool chose.
            // Overwriting it would replace the stronger evidence with the
            // weaker one and leave no trace that it happened.
            continue;
        }

        child->name_attribution = SlotNameAttribution{
            .slot_index = entry.slot_index,
            .name = entry.name,
            .origin = SlotNameOrigin::external_index,
            .corroborated_by_payload = false,
        };

        // Classified from the bytes, with the path explicitly not treated as
        // naming the resource. Passing the sidecar's own name here would make
        // corroboration circular: the name would be confirming itself.
        const auto classification = ResourceClassifier::classify(
            child->payload.resource.id.logical_path,
            std::span<const std::byte>{
                child->payload.bytes.data(), child->payload.bytes.size()},
            false);
        child->name_attribution.corroborated_by_payload =
            profiles::dmc3::AuthoringExtensionContract::names_the_same_resource(
                SlotNameManifest::extension_of(entry.name), classification.format);

        child->payload.resource.display_name = entry.name;
        // The name is no longer one this parser invented, whatever else it is.
        child->payload.resource.synthetic_name = false;

        ++result.named_slots;
        if (child->name_attribution.corroborated_by_payload) {
            ++result.corroborated_slots;
        }
        named.insert(entry.slot_index);
    }

    for (const auto& child : expansion.children) {
        if (child.entry.populated && named.count(child.entry.slot_index) == 0U) {
            ++result.slots_without_a_line;
        }
    }

    if (result.named_slots == 0U) {
        add_diagnostic(
            result,
            DiagnosticSeverity::error,
            "gdspaces.external-index.named-nothing",
            "No line in the external index resolved to a populated slot of this container.");
    }
    return result;
}

ExternalIndexNamingResult ExternalIndexNaming::apply_bytes(
    ContainerExpansion& expansion,
    std::span<const std::byte> sidecar_bytes) {
    ExternalIndexNamingResult result;

    if (IndexSidecarManifest::is_own_rendered_sidecar(sidecar_bytes)) {
        // Reading our own rendering back as authority would be taking this
        // project's decision as evidence about the game — the exact failure it
        // keeps having to undo. Naming the refusal is what makes it visible.
        add_diagnostic(
            result,
            DiagnosticSeverity::error,
            "gdspaces.external-index.own-sidecar",
            "This is a sidecar this project rendered. Reading it back would make our own naming decision look like evidence.");
        return result;
    }

    const auto document = IndexSidecarManifest::parse(sidecar_bytes);
    if (!document.has_value()) {
        add_diagnostic(
            result,
            DiagnosticSeverity::error,
            "gdspaces.external-index.unreadable",
            "These bytes are not an extraction tool's .index: it must open with a container directive line and carry one plain name per line.");
        return result;
    }
    return apply(expansion, *document);
}

} // namespace dmc::rengine::gdspaces
