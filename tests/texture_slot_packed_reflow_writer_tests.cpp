#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_packed_reflow_writer.hpp"

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

[[nodiscard]] std::uint32_t payload_size(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5) {
    std::uint32_t total = 0U;
    while (true) {
        total += std::max(1U, (width + 3U) / 4U) *
            std::max(1U, (height + 3U) / 4U) * (dxt5 ? 16U : 8U);
        if (width == 1U && height == 1U) {
            break;
        }
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return total;
}

[[nodiscard]] std::vector<std::byte> make_payload(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5,
    std::uint8_t seed) {
    std::vector<std::byte> bytes(payload_size(width, height, dxt5), std::byte{0});
    for (std::size_t index = 0U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(
            (static_cast<unsigned>(seed) + index * 31U) & 0xFFU);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_dds(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5,
    std::uint8_t seed) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    const auto payload = make_payload(width, height, dxt5, seed);
    const auto built = dmc3::Dmc3DdsProfile::build(
        width, height,
        dxt5 ? dmc3::Dmc3DdsCompression::dxt5
             : dmc3::Dmc3DdsCompression::dxt1,
        std::span<const std::byte>{payload.data(), payload.size()});
    assert(built.ok());
    return built.bytes;
}

[[nodiscard]] std::vector<std::byte> descriptor_for(
    const std::vector<std::byte>& dds,
    bool secondary_half,
    std::uint32_t auxiliary_mode = 0U,
    std::uint32_t auxiliary_value = 0U) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    const auto parsed = dmc3::Dmc3DdsProfile::parse(
        std::span<const std::byte>{dds.data(), dds.size()});
    assert(parsed.ok());
    const auto& doc = parsed.document;
    const bool dxt5 = doc.compression == dmc3::Dmc3DdsCompression::dxt5;
    const auto sw = secondary_half ? doc.width / 2U : doc.width;
    const auto sh = secondary_half ? doc.height / 2U : doc.height;
    assert(sw != 0U && sh != 0U);

    std::vector<std::byte> descriptor(
        dmc3::TextureSlotFramingParser::k_descriptor_size, std::byte{0});
    put_u32(
        descriptor, 0x08U,
        0x20000U | (doc.mip_map_count << 8U) | (dxt5 ? 0x88U : 0x86U));
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (doc.height << 16U) | doc.width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, doc.width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, doc.payload_size);
    put_u32(descriptor, 0x3CU, auxiliary_mode);
    put_u32(descriptor, 0x40U, auxiliary_value);
    put_u32(descriptor, 0x44U, (sh << 16U) | sw);
    put_u32(
        descriptor, 0x48U,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(sw)));
    put_u32(
        descriptor, 0x4CU,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(sh)));
    put_u32(descriptor, 0x60U, dxt5 ? 4U : 0U);
    put_u32(descriptor, 0x64U, doc.total_size);
    put_u32(descriptor, 0x68U, 8U);
    return descriptor;
}

[[nodiscard]] std::vector<std::byte> wrapped_slot(
    const std::vector<std::byte>& dds,
    bool secondary_half,
    std::uint32_t auxiliary_mode = 0U,
    std::uint32_t auxiliary_value = 0U) {
    auto descriptor = descriptor_for(
        dds, secondary_half, auxiliary_mode, auxiliary_value);
    descriptor.insert(descriptor.end(), dds.begin(), dds.end());
    return descriptor;
}

struct BundleTexture final {
    std::vector<std::byte> dds;
    bool secondary_half{};
};

