#include "dmc_rengine/profiles/dmc3/naming_pipeline.hpp"

#include "dmc_rengine/profiles/dmc3/effect_stored_name_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"

#include <algorithm>
#include <optional>
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

    const auto reconcile = gdspaces::ContainerNamingReconciler::reconcile(
        expansion,
        selected_index,
        resolve_index_display_semantic);
    append_diagnostics(result.diagnostics, reconcile.diagnostics);
    if (!reconcile.ok()) {
        expansion = before;
        return result;
    }

    auto snapshot = gdspaces::ResourceNamingIdentityBuilder::build(expansion);
    append_diagnostics(result.diagnostics, snapshot.diagnostics);
    result.snapshot = std::move(snapshot);
    if (!result.snapshot->ok()) {
        expansion = before;
        return result;
    }

    result.reconciled = true;
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
