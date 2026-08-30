#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

// Reading a texture whose mip chain is not complete.
//
// Every texture in the corpus this framing was recovered from carries a full
// chain, and the parser turned that observation into a refusal: a DDS
// declaring three mips where the dimensions allow eight was rejected outright.
// That is a claim about the runtime the reverse work never established, and a
// real retail `at.ptx` could not be opened because of it.
//
// The bound stays where it was earned. Authoring keeps it, because writing a
// chain length nobody has seen the game load is a risk this project does not
// take on an operator's behalf. Reading drops it, because parsing a DDS header
// does not depend on the chain being complete.

namespace {

namespace dmc3 = dmc::rengine::profiles::dmc3;

void u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] =
            static_cast<std::byte>((value >> (index * 8U)) & 0xFFU);
    }
}

void text(std::vector<std::byte>& bytes, std::size_t offset, std::string_view value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(value[index]);
    }
}

// One descriptor-framed DXT1 texture carrying `mips` levels.
//
// The descriptor field layout is mirrored from a real texture read out of a
// retail PTX (a 128x128 DXT1 with a complete eight-level chain), so the
// fixture exercises the same fields a real slot does rather than a shape
// invented to satisfy the parser. Only the mip count and the two sizes that
// follow from it are varied.
[[nodiscard]] std::vector<std::byte> wrapped_texture(std::uint32_t mips) {
    constexpr std::size_t kDescriptor = 0x70U;
    constexpr std::size_t kDdsHeader = 128U;
    constexpr std::uint32_t kSide = 128U;

    // DXT1: eight bytes per 4x4 block, halving each level.
    std::uint32_t payload = 0U;
    std::uint32_t dimension = kSide;
    for (std::uint32_t level = 0; level < mips; ++level) {
        const auto blocks = (dimension + 3U) / 4U;
        payload += blocks * blocks * 8U;
        dimension = dimension > 1U ? dimension / 2U : 1U;
    }
    const auto dds_size = static_cast<std::uint32_t>(kDdsHeader) + payload;

    std::vector<std::byte> bytes(kDescriptor + dds_size, std::byte{0});

    // Descriptor, in the real one's terms.
    // The encoding word carries the declared mip count, so it follows the
    // chain rather than a full one: 0x20000 | (mips << 8) | low byte, read off
    // the real texture this fixture mirrors.
    u32(bytes, 0x08U, 0x20000U | (mips << 8U) | 0x86U);
    u32(bytes, 0x0CU, 43748U);               // constant
    u32(bytes, 0x10U, (kSide << 16) | kSide); // packed dimensions
    u32(bytes, 0x14U, 1U);                   // constant
    u32(bytes, 0x18U, 256U);                 // row bytes
    u32(bytes, 0x20U, 64U);                  // constant
    u32(bytes, 0x38U, payload);              // payload size
    u32(bytes, 0x44U, (kSide << 16) | kSide); // secondary dimensions
    u32(bytes, 0x48U, 1006632960U);
    u32(bytes, 0x4CU, 1006632960U);
    u32(bytes, 0x64U, dds_size);             // the DDS size the parser checks
    u32(bytes, 0x68U, 8U);                   // sector span

    const auto dds = kDescriptor;
    text(bytes, dds, "DDS ");
    u32(bytes, dds + 4U, 124U);
    u32(bytes, dds + 12U, kSide);  // height
    u32(bytes, dds + 16U, kSide);  // width
    u32(bytes, dds + 28U, mips);
    text(bytes, dds + 84U, "DXT1");
    return bytes;
}

void the_reader_accepts_a_partial_chain() {
    const auto bytes = wrapped_texture(3U);

    dmc3::TextureSlotFramingSafety reading;
    reading.require_full_mip_chain = false;
    const auto framed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes}, reading);
    assert(framed.ok());
    assert(!framed.document.textures.empty());

    const auto& texture = framed.document.textures.front();
    assert(texture.mip_map_count == 3U);
    // What was found travels with the entry, so a caller that relaxed the
    // bound can still tell which textures are partial.
    assert(texture.full_mip_chain_length == 8U);
    assert(texture.partial_mip_chain);
}

void authoring_still_refuses_it() {
    const auto bytes = wrapped_texture(3U);

    // The default is the authoring bound, and the packed-reflow writer takes
    // the default. If this ever passes, the writer has quietly been allowed to
    // emit a chain length no corpus sample demonstrates.
    const auto framed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes});
    assert(!framed.ok());
    assert(framed.status == dmc3::TextureSlotFramingStatus::invalid_dds);
    // The refusal names the numbers rather than gesturing at a domain.
    assert(framed.detail.find("3 mip levels") != std::string::npos);
    assert(framed.detail.find("128x128") != std::string::npos);
}

void a_full_chain_is_accepted_by_both() {
    const auto bytes = wrapped_texture(8U);

    const auto authored = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes});
    assert(authored.ok());
    assert(!authored.document.textures.front().partial_mip_chain);

    dmc3::TextureSlotFramingSafety reading;
    reading.require_full_mip_chain = false;
    const auto read = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes}, reading);
    assert(read.ok());
    assert(!read.document.textures.front().partial_mip_chain);
}

void unusable_dimensions_are_still_refused_either_way() {
    auto bytes = wrapped_texture(8U);
    u32(bytes, 0x70U + 16U, 0U); // width 0

    dmc3::TextureSlotFramingSafety reading;
    reading.require_full_mip_chain = false;
    const auto read = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes}, reading);
    // Relaxing the chain bound must not relax the structural ones: a DDS with
    // a zero dimension cannot be read at all, and saying otherwise would trade
    // one over-strict refusal for a crash.
    assert(!read.ok());
    assert(read.detail.find("unusable dimensions") != std::string::npos);
}

} // namespace

int main() {
    the_reader_accepts_a_partial_chain();
    authoring_still_refuses_it();
    a_full_chain_is_accepted_by_both();
    unusable_dimensions_are_still_refused_either_way();
    return 0;
}
