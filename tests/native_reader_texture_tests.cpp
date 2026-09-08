#include "dmc_rengine/codecs/dds_bc.hpp"
#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/formats/ptx_binary.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

void put_u16(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint16_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::uint32_t full_mip_payload_size(
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
    bool dxt5) {
    std::vector<std::byte> result(
        full_mip_payload_size(width, height, dxt5), std::byte{0});
    for (std::size_t index = 0U; index < result.size(); ++index) {
        result[index] = static_cast<std::byte>((index * 29U + 7U) & 0xFFU);
    }
    return result;
}

[[nodiscard]] std::vector<std::byte> make_reader_single_mip_dds() {
    constexpr std::uint32_t width = 64U;
    constexpr std::uint32_t height = 64U;
    constexpr std::uint32_t payload_size = 64U * 64U / 2U;
    std::vector<std::byte> bytes(128U + payload_size, std::byte{0});
    bytes[0U] = std::byte{'D'};
    bytes[1U] = std::byte{'D'};
    bytes[2U] = std::byte{'S'};
    bytes[3U] = std::byte{' '};
    put_u32(bytes, 4U, 124U);
    put_u32(bytes, 8U, 0x00081007U);
    put_u32(bytes, 12U, height);
    put_u32(bytes, 16U, width);
    put_u32(bytes, 20U, payload_size);
    put_u32(bytes, 28U, 1U);
    put_u32(bytes, 76U, 32U);
    put_u32(bytes, 80U, 4U);
    bytes[84U] = std::byte{'D'};
    bytes[85U] = std::byte{'X'};
    bytes[86U] = std::byte{'T'};
    bytes[87U] = std::byte{'1'};
    put_u32(bytes, 108U, 0x00001000U);

    // First DXT1 block decodes to opaque red. Remaining blocks can stay zero;
    // this fixture tests structural partial-mip acceptance and bounded decode.
    put_u16(bytes, 128U, 0xF800U);
    put_u16(bytes, 130U, 0x07E0U);
    put_u32(bytes, 132U, 0U);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_descriptor(
    const dmc::rengine::profiles::dmc3::Dmc3DdsBuildResult& dds) {
    const auto width = dds.document.width;
    const auto height = dds.document.height;
    const auto mip_count = dds.document.mip_map_count;
    const bool dxt5 = dds.document.compression ==
        dmc::rengine::profiles::dmc3::Dmc3DdsCompression::dxt5;

    std::vector<std::byte> descriptor(0x70U, std::byte{0});
    put_u32(
        descriptor,
        0x08U,
        0x20000U | (mip_count << 8U) | (dxt5 ? 0x88U : 0x86U));
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (height << 16U) | width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, dds.document.payload_size);
    put_u32(descriptor, 0x44U, (height << 16U) | width);
    put_u32(
        descriptor,
        0x48U,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(width)));
    put_u32(
        descriptor,
        0x4CU,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(height)));
    put_u32(descriptor, 0x60U, dxt5 ? 4U : 0U);
    put_u32(descriptor, 0x64U, dds.document.total_size);
    put_u32(descriptor, 0x68U, 8U);
    return descriptor;
}

[[nodiscard]] std::vector<std::byte> make_ptx_bundle() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    constexpr std::uint32_t width = 64U;
    constexpr std::uint32_t height = 64U;
    const auto payload = make_payload(width, height, true);
    const auto dds = dmc3::Dmc3DdsProfile::build(
        width,
        height,
        dmc3::Dmc3DdsCompression::dxt5,
        std::span<const std::byte>{payload.data(), payload.size()});
    assert(dds.ok());

    const auto descriptor = make_descriptor(dds);
    std::vector<std::byte> bundle(
        0x800U + descriptor.size() + dds.bytes.size(), std::byte{0});
    put_u32(bundle, 0U, 1U);
    // A zero final sector span is evidence-backed and means the final DDS ends
    // exactly at EOF rather than at another 0x800 sector boundary.
    put_u32(bundle, 4U, 0U);
    std::copy(descriptor.begin(), descriptor.end(), bundle.begin() + 0x800U);
    std::copy(dds.bytes.begin(), dds.bytes.end(), bundle.begin() + 0x870U);
    return bundle;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef ptx_resource(
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "native-reader-texture-test",
            .logical_path = "texture/test.ptx",
            .container_chain = "NBZ[0]/PAC[0]",
            .offset = 0x2000U,
            .size = size,
        },
        .display_name = "test.ptx",
        .format = "ptx",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };
}

} // namespace

