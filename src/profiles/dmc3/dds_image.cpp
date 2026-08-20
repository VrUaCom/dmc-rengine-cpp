#include "dmc_rengine/profiles/dmc3/dds_image.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <optional>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::size_t kFlagsOffset = 8U;
constexpr std::size_t kHeightOffset = 12U;
constexpr std::size_t kWidthOffset = 16U;
constexpr std::size_t kPitchOrLinearSizeOffset = 20U;
constexpr std::size_t kDepthOffset = 24U;
constexpr std::size_t kMipCountOffset = 28U;
constexpr std::size_t kReserved1Offset = 32U;
constexpr std::size_t kReserved1Count = 11U;
constexpr std::size_t kPixelFormatSizeOffset = 76U;
constexpr std::size_t kPixelFormatFlagsOffset = 80U;
constexpr std::size_t kFourCcOffset = 84U;
constexpr std::size_t kRgbBitCountOffset = 88U;
constexpr std::size_t kRMaskOffset = 92U;
constexpr std::size_t kGMaskOffset = 96U;
constexpr std::size_t kBMaskOffset = 100U;
constexpr std::size_t kAMaskOffset = 104U;
constexpr std::size_t kCapsOffset = 108U;
constexpr std::size_t kCaps2Offset = 112U;
constexpr std::size_t kCaps3Offset = 116U;
constexpr std::size_t kCaps4Offset = 120U;
constexpr std::size_t kReserved2Offset = 124U;

[[nodiscard]] std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] bool is_power_of_two(std::uint32_t value) noexcept {
    return value != 0U && (value & (value - 1U)) == 0U;
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
        if (level_bytes > std::numeric_limits<std::uint32_t>::max() ||
            total > std::numeric_limits<std::uint32_t>::max() - level_bytes) {
            return std::nullopt;
        }
        total += level_bytes;
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return static_cast<std::uint32_t>(total);
}

[[nodiscard]] DdsImageResult failure(
    DdsImageStatus status,
    std::string_view detail) noexcept {
    return DdsImageResult{
        .status = status,
        .document = {},
        .detail = detail,
    };
}

[[nodiscard]] bool reserved_fields_are_zero(
    std::span<const std::byte> bytes) noexcept {
    for (std::size_t index = 0U; index < kReserved1Count; ++index) {
        if (read_u32_le(bytes, kReserved1Offset + index * 4U) != 0U) {
            return false;
        }
    }
    return read_u32_le(bytes, kReserved2Offset) == 0U;
}

} // namespace

bool DdsImageDocument::valid() const noexcept {
    return width != 0U && height != 0U && mip_map_count != 0U &&
        payload_size != 0U && total_size ==
            DdsImageParser::k_file_header_size +
                static_cast<std::uint64_t>(payload_size);
}

bool DdsImageResult::ok() const noexcept {
    return status == DdsImageStatus::ok && document.valid();
}

