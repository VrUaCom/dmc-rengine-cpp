#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"

#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <span>

namespace dmc::rengine::profiles::dmc3 {

std::optional<gdspaces::IndexProfileDisplaySemantic>
resolve_index_display_semantic(
    const gdspaces::ResourcePayload& child,
    const gdspaces::IndexSlotNameAuthority&) {
    if (child.resource.profile != "dmc3-hd" || child.bytes.empty()) {
        return std::nullopt;
    }

    const auto framing = TextureSlotFramingParser::parse(
        std::span<const std::byte>{child.bytes.data(), child.bytes.size()});
    if (!framing.ok()) {
        return std::nullopt;
    }

    if (framing.document.kind == TextureSlotFramingKind::texture_bundle) {
        return gdspaces::IndexProfileDisplaySemantic{
            .canonical_extension = "ptx",
            .semantic_format = "texture-bundle",
        };
    }

    if (framing.document.kind == TextureSlotFramingKind::wrapped_dds) {
        return gdspaces::IndexProfileDisplaySemantic{
            .canonical_extension = "dds",
            .semantic_format = "wrapped-dds",
        };
    }

    return std::nullopt;
}

} // namespace dmc::rengine::profiles::dmc3
