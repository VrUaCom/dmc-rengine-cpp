#include "dmc_rengine/profiles/dmc3/naming_pipeline.hpp"

#include "dmc_rengine/profiles/dmc3/effect_stored_name_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool has_error(
    const std::vector<gdspaces::Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
        });
}

void append_diagnostics(
    std::vector<gdspaces::Diagnostic>& destination,
    const std::vector<gdspaces::Diagnostic>& source) {
    destination.insert(destination.end(), source.begin(), source.end());
}

[[nodiscard]] std::string lower_ascii(std::string_view value) {
    std::string result{value};
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return result;
}

[[nodiscard]] std::string physical_leaf_stem(std::string_view logical_path) {
    // Nested ResourceIds deliberately keep their source-native physical path at
    // the front and append synthetic `::FORMAT/slot-NNNN` identity components.
    // A derived display name must be based on the physical container identity,
    // never on a synthetic child display alias.
    const auto nested = logical_path.find("::");
    if (nested != std::string_view::npos) {
        logical_path = logical_path.substr(0U, nested);
    }

    const auto separator = logical_path.find_last_of("/\\");
    auto leaf = separator == std::string_view::npos
        ? logical_path
        : logical_path.substr(separator + 1U);
    const auto dot = leaf.find_last_of('.');
    if (dot != std::string_view::npos && dot > 0U) {
        leaf = leaf.substr(0U, dot);
    }
    return leaf.empty() ? std::string{"container"} : std::string{leaf};
}

[[nodiscard]] std::string padded_number(
    std::string_view value,
    std::size_t minimum_width) {
    if (value.size() >= minimum_width) {
        return std::string{value};
    }
    return std::string(minimum_width - value.size(), '0') + std::string{value};
}

[[nodiscard]] std::string derived_container_stem(
    const gdspaces::ResourceId& parent) {
    auto stem = physical_leaf_stem(parent.logical_path);

    // For nested physical containers, retain their exact topology in the
    // derived presentation stem. The outer NBZ member index is intentionally
    // omitted: it is storage position, not part of the resource's useful name.
    std::size_t start = 0U;
    while (start <= parent.container_chain.size()) {
        const auto end = parent.container_chain.find('/', start);
        const auto component = end == std::string::npos
            ? std::string_view{parent.container_chain}.substr(start)
            : std::string_view{parent.container_chain}.substr(start, end - start);

        const auto open = component.find('[');
        const auto close = component.find(']', open == std::string_view::npos ? 0U : open + 1U);
        if (open != std::string_view::npos && close != std::string_view::npos &&
            open > 0U && close > open + 1U) {
            const auto kind = lower_ascii(component.substr(0U, open));
            if (kind != "nbz") {
                const auto slot = component.substr(open + 1U, close - open - 1U);
                stem.push_back('_');
                stem.append(kind);
                stem.append(padded_number(slot, 4U));
            }
        }

        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    return stem;
}

[[nodiscard]] std::string derived_display_name(
    std::string_view container_stem,
    std::size_t extracted_ordinal,
    std::string_view canonical_extension,
    std::string_view semantic_format) {
    std::ostringstream output;
    output << container_stem << '_' << std::setfill('0') << std::setw(3)
           << extracted_ordinal << '.';

    // Unknown bytes still receive a useful deterministic presentation name,
    // but `.bin` is explicitly only a product fallback. It is never written to
    // external-index evidence or promoted to historical export authority.
    if (canonical_extension.empty() || canonical_extension == "unknown" ||
        semantic_format == "unknown") {
        output << "bin";
    } else {
        output << canonical_extension;
    }
    return output.str();
}

[[nodiscard]] bool apply_derived_display_names(
    const gdspaces::ContainerExpansion& expansion,
    gdspaces::ContainerNamingIdentitySnapshot& snapshot) {
    const auto container_stem = derived_container_stem(expansion.parent.id);
    bool applied = false;

    for (auto& identity : snapshot.children) {
        if (!identity.populated || !identity.extracted_ordinal.has_value() ||
            identity.external_index_name.has_value() ||
            identity.enclosing_container_stored_name.has_value()) {
            continue;
        }

        const auto child = std::find_if(
            expansion.children.begin(), expansion.children.end(),
            [&](const gdspaces::ContainerChild& candidate) {
                return candidate.entry.slot_index == identity.physical_slot_index &&
                    candidate.payload.resource.id == identity.resource_id;
            });
        if (child == expansion.children.end() ||
            !child->payload.resource.synthetic_name) {
            continue;
        }

        identity.canonical_display_name = derived_display_name(
            container_stem,
            *identity.extracted_ordinal,
            identity.canonical_extension,
            identity.semantic_format);
        applied = true;
    }
    return applied;
}

} // namespace

