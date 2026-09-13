#pragma once

#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_runtime_materialization.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

namespace dmc::rengine::formats::model_family {

// Serialized texture-companion envelope consumed by the canonical DMC3-HD
// model texture path before runtime descriptors are materialized. The envelope
// itself has no promoted magic: entry payloads are independently verified as
// TIM2/TM2 by the canonical helper path.
struct ModelTextureCompanionAbi final {
    static constexpr std::size_t texture_count_field = 0x000U;
    static constexpr std::size_t block_count_table = 0x004U;
    static constexpr std::size_t payload_base = 0x800U;
    static constexpr std::size_t payload_block_size = 0x800U;
    static constexpr std::uint32_t tm2_magic_le = 0x00324D54U; // "TM2\0"

    // The second payload framing, and the one every preserved specimen
    // actually uses: a 0x70-byte texture-slot descriptor followed by the DDS
    // image it describes. Both constants are the canonical framing contract's
    // rather than new numbers — k_descriptor_size from TextureSlotFramingParser
    // and k_dds_magic from the runtime materialization contract — so the two
    // readers cannot drift apart on what a slot payload looks like.
    static constexpr std::size_t wrapped_descriptor_size =
        profiles::dmc3::TextureSlotFramingParser::k_descriptor_size;
    static constexpr std::uint32_t dds_magic_le =
        profiles::dmc3::TextureSlotRuntimeMaterializationInspector::k_dds_magic;
};

// Which framing a companion's payloads use. Recorded rather than normalized
// away: an authoring path that rebuilds a companion has to write back the
// framing it read, and a consumer that silently accepted either would have no
// way to say which.
enum class TextureCompanionFraming : std::uint8_t {
    // No payload has been examined yet.
    unknown,
    // "TM2\0" at payload offset zero.
    tm2_at_payload_start,
    // A 0x70 descriptor and the DDS behind it.
    descriptor_wrapped_dds,
};

[[nodiscard]] constexpr std::string_view to_string(
    TextureCompanionFraming framing) noexcept {
    switch (framing) {
    case TextureCompanionFraming::unknown: return "unknown";
    case TextureCompanionFraming::tm2_at_payload_start: return "tm2";
    case TextureCompanionFraming::descriptor_wrapped_dds: return "wrapped-dds";
    }
    return "unknown";
}

static_assert(ModelTextureCompanionAbi::texture_count_field == 0x000U);
static_assert(ModelTextureCompanionAbi::block_count_table == 0x004U);
static_assert(ModelTextureCompanionAbi::payload_base == 0x800U);
static_assert(ModelTextureCompanionAbi::payload_block_size == 0x800U);
static_assert(ModelTextureCompanionAbi::tm2_magic_le == 0x00324D54U);

enum class TextureCompanionStatus : std::uint8_t {
    ok,
    truncated_count,
    block_table_out_of_bounds,
    block_table_overlaps_payload,
    payload_size_overflow,
    payload_out_of_bounds,
    // No payload signature was recognized. The legacy name is kept so existing
    // consumers keep compiling; it now means "neither framing", not "not TM2".
    tm2_magic_mismatch,
    // Payloads were individually recognized but not all under one framing. A
    // companion is a single runtime table and the canonical path materializes
    // it one way, so a mixture is a refusal rather than a per-entry detail.
    mixed_framing,
};

struct TextureCompanionEntry final {
    std::uint32_t index{};
    std::uint32_t block_count{};
    std::size_t payload_offset{};
    std::size_t allocated_size{};
    TextureCompanionFraming framing{TextureCompanionFraming::unknown};

    /// Where the image itself starts, past the descriptor where there is one.
    [[nodiscard]] constexpr std::size_t image_offset() const noexcept {
        return framing == TextureCompanionFraming::descriptor_wrapped_dds
            ? payload_offset + ModelTextureCompanionAbi::wrapped_descriptor_size
            : payload_offset;
    }
};

struct TextureCompanionParseResult final {
    TextureCompanionStatus status{TextureCompanionStatus::truncated_count};
    std::uint32_t texture_count{};
    std::vector<TextureCompanionEntry> entries;
    /// The one framing every payload used. `unknown` where none were read.
    TextureCompanionFraming framing{TextureCompanionFraming::unknown};

