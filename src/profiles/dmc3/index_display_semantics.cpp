#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"

#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <span>

namespace dmc::rengine::profiles::dmc3 {

std::optional<gdspaces::ResourceProfileSemantic>
resolve_materialized_display_semantic(
    const gdspaces::ResourcePayload& child) {
    if (child.bytes.empty()) {
        return std::nullopt;
    }

    // Do not gate this structural probe on ResourceRef::profile. Retail NBZ
    // member paths such as GData.afs/obj/em000.pac do not themselves carry a
    // "dmc3" token, so generic path classification legitimately reports the
    // physical profile as unknown. This resolver is reached only through the
    // explicit DMC3 naming pipeline/profile adapter; the structure, not a path
    // label, is the evidence for PTX/wrapped-DDS semantics.
    const auto framing = TextureSlotFramingParser::parse(
        std::span<const std::byte>{child.bytes.data(), child.bytes.size()});
    if (!framing.ok()) {
        return std::nullopt;
    }

    if (framing.document.kind == TextureSlotFramingKind::texture_bundle) {
        return gdspaces::ResourceProfileSemantic{
            .canonical_extension = "ptx",
            .semantic_format = "texture-bundle",
        };
    }

    if (framing.document.kind == TextureSlotFramingKind::wrapped_dds) {
        return gdspaces::ResourceProfileSemantic{
            .canonical_extension = "dds",
            .semantic_format = "wrapped-dds",
        };
    }

    return std::nullopt;
}

std::optional<gdspaces::IndexProfileDisplaySemantic>
resolve_index_display_semantic(
    const gdspaces::ResourcePayload& child,
    const gdspaces::IndexSlotNameAuthority&) {
    const auto semantic = resolve_materialized_display_semantic(child);
    if (!semantic.has_value()) {
        return std::nullopt;
    }
    return gdspaces::IndexProfileDisplaySemantic{
        .canonical_extension = semantic->canonical_extension,
        .semantic_format = semantic->semantic_format,
    };
}

} // namespace dmc::rengine::profiles::dmc3
