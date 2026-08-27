#include "dmc_rengine/gdspaces/index_name_overlay.hpp"

#include "dmc_rengine/gdspaces/classifier.hpp"

#include <algorithm>
#include <cctype>
#include <span>
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

[[nodiscard]] std::string canonical_extension(
    std::string_view format) {
    if (format == "pe") {
        return "exe";
    }
    return std::string{format};
}

[[nodiscard]] std::string make_display_name(
    std::string_view stem,
    std::string_view extension) {
    if (extension.empty()) {
        return std::string{stem};
    }
    std::string result;
    result.reserve(stem.size() + extension.size() + 1U);
    result.append(stem);
    result.push_back('.');
    result.append(extension);
    return result;
}

void add_error(
    std::vector<Diagnostic>& diagnostics,
    const ResourceId& resource,
    std::string code,
    std::string message) {
    diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

[[nodiscard]] const ContainerChild* find_child(
    const ContainerExpansion& expansion,
    const IndexSlotNameAuthority& authority) noexcept {
    const auto iterator = std::find_if(
        expansion.children.begin(), expansion.children.end(),
        [&](const ContainerChild& child) {
            return child.entry.slot_index == authority.slot_index() &&
                   child.payload.resource.id == authority.child_resource();
        });
    return iterator == expansion.children.end() ? nullptr : &*iterator;
}

[[nodiscard]] ContainerChild* find_child(
    ContainerExpansion& expansion,
    const IndexNameOverlayEntry& entry) noexcept {
    const auto iterator = std::find_if(
        expansion.children.begin(), expansion.children.end(),
        [&](ContainerChild& child) {
            return child.entry.slot_index == entry.slot_index() &&
                   child.payload.resource.id == entry.child_resource();
        });
    return iterator == expansion.children.end() ? nullptr : &*iterator;
}

} // namespace

IndexNameOverlayEntry::IndexNameOverlayEntry(
    std::uint32_t slot_index,
    ResourceId child_resource,
    std::string display_name,
    std::string raw_index_label,
    std::size_t manifest_line,
    std::string semantic_format,
    IndexDisplayEvidenceKind evidence_kind)
    : slot_index_(slot_index),
      child_resource_(std::move(child_resource)),
      display_name_(std::move(display_name)),
      raw_index_label_(std::move(raw_index_label)),
      manifest_line_(manifest_line),
      semantic_format_(std::move(semantic_format)),
      evidence_kind_(evidence_kind) {}

std::uint32_t IndexNameOverlayEntry::slot_index() const noexcept {
    return slot_index_;
}

const ResourceId& IndexNameOverlayEntry::child_resource() const noexcept {
    return child_resource_;
}

std::string_view IndexNameOverlayEntry::display_name() const noexcept {
    return display_name_;
}

std::string_view IndexNameOverlayEntry::raw_index_label() const noexcept {
    return raw_index_label_;
}

std::size_t IndexNameOverlayEntry::manifest_line() const noexcept {
    return manifest_line_;
}

std::string_view IndexNameOverlayEntry::semantic_format() const noexcept {
    return semantic_format_;
}

IndexDisplayEvidenceKind IndexNameOverlayEntry::evidence_kind() const noexcept {
    return evidence_kind_;
}

IndexNameOverlay::IndexNameOverlay(
    ResourceId parent_resource,
    ResourceId manifest_resource,
    std::string manifest_sha256,
    IndexSlotMappingMode mapping_mode,
    std::vector<IndexNameOverlayEntry> entries)
    : parent_resource_(std::move(parent_resource)),
      manifest_resource_(std::move(manifest_resource)),
      manifest_sha256_(std::move(manifest_sha256)),
      mapping_mode_(mapping_mode),
      entries_(std::move(entries)) {}

const ResourceId& IndexNameOverlay::parent_resource() const noexcept {
    return parent_resource_;
}

const ResourceId& IndexNameOverlay::manifest_resource() const noexcept {
    return manifest_resource_;
}

std::string_view IndexNameOverlay::manifest_sha256() const noexcept {
    return manifest_sha256_;
}

IndexSlotMappingMode IndexNameOverlay::mapping_mode() const noexcept {
    return mapping_mode_;
}

const std::vector<IndexNameOverlayEntry>& IndexNameOverlay::entries() const noexcept {
    return entries_;
}

bool IndexNameOverlay::valid() const noexcept {
    if (!parent_resource_.valid() || !manifest_resource_.valid() ||
        !valid_digest(manifest_sha256_) || entries_.empty()) {
        return false;
    }
    return std::all_of(
        entries_.begin(), entries_.end(),
        [](const IndexNameOverlayEntry& entry) {
            return entry.child_resource().valid() &&
                   !entry.display_name().empty() &&
                   !entry.raw_index_label().empty() &&
                   entry.manifest_line() > 0U;
        });
}

