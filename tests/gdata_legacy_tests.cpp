#include "dmc_rengine/formats/evt.hpp"
#include "dmc_rengine/formats/mod/version.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing_compat.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

namespace dmc3 = dmc::rengine::profiles::dmc3;
namespace evt = dmc::rengine::formats::evt;
namespace mod = dmc::rengine::formats::mod;
namespace gdspaces = dmc::rengine::gdspaces;

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    assert(offset <= bytes.size() && bytes.size() - offset >= 4U);
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::uint32_t dxt5_base_payload(std::uint32_t width, std::uint32_t height) {
    return ((width + 3U) / 4U) * ((height + 3U) / 4U) * 16U;
}

[[nodiscard]] std::vector<std::byte> single_level_dxt5_dds(
    std::uint32_t width,
    std::uint32_t height) {
    const auto payload = dxt5_base_payload(width, height);
    std::vector<std::byte> out(0x80U + payload, std::byte{0});
    out[0] = std::byte{'D'};
    out[1] = std::byte{'D'};
    out[2] = std::byte{'S'};
    out[3] = std::byte{' '};
    put_u32(out, 0x04U, 124U);
    put_u32(out, 0x08U, 0x00081007U);
    put_u32(out, 0x0CU, height);
    put_u32(out, 0x10U, width);
    put_u32(out, 0x14U, payload);
    put_u32(out, 0x18U, 0U);
    put_u32(out, 0x1CU, 0U);
    put_u32(out, 0x4CU, 32U);
    put_u32(out, 0x50U, 4U);
    out[0x54U] = std::byte{'D'};
    out[0x55U] = std::byte{'X'};
    out[0x56U] = std::byte{'T'};
    out[0x57U] = std::byte{'5'};
    put_u32(out, 0x6CU, 0x00001000U);
    put_u32(out, 0x70U, 0U);
    return out;
}

[[nodiscard]] std::vector<std::byte> legacy_bundle_descriptor(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t dds_size) {
    std::vector<std::byte> out(0x70U, std::byte{0});
    const auto dimensions = (height << 16U) | width;
    put_u32(out, 0x08U, 0x00020185U);
    put_u32(out, 0x0CU, 0xAAE4U);
    put_u32(out, 0x10U, dimensions);
    put_u32(out, 0x14U, 1U);
    put_u32(out, 0x18U, 0U);
    put_u32(out, 0x20U, 0x40U);
    put_u32(out, 0x38U, 0U);
    put_u32(out, 0x44U, dimensions);
    put_u32(out, 0x48U, std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(width)));
    put_u32(out, 0x4CU, std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(height)));
    put_u32(out, 0x60U, 4U);
    put_u32(out, 0x64U, dds_size);
    put_u32(out, 0x68U, 8U);
    return out;
}

[[nodiscard]] std::vector<std::byte> legacy_ptx_fixture() {
    const auto dds = single_level_dxt5_dds(128U, 64U);
    const auto descriptor = legacy_bundle_descriptor(
        128U, 64U, static_cast<std::uint32_t>(dds.size()));
    std::vector<std::byte> out(0x3000U, std::byte{0});
    put_u32(out, 0U, 1U);
    put_u32(out, 4U, 5U);
    std::copy(descriptor.begin(), descriptor.end(), out.begin() + 0x800U);
    std::copy(dds.begin(), dds.end(), out.begin() + 0x870U);
    return out;
}

[[nodiscard]] std::vector<std::byte> legacy_tm2_named_fixture() {
    const auto dds = single_level_dxt5_dds(256U, 256U);
    std::vector<std::byte> descriptor(0x70U, std::byte{0});
    constexpr std::uint32_t descriptor_width = 128U;
    constexpr std::uint32_t descriptor_height = 128U;
    const auto dimensions = (descriptor_height << 16U) | descriptor_width;
    put_u32(descriptor, 0x08U, 0x000201A5U);
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, dimensions);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, descriptor_width * 4U);
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, dxt5_base_payload(256U, 256U));
    put_u32(descriptor, 0x44U, dimensions);
    put_u32(descriptor, 0x48U, std::bit_cast<std::uint32_t>(1.0F / 128.0F));
    put_u32(descriptor, 0x4CU, std::bit_cast<std::uint32_t>(1.0F / 128.0F));
    put_u32(descriptor, 0x60U, 5U);
    put_u32(descriptor, 0x64U, static_cast<std::uint32_t>(dds.size()));
    put_u32(descriptor, 0x68U, 8U);

    std::vector<std::byte> out;
    out.reserve(descriptor.size() + dds.size());
    out.insert(out.end(), descriptor.begin(), descriptor.end());
    out.insert(out.end(), dds.begin(), dds.end());
    return out;
}

