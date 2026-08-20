#include "dmc_rengine/profiles/dmc3/dds_image.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::uint32_t full_mip_count(
    std::uint32_t width,
    std::uint32_t height) {
    auto dimension = std::max(width, height);
    std::uint32_t count = 1U;
    while (dimension > 1U) {
        dimension /= 2U;
        ++count;
    }
    return count;
}

[[nodiscard]] std::uint32_t payload_size(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5) {
    std::uint32_t total = 0U;
    for (std::uint32_t level = 0U; level < mip_count; ++level) {
        total += std::max(1U, (width + 3U) / 4U) *
            std::max(1U, (height + 3U) / 4U) * (dxt5 ? 16U : 8U);
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return total;
}

[[nodiscard]] std::vector<std::byte> make_dds(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5,
    bool depth1_exception = false) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    const auto mip_count = full_mip_count(width, height);
    const auto payload = payload_size(width, height, mip_count, dxt5);
    std::vector<std::byte> bytes(
        dmc3::DdsImageParser::k_file_header_size + payload, std::byte{0});

    bytes[0U] = std::byte{'D'};
    bytes[1U] = std::byte{'D'};
    bytes[2U] = std::byte{'S'};
    bytes[3U] = std::byte{' '};
    put_u32(bytes, 4U, dmc3::DdsImageParser::k_header_struct_size);
    put_u32(bytes, 8U, dmc3::DdsImageParser::k_required_flags);
    put_u32(bytes, 12U, height);
    put_u32(bytes, 16U, width);
    put_u32(
        bytes, 20U,
        depth1_exception
            ? 0x00200000U
            : (dxt5
                   ? dmc3::DdsImageParser::k_standard_dxt5_linear_size
                   : dmc3::DdsImageParser::k_standard_dxt1_linear_size));
    put_u32(bytes, 24U, depth1_exception ? 1U : 0U);
    put_u32(bytes, 28U, mip_count);

    put_u32(bytes, 76U, dmc3::DdsImageParser::k_pixel_format_size);
    put_u32(bytes, 80U, dmc3::DdsImageParser::k_pixel_format_fourcc_flag);
    bytes[84U] = std::byte{'D'};
    bytes[85U] = std::byte{'X'};
    bytes[86U] = std::byte{'T'};
    bytes[87U] = dxt5 ? std::byte{'5'} : std::byte{'1'};
    put_u32(bytes, 108U, dmc3::DdsImageParser::k_required_caps);

    for (std::size_t index = 128U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>((index * 31U) & 0xFFU);
    }
    return bytes;
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto dxt1 = make_dds(256U, 256U, false);
    const auto dxt1_result = dmc3::DdsImageParser::parse(dxt1);
    assert(dxt1_result.ok());
    assert(dxt1_result.document.profile == dmc3::DdsHeaderProfile::standard_corpus);
    assert(dxt1_result.document.compression == dmc3::DdsCompressionKind::dxt1);
    assert(dxt1_result.document.width == 256U);
    assert(dxt1_result.document.height == 256U);
    assert(dxt1_result.document.mip_map_count == 9U);
    assert(dxt1_result.document.pitch_or_linear_size == 0x10000U);
    assert(dxt1_result.document.depth == 0U);
    assert(dxt1_result.document.total_size == dxt1.size());

    const auto dxt5 = make_dds(512U, 256U, true);
    const auto dxt5_result = dmc3::DdsImageParser::parse(dxt5);
    assert(dxt5_result.ok());
    assert(dxt5_result.document.compression == dmc3::DdsCompressionKind::dxt5);
    assert(dxt5_result.document.mip_map_count == 10U);
    assert(dxt5_result.document.pitch_or_linear_size == 0x20000U);

    const auto exception = make_dds(1024U, 2048U, true, true);
    const auto exception_result = dmc3::DdsImageParser::parse(exception);
    assert(exception_result.ok());
    assert(
        exception_result.document.profile ==
        dmc3::DdsHeaderProfile::observed_depth1_exception);
    assert(exception_result.document.depth == 1U);
    assert(exception_result.document.pitch_or_linear_size == 0x200000U);
    assert(exception_result.document.mip_map_count == 12U);

    std::vector<std::byte> truncated(127U, std::byte{0});
    assert(
        dmc3::DdsImageParser::parse(truncated).status ==
        dmc3::DdsImageStatus::truncated);

    auto bad_magic = dxt1;
    bad_magic[0U] = std::byte{'X'};
    assert(
        dmc3::DdsImageParser::parse(bad_magic).status ==
        dmc3::DdsImageStatus::invalid_magic);

    auto bad_flags = dxt1;
    put_u32(bad_flags, 8U, 0U);
    assert(
        dmc3::DdsImageParser::parse(bad_flags).status ==
        dmc3::DdsImageStatus::invalid_flags);

    auto bad_mips = dxt1;
    put_u32(bad_mips, 28U, 1U);
    assert(
        dmc3::DdsImageParser::parse(bad_mips).status ==
        dmc3::DdsImageStatus::invalid_mip_chain);

    auto bad_linear = dxt1;
    put_u32(bad_linear, 20U, 0x20000U);
    assert(
        dmc3::DdsImageParser::parse(bad_linear).status ==
        dmc3::DdsImageStatus::invalid_linear_size);

    auto bad_depth = dxt1;
    put_u32(bad_depth, 24U, 1U);
    assert(
        dmc3::DdsImageParser::parse(bad_depth).status ==
        dmc3::DdsImageStatus::invalid_depth);

    auto bad_reserved = dxt1;
    put_u32(bad_reserved, 32U, 7U);
    assert(
        dmc3::DdsImageParser::parse(bad_reserved).status ==
        dmc3::DdsImageStatus::nonzero_reserved_fields);

    auto bad_pf = dxt1;
    put_u32(bad_pf, 80U, 0U);
    assert(
        dmc3::DdsImageParser::parse(bad_pf).status ==
        dmc3::DdsImageStatus::invalid_pixel_format);

    auto bad_caps = dxt1;
    put_u32(bad_caps, 108U, 0U);
    assert(
        dmc3::DdsImageParser::parse(bad_caps).status ==
        dmc3::DdsImageStatus::invalid_caps);

    auto bad_size = dxt1;
    bad_size.push_back(std::byte{0});
    assert(
        dmc3::DdsImageParser::parse(bad_size).status ==
        dmc3::DdsImageStatus::size_mismatch);

    auto bad_fourcc = dxt1;
    bad_fourcc[87U] = std::byte{'3'};
    assert(
        dmc3::DdsImageParser::parse(bad_fourcc).status ==
        dmc3::DdsImageStatus::unsupported_compression);

    return 0;
}
