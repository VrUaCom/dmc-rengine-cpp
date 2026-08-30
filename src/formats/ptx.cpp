#include "dmc_rengine/formats/ptx.hpp"

#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace dmc::rengine::formats {
namespace {

namespace dmc3 = profiles::dmc3;

[[nodiscard]] std::string synthetic_texture_name(std::uint32_t index) {
    std::ostringstream output;
    output << "texture_" << std::setfill('0') << std::setw(4) << index
           << ".dds";
    return output.str();
}

[[nodiscard]] PtxParseResult fail(PtxParseError error, std::string message) {
    return PtxParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

[[nodiscard]] PtxParseError error_for(
    dmc3::TextureSlotFramingStatus status) noexcept {
    switch (status) {
    case dmc3::TextureSlotFramingStatus::ok:
        return PtxParseError::none;
    case dmc3::TextureSlotFramingStatus::not_recognized:
        return PtxParseError::not_a_texture_pack;
    case dmc3::TextureSlotFramingStatus::invalid_count:
        return PtxParseError::texture_count_limit;
    case dmc3::TextureSlotFramingStatus::truncated_header:
        return PtxParseError::truncated_header;
    case dmc3::TextureSlotFramingStatus::truncated_descriptor:
        return PtxParseError::truncated_descriptor;
    case dmc3::TextureSlotFramingStatus::invalid_dds:
        return PtxParseError::missing_image_signature;
    case dmc3::TextureSlotFramingStatus::unsupported_compression:
        return PtxParseError::unsupported_compression;
    case dmc3::TextureSlotFramingStatus::descriptor_mismatch:
        return PtxParseError::descriptor_mismatch;
    case dmc3::TextureSlotFramingStatus::invalid_sector_span:
    case dmc3::TextureSlotFramingStatus::trailing_bytes:
        return PtxParseError::size_table_does_not_close;
    case dmc3::TextureSlotFramingStatus::nonzero_alignment_padding:
        return PtxParseError::nonzero_alignment_padding;
    }
    return PtxParseError::not_a_texture_pack;
}

} // namespace

PtxParseResult PtxParser::parse(std::span<const std::byte> bytes) {
    // One authority for what a texture pack is. The framing parser already
    // validates the whole 0x70 descriptor against the DDS it introduces, and a
    // second recognizer here would eventually disagree with it about a file —
    // which is the failure that matters, because the disagreeing pair would be
    // "the tree says texture pack" and "the writer says it is not".
    // Reading does not require a complete mip chain. Every texture in the
    // corpus this framing was recovered from carries one, but that was never
    // evidence that the runtime demands one, and refusing to read a
    // structurally valid DDS on the strength of it made a real retail at.ptx
    // unopenable. Authoring keeps the bound; the packed-reflow writer takes
    // the default.
    dmc3::TextureSlotFramingSafety reading;
    reading.require_full_mip_chain = false;
    const auto framed = dmc3::TextureSlotFramingParser::parse(bytes, reading);
    if (!framed.ok()) {
        return fail(
            error_for(framed.status),
            framed.detail.empty()
                ? std::string{"texture-slot framing rejected the byte image"}
                : std::string{framed.detail});
    }
    if (framed.document.kind != dmc3::TextureSlotFramingKind::texture_bundle) {
        // A single wrapped DDS is a texture, not a pack. Expanding it into a
        // one-child container would invent a level that is not there.
        return fail(
            PtxParseError::not_a_texture_pack,
            "the byte image is a single wrapped texture, not a texture pack");
    }

    ContainerDocument document;
    document.format = "ptx";
    document.schema_version = 1U;
    document.declared_slot_count =
        static_cast<std::uint32_t>(framed.document.textures.size());
    document.container_size = static_cast<std::uint64_t>(bytes.size());
    document.entries.reserve(framed.document.textures.size());

    for (const auto& texture : framed.document.textures) {
        // The child is the DDS image at its exact extent, not the block it
        // sits in: the descriptor is container framing and the sector padding
        // belongs to the container, so neither is part of the file a caller
        // gets when it asks for this texture.
        document.entries.push_back(ContainerEntry{
            .slot_index = texture.texture_index,
            .offset = texture.dds_offset,
            .size = static_cast<std::uint64_t>(texture.dds_size),
            .logical_name = synthetic_texture_name(texture.texture_index),
            .populated = true,
            .synthetic_name = true,
        });
    }

    if (!document.valid()) {
        return fail(
            PtxParseError::invalid_document,
            "texture pack decoded to an invalid ContainerDocument");
    }

    return PtxParseResult{
        .document = std::move(document),
        .error = PtxParseError::none,
        .message = {},
    };
}

bool PtxParser::structurally_valid(std::span<const std::byte> bytes) noexcept {
    // Recognition and materialization must never disagree about what a texture
    // pack is, so the probe is the parser.
    return parse(bytes).ok();
}

} // namespace dmc::rengine::formats
