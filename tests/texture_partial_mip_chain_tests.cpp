#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/profiles/dmc3/texture_mip_chain_contract.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

// A texture's mip chain, against what the runtime actually does with it.
//
// The parser once refused a DDS declaring three mips where the dimensions
// allow eight, on the strength of every corpus sample carrying a full chain.
// That was an inference from a sample rather than a fact about the game, and
// the image says so plainly: the loader reads dwMipMapCount verbatim,
// substitutes 1 only for a declared 0, bounds it at 15 from above and nowhere
// from below, and answers a single-level file by generating the rest.
// TextureMipChainContract carries the receipts.
//
// So these pin two things at once. Completeness is not a rule, in either
// direction — it is measured, recorded, and available as an opt-in for a
// caller who wants corpus fidelity rather than loadability. And the bounds
// that *are* real are the runtime's own, which are tighter than the round
// numbers that stood in for them.

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

void a_partial_chain_is_read_by_default() {
    const auto bytes = wrapped_texture(3U);

    const auto framed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes});
    assert(framed.ok());
    assert(!framed.document.textures.empty());

    const auto& texture = framed.document.textures.front();
    assert(texture.mip_map_count == 3U);
    // What was found travels with the entry either way, so nothing downstream
    // has to recompute a chain length to know the texture is partial.
    assert(texture.full_mip_chain_length == 8U);
    assert(texture.partial_mip_chain);
}

void corpus_fidelity_is_still_available_on_request() {
    const auto bytes = wrapped_texture(3U);

    // The flag survives the default flip: a caller writing a mod that must
    // match retail byte for byte can still ask for a complete chain. What it
    // no longer does is speak for callers who only need the game to load the
    // file.
    dmc3::TextureSlotFramingSafety fidelity;
    fidelity.require_full_mip_chain = true;
    const auto framed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes}, fidelity);
    assert(!framed.ok());
    assert(framed.status == dmc3::TextureSlotFramingStatus::invalid_dds);
    // The refusal names the numbers, and says whose choice it was — the
    // runtime would have taken this texture.
    assert(framed.detail.find("3 mip levels") != std::string::npos);
    assert(framed.detail.find("128x128") != std::string::npos);
    assert(framed.detail.find("runtime would load this") != std::string::npos);
}

void a_full_chain_is_accepted_either_way() {
    const auto bytes = wrapped_texture(8U);

    const auto relaxed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes});
    assert(relaxed.ok());
    assert(!relaxed.document.textures.front().partial_mip_chain);

    dmc3::TextureSlotFramingSafety fidelity;
    fidelity.require_full_mip_chain = true;
    const auto strict = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes}, fidelity);
    assert(strict.ok());
    assert(!strict.document.textures.front().partial_mip_chain);
}

void a_single_level_texture_is_read() {
    // The case the runtime has a designed path for: one declared level, the
    // rest generated at load. If anything here still treats a short chain as
    // malformed, this is where it shows.
    const auto bytes = wrapped_texture(
        dmc3::TextureMipChainContract::mip_count_that_triggers_autogen);

    const auto framed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes});
    assert(framed.ok());
    assert(framed.document.textures.front().mip_map_count == 1U);
    assert(framed.document.textures.front().partial_mip_chain);
}

void a_chain_longer_than_the_runtime_loads_is_refused() {
    using Contract = dmc3::TextureMipChainContract;
    static_assert(Contract::max_mip_count == 15U);

    // Past D3D11_REQ_MIP_LEVELS the loader refuses outright, so a container
    // declaring this holds a texture the game cannot use. That is structural,
    // not a matter of authoring taste, and it holds for reading too.
    const auto bytes = wrapped_texture(Contract::max_mip_count + 1U);

    const auto framed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes});
    assert(!framed.ok());
    assert(framed.status == dmc3::TextureSlotFramingStatus::invalid_dds);
    assert(framed.detail.find("runtime cannot") != std::string::npos);
    assert(framed.detail.find("15 mip levels") != std::string::npos);
}

void a_dimension_past_the_runtime_bound_is_refused() {
    using Contract = dmc3::TextureMipChainContract;
    static_assert(Contract::max_dimension == 16384U);

    auto bytes = wrapped_texture(8U);
    // Only the DDS width, so the bound is what refuses this rather than the
    // descriptor cross-check further down.
    u32(bytes, 0x70U + 16U, Contract::max_dimension + 1U);

    const auto framed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{bytes});
    assert(!framed.ok());
    assert(framed.status == dmc3::TextureSlotFramingStatus::invalid_dds);
    assert(framed.detail.find("16384") != std::string::npos);
}

void a_zero_dimension_is_refused_either_way() {
    auto bytes = wrapped_texture(8U);
    u32(bytes, 0x70U + 16U, 0U); // width 0

    dmc3::TextureSlotFramingSafety fidelity;
    fidelity.require_full_mip_chain = true;
    for (const auto safety : {dmc3::TextureSlotFramingSafety{}, fidelity}) {
        const auto framed = dmc3::TextureSlotFramingParser::parse(
            std::span<const std::byte>{bytes}, safety);
        // Relaxing the chain rule must not relax the structural bounds: a DDS
        // with a zero dimension cannot be read at all, and saying otherwise
        // would trade one over-strict refusal for a crash.
        assert(!framed.ok());
        assert(framed.detail.find("runtime cannot") != std::string::npos);
    }
}

} // namespace

int main() {
    a_partial_chain_is_read_by_default();
    corpus_fidelity_is_still_available_on_request();
    a_full_chain_is_accepted_either_way();
    a_single_level_texture_is_read();
    a_chain_longer_than_the_runtime_loads_is_refused();
    a_dimension_past_the_runtime_bound_is_refused();
    a_zero_dimension_is_refused_either_way();
    return 0;
}
