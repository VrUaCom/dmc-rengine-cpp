#include "dmc_rengine/formats/ptx_binary.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace dmc::rengine::formats::ptx {
namespace {

[[nodiscard]] std::string compression_name(
    profiles::dmc3::TextureCompressionKind compression) {
    return compression == profiles::dmc3::TextureCompressionKind::dxt1
        ? "DXT1"
        : "DXT5";
}

[[nodiscard]] std::string u32_display(
    const binary::Reader& reader,
    std::size_t offset) {
    const auto value = reader.u32_le(offset);
    return value.has_value() ? std::to_string(*value) : std::string{};
}

[[nodiscard]] bool add_u32_field(
    binary::Document& document,
    const binary::Reader& reader,
    std::string id,
    std::string name,
    std::size_t offset,
    std::string parent_id = {}) {
    return document.add_field(binary::Field{
        .id = std::move(id),
        .name = std::move(name),
        .range = {.offset = offset, .size = 4U},
        .kind = binary::FieldKind::unsigned_integer,
        .type_name = "uint32_le",
        .display_value = u32_display(reader, offset),
        .parent_id = std::move(parent_id),
        .evidence_id = {},
    });
}

[[nodiscard]] std::string texture_prefix(std::uint32_t index) {
    return "ptx-texture-" + std::to_string(index);
}

} // namespace

std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ScanResult& scan) {
    if (!resource.valid() || resource.id.size != bytes.size() || !scan.ok() ||
        bytes.size() < bundle_header_size) {
        return std::nullopt;
    }

    const binary::Reader reader(bytes);
    binary::Document document(std::move(resource), bytes.size());
    if (!document.add_region(binary::Region{
            .id = "ptx-header",
            .name = "PTX bundle header",
            .range = {.offset = 0U, .size = bundle_header_size},
            .kind = binary::RegionKind::header,
            .type_name = "PtxBundleHeader",
            .evidence_id = {},
        }) ||
        !add_u32_field(
            document, reader,
            "ptx-texture-count", "Texture count", 0U)) {
        return std::nullopt;
    }

    const auto& textures = scan.framing.document.textures;
    for (const auto& texture : textures) {
        const auto prefix = texture_prefix(texture.texture_index);
        const auto span_field_offset =
            4U + static_cast<std::size_t>(texture.texture_index) * 4U;
        if (!add_u32_field(
                document,
                reader,
                prefix + "-sector-span",
                "Texture " + std::to_string(texture.texture_index) + " sector span",
                span_field_offset)) {
            return std::nullopt;
        }

        if (!document.add_region(binary::Region{
                .id = prefix + "-descriptor",
                .name = "PTX texture descriptor " +
                    std::to_string(texture.texture_index),
                .range = {
                    .offset = texture.descriptor_offset,
                    .size = descriptor_size,
                },
                .kind = binary::RegionKind::record,
                .type_name = "PtxTextureDescriptor",
                .evidence_id = {},
            }) ||
            !document.add_region(binary::Region{
                .id = prefix + "-dds",
                .name = "DDS texture " + std::to_string(texture.texture_index),
                .range = {
                    .offset = texture.dds_offset,
                    .size = texture.dds_size,
                },
                .kind = binary::RegionKind::payload,
                .type_name = compression_name(texture.compression) + " DDS",
                .evidence_id = {},
            })) {
            return std::nullopt;
        }

        const auto descriptor_offset =
            static_cast<std::size_t>(texture.descriptor_offset);
        if (!add_u32_field(
                document, reader,
                prefix + "-encoding", "Encoding", descriptor_offset + 0x08U) ||
            !document.add_field(binary::Field{
                .id = prefix + "-dimensions",
                .name = "Dimensions",
                .range = {.offset = descriptor_offset + 0x10U, .size = 4U},
                .kind = binary::FieldKind::structure,
                .type_name = "packed_u16_width_height",
                .display_value = std::to_string(texture.width) + "x" +
                    std::to_string(texture.height),
                .parent_id = {},
                .evidence_id = {},
            }) ||
            !add_u32_field(
                document, reader,
                prefix + "-payload-size", "DDS payload size",
                descriptor_offset + 0x38U) ||
            !add_u32_field(
                document, reader,
                prefix + "-aux-mode", "Auxiliary mode",
                descriptor_offset + 0x3CU) ||
            !add_u32_field(
                document, reader,
                prefix + "-aux-value", "Auxiliary value",
                descriptor_offset + 0x40U) ||
            !document.add_field(binary::Field{
                .id = prefix + "-secondary-dimensions",
                .name = "Secondary dimensions",
                .range = {.offset = descriptor_offset + 0x44U, .size = 4U},
                .kind = binary::FieldKind::structure,
                .type_name = "packed_u16_width_height",
                .display_value = std::to_string(texture.secondary_width) + "x" +
                    std::to_string(texture.secondary_height),
                .parent_id = {},
                .evidence_id = {},
            }) ||
            !document.add_field(binary::Field{
                .id = prefix + "-compression",
                .name = "Compression",
                .range = {.offset = descriptor_offset + 0x60U, .size = 4U},
                .kind = binary::FieldKind::enumeration,
                .type_name = "PtxCompression",
                .display_value = compression_name(texture.compression),
                .parent_id = {},
                .evidence_id = {},
            }) ||
            !add_u32_field(
                document, reader,
                prefix + "-dds-size", "DDS total size",
                descriptor_offset + 0x64U)) {
            return std::nullopt;
        }

        const std::uint64_t record_end = texture.sector_span == 0U
            ? static_cast<std::uint64_t>(bytes.size())
            : texture.descriptor_offset +
                static_cast<std::uint64_t>(texture.sector_span) * sector_size;
        const auto dds_end = texture.dds_offset + texture.dds_size;
        if (record_end > dds_end) {
            if (!document.add_region(binary::Region{
                    .id = prefix + "-padding",
                    .name = "Texture alignment padding " +
                        std::to_string(texture.texture_index),
                    .range = {
                        .offset = dds_end,
                        .size = record_end - dds_end,
                    },
                    .kind = binary::RegionKind::padding,
                    .type_name = "ZeroAlignmentPadding",
                    .evidence_id = {},
                })) {
                return std::nullopt;
            }
        }
    }

    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = "formats.ptx",
        .range = {.offset = 0U, .size = bytes.size()},
        .rationale =
            "The modular PTX reader owns the fully validated DMC3 texture-bundle framing, including header, descriptors, DDS child extents and zero alignment padding.",
    }));
    return document;
}

} // namespace dmc::rengine::formats::ptx