[[nodiscard]] std::vector<std::byte> bundle_slot(
    const std::vector<BundleTexture>& textures,
    bool compact_final) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    assert(!textures.empty());
    std::vector<std::byte> output(
        dmc3::TextureSlotFramingParser::k_bundle_header_size, std::byte{0});
    put_u32(output, 0U, static_cast<std::uint32_t>(textures.size()));

    for (std::size_t index = 0U; index < textures.size(); ++index) {
        auto descriptor = descriptor_for(
            textures[index].dds, textures[index].secondary_half);
        const auto record_size = descriptor.size() + textures[index].dds.size();
        const bool final = index + 1U == textures.size();
        const auto sectors = (final && compact_final)
            ? 0U
            : static_cast<std::uint32_t>(
                  (record_size + dmc3::TextureSlotFramingParser::k_sector_size - 1U) /
                  dmc3::TextureSlotFramingParser::k_sector_size);
        put_u32(output, 4U + index * 4U, sectors);
        output.insert(output.end(), descriptor.begin(), descriptor.end());
        output.insert(output.end(), textures[index].dds.begin(), textures[index].dds.end());
        if (sectors != 0U) {
            const auto padded =
                static_cast<std::size_t>(sectors) *
                dmc3::TextureSlotFramingParser::k_sector_size;
            output.insert(output.end(), padded - record_size, std::byte{0});
        }
    }
    return output;
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return dmc::rengine::core::Sha256::compute(bytes).hex();
}

