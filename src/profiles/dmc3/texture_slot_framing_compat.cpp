#include "dmc_rengine/profiles/dmc3/texture_slot_framing_compat.hpp"

#include "dmc_rengine/codecs/dds_bc.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::size_t kDdsHeaderSize = codecs::dds_bc::header_size;
constexpr std::size_t kDdsFlagsOffset = 0x08U;
constexpr std::size_t kDdsHeightOffset = 0x0CU;
constexpr std::size_t kDdsWidthOffset = 0x10U;
constexpr std::size_t kDdsLinearSizeOffset = 0x14U;
constexpr std::size_t kDdsDepthOffset = 0x18U;
constexpr std::size_t kDdsMipCountOffset = 0x1CU;
constexpr std::size_t kDdsPixelFormatSizeOffset = 0x4CU;
constexpr std::size_t kDdsPixelFormatFlagsOffset = 0x50U;
constexpr std::size_t kDdsCapsOffset = 0x6CU;
constexpr std::size_t kDdsCaps2Offset = 0x70U;

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

constexpr std::uint32_t kSingleLevelDdsFlags = 0x00081007U;
constexpr std::uint32_t kSingleLevelDdsCaps = 0x00001000U;
constexpr std::uint32_t kLegacyBundleEncoding = 0x00020185U;
constexpr std::uint32_t kLegacyWrappedEncoding = 0x000201A5U;

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

[[nodiscard]] bool read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset,
    std::uint32_t& value) noexcept {
    if (!contains(bytes, offset, 4U)) return false;
    value = std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
    return true;
}

[[nodiscard]] bool all_zero(
    std::span<const std::byte> bytes,
    std::size_t begin,
    std::size_t end) noexcept {
    if (begin > end || end > bytes.size()) return false;
    return std::all_of(
        bytes.begin() + static_cast<std::ptrdiff_t>(begin),
        bytes.begin() + static_cast<std::ptrdiff_t>(end),
        [](std::byte value) { return value == std::byte{0}; });
}

[[nodiscard]] bool descriptor_zero_fields_are_zero(
    std::span<const std::byte> bytes,
    std::size_t descriptor_offset) noexcept {
    for (const auto relative : kDescriptorZeroOffsets) {
        std::uint32_t value{};
        if (!read_u32_le(bytes, descriptor_offset + relative, value) || value != 0U) {
            return false;
        }
    }
    return true;
}

struct SingleLevelDds final {
    codecs::dds_bc::Document document;
    std::uint32_t raw_mip_count{};
};

