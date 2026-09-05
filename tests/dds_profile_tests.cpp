#include "dmc_rengine/formats/dds.hpp"
#include "dmc_rengine/formats/dds_binary.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

[[nodiscard]] std::uint32_t read_u32(
    const std::vector<std::byte>& bytes,
    std::size_t offset) {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
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

[[nodiscard]] std::vector<std::byte> payload(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5) {
    std::vector<std::byte> bytes(
        full_mip_payload_size(width, height, dxt5), std::byte{0});
    for (std::size_t index = 0U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>((index * 37U + 11U) & 0xFFU);
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "dds-native-reader-test",
            .logical_path = "texture/test.dds",
            .container_chain = "NBZ[0]/PTX[0]",
            .offset = 0U,
            .size = size,
        },
        .display_name = "test.dds",
        .format = "dds",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto dxt1_payload = payload(256U, 128U, false);
    const auto dxt1 = dmc3::Dmc3DdsProfile::build(
        256U, 128U, dmc3::Dmc3DdsCompression::dxt1,
        std::span<const std::byte>{dxt1_payload.data(), dxt1_payload.size()});
    assert(dxt1.ok());
    assert(dxt1.document.width == 256U);
    assert(dxt1.document.height == 128U);
    assert(dxt1.document.mip_map_count == 9U);
    assert(dxt1.document.payload_size == dxt1_payload.size());
    assert(dxt1.bytes.size() == 128U + dxt1_payload.size());
    assert(read_u32(dxt1.bytes, 4U) == 124U);
    assert(read_u32(dxt1.bytes, 8U) == 0x000A1007U);
    assert(read_u32(dxt1.bytes, 20U) == 0x00010000U);
    assert(read_u32(dxt1.bytes, 24U) == 0U);
    assert(read_u32(dxt1.bytes, 76U) == 32U);
    assert(read_u32(dxt1.bytes, 80U) == 4U);
    assert(dxt1.bytes[84U] == std::byte{'D'});
    assert(dxt1.bytes[85U] == std::byte{'X'});
    assert(dxt1.bytes[86U] == std::byte{'T'});
    assert(dxt1.bytes[87U] == std::byte{'1'});
    assert(read_u32(dxt1.bytes, 108U) == 0x00401008U);
    assert(std::all_of(
        dxt1.bytes.begin() + 32, dxt1.bytes.begin() + 76,
        [](std::byte value) { return value == std::byte{0}; }));

    const auto dxt1_reparse = dmc3::Dmc3DdsProfile::parse(
        std::span<const std::byte>{dxt1.bytes.data(), dxt1.bytes.size()});
    assert(dxt1_reparse.ok());
    assert(dxt1_reparse.document.total_size == dxt1.bytes.size());

    const auto native_scan = dmc::rengine::formats::dds::Reader::scan(
        std::span<const std::byte>{dxt1.bytes.data(), dxt1.bytes.size()});
    assert(native_scan.recognized);
    assert(native_scan.ok());
    assert(native_scan.profile.document.width == 256U);
    assert(native_scan.profile.document.height == 128U);
    assert(native_scan.profile.document.compression == dmc3::Dmc3DdsCompression::dxt1);
    const auto native_document =
        dmc::rengine::formats::dds::build_binary_document(
            resource(dxt1.bytes.size()),
            std::span<const std::byte>{dxt1.bytes.data(), dxt1.bytes.size()},
            native_scan);
    assert(native_document.has_value());
    assert(native_document->find_region("dds-header") != nullptr);
    assert(native_document->find_region("dds-payload") != nullptr);
    assert(native_document->find_field("dds-width") != nullptr);
    assert(native_document->find_field("dds-height") != nullptr);
    assert(native_document->find_field("dds-fourcc") != nullptr);
    assert(native_document->coverage_bytes() == dxt1.bytes.size());

    // End-to-end product path: canonical workspace -> modular registry -> DDS
    // module -> Binary Inspector document -> parser completion event.
    dmc::rengine::integration::ProjectWorkspace project;
    const auto dds_resource = resource(dxt1.bytes.size());
    assert(project.create_session(dmc::rengine::gdspaces::ResourcePayload{
        .resource = dds_resource,
        .bytes = dxt1.bytes,
        .diagnostics = {},
    }));
    const auto analysis = dmc::rengine::integration::ResourceAnalyzer::analyze(
        project, dds_resource.id);
    assert(analysis.ok());
    assert(analysis.parser_available);
    assert(analysis.parser_id == "formats.dds-dmc3-reader");
    assert(analysis.binary_document_attached);
    const auto* dds_session = project.find_session(dds_resource.id);
    assert(dds_session != nullptr);
    assert(dds_session->binary_document() != nullptr);
    assert(dds_session->parser_validation() != nullptr);
    assert(dds_session->parser_validation()->recognized);
    assert(dds_session->events().by_type(
        dmc::rengine::integration::WorkspaceEventType::parser_completed).size() == 1U);

    const auto dxt5_payload = payload(512U, 512U, true);
    const auto dxt5 = dmc3::Dmc3DdsProfile::build(
        512U, 512U, dmc3::Dmc3DdsCompression::dxt5,
        std::span<const std::byte>{dxt5_payload.data(), dxt5_payload.size()});
    assert(dxt5.ok());
    assert(dxt5.document.mip_map_count == 10U);
    assert(read_u32(dxt5.bytes, 20U) == 0x00020000U);
    assert(dxt5.bytes[87U] == std::byte{'5'});

    auto bad_flags = dxt5.bytes;
    put_u32(bad_flags, 8U, read_u32(bad_flags, 8U) ^ 0x8U);
    assert(
        dmc3::Dmc3DdsProfile::parse(
            std::span<const std::byte>{bad_flags.data(), bad_flags.size()}).status ==
        dmc3::Dmc3DdsStatus::invalid_header);
    const auto bad_native = dmc::rengine::formats::dds::Reader::scan(
        std::span<const std::byte>{bad_flags.data(), bad_flags.size()});
    assert(bad_native.recognized);
    assert(!bad_native.ok());
    assert(!bad_native.diagnostics.empty());

    auto bad_linear = dxt5.bytes;
    put_u32(bad_linear, 20U, 0x10000U);
    assert(
        dmc3::Dmc3DdsProfile::parse(
            std::span<const std::byte>{bad_linear.data(), bad_linear.size()}).status ==
        dmc3::Dmc3DdsStatus::invalid_header);

    auto bad_reserved = dxt5.bytes;
    put_u32(bad_reserved, 32U, 1U);
    assert(
        dmc3::Dmc3DdsProfile::parse(
            std::span<const std::byte>{bad_reserved.data(), bad_reserved.size()}).status ==
        dmc3::Dmc3DdsStatus::invalid_header);

    auto bad_caps = dxt5.bytes;
    put_u32(bad_caps, 108U, 0x1000U);
    assert(
        dmc3::Dmc3DdsProfile::parse(
            std::span<const std::byte>{bad_caps.data(), bad_caps.size()}).status ==
        dmc3::Dmc3DdsStatus::invalid_header);

    auto bad_mips = dxt5.bytes;
    put_u32(bad_mips, 28U, 1U);
    assert(
        dmc3::Dmc3DdsProfile::parse(
            std::span<const std::byte>{bad_mips.data(), bad_mips.size()}).status ==
        dmc3::Dmc3DdsStatus::invalid_mip_chain);

    auto truncated_payload = dxt5.bytes;
    truncated_payload.pop_back();
    assert(
        dmc3::Dmc3DdsProfile::parse(
            std::span<const std::byte>{
                truncated_payload.data(), truncated_payload.size()}).status ==
        dmc3::Dmc3DdsStatus::invalid_payload_size);

    const auto bad_payload = std::vector<std::byte>(7U, std::byte{0});
    assert(
        dmc3::Dmc3DdsProfile::build(
            256U, 256U, dmc3::Dmc3DdsCompression::dxt1,
            std::span<const std::byte>{bad_payload.data(), bad_payload.size()}).status ==
        dmc3::Dmc3DdsStatus::invalid_payload_size);

    const auto valid_small_payload = payload(64U, 64U, false);
    assert(
        dmc3::Dmc3DdsProfile::build(
            64U, 64U, dmc3::Dmc3DdsCompression::dxt1,
            std::span<const std::byte>{
                valid_small_payload.data(), valid_small_payload.size()}).ok());

    const auto below_domain_payload = payload(32U, 32U, false);
    assert(
        dmc3::Dmc3DdsProfile::build(
            32U, 32U, dmc3::Dmc3DdsCompression::dxt1,
            std::span<const std::byte>{
                below_domain_payload.data(), below_domain_payload.size()}).status ==
        dmc3::Dmc3DdsStatus::unsupported_dimensions);

    const auto above_domain_payload = payload(2048U, 2048U, true);
    assert(
        dmc3::Dmc3DdsProfile::build(
            2048U, 2048U, dmc3::Dmc3DdsCompression::dxt5,
            std::span<const std::byte>{
                above_domain_payload.data(), above_domain_payload.size()}).status ==
        dmc3::Dmc3DdsStatus::unsupported_dimensions);

    const std::vector<std::byte> wrong_magic{
        std::byte{'N'}, std::byte{'O'}, std::byte{'P'}, std::byte{'E'}};
    const auto wrong_scan = dmc::rengine::formats::dds::Reader::scan(
        std::span<const std::byte>{wrong_magic.data(), wrong_magic.size()});
    assert(!wrong_scan.recognized);
    assert(!wrong_scan.ok());

    return 0;
}