bool IndexNameOverlayBuildResult::ok() const noexcept {
    if (!overlay.has_value() || !overlay->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

bool IndexNameOverlayApplyResult::ok() const noexcept {
    if (!applied) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

IndexNameOverlayBuildResult IndexNameOverlayBuilder::build(
    const ContainerExpansion& expansion,
    const IndexSlotBindingResult& binding,
    IndexProfileDisplayResolver profile_resolver) {
    IndexNameOverlayBuildResult result;
    if (!expansion.usable()) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.index-overlay.invalid-expansion",
            "Index display overlay requires a usable physical container expansion.");
        return result;
    }
    if (!binding.valid()) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.index-overlay.invalid-binding",
            "Index display overlay requires sealed valid slot-name authority.");
        return result;
    }
    if (binding.parent_resource() != expansion.parent.id) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.index-overlay.parent-mismatch",
            "Index slot authority belongs to a different physical parent resource.");
        return result;
    }

    std::vector<IndexNameOverlayEntry> overlay_entries;
    overlay_entries.reserve(binding.authorities().size());
    for (const auto& authority : binding.authorities()) {
        const auto* child = find_child(expansion, authority);
        if (child == nullptr) {
            add_error(
                result.diagnostics,
                authority.child_resource(),
                "gdspaces.index-overlay.child-mismatch",
                "Index slot authority does not resolve to the same physical child identity.");
            return result;
        }

        const auto classification = ResourceClassifier::classify(
            authority.index_name(),
            std::span<const std::byte>{
                child->payload.bytes.data(), child->payload.bytes.size()});

        std::string display_extension;
        std::string semantic_format;
        IndexDisplayEvidenceKind evidence_kind =
            IndexDisplayEvidenceKind::index_source_extension;

        if (classification.magic_confirmed) {
            display_extension = canonical_extension(classification.format);
            semantic_format = classification.format;
            evidence_kind = IndexDisplayEvidenceKind::magic_confirmed_format;
        } else if (profile_resolver != nullptr) {
            const auto profile_semantic = profile_resolver(
                child->payload, authority);
            if (profile_semantic.has_value() &&
                !profile_semantic->canonical_extension.empty() &&
                !profile_semantic->semantic_format.empty()) {
                display_extension = profile_semantic->canonical_extension;
                semantic_format = profile_semantic->semantic_format;
                evidence_kind = IndexDisplayEvidenceKind::profile_structural_format;
            }
        }

        if (display_extension.empty() && authority.source_extension().has_value()) {
            display_extension = *authority.source_extension();
            // An extension-only observation is naming evidence, not semantic
            // format authority. In particular, .ukn must remain semantically
            // unknown until bytes or a profile structural parser prove more.
            semantic_format = "unknown";
        }

        const auto display_name = make_display_name(
            authority.stem(), display_extension);
        overlay_entries.push_back(IndexNameOverlayEntry(
            authority.slot_index(),
            authority.child_resource(),
            display_name,
            std::string{authority.raw_index_label()},
            authority.manifest_line(),
            std::move(semantic_format),
            evidence_kind));
    }

    result.overlay = IndexNameOverlay(
        binding.parent_resource(),
        binding.manifest_resource(),
        std::string{binding.manifest_sha256()},
        binding.mapping_mode(),
        std::move(overlay_entries));
    return result;
}

IndexNameOverlayApplyResult IndexNameOverlayBuilder::apply(
    ContainerExpansion& expansion,
    const IndexNameOverlay& overlay) {
    IndexNameOverlayApplyResult result;
    if (!expansion.usable() || !overlay.valid()) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.index-overlay.apply-invalid",
            "Cannot apply an invalid name overlay or apply to an unusable expansion.");
        return result;
    }
    if (overlay.parent_resource() != expansion.parent.id) {
        add_error(
            result.diagnostics,
            expansion.parent.id,
            "gdspaces.index-overlay.apply-parent-mismatch",
            "Name overlay parent identity differs from the target expansion parent.");
        return result;
    }

    for (const auto& entry : overlay.entries()) {
        if (find_child(expansion, entry) == nullptr) {
            add_error(
                result.diagnostics,
                entry.child_resource(),
                "gdspaces.index-overlay.apply-child-mismatch",
                "Name overlay child identity differs from the target expansion child.");
            return result;
        }
    }

    for (const auto& entry : overlay.entries()) {
        auto* child = find_child(expansion, entry);
        child->payload.resource.display_name = std::string{entry.display_name()};
        child->payload.resource.synthetic_name = false;
    }
    result.applied = true;
    return result;
}

} // namespace dmc::rengine::gdspaces
