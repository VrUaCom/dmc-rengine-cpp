#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <limits>
#include <optional>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::size_t kDdsHeaderSize = 128U;
constexpr std::uint32_t kDdsHeaderStructSize = 124U;
constexpr std::size_t kDdsHeightOffset = 12U;
constexpr std::size_t kDdsWidthOffset = 16U;
constexpr std::size_t kDdsMipCountOffset = 28U;
constexpr std::size_t kDdsFourCcOffset = 84U;

constexpr std::size_t kDescriptorEncodingOffset = 0x08U;
constexpr std::size_t kDescriptorConstant0cOffset = 0x0CU;
constexpr std::size_t kDescriptorDimensionsOffset = 0x10U;
constexpr std::size_t kDescriptorConstant14Offset = 0x14U;
constexpr std::size_t kDescriptorRowBytesOffset = 0x18U;
constexpr std::size_t kDescriptorConstant20Offset = 0x20U;
constexpr std::size_t kDescriptorPayloadSizeOffset = 0x38U;
constexpr std::size_t kDescriptorAuxModeOffset = 0x3CU;
constexpr std::size_t kDescriptorAuxValueOffset = 0x40U;
constexpr std::size_t kDescriptorSecondaryDimensionsOffset = 0x44U;
constexpr std::size_t kDescriptorReciprocalWidthOffset = 0x48U;
constexpr std::size_t kDescriptorReciprocalHeightOffset = 0x4CU;
constexpr std::size_t kDescriptorFormatOffset = 0x60U;
constexpr std::size_t kDescriptorDdsSizeOffset = 0x64U;
constexpr std::size_t kDescriptorConstant68Offset = 0x68U;

constexpr std::array<std::size_t, 13> kDescriptorZeroOffsets{
    0x00U, 0x04U, 0x1CU, 0x24U, 0x28U, 0x2CU, 0x30U,
    0x34U, 0x50U, 0x54U, 0x58U, 0x5CU, 0x6CU,
};

[[nodiscard]] bool contains(
    std::span<const std::byte> bytes,
    std::size_t offset,
    std::size_t size) noexcept {
    return offset <= bytes.size() && size <= bytes.size() - offset;
}

[[nodiscard]] std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] bool has_dds_magic(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return contains(bytes, offset, 4U) &&
        bytes[offset + 0U] == std::byte{'D'} &&
        bytes[offset + 1U] == std::byte{'D'} &&
        bytes[offset + 2U] == std::byte{'S'} &&
        bytes[offset + 3U] == std::byte{' '};
}

[[nodiscard]] TextureSlotFramingResult failure(
    TextureSlotFramingStatus status,
    std::string_view detail) {
    return TextureSlotFramingResult{
        .status = status,
        .document = {},
        .detail = detail,
    };
}

[[nodiscard]] bool all_zero(
    std::span<const std::byte> bytes,
    std::size_t begin,
    std::size_t end) noexcept {
    if (begin > end || end > bytes.size()) {
        return false;
    }
    return std::all_of(
        bytes.begin() + static_cast<std::ptrdiff_t>(begin),
        bytes.begin() + static_cast<std::ptrdiff_t>(end),
        [](std::byte value) { return value == std::byte{0}; });
}

[[nodiscard]] bool descriptor_zero_fields_are_zero(
    std::span<const std::byte> bytes,
    std::size_t descriptor_offset) noexcept {
    return std::all_of(
        kDescriptorZeroOffsets.begin(), kDescriptorZeroOffsets.end(),
        [&](std::size_t relative) {
            return read_u32_le(bytes, descriptor_offset + relative) == 0U;
        });
}

[[nodiscard]] std::uint32_t full_mip_count(
    std::uint32_t width,
    std::uint32_t height) noexcept {
    auto dimension = std::max(width, height);
    std::uint32_t count = 1U;
    while (dimension > 1U) {
        dimension /= 2U;
        ++count;
    }
    return count;
}

