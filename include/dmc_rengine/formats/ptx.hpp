#pragma once

#include "dmc_rengine/formats/container.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace dmc::rengine::formats {

// The DMC3 stage texture pack, as a container the resource tree can expand.
//
// It carries no magic: the first dword is the texture count. What identifies
// it instead is that its own arithmetic closes exactly — the declared sector
// sizes plus the header sector reproduce the stored length to the byte — and
// that every block it describes carries a 0x70 descriptor whose fields agree
// with the DDS image behind it. A structure that predicts its own length and
// its own contents is stronger evidence than four magic bytes.
//
// This is a view, not a second parser: recognition and geometry both come from
// `TextureSlotFramingParser`, which already models the descriptor and is what
// the packed-reflow writer authors against. Keeping one authority is what lets
// a texture extracted from the tree be handed back to the writer unchanged.
//
// Observed in `st001.pac` slot 1 (17 textures, 3,485,696 bytes) and
// `st114.pac` slot 1 (17 textures, 2,404,352 bytes), which are the two stages
// of the supplied retail corpus. The name manifest in slot 0 of each container
// calls this record `stNNN.ptx`.
enum class PtxParseError : std::uint8_t {
    none,
    not_a_texture_pack,
    truncated_header,
    truncated_descriptor,
    texture_count_limit,
    size_table_does_not_close,
    missing_image_signature,
    unsupported_compression,
    descriptor_mismatch,
    nonzero_alignment_padding,
    invalid_document,
};

struct PtxParseResult final {
    std::optional<ContainerDocument> document;
    PtxParseError error{PtxParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == PtxParseError::none;
    }
};

class PtxParser final {
public:
    // Every block is aligned to, and measured in, this sector.
    static constexpr std::uint64_t k_sector_bytes = 0x800U;

    // The count and its size table live in the first sector, which is why the
    // table cannot describe more textures than fit in it.
    static constexpr std::uint32_t k_max_texture_count =
        static_cast<std::uint32_t>((k_sector_bytes - 4U) / 4U);

    // Each block opens with a fixed-size descriptor and the image follows it.
    static constexpr std::uint64_t k_descriptor_bytes = 0x70U;

    [[nodiscard]] static PtxParseResult parse(std::span<const std::byte> bytes);

    // A cheap structural recognizer for classification, which must decide
    // without paying for a full parse.
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats
