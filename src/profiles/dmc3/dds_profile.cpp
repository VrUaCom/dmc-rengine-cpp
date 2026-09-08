#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"

#include "dmc_rengine/codecs/dds_bc.hpp"

#include <algorithm>
#include <array>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::uint32_t kDdsStructSize = 124U;
constexpr std::uint32_t kDdsFlags = 0x000A1007U;
constexpr std::uint32_t kPixelFormatSize = 32U;
constexpr std::uint32_t kPixelFormatFlags = 4U;
constexpr std::uint32_t kCaps = 0x00401008U;
constexpr std::uint32_t kDxt1LinearSize = 0x00010000U;
constexpr std::uint32_t kDxt5LinearSize = 0x00020000U;

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

[[nodiscard]] codecs::dds_bc::Compression to_codec(
    Dmc3DdsCompression compression) noexcept {
    return compression == Dmc3DdsCompression::dxt1
        ? codecs::dds_bc::Compression::dxt1
        : codecs::dds_bc::Compression::dxt5;
}

[[nodiscard]] Dmc3DdsCompression from_codec(
    codecs::dds_bc::Compression compression) noexcept {
    return compression == codecs::dds_bc::Compression::dxt1
        ? Dmc3DdsCompression::dxt1
        : Dmc3DdsCompression::dxt5;
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
    write_u32_le(
        header,
        28U,
        codecs::dds_bc::maximum_mip_count(width, height));
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

[[nodiscard]] Dmc3DdsStatus map_codec_status(
    codecs::dds_bc::Status status) noexcept {
    switch (status) {
    case codecs::dds_bc::Status::ok: return Dmc3DdsStatus::ok;
    case codecs::dds_bc::Status::truncated: return Dmc3DdsStatus::truncated;
    case codecs::dds_bc::Status::invalid_magic: return Dmc3DdsStatus::invalid_magic;
    case codecs::dds_bc::Status::unsupported_compression:
        return Dmc3DdsStatus::unsupported_compression;
    case codecs::dds_bc::Status::invalid_dimensions:
        return Dmc3DdsStatus::unsupported_dimensions;
    case codecs::dds_bc::Status::invalid_mip_count:
        return Dmc3DdsStatus::invalid_mip_chain;
    case codecs::dds_bc::Status::payload_overflow:
    case codecs::dds_bc::Status::payload_out_of_bounds:
        return Dmc3DdsStatus::invalid_payload_size;
    case codecs::dds_bc::Status::invalid_header:
        return Dmc3DdsStatus::invalid_header;
    }
    return Dmc3DdsStatus::invalid_header;
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
    const auto portable = codecs::dds_bc::parse(bytes);
    if (!portable.ok()) {
        return parse_failure(map_codec_status(portable.status), portable.detail);
    }

    const auto& image = portable.document;
    if (!dimensions_supported(image.width, image.height, safety)) {
        return parse_failure(
            Dmc3DdsStatus::unsupported_dimensions,
            "DDS dimensions lie outside the Pass 81 product authoring envelope");
    }
    if (image.mip_count != codecs::dds_bc::maximum_mip_count(
            image.width, image.height)) {
        return parse_failure(
            Dmc3DdsStatus::invalid_mip_chain,
            "DDS mip count is not the corpus-confirmed complete mip chain");
    }

    const auto compression = from_codec(image.compression);
    const auto expected_header = canonical_header(
        image.width, image.height, compression);
    if (!std::equal(expected_header.begin(), expected_header.end(), bytes.begin())) {
        return parse_failure(
            Dmc3DdsStatus::invalid_header,
            "DDS header differs from the exact Pass 81 DMC3 canonical profile");
    }
    if (image.total_size != bytes.size()) {
        return parse_failure(
            Dmc3DdsStatus::invalid_payload_size,
            "DDS byte size does not equal the exact full DXT mip-chain size");
    }

    return Dmc3DdsParseResult{
        .status = Dmc3DdsStatus::ok,
        .document = Dmc3DdsDocument{
            .width = image.width,
            .height = image.height,
            .mip_map_count = image.mip_count,
            .compression = compression,
            .payload_size = image.payload_size,
            .total_size = image.total_size,
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

    const auto mip_count = codecs::dds_bc::maximum_mip_count(width, height);
    std::uint32_t expected_payload = 0U;
    if (!codecs::dds_bc::payload_size(
            width, height, mip_count, to_codec(compression), &expected_payload) ||
        payload.size() != expected_payload) {
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
