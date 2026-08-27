#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/index_manifest.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"
#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_expander.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_packed_reflow_writer.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
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

[[nodiscard]] std::vector<std::byte> bytes_from_text(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
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

[[nodiscard]] std::vector<std::byte> make_dds(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5,
    std::uint8_t seed) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    std::vector<std::byte> payload(
        payload_size(width, height, dxt5), std::byte{0});
    for (std::size_t index = 0U; index < payload.size(); ++index) {
        payload[index] = static_cast<std::byte>(
            (static_cast<unsigned>(seed) + index * 29U) & 0xFFU);
    }

    const auto built = dmc3::Dmc3DdsProfile::build(
        width,
        height,
        dxt5 ? dmc3::Dmc3DdsCompression::dxt5
             : dmc3::Dmc3DdsCompression::dxt1,
        std::span<const std::byte>{payload.data(), payload.size()});
    assert(built.ok());
    return built.bytes;
}

[[nodiscard]] std::vector<std::byte> descriptor_for(
    const std::vector<std::byte>& dds) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto parsed = dmc3::Dmc3DdsProfile::parse(
        std::span<const std::byte>{dds.data(), dds.size()});
    assert(parsed.ok());
    const auto& doc = parsed.document;
    const bool dxt5 = doc.compression == dmc3::Dmc3DdsCompression::dxt5;

    std::vector<std::byte> descriptor(
        dmc3::TextureSlotFramingParser::k_descriptor_size, std::byte{0});
    put_u32(
        descriptor,
        0x08U,
        0x20000U | (doc.mip_map_count << 8U) | (dxt5 ? 0x88U : 0x86U));
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (doc.height << 16U) | doc.width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, doc.width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, doc.payload_size);
    put_u32(descriptor, 0x44U, (doc.height << 16U) | doc.width);
    put_u32(
        descriptor,
        0x48U,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(doc.width)));
    put_u32(
        descriptor,
        0x4CU,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(doc.height)));
    put_u32(descriptor, 0x60U, dxt5 ? 4U : 0U);
    put_u32(descriptor, 0x64U, doc.total_size);
    put_u32(descriptor, 0x68U, 8U);
    return descriptor;
}

[[nodiscard]] std::vector<std::byte> make_bundle(
    const std::vector<std::vector<std::byte>>& textures) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    assert(!textures.empty());

    std::vector<std::byte> output(
        dmc3::TextureSlotFramingParser::k_bundle_header_size, std::byte{0});
    put_u32(output, 0U, static_cast<std::uint32_t>(textures.size()));

    for (std::size_t index = 0U; index < textures.size(); ++index) {
        const auto descriptor = descriptor_for(textures[index]);
        const auto record_size = descriptor.size() + textures[index].size();
        const auto sectors = static_cast<std::uint32_t>(
            (record_size + dmc3::TextureSlotFramingParser::k_sector_size - 1U) /
            dmc3::TextureSlotFramingParser::k_sector_size);
        put_u32(output, 4U + index * 4U, sectors);
        output.insert(output.end(), descriptor.begin(), descriptor.end());
        output.insert(output.end(), textures[index].begin(), textures[index].end());
        const auto padded =
            static_cast<std::size_t>(sectors) *
            dmc3::TextureSlotFramingParser::k_sector_size;
        output.insert(output.end(), padded - record_size, std::byte{0});
    }
    return output;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload bundle_payload(
    std::vector<std::byte> bytes) {
    namespace gdspaces = dmc::rengine::gdspaces;
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-nbz-source",
                .logical_path = "GData.afs/st001.pac::PAC/slot-0001",
                .container_chain = "nbz[12]/PAC[1]",
                .offset = 0x4000U,
                .size = size,
            },
            .display_name = "st001_001.ptx",
            .format = "texture-bundle",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = gdspaces::ByteProvenance{
            .kind = gdspaces::ByteOriginKind::materialized_parent_span,
            .authority_id = "retail-nbz-source:GData.afs/st001.pac#nbz[12]@0+0",
            .offset = 0x1000U,
            .stored_size = size,
            .materialized_size = size,
            .transform = gdspaces::ByteTransform::none,
            .crc32 = std::nullopt,
        },
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload index_payload(
    std::string_view text) {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = bytes_from_text(text);
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-index-source",
                .logical_path = "GData.afs/st001_001.index",
                .container_chain = {},
                .offset = 0U,
                .size = size,
            },
            .display_name = "st001_001.index",
            .format = "index",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

