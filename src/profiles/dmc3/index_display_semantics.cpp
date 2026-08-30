#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"

#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <optional>
#include <span>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool starts_with(
    std::span<const std::byte> bytes,
    std::string_view signature) noexcept {
    if (bytes.size() < signature.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < signature.size(); ++index) {
        if (std::to_integer<unsigned char>(bytes[index]) !=
            static_cast<unsigned char>(signature[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::optional<gdspaces::ResourceProfileSemantic>
resolve_runtime_content_tag_semantic(std::span<const std::byte> bytes) {
    using Contract = ResourceTypeContract;
    for (const auto& tagged : Contract::tagged_types) {
        // The recovered DMC3 content probe compares exactly three bytes. Do
        // not strengthen this to a four-byte magic check: that would make the
        // tool stricter than the original runtime and would reject evidence
        // that the game itself classifies.
        if (!starts_with(bytes, tagged.tag)) {
            continue;
        }

        const auto format = Contract::canonical_extension(tagged.code);
        if (format.empty()) {
            return std::nullopt;
        }
        return gdspaces::ResourceProfileSemantic{
            .canonical_extension = std::string{format},
            .semantic_format = std::string{format},
        };
    }
    return std::nullopt;
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
            };
        }

        if (framing.document.kind == TextureSlotFramingKind::wrapped_dds) {
            return gdspaces::ResourceProfileSemantic{
                .canonical_extension = "dds",
                .semantic_format = "wrapped-dds",
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
    return gdspaces::IndexProfileDisplaySemantic{
        .canonical_extension = semantic->canonical_extension,
        .semantic_format = semantic->semantic_format,
    };
}

} // namespace dmc::rengine::profiles::dmc3
