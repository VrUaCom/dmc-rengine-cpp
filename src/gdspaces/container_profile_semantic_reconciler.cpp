#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/resource_semantic_evidence.hpp"

#include <algorithm>
#include <span>
#include <string>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

void add_profile_error(
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

[[nodiscard]] bool has_stronger_semantic_evidence(
    const ResourcePayload& payload) noexcept {
    return std::any_of(
        payload.semantic_evidence.begin(), payload.semantic_evidence.end(),
        [](const ResourceSemanticEvidence& evidence) {
            return evidence.kind() == ResourceSemanticEvidenceKind::embedded_name_list ||
                evidence.kind() == ResourceSemanticEvidenceKind::magic_confirmed_format;
        });
}

} // namespace

ContainerNamingReconcileResult ContainerNamingReconciler::apply_profile_semantics(
    ContainerExpansion& expansion,
    ResourceProfileSemanticResolver resolver) {
    ContainerNamingReconcileResult result;
    if (!expansion.usable()) {
        add_profile_error(
            result,
            expansion.parent.id,
            "gdspaces.naming-reconcile.profile.invalid-expansion",
            "Profile semantic reconciliation requires a usable materialized container expansion.");
        return result;
    }

    if (resolver == nullptr) {
        result.reconciled = true;
        return result;
    }

    const auto before = expansion;
    auto staged = expansion;

    for (auto& child : staged.children) {
        if (!child.entry.populated || child.payload.bytes.empty()) {
            continue;
        }

        const auto semantic = resolver(child.payload);
        if (!semantic.has_value()) {
            continue;
        }
        if (semantic->semantic_format.empty() || semantic->canonical_extension.empty()) {
            add_profile_error(
                result,
                child.payload.resource.id,
                "gdspaces.naming-reconcile.profile.invalid-semantic",
                "A profile structural observation must provide both semantic format and canonical extension.");
            return result;
        }

        // A profile recognizer may add information where generic magic has no
        // answer. It never overrides a stronger sealed observation of the same
        // bytes, such as a magic-confirmed format or the embedded name-list.
        if (has_stronger_semantic_evidence(child.payload)) {
            continue;
        }

        const auto bytes = std::span<const std::byte>{
            child.payload.bytes.data(), child.payload.bytes.size()};
        ResourceSemanticEvidence evidence(
            ResourceSemanticEvidenceKind::profile_structural_format,
            child.payload.resource.id,
            core::Sha256::compute(bytes).hex(),
            semantic->semantic_format,
            semantic->canonical_extension,
            child.entry.slot_index);
        if (!evidence.valid()) {
            add_profile_error(
                result,
                child.payload.resource.id,
                "gdspaces.naming-reconcile.profile.evidence-invalid",
                "The profile structural observation could not form valid sealed semantic evidence.");
            return result;
        }

        child.payload.resource.format = semantic->semantic_format;
        auto& semantic_evidence = child.payload.semantic_evidence;
        semantic_evidence.erase(
            std::remove_if(
                semantic_evidence.begin(), semantic_evidence.end(),
                [](const ResourceSemanticEvidence& existing) {
                    return existing.kind() ==
                        ResourceSemanticEvidenceKind::profile_structural_format;
                }),
            semantic_evidence.end());
        semantic_evidence.push_back(std::move(evidence));
        result.profile_semantics_applied = true;
    }

    if (!physical_state_unchanged(before, staged)) {
        add_profile_error(
            result,
            expansion.parent.id,
            "gdspaces.naming-reconcile.profile.physical-state-changed",
            "Profile semantic reconciliation attempted to change physical identity, topology, or bytes.");
        return result;
    }

    expansion = std::move(staged);
    result.reconciled = true;
    return result;
}

} // namespace dmc::rengine::gdspaces
