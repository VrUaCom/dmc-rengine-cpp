#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/formats/ptx_binary.hpp"
#include "dmc_rengine/profiles/dmc3/index_display_semantics.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
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

[[nodiscard]] std::uint32_t block_payload_size(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5) {
    std::uint32_t total = 0U;
    for (std::uint32_t level = 0U; level < mip_count; ++level) {
        const auto blocks_w = std::max(1U, (width + 3U) / 4U);
        const auto blocks_h = std::max(1U, (height + 3U) / 4U);
        total += blocks_w * blocks_h * (dxt5 ? 16U : 8U);
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return total;
}

[[nodiscard]] std::vector<std::byte> make_dds(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5) {
    const auto payload_size = block_payload_size(
        width, height, mip_count, dxt5);
    std::vector<std::byte> bytes(
        128U + static_cast<std::size_t>(payload_size), std::byte{0});
    bytes[0] = std::byte{'D'};
    bytes[1] = std::byte{'D'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{' '};
    put_u32(bytes, 4U, 124U);
    put_u32(bytes, 12U, height);
    put_u32(bytes, 16U, width);
    put_u32(bytes, 28U, mip_count);
    bytes[84U] = std::byte{'D'};
    bytes[85U] = std::byte{'X'};
    bytes[86U] = std::byte{'T'};
    bytes[87U] = dxt5 ? std::byte{'5'} : std::byte{'1'};
    for (std::size_t index = 128U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>((index * 17U) & 0xFFU);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_descriptor(
    const std::vector<std::byte>& dds,
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5,
    bool secondary_half = false) {
    std::vector<std::byte> descriptor(0x70U, std::byte{0});
    const auto encoding_low = dxt5 ? 0x88U : 0x86U;
    const auto secondary_width = secondary_half ? width / 2U : width;
    const auto secondary_height = secondary_half ? height / 2U : height;
    put_u32(
        descriptor, 0x08U,
        0x20000U | (mip_count << 8U) | encoding_low);
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (height << 16U) | width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(
        descriptor, 0x38U,
        static_cast<std::uint32_t>(dds.size() - 128U));
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

void append_at(
    std::vector<std::byte>& destination,
    std::size_t offset,
    const std::vector<std::byte>& source) {
    assert(offset <= destination.size());
    assert(source.size() <= destination.size() - offset);
    std::copy(
        source.begin(), source.end(),
        destination.begin() + static_cast<std::ptrdiff_t>(offset));
}

[[nodiscard]] std::vector<std::byte> wrapped_dds_fixture() {
    constexpr std::uint32_t mip_count = 5U;
    const auto dds = make_dds(16U, 16U, mip_count, true);
    const auto descriptor = make_descriptor(
        dds, 16U, 16U, mip_count, true, true);
    std::vector<std::byte> bytes;
    bytes.reserve(descriptor.size() + dds.size());
    bytes.insert(bytes.end(), descriptor.begin(), descriptor.end());
    bytes.insert(bytes.end(), dds.begin(), dds.end());
    return bytes;
}

[[nodiscard]] std::vector<std::byte> bundle_fixture() {
    constexpr std::uint32_t mip_count = 5U;
    const auto dds0 = make_dds(16U, 16U, mip_count, false);
    const auto dds1 = make_dds(16U, 16U, mip_count, true);
    const auto descriptor0 = make_descriptor(
        dds0, 16U, 16U, mip_count, false, false);
    const auto descriptor1 = make_descriptor(
        dds1, 16U, 16U, mip_count, true, true);

    std::vector<std::byte> bytes(0x1800U, std::byte{0});
    put_u32(bytes, 0U, 2U);
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, 1U);

    append_at(bytes, 0x800U, descriptor0);
    append_at(bytes, 0x870U, dds0);
    append_at(bytes, 0x1000U, descriptor1);
    append_at(bytes, 0x1070U, dds1);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> zero_final_span_bundle_fixture() {
    constexpr std::uint32_t mip_count = 5U;
    const auto dds = make_dds(16U, 16U, mip_count, false);
    const auto descriptor = make_descriptor(
        dds, 16U, 16U, mip_count, false);
    std::vector<std::byte> bytes(
        0x800U + descriptor.size() + dds.size(), std::byte{0});
    put_u32(bytes, 0U, 1U);
    put_u32(bytes, 4U, 0U);
    append_at(bytes, 0x800U, descriptor);
    append_at(bytes, 0x870U, dds);
    return bytes;
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    namespace ptx = dmc::rengine::formats::ptx;

    const auto wrapped = wrapped_dds_fixture();
    const auto wrapped_result = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{wrapped.data(), wrapped.size()});
    assert(wrapped_result.ok());
    assert(
        wrapped_result.document.kind ==
        dmc3::TextureSlotFramingKind::wrapped_dds);
    assert(wrapped_result.document.textures.size() == 1U);
    assert(wrapped_result.document.textures[0].descriptor_offset == 0U);
    assert(wrapped_result.document.textures[0].dds_offset == 0x70U);
    assert(wrapped_result.document.textures[0].width == 16U);
    assert(wrapped_result.document.textures[0].height == 16U);
    assert(wrapped_result.document.textures[0].mip_map_count == 5U);
    assert(wrapped_result.document.textures[0].secondary_width == 8U);
    assert(wrapped_result.document.textures[0].secondary_height == 8U);
    assert(wrapped_result.document.textures[0].auxiliary_mode == 0U);
    assert(wrapped_result.document.textures[0].auxiliary_value == 0U);
    assert(
        wrapped_result.document.textures[0].compression ==
        dmc3::TextureCompressionKind::dxt5);

    // The PTX module is deliberately narrower than the shared framing parser:
    // descriptor+single-DDS framing is valid texture data but not a PTX bundle.
    const auto wrapped_ptx = ptx::Reader::scan(
        std::span<const std::byte>{wrapped.data(), wrapped.size()});
    assert(!wrapped_ptx.recognized);
    assert(!wrapped_ptx.ok());

    const auto bundle = bundle_fixture();
    const auto bundle_result = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bundle.data(), bundle.size()});
    assert(bundle_result.ok());
    assert(
        bundle_result.document.kind ==
        dmc3::TextureSlotFramingKind::texture_bundle);
    assert(bundle_result.document.textures.size() == 2U);
    assert(bundle_result.document.textures[0].descriptor_offset == 0x800U);
    assert(bundle_result.document.textures[0].dds_offset == 0x870U);
    assert(bundle_result.document.textures[0].secondary_width == 16U);
    assert(bundle_result.document.textures[0].sector_span == 1U);
    assert(bundle_result.document.textures[1].descriptor_offset == 0x1000U);
    assert(bundle_result.document.textures[1].dds_offset == 0x1070U);
    assert(bundle_result.document.textures[1].secondary_width == 8U);
    assert(bundle_result.document.textures[1].sector_span == 1U);

    const gdspaces::ResourcePayload unknown_profile_texture{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "nbz-source",
                .logical_path = "GData.afs/obj/em000.pac::PAC/slot-0000",
                .container_chain = "NBZ[41]/PAC[0]",
                .offset = 0x1000U,
                .size = static_cast<std::uint64_t>(bundle.size()),
            },
            .display_name = "slot_0000.bin",
            .format = "unknown",
            .profile = "unknown",
            .synthetic_name = true,
            .container = false,
        },
        .bytes = bundle,
        .diagnostics = {},
        .byte_provenance = std::nullopt,
        .name_evidence = {},
        .enclosing_container_name_evidence = {},
        .semantic_evidence = {},
    };

    // Real NBZ paths such as GData.afs/obj/em000.pac do not carry a "dmc3"
    // token, so generic path classification can legitimately leave the
    // physical profile unknown. The explicit DMC3 structural resolver must
    // still identify the payload from bytes rather than silently doing nothing.
    const auto unknown_profile_semantic =
        dmc3::resolve_materialized_display_semantic(unknown_profile_texture);
    assert(unknown_profile_semantic.has_value());
    assert(unknown_profile_semantic->semantic_format == "texture-bundle");
    assert(unknown_profile_semantic->canonical_extension == "ptx");

    // Modular Native Reader PTX path: scan -> Binary Inspector document ->
    // stable DDS child materialization.
    const auto ptx_scan = ptx::Reader::scan(
        std::span<const std::byte>{bundle.data(), bundle.size()});
    assert(ptx_scan.recognized);
    assert(ptx_scan.ok());
    assert(ptx_scan.framing.document.textures.size() == 2U);

    auto ptx_resource = unknown_profile_texture.resource;
    ptx_resource.format = "ptx";
    ptx_resource.display_name = "st001.ptx";
    ptx_resource.container = true;
    const auto ptx_document = ptx::build_binary_document(
        ptx_resource,
        std::span<const std::byte>{bundle.data(), bundle.size()},
        ptx_scan);
    assert(ptx_document.has_value());
    assert(ptx_document->find_region("ptx-header") != nullptr);
    assert(ptx_document->find_region("ptx-texture-0-descriptor") != nullptr);
    assert(ptx_document->find_region("ptx-texture-0-dds") != nullptr);
    assert(ptx_document->find_region("ptx-texture-1-dds") != nullptr);
    assert(ptx_document->find_field("ptx-texture-count") != nullptr);
    assert(ptx_document->find_field("ptx-texture-1-compression") != nullptr);
    assert(ptx_document->coverage_bytes() == bundle.size());
    assert(ptx_document->unknown_ranges().empty());

    auto ptx_parent = unknown_profile_texture;
    ptx_parent.resource = ptx_resource;
    const auto ptx_expansion = ptx::Reader::expand_dds_children(ptx_parent);
    assert(ptx_expansion.usable());
    assert(ptx_expansion.parser_format == "PTX");
    assert(ptx_expansion.children.size() == 2U);
    assert(ptx_expansion.children[0].payload.resource.format == "dds");
    assert(ptx_expansion.children[1].payload.resource.format == "dds");
    assert(ptx_expansion.children[0].payload.bytes[0] == std::byte{'D'});
    assert(ptx_expansion.children[0].payload.bytes[1] == std::byte{'D'});
    assert(ptx_expansion.children[0].payload.bytes[2] == std::byte{'S'});

    const auto final_zero = zero_final_span_bundle_fixture();
    const auto final_zero_result = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{final_zero.data(), final_zero.size()});
    assert(final_zero_result.ok());
    assert(final_zero_result.document.textures.size() == 1U);
    assert(final_zero_result.document.textures[0].sector_span == 0U);
    const auto final_zero_ptx = ptx::Reader::scan(
        std::span<const std::byte>{final_zero.data(), final_zero.size()});
    assert(final_zero_ptx.ok());

    auto bad_descriptor = wrapped;
    put_u32(bad_descriptor, 0x64U, 0x80U);
    assert(
        dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{
                bad_descriptor.data(), bad_descriptor.size()}).status ==
        dmc3::TextureSlotFramingStatus::descriptor_mismatch);

    auto bad_zero_constant = wrapped;
    put_u32(bad_zero_constant, 0x24U, 1U);
    assert(
        dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{
                bad_zero_constant.data(), bad_zero_constant.size()}).status ==
        dmc3::TextureSlotFramingStatus::descriptor_mismatch);

    auto bad_reciprocal = wrapped;
    put_u32(bad_reciprocal, 0x48U, 0U);
    assert(
        dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{
                bad_reciprocal.data(), bad_reciprocal.size()}).status ==
        dmc3::TextureSlotFramingStatus::descriptor_mismatch);

    auto bad_auxiliary = wrapped;
    put_u32(bad_auxiliary, 0x3CU, 1U);
    put_u32(bad_auxiliary, 0x40U, 0U);
    assert(
        dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{
                bad_auxiliary.data(), bad_auxiliary.size()}).status ==
        dmc3::TextureSlotFramingStatus::descriptor_mismatch);

    auto bad_mip_chain = wrapped;
    put_u32(bad_mip_chain, 0x70U + 28U, 4U);
    assert(
        dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{
                bad_mip_chain.data(), bad_mip_chain.size()}).status ==
        dmc3::TextureSlotFramingStatus::invalid_dds);

    auto bad_padding = bundle;
    bad_padding[0x0A00U] = std::byte{1};
    assert(
        dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{bad_padding.data(), bad_padding.size()}).status ==
        dmc3::TextureSlotFramingStatus::nonzero_alignment_padding);
    const auto bad_ptx = ptx::Reader::scan(
        std::span<const std::byte>{bad_padding.data(), bad_padding.size()});
    assert(!bad_ptx.ok());
    assert(!bad_ptx.diagnostics.empty());

    auto bad_sector = bundle;
    put_u32(bad_sector, 4U, 0U);
    assert(
        dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{bad_sector.data(), bad_sector.size()}).status ==
        dmc3::TextureSlotFramingStatus::invalid_sector_span);

    std::vector<std::byte> arbitrary(0x200U, std::byte{0});
    assert(
        dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{arbitrary.data(), arbitrary.size()}).status ==
        dmc3::TextureSlotFramingStatus::not_recognized);
    const auto arbitrary_ptx = ptx::Reader::scan(
        std::span<const std::byte>{arbitrary.data(), arbitrary.size()});
    assert(!arbitrary_ptx.recognized);
    assert(!arbitrary_ptx.ok());

    return 0;
}