[[nodiscard]] dmc::rengine::profiles::dmc3::AuthoredPackedTextureDds authored(
    const std::vector<std::byte>& source_slot,
    std::uint32_t index,
    std::vector<std::byte> output_dds) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    const auto parsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{source_slot.data(), source_slot.size()});
    assert(parsed.ok());
    const auto& entry = parsed.document.textures[index];
    const auto source = std::span<const std::byte>{
        source_slot.data() + static_cast<std::ptrdiff_t>(entry.dds_offset),
        static_cast<std::size_t>(entry.dds_size)};
    return dmc3::AuthoredPackedTextureDds{
        .texture_index = index,
        .expected_source_sha256 = sha256_of(source),
        .bytes = std::move(output_dds),
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload pac_parent(
    const std::vector<std::byte>& child) {
    namespace gdspaces = dmc::rengine::gdspaces;
    std::vector<std::byte> bytes(0x10U + child.size(), std::byte{0});
    bytes[0U] = std::byte{'P'};
    bytes[1U] = std::byte{'A'};
    bytes[2U] = std::byte{'C'};
    bytes[3U] = std::byte{0};
    put_u32(bytes, 4U, 1U);
    put_u32(bytes, 8U, 0x10U);
    bytes[0x0CU] = std::byte{0xA1};
    bytes[0x0DU] = std::byte{0xB2};
    bytes[0x0EU] = std::byte{0xC3};
    bytes[0x0FU] = std::byte{0xD4};
    std::copy(child.begin(), child.end(), bytes.begin() + 0x10);
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "pass82-source",
                .logical_path = "GData.afs/pass82.pac",
                .container_chain = "nbz[82]",
                .offset = 0x8200U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "pass82.pac",
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

    // Direct wrapped DDS grow: 256x256 -> 512x512 while preserving the source
    // half-secondary relation. Physical slot size must grow and reparse.
    const auto direct_source_dds = make_dds(256U, 256U, true, 0x11U);
    const auto direct_source = wrapped_slot(direct_source_dds, true);
    const auto direct_output_dds = make_dds(512U, 512U, true, 0x22U);
    const auto direct_authored = authored(direct_source, 0U, direct_output_dds);
    const std::vector<dmc3::AuthoredPackedTextureDds> direct_set{direct_authored};
    const auto direct_result = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{direct_source.data(), direct_source.size()},
        direct_set);
    assert(direct_result.ok());
    assert(direct_result.bytes.size() > direct_source.size());
    assert(direct_result.receipt->patches.size() == 1U);
    assert(direct_result.receipt->patches[0].source_width == 256U);
    assert(direct_result.receipt->patches[0].output_width == 512U);
    const auto direct_reparsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{direct_result.bytes.data(), direct_result.bytes.size()});
    assert(direct_reparsed.ok());
    assert(direct_reparsed.document.textures[0].width == 512U);
    assert(direct_reparsed.document.textures[0].secondary_width == 256U);
    assert(direct_reparsed.document.textures[0].auxiliary_mode == 0U);

    // Direct shrink with same-secondary relation.
    const auto shrink_source_dds = make_dds(512U, 512U, false, 0x31U);
    const auto shrink_source = wrapped_slot(shrink_source_dds, false);
    const auto shrink_output_dds = make_dds(128U, 128U, false, 0x32U);
    const auto shrink_authored = authored(shrink_source, 0U, shrink_output_dds);
    const std::vector<dmc3::AuthoredPackedTextureDds> shrink_set{shrink_authored};
    const auto shrink_result = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{shrink_source.data(), shrink_source.size()},
        shrink_set);
    assert(shrink_result.ok());
    assert(shrink_result.bytes.size() < shrink_source.size());
    const auto shrink_reparsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{shrink_result.bytes.data(), shrink_result.bytes.size()});
    assert(shrink_reparsed.ok());
    assert(shrink_reparsed.document.textures[0].secondary_width == 128U);

    // Bundle reflow with compact final: grow texture 0, leave texture 1 exact.
    const auto bundle_source = bundle_slot(
        {
            BundleTexture{make_dds(128U, 128U, false, 0x41U), false},
            BundleTexture{make_dds(256U, 256U, true, 0x42U), true},
        },
        true);
    const auto bundle_output_dds0 = make_dds(512U, 512U, false, 0x43U);
    const auto bundle_authored0 = authored(bundle_source, 0U, bundle_output_dds0);
    const std::vector<dmc3::AuthoredPackedTextureDds> bundle_set{bundle_authored0};
    const auto bundle_result = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{bundle_source.data(), bundle_source.size()},
        bundle_set);
    assert(bundle_result.ok());
    const auto source_bundle_parsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bundle_source.data(), bundle_source.size()});
    const auto output_bundle_parsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bundle_result.bytes.data(), bundle_result.bytes.size()});
    assert(source_bundle_parsed.ok() && output_bundle_parsed.ok());
    assert(output_bundle_parsed.document.textures.size() == 2U);
    assert(output_bundle_parsed.document.textures[0].width == 512U);
    assert(output_bundle_parsed.document.textures[1].sector_span == 0U);
    const auto source_second = source_bundle_parsed.document.textures[1];
    const auto output_second = output_bundle_parsed.document.textures[1];
    assert(source_second.dds_size == output_second.dds_size);
    assert(std::equal(
        bundle_source.begin() + static_cast<std::ptrdiff_t>(source_second.dds_offset),
        bundle_source.begin() + static_cast<std::ptrdiff_t>(source_second.dds_offset + source_second.dds_size),
        bundle_result.bytes.begin() + static_cast<std::ptrdiff_t>(output_second.dds_offset),
        bundle_result.bytes.begin() + static_cast<std::ptrdiff_t>(output_second.dds_offset + output_second.dds_size)));

    // Aligned final class is preserved too.
    const auto aligned_source = bundle_slot(
        {BundleTexture{make_dds(256U, 256U, true, 0x51U), false}},
        false);
    const auto aligned_out = make_dds(512U, 512U, true, 0x52U);
    const auto aligned_authored = authored(aligned_source, 0U, aligned_out);
    const std::vector<dmc3::AuthoredPackedTextureDds> aligned_set{aligned_authored};
    const auto aligned_result = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{aligned_source.data(), aligned_source.size()},
        aligned_set);
    assert(aligned_result.ok());
    const auto aligned_parsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{aligned_result.bytes.data(), aligned_result.bytes.size()});
    assert(aligned_parsed.ok());
    assert(aligned_parsed.document.textures[0].sector_span != 0U);

    // Unresolved nonzero auxiliary metadata is deliberately fail-closed.
    const auto aux_source_dds = make_dds(256U, 256U, true, 0x61U);
    const auto aux_source = wrapped_slot(
        aux_source_dds, false, 1U, 0xDD308000U);
    const auto aux_output = make_dds(512U, 512U, true, 0x62U);
    const auto aux_authored = authored(aux_source, 0U, aux_output);
    const std::vector<dmc3::AuthoredPackedTextureDds> aux_set{aux_authored};
    assert(
        dmc3::TextureSlotPackedReflowWriter::rebuild(
            std::span<const std::byte>{aux_source.data(), aux_source.size()}, aux_set).status ==
        dmc3::TextureSlotPackedReflowStatus::unresolved_auxiliary_metadata);

    // Compression changes remain outside Pass 82.
    const auto compression_source_dds = make_dds(256U, 256U, false, 0x71U);
    const auto compression_source = wrapped_slot(compression_source_dds, false);
    const auto compression_output = make_dds(256U, 256U, true, 0x72U);
    const auto compression_authored = authored(
        compression_source, 0U, compression_output);
    const std::vector<dmc3::AuthoredPackedTextureDds> compression_set{
        compression_authored};
    assert(
        dmc3::TextureSlotPackedReflowWriter::rebuild(
            std::span<const std::byte>{
                compression_source.data(), compression_source.size()},
            compression_set).status ==
        dmc3::TextureSlotPackedReflowStatus::compression_change_unsupported);

    // Half-secondary source cannot be resized to 64 because that would require
    // an unobserved 32-pixel secondary axis in the current product envelope.
    const auto half_source_dds = make_dds(128U, 128U, true, 0x81U);
    const auto half_source = wrapped_slot(half_source_dds, true);
    const auto half_output = make_dds(64U, 64U, true, 0x82U);
    const auto half_authored = authored(half_source, 0U, half_output);
    const std::vector<dmc3::AuthoredPackedTextureDds> half_set{half_authored};
    assert(
        dmc3::TextureSlotPackedReflowWriter::rebuild(
            std::span<const std::byte>{half_source.data(), half_source.size()}, half_set).status ==
        dmc3::TextureSlotPackedReflowStatus::unsupported_secondary_relation);

    auto stale = direct_authored;
    stale.expected_source_sha256.assign(64U, '0');
    const std::vector<dmc3::AuthoredPackedTextureDds> stale_set{stale};
    assert(
        dmc3::TextureSlotPackedReflowWriter::rebuild(
            std::span<const std::byte>{direct_source.data(), direct_source.size()}, stale_set).status ==
        dmc3::TextureSlotPackedReflowStatus::source_dds_mismatch);

    auto invalid_dds = direct_authored;
    invalid_dds.bytes[8U] ^= std::byte{0x01};
    const std::vector<dmc3::AuthoredPackedTextureDds> invalid_set{invalid_dds};
    assert(
        dmc3::TextureSlotPackedReflowWriter::rebuild(
            std::span<const std::byte>{direct_source.data(), direct_source.size()}, invalid_set).status ==
        dmc3::TextureSlotPackedReflowStatus::authored_dds_invalid);

    // Composition: size-changing intrinsic DDS -> complete physical texture
    // child -> generic PAC physical-child reflow -> PAC reparse.
    const auto parent = pac_parent(direct_source);
    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed_parent = registry.parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        parent.resource.id.logical_path);
    assert(parsed_parent.ok());
    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed_parent);
    assert(expansion.usable());
    assert(expansion.children.size() == 1U);
    const dmc3::AuthoredChildImage child{
        .resource = expansion.children[0].payload.resource.id,
        .source_sha256 = sha256_of(std::span<const std::byte>{
            expansion.children[0].payload.bytes.data(),
            expansion.children[0].payload.bytes.size()}),
        .output_sha256 = sha256_of(std::span<const std::byte>{
            direct_result.bytes.data(), direct_result.bytes.size()}),
        .revision = 1U,
        .writer_mode = "size-changing-texture-packed-reflow",
        .bytes = direct_result.bytes,
    };
    const std::vector<dmc3::AuthoredChildImage> children{child};
    const auto parent_result = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        parent, expansion, children);
    assert(parent_result.ok());
    assert(parent_result.bytes.size() > parent.bytes.size());
    const auto reparsed_parent = registry.parse(
        std::span<const std::byte>{
            parent_result.bytes.data(), parent_result.bytes.size()},
        parent.resource.id.logical_path);
    assert(reparsed_parent.ok());
    assert(reparsed_parent.document.entries.size() == 1U);
    assert(reparsed_parent.document.entries[0].size == direct_result.bytes.size());
    assert(std::equal(
        direct_result.bytes.begin(), direct_result.bytes.end(),
        parent_result.bytes.begin() + 0x10));

    return 0;
}