[[nodiscard]] std::vector<std::byte> evt_fixture() {
    std::vector<std::byte> out(0x60U, std::byte{0});
    out[0] = std::byte{'E'};
    out[1] = std::byte{'V'};
    out[2] = std::byte{'T'};
    out[3] = std::byte{0};
    put_u32(out, 0x04U, 0x00010001U);

    std::size_t cursor = 0x20U;
    put_u32(out, cursor, 0x00000102U); // opcode 02, one argument
    put_u32(out, cursor + 4U, 0x37U);
    cursor += 8U;
    put_u32(out, cursor, 0x00000203U); // opcode 03, two arguments
    put_u32(out, cursor + 4U, 0x11U);
    put_u32(out, cursor + 8U, 0x22U);
    cursor += 12U;
    put_u32(out, cursor, 0x00000001U);
    cursor += 4U;
    const auto terminal = cursor;
    put_u32(out, cursor, 0x00000020U);
    put_u32(out, 0x08U, static_cast<std::uint32_t>(terminal));
    return out;
}

} // namespace

int main() {
    assert(mod::is_corpus_confirmed_structural_version(0.80F));

    const auto ptx = legacy_ptx_fixture();
    assert(!dmc3::TextureSlotFramingParser::parse(ptx).ok());
    const auto ptx_read = dmc3::TextureSlotFramingReader::parse(ptx);
    assert(ptx_read.ok());
    assert(ptx_read.compatibility_used());
    assert(ptx_read.variant == dmc3::TextureSlotReadVariant::legacy_single_mip_bundle_dxt5);
    assert(ptx_read.framing.document.textures.size() == 1U);
    assert(ptx_read.framing.document.textures[0].width == 128U);
    assert(ptx_read.framing.document.textures[0].height == 64U);
    assert(ptx_read.framing.document.textures[0].mip_map_count == 1U);
    const auto ptx_class = gdspaces::ResourceClassifier::classify("basic.ptx", ptx);
    assert(ptx_class.format == "ptx");
    assert(ptx_class.structural_confirmed);

    const auto tm2 = legacy_tm2_named_fixture();
    assert(!dmc3::TextureSlotFramingParser::parse(tm2).ok());
    const auto tm2_read = dmc3::TextureSlotFramingReader::parse(tm2);
    assert(tm2_read.ok());
    assert(tm2_read.variant == dmc3::TextureSlotReadVariant::legacy_single_mip_wrapped_dxt5);
    assert(tm2_read.framing.document.textures[0].width == 256U);
    assert(tm2_read.framing.document.textures[0].height == 256U);
    assert(tm2_read.framing.document.textures[0].secondary_width == 128U);
    assert(tm2_read.framing.document.textures[0].secondary_height == 128U);
    const auto tm2_class = gdspaces::ResourceClassifier::classify("i001_90.tm2", tm2);
    assert(tm2_class.format == "wrapped-dds");
    assert(tm2_class.structural_confirmed);

    const auto event = evt_fixture();
    const auto event_parse = evt::Parser::parse(event);
    assert(event_parse.ok());
    assert(event_parse.document.header.version == evt::corpus_version);
    assert(event_parse.document.commands.size() == 4U);
    assert(event_parse.document.commands[0].opcode == 0x02U);
    assert(event_parse.document.commands[0].argument_count == 1U);
    assert(event_parse.document.commands[0].arguments[0] == 0x37U);
    assert(event_parse.document.commands[1].opcode == 0x03U);
    assert(event_parse.document.commands[1].arguments.size() == 2U);
    assert(event_parse.document.commands.back().opcode == evt::terminal_opcode);
    const auto evt_class = gdspaces::ResourceClassifier::classify("EventTbl20.bin", event);
    assert(evt_class.format == "evt");
    assert(evt_class.magic_confirmed);
    assert(evt_class.structural_confirmed);

    auto broken = event;
    put_u32(broken, 0x20U, 0x00010102U); // upper descriptor bits are outside corpus grammar
    assert(!evt::Parser::parse(broken).ok());

    return 0;
}
