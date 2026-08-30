#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"

#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <optional>
#include <span>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::optional<gdspaces::ResourceProfileSemantic>
resolve_runtime_content_tag_semantic(std::span<const std::byte> bytes) {
    using Contract = ResourceTypeContract;
    const auto code = Contract::type_for_prefix(bytes);
    const auto format = Contract::canonical_extension(code);
    if (format.empty()) {
        return std::nullopt;
    }
    return gdspaces::ResourceProfileSemantic{
        .canonical_extension = std::string{format},
        .semantic_format = std::string{format},
        .evidence_kind = gdspaces::ResourceProfileSemanticKind::runtime_content_tag,
    };
}

} // namespace

std::optional<gdspaces::ResourceProfileSemantic>
resolve_materialized_display_semantic(
    const gdspaces::ResourcePayload& child) {
    if (child.bytes.empty()) {
        return std::nullopt;
    }

    // Do not gate these byte/structural probes on ResourceRef::profile. Retail
    // NBZ member paths such as GData.afs/obj/em000.pac do not themselves carry
    // a "dmc3" token, so generic path classification legitimately reports the
    // physical profile as unknown. This resolver is reached only through the
    // explicit DMC3 naming pipeline/profile adapter.
    const auto bytes = std::span<const std::byte>{
        child.bytes.data(), child.bytes.size()};

    const auto framing = TextureSlotFramingParser::parse(bytes);
    if (framing.ok()) {
        if (framing.document.kind == TextureSlotFramingKind::texture_bundle) {
            return gdspaces::ResourceProfileSemantic{
                .canonical_extension = "ptx",
                .semantic_format = "texture-bundle",
                .evidence_kind = gdspaces::ResourceProfileSemanticKind::structural_format,
            };
        }

        if (framing.document.kind == TextureSlotFramingKind::wrapped_dds) {
            return gdspaces::ResourceProfileSemantic{
                .canonical_extension = "dds",
                .semantic_format = "wrapped-dds",
                .evidence_kind = gdspaces::ResourceProfileSemanticKind::structural_format,
            };
        }
    }

    // Nameless relative-slot payloads can still carry the exact content tags
    // used by the original runtime dispatcher. In particular, em000 model
    // payloads beginning with "MOD" are therefore byte-backed `mod` resources,
    // not files named `.mod` because a UI guessed a suffix.
    return resolve_runtime_content_tag_semantic(bytes);
}

std::optional<gdspaces::IndexProfileDisplaySemantic>
resolve_index_display_semantic(
    const gdspaces::ResourcePayload& child,
    const gdspaces::IndexSlotNameAuthority&) {
    const auto semantic = resolve_materialized_display_semantic(child);
    if (!semantic.has_value()) {
        return std::nullopt;
    }

    const auto evidence_kind =
        semantic->evidence_kind == gdspaces::ResourceProfileSemanticKind::runtime_content_tag
            ? gdspaces::IndexDisplayEvidenceKind::profile_runtime_content_tag
            : gdspaces::IndexDisplayEvidenceKind::profile_structural_format;
    return gdspaces::IndexProfileDisplaySemantic{
        .canonical_extension = semantic->canonical_extension,
        .semantic_format = semantic->semantic_format,
        .evidence_kind = evidence_kind,
    };
}

} // namespace dmc::rengine::profiles::dmc3