[[nodiscard]] std::string index_text(std::string_view prefix, std::size_t count) {
    std::string text;
    for (std::size_t index = 0U; index < count; ++index) {
        text.append(prefix);
        if (index < 10U) {
            text.append("00");
        } else if (index < 100U) {
            text.push_back('0');
        }
        text.append(std::to_string(index));
        text.append(".dds\n");
    }
    return text;
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return dmc::rengine::core::Sha256::compute(bytes).hex();
}

[[nodiscard]] dmc::rengine::profiles::dmc3::AuthoredPackedTextureDds authored(
    const dmc::rengine::gdspaces::ContainerExpansion& expansion,
    std::uint32_t texture_index,
    std::vector<std::byte> replacement) {
    const auto& child = expansion.children[texture_index].payload;
    return dmc::rengine::profiles::dmc3::AuthoredPackedTextureDds{
        .texture_index = texture_index,
        .expected_source_sha256 = sha256_of(
            std::span<const std::byte>{child.bytes.data(), child.bytes.size()}),
        .bytes = std::move(replacement),
    };
}

void apply_index(
    dmc::rengine::gdspaces::ContainerExpansion& expansion,
    const dmc::rengine::gdspaces::ResourcePayload& index) {
    namespace gdspaces = dmc::rengine::gdspaces;
    const auto manifest = gdspaces::IndexManifestParser::parse(index);
    assert(manifest.ok());
    const auto binding = gdspaces::IndexSlotNameBinder::bind(
        expansion, *manifest.manifest);
    assert(binding.ok());
    const auto overlay = gdspaces::IndexNameOverlayBuilder::build(
        expansion, *binding.binding);
    assert(overlay.ok());
    const auto applied = gdspaces::IndexNameOverlayBuilder::apply(
        expansion, *overlay.overlay);
    assert(applied.ok());
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    // The retained corpus audit proves that st001_001.index contains 17 DDS
    // labels. The exact historical label spellings are not reasserted here;
    // this deterministic fixture exercises the confirmed 17-entry topology.
    std::vector<std::vector<std::byte>> source_textures;
    source_textures.reserve(17U);
    for (std::uint32_t index = 0U; index < 17U; ++index) {
        source_textures.push_back(make_dds(
            64U, 64U, true, static_cast<std::uint8_t>(0x20U + index)));
    }

    auto parent = bundle_payload(make_bundle(source_textures));
    auto expansion = dmc3::TextureSlotExpander::expand(parent);
    assert(expansion.usable());
    assert(expansion.children.size() == 17U);

    std::vector<gdspaces::ResourceId> original_ids;
    std::vector<std::vector<std::byte>> original_bytes;
    original_ids.reserve(expansion.children.size());
    original_bytes.reserve(expansion.children.size());
    for (std::size_t index = 0U; index < expansion.children.size(); ++index) {
        const auto& child = expansion.children[index].payload;
        original_ids.push_back(child.resource.id);
        original_bytes.push_back(child.bytes);
        assert(child.resource.format == "dds");
        assert(child.resource.synthetic_name);
        assert(child.resource.id.logical_path ==
               parent.resource.id.logical_path + "::TEXTURE/slot-" +
                   (index < 10U ? "000" : "00") + std::to_string(index));
        assert(child.byte_provenance.has_value());
        assert(
            child.byte_provenance->kind ==
            gdspaces::ByteOriginKind::materialized_parent_span);
        assert(child.byte_provenance->authority_id == parent.resource.id.canonical());
    }

    const auto source_index = index_payload(index_text("st001_001_", 17U));
    const auto manifest = gdspaces::IndexManifestParser::parse(source_index);
    assert(manifest.ok());
    assert(manifest.manifest->entries().size() == 17U);
    const auto binding = gdspaces::IndexSlotNameBinder::bind(
        expansion, *manifest.manifest);
    assert(binding.ok());
    assert(binding.binding->authorities().size() == 17U);
    for (std::size_t index = 0U; index < 17U; ++index) {
        assert(binding.binding->authorities()[index].slot_index() == index);
        assert(binding.binding->authorities()[index].child_resource() ==
               original_ids[index]);
    }

    apply_index(expansion, source_index);
    for (std::size_t index = 0U; index < 17U; ++index) {
        const auto& child = expansion.children[index].payload;
        assert(child.resource.id == original_ids[index]);
        assert(child.bytes == original_bytes[index]);
        assert(!child.resource.synthetic_name);
    }
    assert(expansion.children[0].payload.resource.display_name ==
           "st001_001_000.dds");
    assert(expansion.children[16].payload.resource.display_name ==
           "st001_001_016.dds");

    // A different 17-line .index observation may change presentation only.
    const auto renamed_index = index_payload(index_text("renamed_tex_", 17U));
    const auto ids_before_rename = original_ids;
    const auto bytes_before_rename = original_bytes;
    apply_index(expansion, renamed_index);
    assert(expansion.children[7].payload.resource.display_name ==
           "renamed_tex_007.dds");
    for (std::size_t index = 0U; index < 17U; ++index) {
        assert(expansion.children[index].payload.resource.id == ids_before_rename[index]);
        assert(expansion.children[index].payload.bytes == bytes_before_rename[index]);
    }

    // Same-size DDS authoring keeps the physical layout stable. Reopen must
    // therefore recover the exact same TEXTURE[7] ResourceId despite the
    // presentation rename.
    const auto replacement1 = make_dds(64U, 64U, true, 0xA1U);
    const auto authored1 = authored(expansion, 7U, replacement1);
    const std::vector<dmc3::AuthoredPackedTextureDds> authored_set1{authored1};
    const auto rebuilt1 = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        authored_set1);
    assert(rebuilt1.ok());
    assert(rebuilt1.bytes.size() == parent.bytes.size());

    auto reopened_parent = parent;
    reopened_parent.bytes = rebuilt1.bytes;
    auto reopened = dmc3::TextureSlotExpander::expand(reopened_parent);
    assert(reopened.usable());
    assert(reopened.children.size() == 17U);
    assert(reopened.children[7].payload.resource.id == original_ids[7]);
    assert(reopened.children[7].payload.bytes != original_bytes[7]);
    for (std::size_t index = 0U; index < 17U; ++index) {
        if (index != 7U) {
            assert(reopened.children[index].payload.resource.id == original_ids[index]);
            assert(reopened.children[index].payload.bytes == original_bytes[index]);
        }
    }

    // Reapply renamed naming evidence, then author the same physical texture
    // again. The name must not become writer authority.
    apply_index(reopened, renamed_index);
    assert(reopened.children[7].payload.resource.display_name ==
           "renamed_tex_007.dds");
    const auto target_id = reopened.children[7].payload.resource.id;
    const auto replacement2 = make_dds(64U, 64U, true, 0xB2U);
    const auto authored2 = authored(reopened, 7U, replacement2);
    const std::vector<dmc3::AuthoredPackedTextureDds> authored_set2{authored2};
    const auto rebuilt2 = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{
            reopened_parent.bytes.data(), reopened_parent.bytes.size()},
        authored_set2);
    assert(rebuilt2.ok());

    auto second_parent = reopened_parent;
    second_parent.bytes = rebuilt2.bytes;
    const auto second_reopen = dmc3::TextureSlotExpander::expand(second_parent);
    assert(second_reopen.usable());
    assert(second_reopen.children[7].payload.resource.id == target_id);
    assert(second_reopen.children[7].payload.bytes == replacement2);

    // Count mismatch is not partial authority: 16 names cannot name a 17-DDS
    // physical bundle.
    const auto short_index = index_payload(index_text("short_", 16U));
    const auto short_manifest = gdspaces::IndexManifestParser::parse(short_index);
    assert(short_manifest.ok());
    const auto short_binding = gdspaces::IndexSlotNameBinder::bind(
        second_reopen, *short_manifest.manifest);
    assert(!short_binding.ok());
    assert(short_binding.binding.has_value());
    assert(!short_binding.binding->valid());

    return 0;
}
