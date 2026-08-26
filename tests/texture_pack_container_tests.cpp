#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_packed_reflow_writer.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

// The texture pack seen as a container the resource tree can expand.
//
// The framing parser owns what a pack *is* and the packed-reflow writer owns
// how one is authored. These check that the container view agrees with both:
// that it hands back exact DDS images, that it never invents a level, and that
// a texture taken out through the tree goes back in through the writer without
// translation. That last one is the whole point of having a single authority.

namespace {

namespace formats = dmc::rengine::formats;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
namespace core = dmc::rengine::core;

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


[[nodiscard]] std::vector<std::byte> pack_fixture() {
    // The real stage packs run 128x128 to 1024x1024 and mix DXT1 with DXT5,
    // so the fixture does too.
    return bundle_slot(
        {
            BundleTexture{make_dds(128U, 128U, false, 0x11U), false},
            BundleTexture{make_dds(256U, 256U, true, 0x33U), false},
        },
        false);
}

void a_pack_expands_to_its_textures() {
    const auto pack = pack_fixture();
    const auto parsed = formats::PtxParser::parse(pack);
    assert(parsed.ok());
    assert(parsed.document->format == "ptx");
    assert(parsed.document->declared_slot_count == 2U);

    // Geometry comes from one authority. If these ever diverge, the tree and
    // the writer are looking at different files.
    const auto framed = dmc3::TextureSlotFramingParser::parse(pack);
    assert(framed.ok());
    assert(framed.document.kind == dmc3::TextureSlotFramingKind::texture_bundle);
    for (std::size_t index = 0U; index < 2U; ++index) {
        const auto& entry = parsed.document->entries[index];
        const auto& texture = framed.document.textures[index];
        assert(entry.slot_index == texture.texture_index);
        assert(entry.offset == texture.dds_offset);
        assert(entry.size == texture.dds_size);
        assert(entry.populated);
        assert(entry.synthetic_name);
        assert(entry.valid(parsed.document->container_size));
    }
}

void a_child_is_the_image_and_nothing_else() {
    // What the tree hands back for a texture must be a DDS file, byte for
    // byte: not the block it sits in, not the descriptor that introduces it,
    // and not the sector padding that follows. Anything else and an extracted
    // texture is a reconstruction rather than a copy.
    const auto pack = pack_fixture();
    const auto parsed = formats::PtxParser::parse(pack);
    assert(parsed.ok());

    const auto& entry = parsed.document->entries[0];
    const std::span<const std::byte> child{
        pack.data() + entry.offset, static_cast<std::size_t>(entry.size)};
    const auto profiled = dmc3::Dmc3DdsProfile::parse(child);
    assert(profiled.ok());
    assert(profiled.document.total_size == entry.size);
}

void a_single_wrapped_texture_is_not_a_pack() {
    // One descriptor and one image is a texture. Expanding it into a container
    // with one child would invent a level of structure that is not there.
    const auto wrapped = wrapped_slot(make_dds(128U, 128U, false, 0x55U), false);
    const auto framed = dmc3::TextureSlotFramingParser::parse(wrapped);
    assert(framed.ok());
    assert(framed.document.kind == dmc3::TextureSlotFramingKind::wrapped_dds);

    const auto parsed = formats::PtxParser::parse(wrapped);
    assert(!parsed.ok());
    assert(parsed.error == formats::PtxParseError::not_a_texture_pack);
    assert(!formats::PtxParser::structurally_valid(wrapped));
}

void a_damaged_pack_is_refused_not_repaired() {
    // A pack has no magic, so the arithmetic is the identification. Move one
    // sector count and the file no longer describes the bytes it is stored in;
    // that has to be a refusal rather than a best effort.
    auto pack = pack_fixture();
    put_u32(pack, 4U, 2U);
    assert(!formats::PtxParser::structurally_valid(pack));

    // Real containers are not packs either, and must not be swept up.
    std::vector<std::byte> pac(0x2000U, std::byte{0});
    pac[0] = static_cast<std::byte>('P');
    pac[1] = static_cast<std::byte>('A');
    pac[2] = static_cast<std::byte>('C');
    pac[4] = std::byte{1};
    pac[12] = std::byte{0x40};
    assert(!formats::PtxParser::structurally_valid(pac));
}

void the_cycle_closes_from_the_tree_back_into_the_container() {
    // Unpack, edit, repack, unpack. A texture taken out through the container
    // view must be acceptable to the writer without translation, and what the
    // writer produces must come back through the container view unchanged
    // except where it was edited.
    const auto pack = pack_fixture();

    const auto parsed = formats::PtxParser::parse(pack);
    assert(parsed.ok());

    std::vector<dmc3::AuthoredPackedTextureDds> authored_textures;
    for (const auto& entry : parsed.document->entries) {
        std::vector<std::byte> image(
            pack.begin() + static_cast<std::ptrdiff_t>(entry.offset),
            pack.begin() + static_cast<std::ptrdiff_t>(entry.offset + entry.size));
        authored_textures.push_back(dmc3::AuthoredPackedTextureDds{
            .texture_index = entry.slot_index,
            // The hash the writer demands is the hash of exactly these bytes.
            // If the container view handed back a block instead of an image,
            // this is where it would fail.
            .expected_source_sha256 = core::Sha256::compute(image).hex(),
            .bytes = std::move(image),
        });
    }
    assert(authored_textures.size() == 2U);

    // One byte of pixel data. A cycle has to survive the smallest possible
    // edit before a larger one is worth attempting.
    authored_textures[0].bytes[0x90] ^= std::byte{0xFF};

    const auto rebuilt = dmc3::TextureSlotPackedReflowWriter::rebuild(
        pack, authored_textures);
    assert(rebuilt.ok());
    assert(rebuilt.bytes.size() == pack.size());

    std::size_t differing = 0U;
    for (std::size_t index = 0U; index < pack.size(); ++index) {
        differing += pack[index] != rebuilt.bytes[index] ? 1U : 0U;
    }
    assert(differing == 1U);

    const auto reparsed = formats::PtxParser::parse(rebuilt.bytes);
    assert(reparsed.ok());
    assert(reparsed.document->entries.size() == parsed.document->entries.size());
    for (std::size_t index = 0U; index < reparsed.document->entries.size(); ++index) {
        assert(reparsed.document->entries[index].offset ==
            parsed.document->entries[index].offset);
        assert(reparsed.document->entries[index].size ==
            parsed.document->entries[index].size);
    }

    assert(rebuilt.receipt.has_value());
    assert(rebuilt.receipt->source_sha256 == core::Sha256::compute(pack).hex());
    assert(rebuilt.receipt->output_sha256 ==
        core::Sha256::compute(rebuilt.bytes).hex());
}

void classification_and_selection_agree() {
    const auto pack = pack_fixture();

    // The slot that holds a pack has no name of its own, which is the case the
    // structural recognizer exists for.
    const auto classified = gdspaces::ResourceClassifier::classify(
        "slot_0001.bin", std::span<const std::byte>{pack}, false);
    assert(classified.format == "ptx");
    assert(classified.container);
    assert(classified.byte_derived);
    // There is no magic here, and the classification must not pretend there is.
    assert(!classified.magic_confirmed);

    const auto registry = dmc3::make_container_parser_registry();
    const auto* selected = registry.select(
        std::span<const std::byte>{pack}, "slot_0001.bin");
    assert(selected != nullptr);
    assert(selected->id() == "dmc3-ptx-structural-v1");
    const auto result = registry.parse(
        std::span<const std::byte>{pack}, "slot_0001.bin");
    assert(result.ok());
    assert(result.document.format == "ptx");
}

} // namespace

int main() {
    a_pack_expands_to_its_textures();
    a_child_is_the_image_and_nothing_else();
    a_single_wrapped_texture_is_not_a_pack();
    a_damaged_pack_is_refused_not_repaired();
    the_cycle_closes_from_the_tree_back_into_the_container();
    classification_and_selection_agree();
    return 0;
}