[[nodiscard]] std::optional<std::uint32_t> block_payload_size(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    std::uint32_t block_bytes) noexcept {
    std::uint64_t total = 0U;
    for (std::uint32_t level = 0U; level < mip_count; ++level) {
        const auto blocks_w = std::max(1U, (width + 3U) / 4U);
        const auto blocks_h = std::max(1U, (height + 3U) / 4U);
        const auto level_bytes =
            static_cast<std::uint64_t>(blocks_w) * blocks_h * block_bytes;
        if (total > std::numeric_limits<std::uint32_t>::max() - level_bytes) {
            return std::nullopt;
        }
        total += level_bytes;
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return static_cast<std::uint32_t>(total);
}

struct DescriptorParseResult final {
    TextureSlotFramingStatus status{TextureSlotFramingStatus::invalid_dds};
    TextureSlotEntry entry;
    std::string_view detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == TextureSlotFramingStatus::ok;
    }
};

[[nodiscard]] DescriptorParseResult parse_descriptor(
    std::span<const std::byte> bytes,
    std::uint32_t texture_index,
    std::size_t descriptor_offset,
    std::uint32_t sector_span,
    std::size_t bounded_end) {
    if (!contains(
            bytes, descriptor_offset,
            TextureSlotFramingParser::k_descriptor_size)) {
        return {
            .status = TextureSlotFramingStatus::truncated_descriptor,
            .entry = {},
            .detail = "texture descriptor exceeds the physical slot span",
        };
    }

    if (descriptor_offset > std::numeric_limits<std::size_t>::max() -
            TextureSlotFramingParser::k_descriptor_size) {
        return {
            .status = TextureSlotFramingStatus::truncated_descriptor,
            .entry = {},
            .detail = "texture descriptor offset overflows host size",
        };
    }
    const auto dds_offset =
        descriptor_offset + TextureSlotFramingParser::k_descriptor_size;
    if (!contains(bytes, dds_offset, kDdsHeaderSize) ||
        dds_offset + kDdsHeaderSize > bounded_end ||
        !has_dds_magic(bytes, dds_offset) ||
        read_u32_le(bytes, dds_offset + 4U) != kDdsHeaderStructSize) {
        return {
            .status = TextureSlotFramingStatus::invalid_dds,
            .entry = {},
            .detail = "descriptor is not followed by a bounded standard DDS header",
        };
    }

    const auto width = read_u32_le(bytes, dds_offset + kDdsWidthOffset);
    const auto height = read_u32_le(bytes, dds_offset + kDdsHeightOffset);
    const auto mip_count = read_u32_le(bytes, dds_offset + kDdsMipCountOffset);
    if (width == 0U || height == 0U || width > 0xFFFFU ||
        height > 0xFFFFU || mip_count == 0U || mip_count > 0xFFU ||
        mip_count != full_mip_count(width, height)) {
        return {
            .status = TextureSlotFramingStatus::invalid_dds,
            .entry = {},
            .detail = "DDS dimensions or mip count lie outside the evidenced full-chain descriptor domain",
        };
    }

    const std::array<std::byte, 4> fourcc{
        bytes[dds_offset + kDdsFourCcOffset + 0U],
        bytes[dds_offset + kDdsFourCcOffset + 1U],
        bytes[dds_offset + kDdsFourCcOffset + 2U],
        bytes[dds_offset + kDdsFourCcOffset + 3U],
    };

    TextureCompressionKind compression{};
    std::uint32_t descriptor_format{};
    std::uint32_t encoding_low_byte{};
    std::uint32_t bytes_per_width_unit{};
    std::uint32_t block_bytes{};
    if (fourcc == std::array<std::byte, 4>{
            std::byte{'D'}, std::byte{'X'}, std::byte{'T'}, std::byte{'1'}}) {
        compression = TextureCompressionKind::dxt1;
        descriptor_format = 0U;
        encoding_low_byte = 0x86U;
        bytes_per_width_unit = 2U;
        block_bytes = 8U;
    } else if (fourcc == std::array<std::byte, 4>{
                   std::byte{'D'}, std::byte{'X'}, std::byte{'T'},
                   std::byte{'5'}}) {
        compression = TextureCompressionKind::dxt5;
        descriptor_format = 4U;
        encoding_low_byte = 0x88U;
        bytes_per_width_unit = 4U;
        block_bytes = 16U;
    } else {
        return {
            .status = TextureSlotFramingStatus::unsupported_compression,
            .entry = {},
            .detail = "only DXT1 and DXT5 descriptor mappings are corpus-confirmed",
        };
    }

    const auto expected_payload = block_payload_size(
        width, height, mip_count, block_bytes);
    if (!expected_payload.has_value()) {
        return {
            .status = TextureSlotFramingStatus::invalid_dds,
            .entry = {},
            .detail = "DDS block-compressed mip chain exceeds the descriptor size domain",
        };
    }

    const auto dds_size = read_u32_le(
        bytes, descriptor_offset + kDescriptorDdsSizeOffset);
    if (dds_size < kDdsHeaderSize ||
        static_cast<std::uint64_t>(dds_offset) + dds_size > bounded_end ||
        static_cast<std::uint64_t>(dds_offset) + dds_size > bytes.size() ||
        dds_size != kDdsHeaderSize + *expected_payload) {
        return {
            .status = TextureSlotFramingStatus::descriptor_mismatch,
            .entry = {},
            .detail = "descriptor DDS size escapes its record or disagrees with the exact DXT mip-chain size",
        };
    }

    const auto payload_size = read_u32_le(
        bytes, descriptor_offset + kDescriptorPayloadSizeOffset);
    const auto packed_dimensions = (height << 16U) | width;
    const auto expected_encoding =
        0x20000U | (mip_count << 8U) | encoding_low_byte;

    const auto packed_secondary = read_u32_le(
        bytes, descriptor_offset + kDescriptorSecondaryDimensionsOffset);
    const auto secondary_width = packed_secondary & 0xFFFFU;
    const auto secondary_height = packed_secondary >> 16U;
    const bool secondary_same =
        secondary_width == width && secondary_height == height;
    const bool secondary_half =
        secondary_width * 2U == width && secondary_height * 2U == height;
    if (secondary_width == 0U || secondary_height == 0U ||
        (!secondary_same && !secondary_half)) {
        return {
            .status = TextureSlotFramingStatus::descriptor_mismatch,
            .entry = {},
            .detail = "secondary descriptor dimensions are not the evidenced 1x/2x relation to DDS dimensions",
        };
    }

    const auto reciprocal_width_bits = std::bit_cast<std::uint32_t>(
        1.0F / static_cast<float>(secondary_width));
    const auto reciprocal_height_bits = std::bit_cast<std::uint32_t>(
        1.0F / static_cast<float>(secondary_height));
    const auto auxiliary_mode = read_u32_le(
        bytes, descriptor_offset + kDescriptorAuxModeOffset);
    const auto auxiliary_value = read_u32_le(
        bytes, descriptor_offset + kDescriptorAuxValueOffset);
    if (auxiliary_mode > 2U ||
        ((auxiliary_mode == 0U) != (auxiliary_value == 0U)) ||
        (auxiliary_mode != 0U && compression != TextureCompressionKind::dxt5)) {
        return {
            .status = TextureSlotFramingStatus::descriptor_mismatch,
            .entry = {},
            .detail = "descriptor auxiliary pair lies outside the corpus-confirmed bounded relation",
        };
    }

    if (payload_size != *expected_payload ||
        read_u32_le(bytes, descriptor_offset + kDescriptorDimensionsOffset) !=
            packed_dimensions ||
        read_u32_le(bytes, descriptor_offset + kDescriptorEncodingOffset) !=
            expected_encoding ||
        read_u32_le(bytes, descriptor_offset + kDescriptorRowBytesOffset) !=
            width * bytes_per_width_unit ||
        read_u32_le(bytes, descriptor_offset + kDescriptorFormatOffset) !=
            descriptor_format ||
        read_u32_le(bytes, descriptor_offset + kDescriptorReciprocalWidthOffset) !=
            reciprocal_width_bits ||
        read_u32_le(bytes, descriptor_offset + kDescriptorReciprocalHeightOffset) !=
            reciprocal_height_bits ||
        read_u32_le(bytes, descriptor_offset + kDescriptorConstant0cOffset) !=
            0xAAE4U ||
        read_u32_le(bytes, descriptor_offset + kDescriptorConstant14Offset) !=
            1U ||
        read_u32_le(bytes, descriptor_offset + kDescriptorConstant20Offset) !=
            0x40U ||
        read_u32_le(bytes, descriptor_offset + kDescriptorConstant68Offset) !=
            8U ||
        !descriptor_zero_fields_are_zero(bytes, descriptor_offset)) {
        return {
            .status = TextureSlotFramingStatus::descriptor_mismatch,
            .entry = {},
            .detail = "descriptor fields do not match the full corpus-confirmed structural envelope",
        };
    }

    return {
        .status = TextureSlotFramingStatus::ok,
        .entry = TextureSlotEntry{
            .texture_index = texture_index,
            .descriptor_offset = descriptor_offset,
            .dds_offset = dds_offset,
            .dds_size = dds_size,
            .dds_payload_size = payload_size,
            .width = width,
            .height = height,
            .mip_map_count = mip_count,
            .compression = compression,
            .secondary_width = secondary_width,
            .secondary_height = secondary_height,
            .auxiliary_mode = auxiliary_mode,
            .auxiliary_value = auxiliary_value,
            .sector_span = sector_span,
        },
        .detail = {},
    };
}

