#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/formats/ptx_binary.hpp"

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
        bytes[index] = static_cast<std::byte>((index * 29U + 7U) & 0xFFU);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_descriptor(
    const std::vector<std::byte>& dds,
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    bool dxt5,
    bool secondary_half) {
    std::vector<std::byte> descriptor(0x70U, std::byte{0});
    const auto secondary_width = secondary_half ? width / 2U : width;
    const auto secondary_height = secondary_half ? height / 2U : height;
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

[[nodiscard]] std::vector<std::byte> wrapped_dds_fixture() {
    constexpr std::uint32_t mip_count = 5U;
    const auto dds = make_dds(16U, 16U, mip_count, true);
    const auto descriptor = make_descriptor(
        dds, 16U, 16U, mip_count, true, false);
    std::vector<std::byte> bytes;
    bytes.reserve(descriptor.size() + dds.size());
    bytes.insert(bytes.end(), descriptor.begin(), descriptor.end());
    bytes.insert(bytes.end(), dds.begin(), dds.end());
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload payload(
    const std::vector<std::byte>& bytes) {
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "ptx-native-reader-test",
                .logical_path = "texture/test.ptx",
                .container_chain = "NBZ[0]/PAC[0]/TEXTURE_BUNDLE[0]",
                .offset = 0x2000U,
                .size = bytes.size(),
            },
            .display_name = "test.ptx",
            .format = "ptx",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = bytes,
        .diagnostics = {},
    };
}

} // namespace

int main() {
    namespace ptx = dmc::rengine::formats::ptx;

    const auto bytes = bundle_fixture();
    const auto scan = ptx::Reader::scan(
        std::span<const std::byte>{bytes.data(), bytes.size()});
    assert(scan.recognized);
    assert(scan.ok());
    assert(scan.framing.document.textures.size() == 2U);
    assert(scan.framing.document.textures[0].dds_offset == 0x870U);
    assert(scan.framing.document.textures[1].dds_offset == 0x1070U);

    const auto parent = payload(bytes);
    const auto document = ptx::build_binary_document(
        parent.resource,
        std::span<const std::byte>{bytes.data(), bytes.size()},
        scan);
    assert(document.has_value());
    assert(document->find_region("ptx-header") != nullptr);
    assert(document->find_region("ptx-texture-0-descriptor") != nullptr);
    assert(document->find_region("ptx-texture-0-dds") != nullptr);
    assert(document->find_region("ptx-texture-0-padding") != nullptr);
    assert(document->find_region("ptx-texture-1-descriptor") != nullptr);
    assert(document->find_region("ptx-texture-1-dds") != nullptr);
    assert(document->find_field("ptx-texture-count") != nullptr);
    assert(document->find_field("ptx-texture-1-compression") != nullptr);
    assert(document->coverage_bytes() == bytes.size());
    assert(document->unknown_ranges().empty());

    const auto expansion = ptx::Reader::expand_dds_children(parent);
    assert(expansion.usable());
    assert(expansion.parser_format == "PTX");
    assert(expansion.children.size() == 2U);
    assert(expansion.children[0].payload.resource.format == "dds");
    assert(expansion.children[0].payload.resource.id.offset == 0x2870U);
    assert(expansion.children[1].payload.resource.format == "dds");
    assert(expansion.children[1].payload.resource.id.offset == 0x3070U);
    assert(expansion.children[0].payload.bytes[0] == std::byte{'D'});
    assert(expansion.children[0].payload.bytes[1] == std::byte{'D'});
    assert(expansion.children[0].payload.bytes[2] == std::byte{'S'});

    const auto wrapped = wrapped_dds_fixture();
    const auto wrapped_scan = ptx::Reader::scan(
        std::span<const std::byte>{wrapped.data(), wrapped.size()});
    assert(!wrapped_scan.recognized);
    assert(!wrapped_scan.ok());

    auto bad_padding = bytes;
    bad_padding[0x0A00U] = std::byte{1};
    const auto bad_scan = ptx::Reader::scan(
        std::span<const std::byte>{bad_padding.data(), bad_padding.size()});
    assert(!bad_scan.ok());
    assert(!bad_scan.diagnostics.empty());

    std::vector<std::byte> arbitrary(0x200U, std::byte{0});
    const auto arbitrary_scan = ptx::Reader::scan(
        std::span<const std::byte>{arbitrary.data(), arbitrary.size()});
    assert(!arbitrary_scan.recognized);
    assert(!arbitrary_scan.ok());

    return 0;
}
