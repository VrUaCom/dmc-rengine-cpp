#include "dmc_rengine/formats/ptx.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

// A texture pack has no magic of its own; recognition comes entirely from
// TextureSlotFramingParser closing its own arithmetic. This builds the same
// two-texture bundle shape texture_slot_framing_tests.cpp already validates
// against that parser directly, so PtxParser is exercised as the thin
// container view over it that it claims to be.

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
    const auto payload_size = block_payload_size(width, height, mip_count, dxt5);
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
    put_u32(descriptor, 0x08U, 0x20000U | (mip_count << 8U) | encoding_low);
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (height << 16U) | width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, static_cast<std::uint32_t>(dds.size() - 128U));
    put_u32(descriptor, 0x44U, (secondary_height << 16U) | secondary_width);
    put_u32(
        descriptor, 0x48U,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(secondary_width)));
    put_u32(
        descriptor, 0x4CU,
        std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(secondary_height)));
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
    const auto descriptor0 = make_descriptor(dds0, 16U, 16U, mip_count, false, false);
    const auto descriptor1 = make_descriptor(dds1, 16U, 16U, mip_count, true, true);

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

void test_empty_bytes_are_rejected() {
    using namespace dmc::rengine::formats;
    const auto result = PtxParser::parse({});
    assert(!result.ok());
    assert(!PtxParser::structurally_valid({}));
}

void test_two_texture_bundle_is_accepted_as_a_container() {
    using namespace dmc::rengine::formats;
    const auto bundle = bundle_fixture();
    const auto result = PtxParser::parse(bundle);
    assert(result.ok());
    assert(PtxParser::structurally_valid(bundle));
    assert(result.document->format == "ptx");
    assert(result.document->declared_slot_count == 2U);
    assert(result.document->entries.size() == 2U);
    for (const auto& entry : result.document->entries) {
        assert(entry.populated);
        assert(entry.synthetic_name);
    }
}

// A single wrapped DDS is a texture, not a pack, so PTX must decline it
// rather than invent a one-child container.
void test_single_wrapped_texture_is_not_a_pack() {
    using namespace dmc::rengine::formats;
    constexpr std::uint32_t mip_count = 5U;
    const auto dds = make_dds(16U, 16U, mip_count, true);
    const auto descriptor = make_descriptor(dds, 16U, 16U, mip_count, true, true);
    std::vector<std::byte> bytes;
    bytes.reserve(descriptor.size() + dds.size());
    bytes.insert(bytes.end(), descriptor.begin(), descriptor.end());
    bytes.insert(bytes.end(), dds.begin(), dds.end());

    const auto result = PtxParser::parse(bytes);
    assert(!result.ok());
    assert(result.error == PtxParseError::not_a_texture_pack);
}

} // namespace

int main() {
    test_empty_bytes_are_rejected();
    test_two_texture_bundle_is_accepted_as_a_container();
    test_single_wrapped_texture_is_not_a_pack();
    return 0;
}