[[nodiscard]] bool parse_single_level_dxt5(
    std::span<const std::byte> bytes,
    std::size_t dds_offset,
    std::size_t bounded_end,
    SingleLevelDds& output) noexcept {
    if (dds_offset > bounded_end || bounded_end > bytes.size() ||
        bounded_end - dds_offset < kDdsHeaderSize) {
        return false;
    }

    const auto bounded = bytes.subspan(dds_offset, bounded_end - dds_offset);
    const auto parsed = codecs::dds_bc::parse(bounded);
    if (!parsed.ok() ||
        parsed.document.compression != codecs::dds_bc::Compression::dxt5 ||
        parsed.document.mip_count != 1U ||
        parsed.document.total_size > bounded.size()) {
        return false;
    }

    std::uint32_t flags{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t linear_size{};
    std::uint32_t depth{};
    std::uint32_t raw_mip_count{};
    std::uint32_t pixel_format_size{};
    std::uint32_t pixel_format_flags{};
    std::uint32_t caps{};
    std::uint32_t caps2{};
    if (!read_u32_le(bytes, dds_offset + kDdsFlagsOffset, flags) ||
        !read_u32_le(bytes, dds_offset + kDdsWidthOffset, width) ||
        !read_u32_le(bytes, dds_offset + kDdsHeightOffset, height) ||
        !read_u32_le(bytes, dds_offset + kDdsLinearSizeOffset, linear_size) ||
        !read_u32_le(bytes, dds_offset + kDdsDepthOffset, depth) ||
        !read_u32_le(bytes, dds_offset + kDdsMipCountOffset, raw_mip_count) ||
        !read_u32_le(bytes, dds_offset + kDdsPixelFormatSizeOffset, pixel_format_size) ||
        !read_u32_le(bytes, dds_offset + kDdsPixelFormatFlagsOffset, pixel_format_flags) ||
        !read_u32_le(bytes, dds_offset + kDdsCapsOffset, caps) ||
        !read_u32_le(bytes, dds_offset + kDdsCaps2Offset, caps2)) {
        return false;
    }

    if (flags != kSingleLevelDdsFlags || raw_mip_count != 0U || depth != 0U ||
        pixel_format_size != 32U || pixel_format_flags != 4U ||
        caps != kSingleLevelDdsCaps || caps2 != 0U ||
        width != parsed.document.width || height != parsed.document.height ||
        linear_size != parsed.document.payload_size) {
        return false;
    }

    output.document = parsed.document;
    output.raw_mip_count = raw_mip_count;
    return true;
}

[[nodiscard]] bool read_descriptor_fields(
    std::span<const std::byte> bytes,
    std::size_t descriptor_offset,
    std::array<std::uint32_t, 14>& fields) noexcept {
    constexpr std::array<std::size_t, 14> offsets{
        kDescriptorEncodingOffset,
        kDescriptorConstant0cOffset,
        kDescriptorDimensionsOffset,
        kDescriptorConstant14Offset,
        kDescriptorRowBytesOffset,
        kDescriptorConstant20Offset,
        kDescriptorPayloadSizeOffset,
        kDescriptorAuxModeOffset,
        kDescriptorAuxValueOffset,
        kDescriptorSecondaryDimensionsOffset,
        kDescriptorReciprocalWidthOffset,
        kDescriptorReciprocalHeightOffset,
        kDescriptorFormatOffset,
        kDescriptorDdsSizeOffset,
    };
    for (std::size_t index = 0U; index < offsets.size(); ++index) {
        if (!read_u32_le(bytes, descriptor_offset + offsets[index], fields[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] TextureSlotFramingResult make_result(
    TextureSlotFramingKind kind,
    std::size_t slot_size,
    std::uint32_t sector_span,
    std::size_t descriptor_offset,
    const SingleLevelDds& dds,
    std::uint32_t secondary_width,
    std::uint32_t secondary_height) {
    const auto dds_offset = descriptor_offset + TextureSlotFramingParser::k_descriptor_size;
    TextureSlotFramingDocument document{
        .kind = kind,
        .slot_size = slot_size,
        .textures = {
            TextureSlotEntry{
                .texture_index = 0U,
                .descriptor_offset = descriptor_offset,
                .dds_offset = dds_offset,
                .dds_size = dds.document.total_size,
                .dds_payload_size = dds.document.payload_size,
                .width = dds.document.width,
                .height = dds.document.height,
                .mip_map_count = dds.document.mip_count,
                .compression = TextureCompressionKind::dxt5,
                .secondary_width = secondary_width,
                .secondary_height = secondary_height,
                .auxiliary_mode = 0U,
                .auxiliary_value = 0U,
                .sector_span = sector_span,
            },
        },
    };
    if (!document.valid()) {
        return TextureSlotFramingResult{
            .status = TextureSlotFramingStatus::descriptor_mismatch,
            .document = {},
            .detail = "legacy single-level texture decoded to an invalid framing document",
        };
    }
    return TextureSlotFramingResult{
        .status = TextureSlotFramingStatus::ok,
        .document = std::move(document),
        .detail = {},
    };
}

[[nodiscard]] TextureSlotFramingResult parse_legacy_bundle(
    std::span<const std::byte> bytes,
    TextureSlotFramingSafety safety) {
    if (safety.max_texture_count == 0U ||
        bytes.size() < TextureSlotFramingParser::k_bundle_header_size +
            TextureSlotFramingParser::k_descriptor_size + kDdsHeaderSize) {
        return {};
    }

    std::uint32_t texture_count{};
    std::uint32_t sector_span{};
    if (!read_u32_le(bytes, 0U, texture_count) || texture_count != 1U ||
        texture_count > safety.max_texture_count ||
        !read_u32_le(bytes, 4U, sector_span) || sector_span == 0U ||
        !all_zero(bytes, 8U, TextureSlotFramingParser::k_bundle_header_size) ||
        sector_span > std::numeric_limits<std::size_t>::max() /
            TextureSlotFramingParser::k_sector_size) {
        return {};
    }

    const auto payload_span = static_cast<std::size_t>(sector_span) *
        TextureSlotFramingParser::k_sector_size;
    if (payload_span > std::numeric_limits<std::size_t>::max() -
            TextureSlotFramingParser::k_bundle_header_size) {
        return {};
    }
    const auto bounded_end = TextureSlotFramingParser::k_bundle_header_size + payload_span;
    if (bounded_end != bytes.size()) return {};

    const auto descriptor_offset = TextureSlotFramingParser::k_bundle_header_size;
    const auto dds_offset = descriptor_offset + TextureSlotFramingParser::k_descriptor_size;
    SingleLevelDds dds;
    if (!parse_single_level_dxt5(bytes, dds_offset, bounded_end, dds)) return {};

    std::array<std::uint32_t, 14> fields{};
    std::uint32_t constant68{};
    if (!read_descriptor_fields(bytes, descriptor_offset, fields) ||
        !read_u32_le(bytes, descriptor_offset + kDescriptorConstant68Offset, constant68) ||
        !descriptor_zero_fields_are_zero(bytes, descriptor_offset)) {
        return {};
    }

    const auto packed_dimensions = (dds.document.height << 16U) | dds.document.width;
    const auto reciprocal_width = std::bit_cast<std::uint32_t>(
        1.0F / static_cast<float>(dds.document.width));
    const auto reciprocal_height = std::bit_cast<std::uint32_t>(
        1.0F / static_cast<float>(dds.document.height));

    if (dds.document.width > 0xFFFFU || dds.document.height > 0xFFFFU ||
        fields[0] != kLegacyBundleEncoding || fields[1] != 0xAAE4U ||
        fields[2] != packed_dimensions || fields[3] != 1U ||
        fields[4] != 0U || fields[5] != 0x40U || fields[6] != 0U ||
        fields[7] != 0U || fields[8] != 0U || fields[9] != packed_dimensions ||
        fields[10] != reciprocal_width || fields[11] != reciprocal_height ||
        fields[12] != 4U || fields[13] != dds.document.total_size ||
        constant68 != 8U) {
        return {};
    }

    const auto dds_end = dds_offset + static_cast<std::size_t>(dds.document.total_size);
    if (!all_zero(bytes, dds_end, bounded_end)) return {};

    return make_result(
        TextureSlotFramingKind::texture_bundle,
        bytes.size(), sector_span, descriptor_offset, dds,
        dds.document.width, dds.document.height);
}

[[nodiscard]] TextureSlotFramingResult parse_legacy_wrapped(
    std::span<const std::byte> bytes) {
    if (bytes.size() < TextureSlotFramingParser::k_descriptor_size + kDdsHeaderSize) {
        return {};
    }

    constexpr std::size_t descriptor_offset = 0U;
    constexpr std::size_t dds_offset = TextureSlotFramingParser::k_descriptor_size;
    SingleLevelDds dds;
    if (!parse_single_level_dxt5(bytes, dds_offset, bytes.size(), dds) ||
        dds_offset + static_cast<std::size_t>(dds.document.total_size) != bytes.size()) {
        return {};
    }

    std::array<std::uint32_t, 14> fields{};
    std::uint32_t constant68{};
    if (!read_descriptor_fields(bytes, descriptor_offset, fields) ||
        !read_u32_le(bytes, descriptor_offset + kDescriptorConstant68Offset, constant68) ||
        !descriptor_zero_fields_are_zero(bytes, descriptor_offset)) {
        return {};
    }

    const auto descriptor_width = fields[2] & 0xFFFFU;
    const auto descriptor_height = fields[2] >> 16U;
    if (descriptor_width == 0U || descriptor_height == 0U ||
        descriptor_width > std::numeric_limits<std::uint32_t>::max() / 2U ||
        descriptor_height > std::numeric_limits<std::uint32_t>::max() / 2U) {
        return {};
    }
    const auto secondary_width = fields[9] & 0xFFFFU;
    const auto secondary_height = fields[9] >> 16U;
    const auto reciprocal_width = std::bit_cast<std::uint32_t>(
        1.0F / static_cast<float>(descriptor_width));
    const auto reciprocal_height = std::bit_cast<std::uint32_t>(
        1.0F / static_cast<float>(descriptor_height));

    if (fields[0] != kLegacyWrappedEncoding || fields[1] != 0xAAE4U ||
        fields[3] != 1U || fields[4] != descriptor_width * 4U ||
        fields[5] != 0x40U || fields[6] != dds.document.payload_size ||
        fields[7] != 0U || fields[8] != 0U ||
        secondary_width != descriptor_width || secondary_height != descriptor_height ||
        fields[10] != reciprocal_width || fields[11] != reciprocal_height ||
        fields[12] != 5U || fields[13] != dds.document.total_size ||
        constant68 != 8U ||
        dds.document.width != descriptor_width * 2U ||
        dds.document.height != descriptor_height * 2U) {
        return {};
    }

    return make_result(
        TextureSlotFramingKind::wrapped_dds,
        bytes.size(), 0U, descriptor_offset, dds,
        descriptor_width, descriptor_height);
}

} // namespace

TextureSlotFramingReadResult TextureSlotFramingReader::parse(
    std::span<const std::byte> bytes,
    TextureSlotFramingSafety safety) {
    auto canonical = TextureSlotFramingParser::parse(bytes, safety);
    if (canonical.ok()) {
        return TextureSlotFramingReadResult{
            .framing = std::move(canonical),
            .variant = TextureSlotReadVariant::canonical,
        };
    }

    auto bundle = parse_legacy_bundle(bytes, safety);
    if (bundle.ok()) {
        return TextureSlotFramingReadResult{
            .framing = std::move(bundle),
            .variant = TextureSlotReadVariant::legacy_single_mip_bundle_dxt5,
        };
    }

    auto wrapped = parse_legacy_wrapped(bytes);
    if (wrapped.ok()) {
        return TextureSlotFramingReadResult{
            .framing = std::move(wrapped),
            .variant = TextureSlotReadVariant::legacy_single_mip_wrapped_dxt5,
        };
    }

    return TextureSlotFramingReadResult{
        .framing = std::move(canonical),
        .variant = TextureSlotReadVariant::canonical,
    };
}

} // namespace dmc::rengine::profiles::dmc3
