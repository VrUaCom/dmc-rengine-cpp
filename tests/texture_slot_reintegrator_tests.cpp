#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_reintegrator.hpp"

#include <algorithm>
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
        bytes[index] = static_cast<std::byte>((index * 29U) & 0xFFU);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_descriptor(
    const std::vector<std::byte>& dds,
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5) {
    std::vector<std::byte> descriptor(0x70U, std::byte{0});
    put_u32(
        descriptor, 0x08U,
        0x20000U | (mip_count << 8U) | (dxt5 ? 0x88U : 0x86U));
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (height << 16U) | width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(
        descriptor, 0x38U,
        static_cast<std::uint32_t>(dds.size() - 128U));
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

[[nodiscard]] std::vector<std::byte> wrapped_fixture() {
    const auto dds = make_dds(16U, 16U, 1U, true);
    const auto descriptor = make_descriptor(dds, 16U, 16U, 1U, true);
    std::vector<std::byte> slot;
    slot.reserve(descriptor.size() + dds.size());
    slot.insert(slot.end(), descriptor.begin(), descriptor.end());
    slot.insert(slot.end(), dds.begin(), dds.end());
    return slot;
}

[[nodiscard]] std::vector<std::byte> bundle_fixture() {
    const auto dds0 = make_dds(16U, 16U, 1U, false);
    const auto dds1 = make_dds(16U, 16U, 1U, true);
    const auto descriptor0 = make_descriptor(dds0, 16U, 16U, 1U, false);
    const auto descriptor1 = make_descriptor(dds1, 16U, 16U, 1U, true);

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
    assert(texture_index < parsed.document.textures.size());
    const auto& entry = parsed.document.textures[texture_index];
    return std::vector<std::byte>{
        slot.begin() + static_cast<std::ptrdiff_t>(entry.dds_offset),
        slot.begin() + static_cast<std::ptrdiff_t>(entry.dds_offset + entry.dds_size)};
}

[[nodiscard]] dmc::rengine::profiles::dmc3::AuthoredTextureDds authored_dds(
    const std::vector<std::byte>& slot,
    std::uint32_t texture_index,
    std::vector<std::byte> bytes) {
    const auto source = intrinsic_dds(slot, texture_index);
    return dmc::rengine::profiles::dmc3::AuthoredTextureDds{
        .texture_index = texture_index,
        .expected_source_sha256 = sha256_of(
            std::span<const std::byte>{source.data(), source.size()}),
        .bytes = std::move(bytes),
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload pac_parent(
    const std::vector<std::byte>& physical_child) {
    namespace gdspaces = dmc::rengine::gdspaces;
    std::vector<std::byte> bytes(0x10U + physical_child.size(), std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, 0x10U);
    bytes[0x0CU] = std::byte{0xA1};
    bytes[0x0DU] = std::byte{0xB2};
    bytes[0x0EU] = std::byte{0xC3};
    bytes[0x0FU] = std::byte{0xD4};
    append_at(bytes, 0x10U, physical_child);

    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "pass79-composition-source",
                .logical_path = "GData.afs/pass79-texture.pac",
                .container_chain = "nbz[11]",
                .offset = 0x4000U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "pass79-texture.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto wrapped = wrapped_fixture();
    auto wrapped_dds = intrinsic_dds(wrapped, 0U);
    wrapped_dds.back() ^= std::byte{0x5A};
    const auto wrapped_authored = authored_dds(wrapped, 0U, wrapped_dds);
    const std::vector<dmc3::AuthoredTextureDds> wrapped_set{wrapped_authored};

    const auto wrapped_result = dmc3::TextureSlotReintegrator::rebuild(
        std::span<const std::byte>{wrapped.data(), wrapped.size()}, wrapped_set);
    assert(wrapped_result.ok());
    assert(wrapped_result.bytes.size() == wrapped.size());
    assert(
        wrapped_result.receipt->framing_kind ==
        dmc3::TextureSlotFramingKind::wrapped_dds);
    assert(wrapped_result.receipt->patches.size() == 1U);
    assert(wrapped_result.receipt->patches[0].texture_index == 0U);
    assert(wrapped_result.receipt->patches[0].dds_offset == 0x70U);
    assert(std::equal(
        wrapped.begin(), wrapped.begin() + 0x70,
        wrapped_result.bytes.begin(), wrapped_result.bytes.begin() + 0x70));
    const auto wrapped_reparsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{
            wrapped_result.bytes.data(), wrapped_result.bytes.size()});
    assert(wrapped_reparsed.ok());

    const auto unchanged = authored_dds(wrapped, 0U, intrinsic_dds(wrapped, 0U));
    const std::vector<dmc3::AuthoredTextureDds> unchanged_set{unchanged};
    assert(
        dmc3::TextureSlotReintegrator::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()},
            unchanged_set).status == dmc3::TextureSlotReintegrationStatus::no_changes);

    auto stale = wrapped_authored;
    stale.expected_source_sha256.assign(64U, '0');
    const std::vector<dmc3::AuthoredTextureDds> stale_set{stale};
    assert(
        dmc3::TextureSlotReintegrator::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()}, stale_set).status ==
        dmc3::TextureSlotReintegrationStatus::source_dds_mismatch);

    auto missing_hash = wrapped_authored;
    missing_hash.expected_source_sha256.clear();
    const std::vector<dmc3::AuthoredTextureDds> missing_hash_set{missing_hash};
    assert(
        dmc3::TextureSlotReintegrator::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()},
            missing_hash_set).status ==
        dmc3::TextureSlotReintegrationStatus::missing_source_hash);

    auto grown = wrapped_authored;
    grown.bytes.push_back(std::byte{0});
    const std::vector<dmc3::AuthoredTextureDds> grown_set{grown};
    assert(
        dmc3::TextureSlotReintegrator::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()}, grown_set).status ==
        dmc3::TextureSlotReintegrationStatus::dds_size_changed);

    const std::vector<dmc3::AuthoredTextureDds> duplicate_set{
        wrapped_authored, wrapped_authored};
    assert(
        dmc3::TextureSlotReintegrator::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()},
            duplicate_set).status ==
        dmc3::TextureSlotReintegrationStatus::duplicate_texture_input);

    auto missing_index = wrapped_authored;
    missing_index.texture_index = 7U;
    const std::vector<dmc3::AuthoredTextureDds> missing_index_set{missing_index};
    assert(
        dmc3::TextureSlotReintegrator::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()},
            missing_index_set).status ==
        dmc3::TextureSlotReintegrationStatus::texture_not_found);

    auto invalid_output_dds = intrinsic_dds(wrapped, 0U);
    invalid_output_dds[16U] ^= std::byte{0x01};
    const auto invalid_output = authored_dds(
        wrapped, 0U, std::move(invalid_output_dds));
    const std::vector<dmc3::AuthoredTextureDds> invalid_output_set{invalid_output};
    assert(
        dmc3::TextureSlotReintegrator::rebuild(
            std::span<const std::byte>{wrapped.data(), wrapped.size()},
            invalid_output_set).status ==
        dmc3::TextureSlotReintegrationStatus::output_validation_failed);

    const std::vector<std::byte> arbitrary(0x200U, std::byte{0});
    assert(
        dmc3::TextureSlotReintegrator::rebuild(
            std::span<const std::byte>{arbitrary.data(), arbitrary.size()},
            wrapped_set).status ==
        dmc3::TextureSlotReintegrationStatus::invalid_source_framing);

    const auto bundle = bundle_fixture();
    auto bundle_dds1 = intrinsic_dds(bundle, 1U);
    bundle_dds1[bundle_dds1.size() - 3U] ^= std::byte{0x33};
    const auto bundle_authored = authored_dds(bundle, 1U, std::move(bundle_dds1));
    const std::vector<dmc3::AuthoredTextureDds> bundle_set{bundle_authored};
    const auto bundle_result = dmc3::TextureSlotReintegrator::rebuild(
        std::span<const std::byte>{bundle.data(), bundle.size()}, bundle_set);
    assert(bundle_result.ok());
    assert(
        bundle_result.receipt->framing_kind ==
        dmc3::TextureSlotFramingKind::texture_bundle);
    assert(bundle_result.receipt->patches.size() == 1U);
    assert(bundle_result.receipt->patches[0].texture_index == 1U);
    assert(std::equal(
        bundle.begin(), bundle.begin() + 0x1070,
        bundle_result.bytes.begin(), bundle_result.bytes.begin() + 0x1070));
    const auto bundle_reparsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{
            bundle_result.bytes.data(), bundle_result.bytes.size()});
    assert(bundle_reparsed.ok());

    // Composition proof: intrinsic DDS -> validated complete physical slot ->
    // generic PAC/PNST physical-child writer. The generic writer never receives
    // bare DDS bytes and therefore does not need DMC3 texture framing knowledge.
    const auto parent = pac_parent(wrapped);
    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed_parent = registry.parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        parent.resource.id.logical_path);
    assert(parsed_parent.ok());
    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed_parent);
    assert(expansion.usable());
    assert(expansion.children.size() == 1U);
    assert(expansion.children[0].entry.offset == 0x10U);
    assert(expansion.children[0].payload.bytes == wrapped);

    const dmc3::AuthoredChildImage physical_child{
        .resource = expansion.children[0].payload.resource.id,
        .source_sha256 = sha256_of(std::span<const std::byte>{
            expansion.children[0].payload.bytes.data(),
            expansion.children[0].payload.bytes.size()}),
        .output_sha256 = sha256_of(std::span<const std::byte>{
            wrapped_result.bytes.data(), wrapped_result.bytes.size()}),
        .revision = 1U,
        .writer_mode = "same-layout-intrinsic-dds-reintegration",
        .bytes = wrapped_result.bytes,
    };
    const std::vector<dmc3::AuthoredChildImage> physical_set{physical_child};
    const auto parent_result = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        parent, expansion, physical_set);
    assert(parent_result.ok());
    assert(parent_result.bytes.size() == parent.bytes.size());
    assert(parent_result.receipt->spans.size() == 1U);
    assert(parent_result.receipt->spans[0].changed);
    assert(parent_result.receipt->spans[0].source_size == wrapped.size());
    assert(parent_result.receipt->spans[0].output_size == wrapped.size());
    assert(std::equal(
        wrapped_result.bytes.begin(), wrapped_result.bytes.end(),
        parent_result.bytes.begin() + 0x10));

    const auto reparsed_parent = registry.parse(
        std::span<const std::byte>{
            parent_result.bytes.data(), parent_result.bytes.size()},
        parent.resource.id.logical_path);
    assert(reparsed_parent.ok());
    assert(reparsed_parent.document.entries.size() == 1U);
    assert(reparsed_parent.document.entries[0].offset == 0x10U);
    assert(reparsed_parent.document.entries[0].size == wrapped.size());

    return 0;
}
