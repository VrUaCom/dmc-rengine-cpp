#include "dmc_rengine/formats/dds_binary.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <string>
#include <utility>

namespace dmc::rengine::formats::dds {
namespace {

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
    std::size_t offset) {
    return document.add_field(binary::Field{
        .id = std::move(id),
        .name = std::move(name),
        .range = {.offset = offset, .size = 4U},
        .kind = binary::FieldKind::unsigned_integer,
        .type_name = "uint32_le",
        .display_value = u32_display(reader, offset),
        .parent_id = {},
        .evidence_id = {},
    });
}

} // namespace

std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ScanResult& scan) {
    if (!resource.valid() || resource.id.size != bytes.size() || !scan.ok() ||
        bytes.size() < header_size) {
        return std::nullopt;
    }

    const binary::Reader reader(bytes);
    binary::Document document(std::move(resource), bytes.size());
    if (!document.add_region(binary::Region{
            .id = "dds-header",
            .name = "DDS header",
            .range = {.offset = 0U, .size = header_size},
            .kind = binary::RegionKind::header,
            .type_name = "DdsHeader",
            .evidence_id = {},
        }) ||
        !document.add_region(binary::Region{
            .id = "dds-payload",
            .name = "DDS compressed mip payload",
            .range = {
                .offset = header_size,
                .size = scan.profile.document.payload_size,
            },
            .kind = binary::RegionKind::payload,
            .type_name = scan.profile.document.compression ==
                    profiles::dmc3::Dmc3DdsCompression::dxt1
                ? "DXT1MipChain"
                : "DXT5MipChain",
            .evidence_id = {},
        })) {
        return std::nullopt;
    }

    if (!document.add_field(binary::Field{
            .id = "dds-magic",
            .name = "Magic",
            .range = {.offset = 0U, .size = 4U},
            .kind = binary::FieldKind::string,
            .type_name = "char[4]",
            .display_value = "DDS ",
            .parent_id = {},
            .evidence_id = {},
        }) ||
        !add_u32_field(document, reader, "dds-header-size", "Header size", 4U) ||
        !add_u32_field(document, reader, "dds-flags", "Flags", 8U) ||
        !add_u32_field(document, reader, "dds-height", "Height", 12U) ||
        !add_u32_field(document, reader, "dds-width", "Width", 16U) ||
        !add_u32_field(document, reader, "dds-linear-size", "Linear size", 20U) ||
        !add_u32_field(document, reader, "dds-depth", "Depth", 24U) ||
        !add_u32_field(document, reader, "dds-mip-count", "Mip map count", 28U) ||
        !add_u32_field(document, reader, "dds-pixel-format-size", "Pixel format size", 76U) ||
        !add_u32_field(document, reader, "dds-pixel-format-flags", "Pixel format flags", 80U) ||
        !document.add_field(binary::Field{
            .id = "dds-fourcc",
            .name = "Compression FourCC",
            .range = {.offset = 84U, .size = 4U},
            .kind = binary::FieldKind::string,
            .type_name = "char[4]",
            .display_value = scan.profile.document.compression ==
                    profiles::dmc3::Dmc3DdsCompression::dxt1
                ? "DXT1"
                : "DXT5",
            .parent_id = {},
            .evidence_id = {},
        }) ||
        !add_u32_field(document, reader, "dds-caps", "Caps", 108U)) {
        return std::nullopt;
    }

    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = "formats.dds",
        .range = {.offset = 0U, .size = header_size},
        .rationale = "The modular DDS reader owns the corpus-confirmed DMC3 DDS header.",
    }));
    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = "formats.dds",
        .range = {.offset = header_size, .size = scan.profile.document.payload_size},
        .rationale = "The modular DDS reader owns the bounded DXT mip payload extent.",
    }));
    return document;
}

} // namespace dmc::rengine::formats::dds
