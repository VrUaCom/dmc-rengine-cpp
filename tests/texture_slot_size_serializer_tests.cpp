#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_size_serializer.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
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

[[nodiscard]] std::uint32_t read_u32(
    const std::vector<std::byte>& bytes,
    std::size_t offset) {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
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

[[nodiscard]] std::uint32_t block_payload_size(
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
    const auto payload_size = block_payload_size(width, height, mip_count, dxt5);
    std::vector<std::byte> bytes(
        dmc3::DdsImageParser::k_file_header_size + payload_size,
        std::byte{0});
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
        bytes[index] = static_cast<std::byte>((index * 37U) & 0xFFU);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_descriptor(
    const std::vector<std::byte>& dds,
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5,
    bool secondary_half,
    std::uint32_t auxiliary_mode = 0U,
    std::uint32_t auxiliary_value = 0U) {
    const auto mip_count = full_mip_count(width, height);
    const auto secondary_width = secondary_half ? width / 2U : width;
    const auto secondary_height = secondary_half ? height / 2U : height;
    std::vector<std::byte> descriptor(0x70U, std::byte{0});
    put_u32(
        descriptor, 0x08U,
        0x20000U | (mip_count << 8U) | (dxt5 ? 0x88U : 0x86U));
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (height << 16U) | width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, static_cast<std::uint32_t>(dds.size() - 128U));
    put_u32(descriptor, 0x3CU, auxiliary_mode);
    put_u32(descriptor, 0x40U, auxiliary_value);
    put_u32(
        descriptor, 0x44U,
        (secondary_height << 16U) | secondary_width);
    put_u32(
        descriptor, 0x48U,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_width)));
    put_u32(
        descriptor, 0x4CU,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_height)));
    put_u32(descriptor, 0x60U, dxt5 ? 4U : 0U);
    put_u32(descriptor, 0x64U, static_cast<std::uint32_t>(dds.size()));
    put_u32(descriptor, 0x68U, 8U);
    return descriptor;
}

[[nodiscard]] std::size_t aligned_record_size(std::size_t exact_size) {
    constexpr std::size_t sector = 0x800U;
    return ((exact_size + sector - 1U) / sector) * sector;
}

void append_bytes(
    std::vector<std::byte>& destination,
    const std::vector<std::byte>& source) {
    destination.insert(destination.end(), source.begin(), source.end());
}

void append_record(
    std::vector<std::byte>& destination,
    const std::vector<std::byte>& descriptor,
    const std::vector<std::byte>& dds,
    bool aligned) {
    const auto begin = destination.size();
    append_bytes(destination, descriptor);
    append_bytes(destination, dds);
    if (aligned) {
        destination.resize(
            begin + aligned_record_size(descriptor.size() + dds.size()),
            std::byte{0});
    }
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return dmc::rengine::core::Sha256::compute(bytes).hex();
}

[[nodiscard]] std::vector<std::byte> intrinsic_dds(
    const std::vector<std::byte>& slot,
    std::uint32_t texture_index) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    const auto parsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{slot.data(), slot.size()});
    assert(parsed.ok());
    const auto& entry = parsed.document.textures[texture_index];
    return std::vector<std::byte>{
        slot.begin() + static_cast<std::ptrdiff_t>(entry.dds_offset),
        slot.begin() + static_cast<std::ptrdiff_t>(entry.dds_offset + entry.dds_size)};
}

[[nodiscard]] dmc::rengine::profiles::dmc3::AuthoredTextureDds authored(
    const std::vector<std::byte>& source_slot,
    std::uint32_t texture_index,
    std::vector<std::byte> output_dds) {
    const auto source_dds = intrinsic_dds(source_slot, texture_index);
    return dmc::rengine::profiles::dmc3::AuthoredTextureDds{
        .texture_index = texture_index,
        .expected_source_sha256 = sha256_of(
            std::span<const std::byte>{source_dds.data(), source_dds.size()}),
        .bytes = std::move(output_dds),
    };
}

