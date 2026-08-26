#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <cassert>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

// The DMC3 stage texture pack carries no magic, so everything here is about
// one question: can a format be recognized safely with nothing but its own
// arithmetic? The answer this parser gives is that the arithmetic has to close
// exactly and the blocks it predicts have to be there.

namespace {

namespace formats = dmc::rengine::formats;
namespace gdspaces = dmc::rengine::gdspaces;

constexpr std::uint64_t k_sector = formats::PtxParser::k_sector_bytes;
constexpr std::uint64_t k_descriptor = formats::PtxParser::k_descriptor_bytes;

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::vector<std::byte> make_pack(
    const std::vector<std::uint32_t>& sectors,
    bool write_images = true) {
    std::uint64_t total = 0U;
    for (const auto value : sectors) {
        total += value;
    }
    std::vector<std::byte> bytes(
        static_cast<std::size_t>((total + 1U) * k_sector), std::byte{0});
    put_u32(bytes, 0U, static_cast<std::uint32_t>(sectors.size()));
    for (std::size_t index = 0U; index < sectors.size(); ++index) {
        put_u32(bytes, 4U + index * 4U, sectors[index]);
    }

    std::uint64_t block = k_sector;
    for (const auto value : sectors) {
        if (write_images) {
            const auto image = static_cast<std::size_t>(block + k_descriptor);
            bytes[image + 0U] = static_cast<std::byte>('D');
            bytes[image + 1U] = static_cast<std::byte>('D');
            bytes[image + 2U] = static_cast<std::byte>('S');
            bytes[image + 3U] = static_cast<std::byte>(' ');
        }
        block += static_cast<std::uint64_t>(value) * k_sector;
    }
    return bytes;
}

void a_closing_pack_is_recognized() {
    const auto pack = make_pack({1U, 2U});
    const auto parsed = formats::PtxParser::parse(std::span<const std::byte>{pack});
    assert(parsed.ok());
    assert(parsed.document->format == "ptx");
    assert(parsed.document->declared_slot_count == 2U);
    assert(parsed.document->entries.size() == 2U);

    // The addressable child is the image, not the block: the per-texture
    // descriptor is container framing.
    assert(parsed.document->entries[0].offset == k_sector + k_descriptor);
    assert(parsed.document->entries[0].size == k_sector - k_descriptor);
    assert(parsed.document->entries[1].offset == 2U * k_sector + k_descriptor);
    assert(parsed.document->entries[1].size == 2U * k_sector - k_descriptor);
    for (const auto& entry : parsed.document->entries) {
        assert(entry.populated);
        assert(entry.synthetic_name);
        assert(entry.valid(parsed.document->container_size));
    }
}

void the_real_stage_header_closes_on_the_real_stored_size() {
    // The header of `st001.pac` slot 1, dword for dword. Seventeen textures,
    // 1,701 sectors, and a stored length of 3,485,696 bytes — which is exactly
    // what the table predicts. That agreement is the whole identification.
    const std::vector<std::uint32_t> sectors{
        0x16U, 0x56U, 0x156U, 0x156U, 0x16U, 0x16U, 0x56U, 0x56U, 0x16U,
        0x56U, 0xABU, 0xABU, 0x56U, 0x16U, 0x06U, 0x2BU, 0x56U};
    const auto pack = make_pack(sectors);
    assert(pack.size() == 3485696U);

    const auto parsed = formats::PtxParser::parse(std::span<const std::byte>{pack});
    assert(parsed.ok());
    assert(parsed.document->declared_slot_count == 17U);
    assert(parsed.document->entries.back().offset +
        parsed.document->entries.back().size == 3485696U);
}

void a_table_that_does_not_close_is_refused() {
    // One sector too many in the table and the pack no longer describes the
    // bytes it is stored in. A format with no magic cannot afford to accept
    // that, so it is refused rather than repaired.
    auto pack = make_pack({1U, 2U});
    put_u32(pack, 8U, 3U);
    const auto parsed = formats::PtxParser::parse(std::span<const std::byte>{pack});
    assert(!parsed.ok());
    assert(parsed.error == formats::PtxParseError::size_table_does_not_close);
    assert(!formats::PtxParser::structurally_valid(std::span<const std::byte>{pack}));
}

void a_block_without_an_image_is_refused() {
    const auto pack = make_pack({1U, 2U}, false);
    const auto parsed = formats::PtxParser::parse(std::span<const std::byte>{pack});
    assert(!parsed.ok());
    assert(parsed.error == formats::PtxParseError::missing_image_signature);
}

void an_unrelated_container_is_not_a_texture_pack() {
    // A PAC opens with a magic where a texture pack opens with a count, so the
    // two recognizers can never both claim the same bytes.
    std::vector<std::byte> pac(0x2000U, std::byte{0});
    const std::string_view magic{"PAC\0", 4U};
    for (std::size_t index = 0U; index < magic.size(); ++index) {
        pac[index] = static_cast<std::byte>(magic[index]);
    }
    pac[4] = std::byte{1};
    pac[12] = std::byte{0x40};
    assert(!formats::PtxParser::structurally_valid(std::span<const std::byte>{pac}));
}

void classification_and_selection_agree() {
    const auto pack = make_pack({1U, 2U});

    // The slot that holds this record has no name of its own, which is exactly
    // the case the structural recognizer exists for.
    const auto classified = gdspaces::ResourceClassifier::classify(
        "slot_0001.bin", std::span<const std::byte>{pack}, false);
    assert(classified.format == "ptx");
    assert(classified.container);
    assert(classified.byte_derived);
    // There is no magic here, and the classification must not pretend there is.
    assert(!classified.magic_confirmed);

    const auto registry =
        dmc::rengine::profiles::dmc3::make_container_parser_registry();
    const auto* selected = registry.select(
        std::span<const std::byte>{pack}, "slot_0001.bin");
    assert(selected != nullptr);
    assert(selected->id() == "dmc3-ptx-structural-v1");
    const auto result = registry.parse(
        std::span<const std::byte>{pack}, "slot_0001.bin");
    assert(result.ok());
    assert(result.document.format == "ptx");
}

} // namespace

int main() {
    a_closing_pack_is_recognized();
    the_real_stage_header_closes_on_the_real_stored_size();
    a_table_that_does_not_close_is_refused();
    a_block_without_an_image_is_refused();
    an_unrelated_container_is_not_a_texture_pack();
    classification_and_selection_agree();
    return 0;
}
