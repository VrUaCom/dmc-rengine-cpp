#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::uint32_t kDdsStructSize = 124U;
constexpr std::uint32_t kDdsFlags = 0x000A1007U;
constexpr std::uint32_t kPixelFormatSize = 32U;
constexpr std::uint32_t kPixelFormatFlags = 4U;
constexpr std::uint32_t kCaps = 0x00401008U;
constexpr std::uint32_t kDxt1LinearSize = 0x00010000U;
constexpr std::uint32_t kDxt5LinearSize = 0x00020000U;

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

void write_u32_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] bool power_of_two(std::uint32_t value) noexcept {
    return value != 0U && (value & (value - 1U)) == 0U;
}

[[nodiscard]] bool dimensions_supported(
    std::uint32_t width,
    std::uint32_t height,
    Dmc3DdsSafety safety) noexcept {
    if (safety.min_dimension == 0U || safety.max_dimension < safety.min_dimension) {
        return false;
    }
    return power_of_two(width) && power_of_two(height) &&
        width >= safety.min_dimension && height >= safety.min_dimension &&
        width <= safety.max_dimension && height <= safety.max_dimension;
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

[[nodiscard]] std::uint32_t block_bytes(Dmc3DdsCompression compression) noexcept {
    return compression == Dmc3DdsCompression::dxt1 ? 8U : 16U;
}

[[nodiscard]] std::uint32_t linear_size(Dmc3DdsCompression compression) noexcept {
    return compression == Dmc3DdsCompression::dxt1
        ? kDxt1LinearSize
        : kDxt5LinearSize;
}

[[nodiscard]] std::array<std::byte, 4> fourcc(
    Dmc3DdsCompression compression) noexcept {
    return compression == Dmc3DdsCompression::dxt1
        ? std::array<std::byte, 4>{
              std::byte{'D'}, std::byte{'X'}, std::byte{'T'}, std::byte{'1'}}
        : std::array<std::byte, 4>{
              std::byte{'D'}, std::byte{'X'}, std::byte{'T'}, std::byte{'5'}};
}

[[nodiscard]] bool expected_payload_size(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    Dmc3DdsCompression compression,
    std::uint32_t& output) noexcept {
    std::uint64_t total = 0U;
    const auto bytes_per_block = block_bytes(compression);
    for (std::uint32_t level = 0U; level < mip_count; ++level) {
        const auto blocks_w = std::max(1U, (width + 3U) / 4U);
        const auto blocks_h = std::max(1U, (height + 3U) / 4U);
        const auto level_bytes =
            static_cast<std::uint64_t>(blocks_w) * blocks_h * bytes_per_block;
        if (total > std::numeric_limits<std::uint32_t>::max() - level_bytes) {
            return false;
        }
        total += level_bytes;
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    output = static_cast<std::uint32_t>(total);
    return true;
}

[[nodiscard]] std::vector<std::byte> canonical_header(
    std::uint32_t width,
    std::uint32_t height,
    Dmc3DdsCompression compression) {
    std::vector<std::byte> header(Dmc3DdsProfile::k_header_size, std::byte{0});
    header[0U] = std::byte{'D'};
    header[1U] = std::byte{'D'};
    header[2U] = std::byte{'S'};
    header[3U] = std::byte{' '};
    write_u32_le(header, 4U, kDdsStructSize);
    write_u32_le(header, 8U, kDdsFlags);
    write_u32_le(header, 12U, height);
    write_u32_le(header, 16U, width);
    write_u32_le(header, 20U, linear_size(compression));
    write_u32_le(header, 24U, 0U);
    write_u32_le(header, 28U, full_mip_count(width, height));
    write_u32_le(header, 76U, kPixelFormatSize);
    write_u32_le(header, 80U, kPixelFormatFlags);
    const auto code = fourcc(compression);
    std::copy(code.begin(), code.end(), header.begin() + 84);
    write_u32_le(header, 108U, kCaps);
    return header;
}

[[nodiscard]] Dmc3DdsParseResult parse_failure(
    Dmc3DdsStatus status,
    std::string_view detail) noexcept {
    return Dmc3DdsParseResult{
        .status = status,
        .document = {},
        .detail = detail,
    };
}

[[nodiscard]] Dmc3DdsBuildResult build_failure(
    Dmc3DdsStatus status,
    std::string_view detail) {
    return Dmc3DdsBuildResult{
        .status = status,
        .document = {},
        .bytes = {},
        .detail = detail,
    };
}

} // namespace

bool Dmc3DdsDocument::valid() const noexcept {
    return width != 0U && height != 0U && mip_map_count != 0U &&
        payload_size != 0U && total_size == Dmc3DdsProfile::k_header_size + payload_size;
}

bool Dmc3DdsParseResult::ok() const noexcept {
    return status == Dmc3DdsStatus::ok && document.valid();
}

bool Dmc3DdsBuildResult::ok() const noexcept {
    return status == Dmc3DdsStatus::ok && document.valid() &&
        bytes.size() == document.total_size;
}

Dmc3DdsParseResult Dmc3DdsProfile::parse(
    std::span<const std::byte> bytes,
    Dmc3DdsSafety safety) {
    if (bytes.size() < k_header_size) {
        return parse_failure(Dmc3DdsStatus::truncated, "DDS is shorter than the 128-byte header");
    }
    if (bytes[0U] != std::byte{'D'} || bytes[1U] != std::byte{'D'} ||
        bytes[2U] != std::byte{'S'} || bytes[3U] != std::byte{' '}) {
        return parse_failure(Dmc3DdsStatus::invalid_magic, "DDS magic is not present");
    }

    const auto width = read_u32_le(bytes, 16U);
    const auto height = read_u32_le(bytes, 12U);
    if (!dimensions_supported(width, height, safety)) {
        return parse_failure(
            Dmc3DdsStatus::unsupported_dimensions,
            "DDS dimensions lie outside the Pass 81 product authoring envelope");
    }

    Dmc3DdsCompression compression{};
    const std::array<std::byte, 4> code{
        bytes[84U], bytes[85U], bytes[86U], bytes[87U]};
    if (code == fourcc(Dmc3DdsCompression::dxt1)) {
        compression = Dmc3DdsCompression::dxt1;
    } else if (code == fourcc(Dmc3DdsCompression::dxt5)) {
        compression = Dmc3DdsCompression::dxt5;
    } else {
        return parse_failure(
            Dmc3DdsStatus::unsupported_compression,
            "Only DXT1 and DXT5 are confirmed by the descriptor-backed corpus");
    }

    const auto mip_count = read_u32_le(bytes, 28U);
    if (mip_count != full_mip_count(width, height)) {
        return parse_failure(
            Dmc3DdsStatus::invalid_mip_chain,
            "DDS mip count is not the corpus-confirmed complete mip chain");
    }

    const auto expected_header = canonical_header(width, height, compression);
    if (!std::equal(expected_header.begin(), expected_header.end(), bytes.begin())) {
        return parse_failure(
            Dmc3DdsStatus::invalid_header,
            "DDS header differs from the exact Pass 81 DMC3 canonical profile");
    }

    std::uint32_t payload_size = 0U;
    if (!expected_payload_size(width, height, mip_count, compression, payload_size)) {
        return parse_failure(
            Dmc3DdsStatus::invalid_payload_size,
            "DDS full mip chain exceeds the supported size domain");
    }
    if (bytes.size() != k_header_size + static_cast<std::size_t>(payload_size)) {
        return parse_failure(
            Dmc3DdsStatus::invalid_payload_size,
            "DDS byte size does not equal the exact full DXT mip-chain size");
    }

    return Dmc3DdsParseResult{
        .status = Dmc3DdsStatus::ok,
        .document = Dmc3DdsDocument{
            .width = width,
            .height = height,
            .mip_map_count = mip_count,
            .compression = compression,
            .payload_size = payload_size,
            .total_size = static_cast<std::uint32_t>(bytes.size()),
        },
        .detail = {},
    };
}

Dmc3DdsBuildResult Dmc3DdsProfile::build(
    std::uint32_t width,
    std::uint32_t height,
    Dmc3DdsCompression compression,
    std::span<const std::byte> payload,
    Dmc3DdsSafety safety) {
    if (!dimensions_supported(width, height, safety)) {
        return build_failure(
            Dmc3DdsStatus::unsupported_dimensions,
            "Requested dimensions lie outside the Pass 81 product authoring envelope");
    }

    const auto mip_count = full_mip_count(width, height);
    std::uint32_t payload_size = 0U;
    if (!expected_payload_size(width, height, mip_count, compression, payload_size) ||
        payload.size() != payload_size) {
        return build_failure(
            Dmc3DdsStatus::invalid_payload_size,
            "Authored payload does not contain the exact full DXT mip chain");
    }

    auto bytes = canonical_header(width, height, compression);
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    const auto parsed = parse(
        std::span<const std::byte>{bytes.data(), bytes.size()}, safety);
    if (!parsed.ok()) {
        return build_failure(parsed.status, parsed.detail);
    }

    return Dmc3DdsBuildResult{
        .status = Dmc3DdsStatus::ok,
        .document = parsed.document,
        .bytes = std::move(bytes),
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