bool Dmc3NamingPipelineResult::ok() const noexcept {
    return reconciled && snapshot.has_value() && snapshot->ok() &&
        !has_error(diagnostics);
}

Dmc3NamingPipelineResult Dmc3NamingPipeline::apply(
    gdspaces::ContainerExpansion& expansion,
    const gdspaces::ResourcePayload* external_index,
    const gdspaces::ISource* companion_source,
    const gdspaces::ResourcePayload* enclosing_container) {
    Dmc3NamingPipelineResult result;
    const auto before = expansion;

    if (enclosing_container != nullptr) {
        const auto enclosing = EffectStoredNameEvidenceBuilder::apply(
            *enclosing_container, expansion);
        append_diagnostics(result.diagnostics, enclosing.diagnostics);
        if (!enclosing.ok()) {
            expansion = before;
            return result;
        }
        result.enclosing_stored_names_applied = true;
    }

    const gdspaces::ResourcePayload* selected_index = external_index;
    std::optional<gdspaces::ResourcePayload> discovered_index;

    if (external_index != nullptr) {
        result.explicit_external_index_used = true;
    } else if (companion_source != nullptr) {
        const auto discovery = CompanionIndexLocator::discover(
            *companion_source, expansion.parent.id);
        append_diagnostics(result.diagnostics, discovery.diagnostics);
        if (has_error(discovery.diagnostics)) {
            expansion = before;
            return result;
        }

        if (discovery.payload.has_value()) {
            discovered_index = *discovery.payload;
            selected_index = &*discovered_index;
            result.companion_index_discovered = true;
            result.companion_kind = discovery.matched_kind;
        }
    }

    // Profiled reconciliation is private to this class by friendship. Product
    // callers can invoke public reconcile, but cannot inject their own profile
    // resolver and have arbitrary semantic strings sealed as trusted evidence.
    const auto reconcile =
        gdspaces::ContainerNamingReconciler::reconcile_profiled(
            expansion,
            selected_index,
            resolve_index_display_semantic);
    append_diagnostics(result.diagnostics, reconcile.diagnostics);
    if (!reconcile.ok()) {
        expansion = before;
        return result;
    }

    // DMC3 structural/runtime semantics are valid even when no extraction
    // sidecar exists. Run them independently so texture-bundle/wrapped-DDS or
    // recovered runtime-tag identity is not gated on historical `.index`.
    const auto profile_semantics =
        gdspaces::ContainerNamingReconciler::apply_profile_semantics(
            expansion, resolve_materialized_display_semantic);
    append_diagnostics(result.diagnostics, profile_semantics.diagnostics);
    if (!profile_semantics.ok()) {
        expansion = before;
        return result;
    }
    result.profile_semantics_applied = profile_semantics.profile_semantics_applied;

    auto snapshot = gdspaces::ResourceNamingIdentityBuilder::build(expansion);
    append_diagnostics(result.diagnostics, snapshot.diagnostics);
    if (!snapshot.ok()) {
        expansion = before;
        return result;
    }

    // This is presentation only. Exact external extraction names and direct
    // enclosing-container stored names always win. If neither exists, a
    // synthetic child gets a deterministic topology/ordinal/semantic name so
    // tools do not expose meaningless `slot_XXXX` as the primary row label.
    result.derived_display_names_applied =
        apply_derived_display_names(expansion, snapshot);
    if (!snapshot.ok()) {
        expansion = before;
        return result;
    }

    result.snapshot = std::move(snapshot);
    result.reconciled = true;
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