DdsImageResult DdsImageParser::parse(
    std::span<const std::byte> bytes) noexcept {
    if (bytes.size() < k_file_header_size) {
        return failure(
            DdsImageStatus::truncated,
            "DDS image is smaller than the 128-byte magic+header envelope");
    }
    if (bytes[0U] != std::byte{'D'} || bytes[1U] != std::byte{'D'} ||
        bytes[2U] != std::byte{'S'} || bytes[3U] != std::byte{' '}) {
        return failure(DdsImageStatus::invalid_magic, "DDS magic mismatch");
    }
    if (read_u32_le(bytes, 4U) != k_header_struct_size) {
        return failure(
            DdsImageStatus::invalid_header_size,
            "DDS_HEADER size is not the corpus-confirmed 124 bytes");
    }
    if (read_u32_le(bytes, kFlagsOffset) != k_required_flags) {
        return failure(
            DdsImageStatus::invalid_flags,
            "DDS flags differ from the corpus-confirmed DMC3 envelope");
    }

    const auto width = read_u32_le(bytes, kWidthOffset);
    const auto height = read_u32_le(bytes, kHeightOffset);
    const auto mip_count = read_u32_le(bytes, kMipCountOffset);
    if (!is_power_of_two(width) || !is_power_of_two(height) ||
        width > 0xFFFFU || height > 0xFFFFU) {
        return failure(
            DdsImageStatus::invalid_dimensions,
            "DDS dimensions are not positive power-of-two values inside the structural domain");
    }
    if (mip_count == 0U || mip_count > 0xFFU ||
        mip_count != full_mip_count(width, height)) {
        return failure(
            DdsImageStatus::invalid_mip_chain,
            "DDS mip count is not the complete power-of-two chain observed in the corpus");
    }

    if (read_u32_le(bytes, kPixelFormatSizeOffset) != k_pixel_format_size ||
        read_u32_le(bytes, kPixelFormatFlagsOffset) !=
            k_pixel_format_fourcc_flag ||
        read_u32_le(bytes, kRgbBitCountOffset) != 0U ||
        read_u32_le(bytes, kRMaskOffset) != 0U ||
        read_u32_le(bytes, kGMaskOffset) != 0U ||
        read_u32_le(bytes, kBMaskOffset) != 0U ||
        read_u32_le(bytes, kAMaskOffset) != 0U) {
        return failure(
            DdsImageStatus::invalid_pixel_format,
            "DDS_PIXELFORMAT fields differ from the corpus-confirmed FourCC envelope");
    }

    DdsCompressionKind compression{};
    std::uint32_t block_bytes{};
    std::uint32_t standard_linear_size{};
    const std::array<std::byte, 4> fourcc{
        bytes[kFourCcOffset + 0U], bytes[kFourCcOffset + 1U],
        bytes[kFourCcOffset + 2U], bytes[kFourCcOffset + 3U],
    };
    if (fourcc == std::array<std::byte, 4>{
            std::byte{'D'}, std::byte{'X'}, std::byte{'T'}, std::byte{'1'}}) {
        compression = DdsCompressionKind::dxt1;
        block_bytes = 8U;
        standard_linear_size = k_standard_dxt1_linear_size;
    } else if (fourcc == std::array<std::byte, 4>{
                   std::byte{'D'}, std::byte{'X'}, std::byte{'T'},
                   std::byte{'5'}}) {
        compression = DdsCompressionKind::dxt5;
        block_bytes = 16U;
        standard_linear_size = k_standard_dxt5_linear_size;
    } else {
        return failure(
            DdsImageStatus::unsupported_compression,
            "Only DXT1 and DXT5 DDS images are present in the preserved DMC3 corpus");
    }

    const auto payload_size = block_payload_size(
        width, height, mip_count, block_bytes);
    if (!payload_size.has_value()) {
        return failure(
            DdsImageStatus::size_mismatch,
            "DDS DXT mip chain exceeds the bounded 32-bit payload domain");
    }
    if (bytes.size() !=
        k_file_header_size + static_cast<std::uint64_t>(*payload_size)) {
        return failure(
            DdsImageStatus::size_mismatch,
            "DDS byte length does not equal the exact full DXT mip-chain size");
    }

    if (!reserved_fields_are_zero(bytes)) {
        return failure(
            DdsImageStatus::nonzero_reserved_fields,
            "DDS reserved fields are non-zero outside the preserved corpus envelope");
    }
    if (read_u32_le(bytes, kCapsOffset) != k_required_caps ||
        read_u32_le(bytes, kCaps2Offset) != 0U ||
        read_u32_le(bytes, kCaps3Offset) != 0U ||
        read_u32_le(bytes, kCaps4Offset) != 0U) {
        return failure(
            DdsImageStatus::invalid_caps,
            "DDS caps fields differ from the corpus-confirmed DMC3 envelope");
    }

    const auto depth = read_u32_le(bytes, kDepthOffset);
    const auto linear_size = read_u32_le(bytes, kPitchOrLinearSizeOffset);
    DdsHeaderProfile profile{};
    if (depth == 0U && linear_size == standard_linear_size) {
        profile = DdsHeaderProfile::standard_corpus;
    } else if (
        compression == DdsCompressionKind::dxt5 && width == 1024U &&
        height == 2048U && mip_count == 12U && depth == 1U &&
        linear_size == 0x00200000U) {
        profile = DdsHeaderProfile::observed_depth1_exception;
    } else if (depth != 0U) {
        return failure(
            DdsImageStatus::invalid_depth,
            "DDS depth differs from the standard corpus profile and is not the one observed exception");
    } else {
        return failure(
            DdsImageStatus::invalid_linear_size,
            "DDS pitchOrLinearSize differs from the corpus-confirmed profile value");
    }

    DdsImageDocument document{
        .profile = profile,
        .compression = compression,
        .width = width,
        .height = height,
        .mip_map_count = mip_count,
        .pitch_or_linear_size = linear_size,
        .depth = depth,
        .payload_size = *payload_size,
        .total_size = bytes.size(),
    };
    if (!document.valid()) {
        return failure(
            DdsImageStatus::size_mismatch,
            "DDS parser produced an internally invalid structural document");
    }
    return DdsImageResult{
        .status = DdsImageStatus::ok,
        .document = document,
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