[[nodiscard]] TextureSlotFramingResult parse_wrapped_dds(
    std::span<const std::byte> bytes) {
    if (!has_dds_magic(bytes, TextureSlotFramingParser::k_descriptor_size)) {
        return failure(
            TextureSlotFramingStatus::not_recognized,
            "slot does not expose descriptor-plus-DDS framing");
    }

    const auto parsed = parse_descriptor(
        bytes, 0U, 0U, 0U, bytes.size());
    if (!parsed.ok()) {
        return failure(parsed.status, parsed.detail);
    }
    if (parsed.entry.dds_offset + parsed.entry.dds_size != bytes.size()) {
        return failure(
            TextureSlotFramingStatus::trailing_bytes,
            "direct wrapped DDS has bytes after the declared DDS image");
    }

    TextureSlotFramingDocument document{
        .kind = TextureSlotFramingKind::wrapped_dds,
        .slot_size = bytes.size(),
        .textures = {parsed.entry},
    };
    if (!document.valid()) {
        return failure(
            TextureSlotFramingStatus::descriptor_mismatch,
            "direct wrapped DDS decoded to an invalid framing document");
    }
    return {
        .status = TextureSlotFramingStatus::ok,
        .document = std::move(document),
        .detail = {},
    };
}

[[nodiscard]] TextureSlotFramingResult parse_bundle(
    std::span<const std::byte> bytes,
    TextureSlotFramingSafety safety) {
    if (!contains(bytes, 0U, TextureSlotFramingParser::k_bundle_header_size) ||
        !has_dds_magic(
            bytes,
            TextureSlotFramingParser::k_bundle_header_size +
                TextureSlotFramingParser::k_descriptor_size)) {
        return failure(
            TextureSlotFramingStatus::not_recognized,
            "slot does not expose the evidenced DMC3 texture-bundle framing");
    }

    const auto texture_count = read_u32_le(bytes, 0U);
    if (texture_count == 0U || texture_count > safety.max_texture_count) {
        return failure(
            TextureSlotFramingStatus::invalid_count,
            "texture-bundle count lies outside the product safety domain");
    }
    if (texture_count >
        (TextureSlotFramingParser::k_bundle_header_size - 4U) / 4U) {
        return failure(
            TextureSlotFramingStatus::truncated_header,
            "texture-bundle sector-span table exceeds the 0x800-byte header");
    }

    TextureSlotFramingDocument document{
        .kind = TextureSlotFramingKind::texture_bundle,
        .slot_size = bytes.size(),
        .textures = {},
    };
    document.textures.reserve(texture_count);

    std::size_t descriptor_offset = TextureSlotFramingParser::k_bundle_header_size;
    for (std::uint32_t index = 0U; index < texture_count; ++index) {
        const auto sector_span = read_u32_le(
            bytes, 4U + static_cast<std::size_t>(index) * 4U);
        const bool final = index + 1U == texture_count;

        std::size_t bounded_end = bytes.size();
        if (!final || sector_span != 0U) {
            if (sector_span == 0U ||
                sector_span > std::numeric_limits<std::size_t>::max() /
                    TextureSlotFramingParser::k_sector_size) {
                return failure(
                    TextureSlotFramingStatus::invalid_sector_span,
                    "texture descriptor has an invalid zero/overflow sector span");
            }
            const auto span_bytes =
                static_cast<std::size_t>(sector_span) *
                TextureSlotFramingParser::k_sector_size;
            if (descriptor_offset >
                std::numeric_limits<std::size_t>::max() - span_bytes) {
                return failure(
                    TextureSlotFramingStatus::invalid_sector_span,
                    "texture descriptor sector span overflows host size");
            }
            bounded_end = descriptor_offset + span_bytes;
            if (bounded_end > bytes.size() ||
                (final && bounded_end != bytes.size())) {
                return failure(
                    TextureSlotFramingStatus::invalid_sector_span,
                    "texture descriptor sector span escapes the bundle framing");
            }
        }

        const auto parsed = parse_descriptor(
            bytes, index, descriptor_offset, sector_span, bounded_end);
        if (!parsed.ok()) {
            return failure(parsed.status, parsed.detail);
        }
        const auto dds_end = static_cast<std::size_t>(
            parsed.entry.dds_offset + parsed.entry.dds_size);
        if (dds_end > bounded_end) {
            return failure(
                TextureSlotFramingStatus::descriptor_mismatch,
                "DDS image exceeds its texture-bundle sector span");
        }

        if (final && sector_span == 0U) {
            if (dds_end != bytes.size()) {
                return failure(
                    TextureSlotFramingStatus::trailing_bytes,
                    "zero-span final texture does not end exactly at bundle EOF");
            }
        } else if (!all_zero(bytes, dds_end, bounded_end)) {
            return failure(
                TextureSlotFramingStatus::nonzero_alignment_padding,
                "texture-bundle alignment region contains non-zero bytes");
        }

        document.textures.push_back(parsed.entry);
        if (!final) {
            descriptor_offset = bounded_end;
        }
    }

    if (!document.valid()) {
        return failure(
            TextureSlotFramingStatus::descriptor_mismatch,
            "texture bundle decoded to an invalid framing document");
    }
    return {
        .status = TextureSlotFramingStatus::ok,
        .document = std::move(document),
        .detail = {},
    };
}

} // namespace