    [[nodiscard]] bool ok() const noexcept {
        return status == TextureCompanionStatus::ok;
    }
};

namespace texture_companion_detail {

[[nodiscard]] inline std::uint32_t read_u32_le(
    const std::span<const std::byte> bytes,
    const std::size_t offset) noexcept {
    std::uint32_t value{};
    for (std::size_t index = 0U; index < 4U; ++index) {
        value |= static_cast<std::uint32_t>(
            std::to_integer<std::uint8_t>(bytes[offset + index])) <<
            (index * 8U);
    }
    return value;
}

[[nodiscard]] inline bool range_fits(
    const std::size_t offset,
    const std::size_t size,
    const std::size_t total) noexcept {
    return offset <= total && size <= total - offset;
}

} // namespace texture_companion_detail

// Safe structural reconstruction of the serialized companion envelope consumed
// by canonical texture materialization. Each u32 table value is a count of
// 0x800-byte blocks. Payloads are laid out consecutively from +0x800 and each
// payload begins with the canonical little-endian "TM2\0" signature.
//
// This parser deliberately stops at the companion allocation boundary. It does
// not decode TIM2 internals or construct the later 0x40-byte runtime descriptor
// entries held by the model manager.
[[nodiscard]] inline TextureCompanionParseResult parse_texture_companion(
    const std::span<const std::byte> bytes) {
    using namespace texture_companion_detail;

    TextureCompanionParseResult out;
    if (!range_fits(
            ModelTextureCompanionAbi::texture_count_field,
            4U,
            bytes.size())) {
        return out;
    }

    out.texture_count = read_u32_le(
        bytes, ModelTextureCompanionAbi::texture_count_field);

    if (out.texture_count >
        std::numeric_limits<std::size_t>::max() / 4U) {
        out.status = TextureCompanionStatus::block_table_out_of_bounds;
        return out;
    }
    const auto table_size =
        static_cast<std::size_t>(out.texture_count) * 4U;
    if (!range_fits(
            ModelTextureCompanionAbi::block_count_table,
            table_size,
            bytes.size())) {
        out.status = TextureCompanionStatus::block_table_out_of_bounds;
        return out;
    }
    if (table_size >
        ModelTextureCompanionAbi::payload_base -
            ModelTextureCompanionAbi::block_count_table) {
        out.status = TextureCompanionStatus::block_table_overlaps_payload;
        return out;
    }

    out.entries.reserve(out.texture_count);
    std::size_t payload_offset = ModelTextureCompanionAbi::payload_base;
    for (std::uint32_t index = 0U; index < out.texture_count; ++index) {
        const auto blocks = read_u32_le(
            bytes,
            ModelTextureCompanionAbi::block_count_table +
                static_cast<std::size_t>(index) * 4U);
        if (blocks >
            std::numeric_limits<std::size_t>::max() /
                ModelTextureCompanionAbi::payload_block_size) {
            out.status = TextureCompanionStatus::payload_size_overflow;
            return out;
        }
        const auto allocated_size =
            static_cast<std::size_t>(blocks) *
            ModelTextureCompanionAbi::payload_block_size;
        if (!range_fits(payload_offset, allocated_size, bytes.size()) ||
            allocated_size < 4U) {
            out.status = TextureCompanionStatus::payload_out_of_bounds;
            return out;
        }
        // Which of the two framings this payload uses. TM2 at offset zero is
        // the framing the contract was written against; the descriptor-wrapped
        // DDS is the one every preserved specimen turns out to use, so
        // requiring the first alone made this parser refuse the entire corpus.
        auto framing = TextureCompanionFraming::unknown;
        if (read_u32_le(bytes, payload_offset) ==
            ModelTextureCompanionAbi::tm2_magic_le) {
            framing = TextureCompanionFraming::tm2_at_payload_start;
        } else if (
            allocated_size >=
                ModelTextureCompanionAbi::wrapped_descriptor_size + 4U &&
            read_u32_le(
                bytes,
                payload_offset +
                    ModelTextureCompanionAbi::wrapped_descriptor_size) ==
                ModelTextureCompanionAbi::dds_magic_le) {
            framing = TextureCompanionFraming::descriptor_wrapped_dds;
        } else {
            out.status = TextureCompanionStatus::tm2_magic_mismatch;
            return out;
        }
        if (out.framing == TextureCompanionFraming::unknown) {
            out.framing = framing;
        } else if (out.framing != framing) {
            out.status = TextureCompanionStatus::mixed_framing;
            return out;
        }

        out.entries.push_back(TextureCompanionEntry{
            .index = index,
            .block_count = blocks,
            .payload_offset = payload_offset,
            .allocated_size = allocated_size,
            .framing = framing,
        });
        payload_offset += allocated_size;
    }

    out.status = TextureCompanionStatus::ok;
    return out;
}

} // namespace dmc::rengine::formats::model_family