int main() {
    namespace codecs = dmc::rengine::codecs;
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    namespace formats = dmc::rengine::formats;
    namespace integration = dmc::rengine::integration;

    // Reader-side DDS support is intentionally broader than the strict DMC3
    // authoring profile. This mirrors real standalone DXT DDS resources that
    // carry only the base mip and standard one-mip caps/flags.
    auto reader_dds = make_reader_single_mip_dds();
    auto reader_parse = codecs::dds_bc::parse(
        std::span<const std::byte>{reader_dds.data(), reader_dds.size()});
    assert(reader_parse.ok());
    assert(reader_parse.document.width == 64U);
    assert(reader_parse.document.height == 64U);
    assert(reader_parse.document.mip_count == 1U);
    assert(reader_parse.document.total_size == reader_dds.size());
    assert(dmc3::Dmc3DdsProfile::parse(
        std::span<const std::byte>{reader_dds.data(), reader_dds.size()}).status ==
        dmc3::Dmc3DdsStatus::invalid_mip_chain);

    const auto decoded = codecs::dds_bc::decode_base_mip_rgba8(
        std::span<const std::byte>{reader_dds.data(), reader_dds.size()},
        reader_parse.document);
    assert(decoded.ok);
    assert(decoded.image.available());
    assert(decoded.image.width == 64U);
    assert(decoded.image.height == 64U);
    assert(decoded.image.rgba8[0U] == 255U);
    assert(decoded.image.rgba8[1U] == 0U);
    assert(decoded.image.rgba8[2U] == 0U);
    assert(decoded.image.rgba8[3U] == 255U);

    // The codec reports the exact bounded DDS extent and deliberately leaves
    // any resource-level carrier/alignment bytes to the caller.
    reader_dds.insert(reader_dds.end(), 32U, std::byte{0});
    reader_parse = codecs::dds_bc::parse(
        std::span<const std::byte>{reader_dds.data(), reader_dds.size()});
    assert(reader_parse.ok());
    assert(reader_parse.document.total_size + 32U == reader_dds.size());

    const auto bytes = make_ptx_bundle();
    const auto resource = ptx_resource(bytes.size());

    const auto scan = formats::ptx::Reader::scan(
        std::span<const std::byte>{bytes.data(), bytes.size()});
    assert(scan.ok());
    assert(scan.recognized);
    assert(scan.framing.document.textures.size() == 1U);
    assert(scan.framing.document.textures[0].width == 64U);
    assert(scan.framing.document.textures[0].height == 64U);

    const auto document = formats::ptx::build_binary_document(
        resource,
        std::span<const std::byte>{bytes.data(), bytes.size()},
        scan);
    assert(document.has_value());
    assert(document->find_region("ptx-header") != nullptr);
    assert(document->find_region("ptx-texture-0-descriptor") != nullptr);
    assert(document->find_region("ptx-texture-0-dds") != nullptr);
    assert(document->find_field("ptx-texture-count") != nullptr);
    assert(document->coverage_bytes() == bytes.size());
    assert(document->unknown_ranges().empty());

    dmc::rengine::gdspaces::ResourcePayload parent{
        .resource = resource,
        .bytes = bytes,
        .diagnostics = {},
    };
    const auto expansion = formats::ptx::Reader::expand_dds_children(parent);
    assert(expansion.usable());
    assert(expansion.parser_format == "PTX");
    assert(expansion.children.size() == 1U);
    assert(expansion.children[0].payload.resource.format == "dds");
    assert(expansion.children[0].payload.byte_provenance.has_value());

    // End-to-end PTX product path through the modular registry.
    integration::ProjectWorkspace project;
    assert(project.create_session(parent));
    const auto ptx_analysis = integration::ResourceAnalyzer::analyze(
        project, resource.id);
    assert(ptx_analysis.ok());
    assert(ptx_analysis.parser_id == "formats.ptx-dmc3-reader");
    assert(ptx_analysis.binary_document_attached);
    const auto* ptx_session = project.find_session(resource.id);
    assert(ptx_session != nullptr);
    assert(ptx_session->binary_document() != nullptr);
    assert(ptx_session->parser_validation() != nullptr);
    assert(ptx_session->parser_validation()->recognized);

    // The child produced by PTX is a canonical DDS resource and therefore
    // re-enters the same Native Reader registry without any special case.
    const auto child = expansion.children[0].payload;
    assert(project.create_session(child));
    const auto dds_analysis = integration::ResourceAnalyzer::analyze(
        project, child.resource.id);
    assert(dds_analysis.ok());
    assert(dds_analysis.parser_id == "formats.dds-dmc3-reader");
    assert(dds_analysis.binary_document_attached);
    const auto* dds_session = project.find_session(child.resource.id);
    assert(dds_session != nullptr);
    assert(dds_session->binary_document() != nullptr);
    assert(dds_session->parser_validation() != nullptr);
    assert(dds_session->parser_validation()->recognized);

    return 0;
}