[[nodiscard]] std::vector<std::byte> wrapped_source(
    bool auxiliary = false) {
    const auto dds = make_dds(128U, 128U, true);
    const auto descriptor = make_descriptor(
        dds, 128U, 128U, true, true,
        auxiliary ? 1U : 0U,
        auxiliary ? 7U : 0U);
    std::vector<std::byte> bytes;
    bytes.reserve(descriptor.size() + dds.size());
    append_bytes(bytes, descriptor);
    append_bytes(bytes, dds);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> two_record_bundle() {
    const auto dds0 = make_dds(128U, 128U, true);
    const auto dds1 = make_dds(128U, 128U, false);
    const auto descriptor0 = make_descriptor(dds0, 128U, 128U, true, true);
    const auto descriptor1 = make_descriptor(dds1, 128U, 128U, false, false);
    const auto span0 = static_cast<std::uint32_t>(
        aligned_record_size(descriptor0.size() + dds0.size()) / 0x800U);
    const auto span1 = static_cast<std::uint32_t>(
        aligned_record_size(descriptor1.size() + dds1.size()) / 0x800U);

    std::vector<std::byte> bytes(0x800U, std::byte{0});
    put_u32(bytes, 0U, 2U);
    put_u32(bytes, 4U, span0);
    put_u32(bytes, 8U, span1);
    append_record(bytes, descriptor0, dds0, true);
    append_record(bytes, descriptor1, dds1, true);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> compact_final_bundle() {
    const auto dds = make_dds(128U, 128U, true);
    const auto descriptor = make_descriptor(dds, 128U, 128U, true, false);
    std::vector<std::byte> bytes(0x800U, std::byte{0});
    put_u32(bytes, 0U, 1U);
    put_u32(bytes, 4U, 0U);
    append_record(bytes, descriptor, dds, false);
    return bytes;
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    // Direct wrapped DDS: grow from one corpus-observed DXT5 geometry to another.
    const auto wrapped = wrapped_source();
    const auto grown_dds = make_dds(256U, 256U, true);
    const auto grown = authored(wrapped, 0U, grown_dds);
    const std::vector<dmc3::AuthoredTextureDds> grown_set{grown};
    const auto wrapped_result = dmc3::TextureSlotSizeSerializer::rebuild(
        std::span<const std::byte>{wrapped.data(), wrapped.size()}, grown_set);
    assert(wrapped_result.ok());
    assert(wrapped_result.bytes.size() > wrapped.size());
    assert(wrapped_result.receipt->patches.size() == 1U);
    assert(
        wrapped_result.receipt->patches[0].output_dds_size == grown_dds.size());
    const auto wrapped_reparsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{
            wrapped_result.bytes.data(), wrapped_result.bytes.size()});
    assert(wrapped_reparsed.ok());
    assert(wrapped_reparsed.document.textures[0].width == 256U);
    assert(wrapped_reparsed.document.textures[0].height == 256U);
    assert(wrapped_reparsed.document.textures[0].secondary_width == 128U);
    assert(wrapped_reparsed.document.textures[0].secondary_height == 128U);
    assert(intrinsic_dds(wrapped_result.bytes, 0U) == grown_dds);

    // No size change remains Pass 79 authority.
    const auto same = authored(wrapped, 0U, intrinsic_dds(wrapped, 0U));
    const std::vector<dmc3::AuthoredTextureDds> same_set{same};
    assert(
        dmc3::TextureSlotSizeSerializer::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()}, same_set).status ==
        dmc3::TextureSlotSizeSerializerStatus::no_size_change);

    auto stale = grown;
    stale.expected_source_sha256.assign(64U, '0');
    const std::vector<dmc3::AuthoredTextureDds> stale_set{stale};
    assert(
        dmc3::TextureSlotSizeSerializer::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()}, stale_set).status ==
        dmc3::TextureSlotSizeSerializerStatus::source_dds_mismatch);

    auto invalid_dds = grown;
    put_u32(invalid_dds.bytes, 8U, 0U);
    const std::vector<dmc3::AuthoredTextureDds> invalid_set{invalid_dds};
    assert(
        dmc3::TextureSlotSizeSerializer::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()}, invalid_set).status ==
        dmc3::TextureSlotSizeSerializerStatus::authored_dds_invalid);

    // Compression changes are intentionally outside the first authoring envelope.
    const auto changed_compression_dds = make_dds(256U, 256U, false);
    const auto changed_compression = authored(
        wrapped, 0U, changed_compression_dds);
    const std::vector<dmc3::AuthoredTextureDds> changed_compression_set{
        changed_compression};
    assert(
        dmc3::TextureSlotSizeSerializer::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()},
            changed_compression_set).status ==
        dmc3::TextureSlotSizeSerializerStatus::authored_dds_invalid);

    // Parser-valid but unobserved 64x64 geometry is not promoted for Pass 82 writing.
    const auto unobserved_dds = make_dds(64U, 64U, true);
    assert(dmc3::DdsImageParser::parse(unobserved_dds).ok());
    const auto unobserved = authored(wrapped, 0U, unobserved_dds);
    const std::vector<dmc3::AuthoredTextureDds> unobserved_set{unobserved};
    assert(
        dmc3::TextureSlotSizeSerializer::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()},
            unobserved_set).status ==
        dmc3::TextureSlotSizeSerializerStatus::authored_dds_invalid);

    const auto exceptional_dds = make_dds(1024U, 2048U, true, true);
    assert(
        dmc3::DdsImageParser::parse(exceptional_dds).document.profile ==
        dmc3::DdsHeaderProfile::observed_depth1_exception);
    const auto exceptional = authored(wrapped, 0U, exceptional_dds);
    const std::vector<dmc3::AuthoredTextureDds> exceptional_set{exceptional};
    assert(
        dmc3::TextureSlotSizeSerializer::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()},
            exceptional_set).status ==
        dmc3::TextureSlotSizeSerializerStatus::authored_dds_exception_profile);

    const auto auxiliary_source = wrapped_source(true);
    const auto auxiliary_authored = authored(
        auxiliary_source, 0U, make_dds(256U, 256U, true));
    const std::vector<dmc3::AuthoredTextureDds> auxiliary_set{auxiliary_authored};
    assert(
        dmc3::TextureSlotSizeSerializer::rebuild(
            std::span<const std::byte>{
                auxiliary_source.data(), auxiliary_source.size()},
            auxiliary_set).status ==
        dmc3::TextureSlotSizeSerializerStatus::source_auxiliary_unsupported);

    assert(
        dmc3::TextureSlotSizeSerializer::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()}, grown_set,
            dmc3::TextureSlotSizeSerializerSafety{.max_output_bytes = 1024U}).status ==
        dmc3::TextureSlotSizeSerializerStatus::output_too_large);

    // Bundle growth: first record moves the second record, which must remain exact.
    const auto bundle = two_record_bundle();
    const auto source_bundle = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bundle.data(), bundle.size()});
    assert(source_bundle.ok());
    assert(source_bundle.document.textures.size() == 2U);
    const auto source_second_begin = static_cast<std::size_t>(
        source_bundle.document.textures[1].descriptor_offset);
    const std::vector<std::byte> source_second{
        bundle.begin() + static_cast<std::ptrdiff_t>(source_second_begin),
        bundle.end()};

    const auto bundle_grown_dds = make_dds(512U, 512U, true);
    const auto bundle_authored = authored(bundle, 0U, bundle_grown_dds);
    const std::vector<dmc3::AuthoredTextureDds> bundle_set{bundle_authored};
    const auto bundle_result = dmc3::TextureSlotSizeSerializer::rebuild(
        std::span<const std::byte>{bundle.data(), bundle.size()}, bundle_set);
    assert(bundle_result.ok());
    const auto output_bundle = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{
            bundle_result.bytes.data(), bundle_result.bytes.size()});
    assert(output_bundle.ok());
    assert(output_bundle.document.textures.size() == 2U);
    assert(
        output_bundle.document.textures[1].descriptor_offset >
        source_bundle.document.textures[1].descriptor_offset);
    assert(
        bundle_result.receipt->patches[0].output_sector_span >
        bundle_result.receipt->patches[0].source_sector_span);
    assert(
        read_u32(bundle_result.bytes, 4U) ==
        bundle_result.receipt->patches[0].output_sector_span);
    assert(intrinsic_dds(bundle_result.bytes, 0U) == bundle_grown_dds);
    const auto output_second_begin = static_cast<std::size_t>(
        output_bundle.document.textures[1].descriptor_offset);
    const std::vector<std::byte> output_second{
        bundle_result.bytes.begin() +
            static_cast<std::ptrdiff_t>(output_second_begin),
        bundle_result.bytes.end()};
    assert(output_second == source_second);

    // Compact final policy is preserved: span remains zero and DDS ends at EOF.
    const auto compact = compact_final_bundle();
    const auto compact_authored = authored(
        compact, 0U, make_dds(256U, 256U, true));
    const std::vector<dmc3::AuthoredTextureDds> compact_set{compact_authored};
    const auto compact_result = dmc3::TextureSlotSizeSerializer::rebuild(
        std::span<const std::byte>{compact.data(), compact.size()}, compact_set);
    assert(compact_result.ok());
    assert(read_u32(compact_result.bytes, 4U) == 0U);
    const auto compact_reparsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{
            compact_result.bytes.data(), compact_result.bytes.size()});
    assert(compact_reparsed.ok());
    assert(compact_reparsed.document.textures[0].sector_span == 0U);
    const auto& compact_entry = compact_reparsed.document.textures[0];
    assert(
        compact_entry.dds_offset + compact_entry.dds_size ==
        compact_result.bytes.size());

    return 0;
}