bool TextureSlotEntry::valid(std::uint64_t slot_size) const noexcept {
    const bool secondary_same =
        secondary_width == width && secondary_height == height;
    const bool secondary_half =
        secondary_width * 2U == width && secondary_height * 2U == height;
    return dds_size >= kDdsHeaderSize &&
        dds_payload_size == dds_size - kDdsHeaderSize &&
        width != 0U && height != 0U && mip_map_count != 0U &&
        secondary_width != 0U && secondary_height != 0U &&
        (secondary_same || secondary_half) && auxiliary_mode <= 2U &&
        ((auxiliary_mode == 0U) == (auxiliary_value == 0U)) &&
        (auxiliary_mode == 0U || compression == TextureCompressionKind::dxt5) &&
        descriptor_offset < dds_offset &&
        dds_offset - descriptor_offset == TextureSlotFramingParser::k_descriptor_size &&
        dds_offset <= slot_size && dds_size <= slot_size - dds_offset;
}

bool TextureSlotFramingDocument::valid() const noexcept {
    if (slot_size == 0U || textures.empty()) {
        return false;
    }
    for (std::size_t index = 0U; index < textures.size(); ++index) {
        if (textures[index].texture_index != index ||
            !textures[index].valid(slot_size)) {
            return false;
        }
        if (index != 0U &&
            textures[index - 1U].descriptor_offset >=
                textures[index].descriptor_offset) {
            return false;
        }
    }
    return true;
}

bool TextureSlotFramingResult::ok() const noexcept {
    return status == TextureSlotFramingStatus::ok && document.valid();
}

TextureSlotFramingResult TextureSlotFramingParser::parse(
    std::span<const std::byte> bytes,
    TextureSlotFramingSafety safety) {
    if (safety.max_texture_count == 0U) {
        return failure(
            TextureSlotFramingStatus::invalid_count,
            "texture framing parser requires a non-zero product count budget");
    }

    const auto wrapped = parse_wrapped_dds(bytes);
    if (wrapped.status != TextureSlotFramingStatus::not_recognized) {
        return wrapped;
    }
    return parse_bundle(bytes, safety);
}

} // namespace dmc::rengine::profiles::dmc3
