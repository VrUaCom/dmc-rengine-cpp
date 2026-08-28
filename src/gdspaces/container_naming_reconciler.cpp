#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"

#include "dmc_rengine/gdspaces/embedded_name_evidence.hpp"
#include "dmc_rengine/gdspaces/format_identity.hpp"
#include "dmc_rengine/gdspaces/index_manifest.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"
#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"
#include "dmc_rengine/gdspaces/resource_semantic_evidence.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] bool has_error(const std::vector<Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

void append_diagnostics(
    std::vector<Diagnostic>& destination,
    const std::vector<Diagnostic>& source) {
    destination.insert(destination.end(), source.begin(), source.end());
}

void add_error(
    ContainerNamingReconcileResult& result,
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

[[nodiscard]] ContainerChild* find_slot(
    ContainerExpansion& expansion,
    std::uint32_t slot_index) noexcept {
    const auto iterator = std::find_if(
        expansion.children.begin(), expansion.children.end(),
        [slot_index](const ContainerChild& child) {
            return child.entry.slot_index == slot_index;
        });
    return iterator == expansion.children.end() ? nullptr : &*iterator;
}

[[nodiscard]] std::string leaf_stem(std::string_view value) {
    const auto separator = value.find_last_of("/\\");
    auto leaf = separator == std::string_view::npos
        ? value
        : value.substr(separator + 1U);
    const auto dot = leaf.find_last_of('.');
    if (dot != std::string_view::npos && dot > 0U) {
        leaf = leaf.substr(0U, dot);
    }
    return leaf.empty() ? std::string{"container"} : std::string{leaf};
}

[[nodiscard]] std::string semantic_slot_zero_display(
    const ContainerExpansion& expansion,
    std::string_view canonical_extension) {
    const auto source = expansion.parent.display_name.empty()
        ? std::string_view{expansion.parent.id.logical_path}
        : std::string_view{expansion.parent.display_name};
    auto result = leaf_stem(source);
    result.append("_000");
    if (!canonical_extension.empty()) {
        result.push_back('.');
        result.append(canonical_extension);
    }
    return result;
}

[[nodiscard]] bool physical_state_unchanged(
    const ContainerExpansion& before,
    const ContainerExpansion& after) {
    if (before.parent.id != after.parent.id ||
        before.parser_format != after.parser_format ||
        before.children.size() != after.children.size()) {
        return false;
    }

    for (const auto& before_child : before.children) {
        const auto iterator = std::find_if(
            after.children.begin(), after.children.end(),
            [&](const ContainerChild& after_child) {
                return after_child.entry.slot_index == before_child.entry.slot_index;
            });
        if (iterator == after.children.end() ||
            iterator->payload.resource.id != before_child.payload.resource.id ||
            iterator->entry.offset != before_child.entry.offset ||
            iterator->entry.size != before_child.entry.size ||
            iterator->entry.populated != before_child.entry.populated ||
            iterator->payload.bytes != before_child.payload.bytes) {
            return false;
        }
    }
    return true;
}

} // namespace

bool ContainerNamingReconcileResult::ok() const noexcept {
    return reconciled && !has_error(diagnostics);
}

ContainerNamingReconcileResult ContainerNamingReconciler::reconcile(
    ContainerExpansion& expansion,
    const ResourcePayload* external_index) {
    ContainerNamingReconcileResult result;
    if (!expansion.usable()) {
        add_error(
            result,
            expansion.parent.id,
            "gdspaces.naming-reconcile.invalid-expansion",
            "Naming reconciliation requires a usable materialized container expansion.");
        return result;
    }

    const auto before = expansion;
    auto staged = expansion;

    const auto embedded_observation = EmbeddedNameEvidenceBuilder::observe(staged);
    append_diagnostics(result.diagnostics, embedded_observation.diagnostics);
    if (has_error(result.diagnostics)) {
        return result;
    }
    if (embedded_observation.observation.has_value()) {
        const auto embedded_apply = EmbeddedNameEvidenceBuilder::apply(
            staged, *embedded_observation.observation);
        append_diagnostics(result.diagnostics, embedded_apply.diagnostics);
        if (!embedded_apply.ok()) {
            return result;
        }

        auto* slot_zero = find_slot(staged, 0U);
        if (slot_zero == nullptr ||
            slot_zero->payload.resource.id !=
                embedded_observation.observation->authority_resource()) {
            add_error(
                result,
                embedded_observation.observation->authority_resource(),
                "gdspaces.naming-reconcile.semantic-authority-missing",
                "The proven embedded name-list authority no longer resolves to physical slot 0.");
            return result;
        }

        const auto canonical_extension =
            ResourceFormatIdentity::canonical_extension("name-list");
        ResourceSemanticEvidence semantic_evidence(
            ResourceSemanticEvidenceKind::embedded_name_list,
            embedded_observation.observation->authority_resource(),
            std::string{embedded_observation.observation->authority_sha256()},
            "name-list",
            canonical_extension,
            0U);
        if (!semantic_evidence.valid()) {
            add_error(
                result,
                slot_zero->payload.resource.id,
                "gdspaces.naming-reconcile.semantic-evidence-invalid",
                "The sealed embedded name-list observation could not form valid semantic evidence.");
            return result;
        }

        // Synthetic presentation is deliberately leaf-only. A path-like
        // upstream display hint (for example "DMC3/st001.pac") must never leak
        // namespace components into a loose filename. External .index evidence,
        // when supplied below, still owns the stronger canonical stem.
        if (slot_zero->payload.resource.synthetic_name) {
            slot_zero->payload.resource.display_name =
                semantic_slot_zero_display(staged, canonical_extension);
        }

        auto& evidence = slot_zero->payload.semantic_evidence;
        evidence.erase(
            std::remove_if(
                evidence.begin(), evidence.end(),
                [](const ResourceSemanticEvidence& existing) {
                    return existing.kind() ==
                        ResourceSemanticEvidenceKind::embedded_name_list;
                }),
            evidence.end());
        evidence.push_back(std::move(semantic_evidence));
        result.embedded_name_list_applied = true;
    }

    if (external_index != nullptr) {
        const auto manifest = IndexManifestParser::parse(*external_index);
        append_diagnostics(result.diagnostics, manifest.diagnostics);
        if (!manifest.ok()) {
            return result;
        }

        const auto binding = IndexSlotNameBinder::bind(
            staged, *manifest.manifest);
        append_diagnostics(result.diagnostics, binding.diagnostics);
        if (!binding.ok()) {
            return result;
        }

        const auto overlay = IndexNameOverlayBuilder::build(
            staged, *binding.binding);
        append_diagnostics(result.diagnostics, overlay.diagnostics);
        if (!overlay.ok()) {
            return result;
        }

        const auto applied = IndexNameOverlayBuilder::apply(
            staged, *overlay.overlay);
        append_diagnostics(result.diagnostics, applied.diagnostics);
        if (!applied.ok()) {
            return result;
        }
        result.external_index_applied = true;
    }

    if (!physical_state_unchanged(before, staged)) {
        add_error(
            result,
            expansion.parent.id,
            "gdspaces.naming-reconcile.physical-state-changed",
            "Naming reconciliation attempted to change physical identity, topology, or bytes.");
        return result;
    }

    expansion = std::move(staged);
    result.reconciled = true;
    return result;
}

} // namespace dmc::rengine::gdspaces
