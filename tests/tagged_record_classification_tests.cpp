#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/formats/pac.hpp"

#include <cassert>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Slot payloads inside a DMC3 relative-slot container carry a four-byte type
// tag and no name. These are the tags observed in the real stage corpus:
// st001.pac, st001cfg.pac, st001_effect.pac and their st114 counterparts.

namespace {

namespace formats = dmc::rengine::formats;
namespace gdspaces = dmc::rengine::gdspaces;

[[nodiscard]] std::vector<std::byte> tagged(std::string_view tag, std::size_t size) {
    std::vector<std::byte> bytes(size, std::byte{0});
    for (std::size_t index = 0U; index < tag.size() && index < size; ++index) {
        bytes[index] = static_cast<std::byte>(tag[index]);
    }
    return bytes;
}

void observed_tags_are_recognized() {
    const std::pair<std::string_view, std::string_view> corpus[]{
        {std::string_view{"LIG2", 4U}, "lig2"},
        {std::string_view{"SEF\0", 4U}, "sef"},
        {std::string_view{"CAM\0", 4U}, "cam"},
        {std::string_view{"EVE\0", 4U}, "eve"},
        {std::string_view{"POS\0", 4U}, "pos"},
        {std::string_view{"ITM\0", 4U}, "itm"},
        {std::string_view{"STE\0", 4U}, "ste"},
        {std::string_view{"EST\0", 4U}, "est"},
    };
    for (const auto& [tag, format] : corpus) {
        const auto bytes = tagged(tag, 64U);
        // The slot has no name to classify by, so the tag has to carry it.
        const auto classified = gdspaces::ResourceClassifier::classify(
            "slot_0003.bin", std::span<const std::byte>{bytes});
        assert(classified.format == format);
        assert(classified.magic_confirmed);
        assert(!classified.container);
    }
}

void an_untagged_payload_stays_untagged() {
    // A record with no tag is not forced into a type. The st001cfg corpus has
    // exactly this: a trailing text block with no four-byte magic.
    const std::vector<std::byte> text{
        std::byte{'\r'}, std::byte{'\n'}, std::byte{'#'}, std::byte{' '},
        std::byte{'E'}, std::byte{'N'}, std::byte{'D'}, std::byte{0}};
    const auto classified = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{text});
    assert(!classified.magic_confirmed);
    assert(classified.format == "bin");
}

void a_zero_offset_slot_is_absent_not_broken() {
    // Real stage containers are sparse: st001.pac slot 5 declares 11 slots with
    // 9 zero offsets, and st114.pac slot 5 declares 21 with 18. The slot index
    // is an identity, not a position in a packed list, so a zero offset means
    // this index carries nothing — it is not a parse failure and must not be
    // repaired into one.
    std::vector<std::byte> container(0x80U, std::byte{0});
    const std::string_view magic{"PAC\0", 4U};
    for (std::size_t index = 0U; index < magic.size(); ++index) {
        container[index] = static_cast<std::byte>(magic[index]);
    }
    container[4] = std::byte{3};  // three slots
    // slot 0 empty, slot 1 empty, slot 2 at 0x40.
    container[8U + 8U] = std::byte{0x40};
    container[0x40] = static_cast<std::byte>('C');
    container[0x41] = static_cast<std::byte>('A');
    container[0x42] = static_cast<std::byte>('M');

    const auto parsed = formats::PacParser::parse(container);
    assert(parsed.ok());
    assert(parsed.document->declared_slot_count == 3U);
    assert(!parsed.document->entries[0].populated);
    assert(!parsed.document->entries[1].populated);
    assert(parsed.document->entries[2].populated);
    // The empty slots keep their index, so the third payload stays slot 2.
    assert(parsed.document->entries[2].slot_index == 2U);
}

} // namespace

int main() {
    observed_tags_are_recognized();
    an_untagged_payload_stays_untagged();
    a_zero_offset_slot_is_absent_not_broken();
    return 0;
}
