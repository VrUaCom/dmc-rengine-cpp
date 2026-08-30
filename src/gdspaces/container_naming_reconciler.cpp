#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/embedded_name_evidence.hpp"
#include "dmc_rengine/gdspaces/format_identity.hpp"
#include "dmc_rengine/gdspaces/index_manifest.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"
#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"
#include "dmc_rengine/gdspaces/resource_semantic_evidence.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
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

[[nodiscard]] std::string manifest_directive_text(
    IndexContainerDirective directive) {
    return directive == IndexContainerDirective::pnst_non_empty_slots
        ? std::string{"PNST"}
        : std::string{};
}

[[nodiscard]] std::optional<ResourceSemanticEvidenceKind> overlay_profile_kind(
    IndexDisplayEvidenceKind kind) noexcept {
    switch (kind) {
    case IndexDisplayEvidenceKind::profile_structural_format:
        return ResourceSemanticEvidenceKind::profile_structural_format;
    case IndexDisplayEvidenceKind::profile_runtime_content_tag:
        return ResourceSemanticEvidenceKind::profile_runtime_content_tag;
    default:
        return std::nullopt;
    }
}

[[nodiscard]] bool is_profile_semantic_kind(
    ResourceSemanticEvidenceKind kind) noexcept {
    return kind == ResourceSemanticEvidenceKind::profile_structural_format ||
        kind == ResourceSemanticEvidenceKind::profile_runtime_content_tag;
}

} // namespace

bool ContainerNamingReconcileResult::ok() const noexcept {
    return reconciled && !has_error(diagnostics);
}

bool ContainerNamingReconciler::persist_magic_semantics(
    ContainerExpansion& expansion,
    ContainerNamingReconcileResult& result) {
    for (auto& child : expansion.children) {
        if (!child.entry.populated || child.payload.bytes.empty()) {
            continue;
        }

        const auto bytes = std::span<const std::byte>{
            child.payload.bytes.data(), child.payload.bytes.size()};
        const auto classification = ResourceClassifier::classify(
            child.payload.resource.display_name, bytes);
        if (!classification.magic_confirmed || classification.format.empty()) {
            continue;
        }

        const auto canonical_extension =
            ResourceFormatIdentity::canonical_extension(classification.format);
        if (canonical_extension.empty()) {
            add_error(
                result,
                child.payload.resource.id,
                "gdspaces.naming-reconcile.magic-semantic-extension-missing",
                "Magic-confirmed semantic format has no canonical extension and cannot become sealed naming evidence.");
            return false;
        }

        ResourceSemanticEvidence semantic_evidence(
            ResourceSemanticEvidenceKind::magic_confirmed_format,
            child.payload.resource.id,
            core::Sha256::compute(bytes).hex(),
            classification.format,
            canonical_extension,
            child.entry.slot_index);
        if (!semantic_evidence.valid()) {
            add_error(
                result,
                child.payload.resource.id,
                "gdspaces.naming-reconcile.magic-semantic-evidence-invalid",
                "Magic-confirmed byte classification could not form valid sealed semantic evidence.");
            return false;
        }

        child.payload.resource.format = classification.format;
        auto& evidence = child.payload.semantic_evidence;
        evidence.erase(
            std::remove_if(
                evidence.begin(), evidence.end(),
                [](const ResourceSemanticEvidence& existing) {
                    return existing.kind() ==
                        ResourceSemanticEvidenceKind::magic_confirmed_format;
                }),
            evidence.end());
        evidence.push_back(std::move(semantic_evidence));
        result.magic_semantics_applied = true;
    }
    return true;
}

bool ContainerNamingReconciler::persist_overlay_semantics(
    ContainerExpansion& expansion,
    const IndexNameOverlay& overlay,
    ContainerNamingReconcileResult& result) {
    for (const auto& entry : overlay.entries()) {
        if (entry.semantic_format().empty() || entry.semantic_format() == "unknown") {
            continue;
        }

        auto* child = find_slot(expansion, entry.slot_index());
        if (child == nullptr || child->payload.resource.id != entry.child_resource()) {
            add_error(
                result,
                entry.child_resource(),
                "gdspaces.naming-reconcile.overlay-semantic-child-missing",
                "Overlay semantic evidence no longer resolves to the same physical child.");
            return false;
        }

        child->payload.resource.format = std::string{entry.semantic_format()};

        const auto semantic_kind = overlay_profile_kind(entry.evidence_kind());
        if (!semantic_kind.has_value()) {
            continue;
        }
        if (entry.canonical_extension().empty()) {
            add_error(
                result,
                child->payload.resource.id,
                "gdspaces.naming-reconcile.profile-semantic-extension-missing",
                "Profile semantic evidence must carry its canonical presentation extension.");
            return false;
        }

        const auto bytes = std::span<const std::byte>{
            child->payload.bytes.data(), child->payload.bytes.size()};
        ResourceSemanticEvidence semantic_evidence(
            *semantic_kind,
            child->payload.resource.id,
            core::Sha256::compute(bytes).hex(),
            std::string{entry.semantic_format()},
            std::string{entry.canonical_extension()},
            entry.slot_index());
        if (!semantic_evidence.valid()) {
            add_error(
                result,
                child->payload.resource.id,
                "gdspaces.naming-reconcile.profile-semantic-evidence-invalid",
                "Profile byte/structural classification could not form valid sealed semantic evidence.");
            return false;
        }

        auto& evidence = child->payload.semantic_evidence;
        evidence.erase(
            std::remove_if(
                evidence.begin(), evidence.end(),
                [](const ResourceSemanticEvidence& existing) {
                    return is_profile_semantic_kind(existing.kind());
                }),
            evidence.end());
        evidence.push_back(std::move(semantic_evidence));
    }
    return true;
}

ContainerNamingReconcileResult ContainerNamingReconciler::reconcile(
    ContainerExpansion& expansion,
    const ResourcePayload* external_index) {
    return reconcile_profiled(expansion, external_index, nullptr);
}

ContainerNamingReconcileResult ContainerNamingReconciler::reconcile_profiled(
    ContainerExpansion& expansion,
    const ResourcePayload* external_index,
    IndexProfileDisplayResolver profile_resolver) {
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

    if (!persist_magic_semantics(staged, result)) {
        return result;
    }

    if (external_index != nullptr) {
        const auto manifest = IndexManifestParser::parse(*external_index);
        append_diagnostics(result.diagnostics, manifest.diagnostics);
        if (!manifest.ok()) {
            return result;
        }

        staged.external_index_evidence = ContainerIndexNamingEvidence{
            .manifest_resource = manifest.manifest->source(),
            .manifest_sha256 = std::string{manifest.manifest->observed_sha256()},
            .directive = manifest_directive_text(manifest.manifest->directive()),
            .entry_count = manifest.manifest->entries().size(),
        };

        const auto binding = IndexSlotNameBinder::bind(
            staged, *manifest.manifest);
        append_diagnostics(result.diagnostics, binding.diagnostics);
        if (!binding.ok()) {
            return result;
        }

        const auto overlay = IndexNameOverlayBuilder::build(
            staged, *binding.binding, profile_resolver);
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
        if (!persist_overlay_semantics(staged, *overlay.overlay, result)) {
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
