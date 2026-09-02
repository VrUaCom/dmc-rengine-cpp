#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/text_record.hpp"
#include "dmc_rengine/formats/pac.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
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
            "slot_0003.bin", std::span<const std::byte>{bytes}, false);
        assert(classified.format == format);
        assert(classified.magic_confirmed);
        assert(!classified.container);
    }
}

[[nodiscard]] std::vector<std::byte> literal(std::string_view text, std::size_t size) {
    std::vector<std::byte> bytes(std::max(size, text.size()), std::byte{0});
    for (std::size_t index = 0U; index < text.size(); ++index) {
        bytes[index] = static_cast<std::byte>(text[index]);
    }
    return bytes;
}

void authoring_text_records_are_read_as_text() {
    // Every one of these is a real payload from the stage corpus, byte for
    // byte, including its NUL padding to the container alignment. Before the
    // classifier looked at them they were all "slot_NNNN.bin".
    const std::pair<std::string_view, std::size_t> corpus[]{
        // st001.pac slot 0 — the name manifest.
        {"st001.ptx\r\nst001.scm\r\nst001.sch\r\n", 48U},
        // st001cfg.pac slot 0 — a bare terminator block.
        {"\r\n# END", 16U},
        // st001_effect.pac slot 0 — the effect id list.
        {"V 922\r\nE 1454\r\nE 1456\r\nE 1460\r\nP 469\r\n"
         "P 471\r\nT 125\r\nT 48\r\nA 220\r\n# End\r\n", 80U},
        // st114cfg.pac slot 0 — the door table, tab separated.
        {"# DOOR 0\r\nBoxIn\t\t0\t1943.0 -10.0 3062.0 120.0 300.0 70.0 135\r\n"
         "NextRoom\t\t115\r\nType\t\t\tBtnOn\r\n", 560U},
    };
    for (const auto& [text, size] : corpus) {
        const auto bytes = literal(text, size);
        const auto classified = gdspaces::ResourceClassifier::classify(
            "slot_0000.bin", std::span<const std::byte>{bytes}, false);
        assert(classified.format == "txt");
        // No signature was matched. The claim is only that every byte of the
        // record is text, and the naming path must accept that as authority.
        assert(!classified.magic_confirmed);
        assert(classified.byte_derived);
        assert(!classified.container);
    }
}

void shift_jis_comments_do_not_make_a_record_binary() {
    // st001.pac slot 4 is the `# GAME` scene block, and its comment is
    // Shift-JIS: "\x83p\x81[\x83c\x94\xd4\x8d\x86" is パーツ番号. An
    // ASCII-only rule would call this record binary and hide it.
    const std::string block =
        std::string{"# GAME\r\n\r\n# SET 0 CONFIG\r\n\tcam_init\t2500\r\n"
                    "\tuv\t\t\t0, 14, -6, 0\t\t; "} +
        "\x83\x70\x81\x5b\x83\x63\x94\xd4\x8d\x86" +
        std::string{"\r\n# GAME_END\r\n$\r\n"};
    const auto bytes = literal(block, 256U);
    const auto classified = gdspaces::ResourceClassifier::classify(
        "slot_0004.bin", std::span<const std::byte>{bytes}, false);
    assert(classified.format == "txt");
    assert(classified.byte_derived);
}

void the_padding_is_reported_apart_from_the_text() {
    // A reader that shows this record to an operator must stop where the text
    // stops. `st001.pac` slot 0 is 33 bytes of manifest in a 48-byte slot, and
    // the 15 NULs after it are container alignment, not content.
    const auto manifest = literal(
        "st001.ptx\r\nst001.scm\r\nst001.sch\r\n", 48U);
    const auto view = gdspaces::TextRecord::inspect(
        std::span<const std::byte>{manifest});
    assert(view.recognized);
    assert(view.encoding == "shift-jis");
    assert(view.text_bytes == 33U);
    assert(view.padding_bytes == 15U);

    // The same inspection on a record that is not text still reports where the
    // padding is, and simply does not claim the bytes are readable.
    std::vector<std::byte> record(64U, std::byte{0});
    record[0] = std::byte{0x11};
    const auto binary = gdspaces::TextRecord::inspect(
        std::span<const std::byte>{record});
    assert(!binary.recognized);
    assert(binary.text_bytes == 1U);
}

void a_named_text_file_keeps_its_own_extension() {
    // The same rule read the other way: when the resource really is named,
    // the name wins. A `.index` manifest is text, and "index" says more about
    // it than "txt" does.
    const auto bytes = literal("PNST\r\nem035_057_000.txt\r\n", 0U);
    const auto classified = gdspaces::ResourceClassifier::classify(
        "em035_057.index", std::span<const std::byte>{bytes});
    assert(classified.format == "index");
    assert(!classified.byte_derived);
}

void a_binary_record_is_never_called_text() {
    // st001.pac slot 1 is the untagged record whose first dword is 0x00000011.
    // It has no tag and no signature, and it must stay untyped rather than be
    // swept up by the text rule.
    std::vector<std::byte> record(4096U, std::byte{0});
    record[0] = std::byte{0x11};
    record[4] = std::byte{0x16};
    record[8] = std::byte{0x56};
    const auto classified = gdspaces::ResourceClassifier::classify(
        "slot_0001.bin", std::span<const std::byte>{record}, false);
    // The parser's own placeholder suffix is not evidence, so an untagged
    // record stays untyped instead of being reported as a "bin" format.
    assert(classified.format == "unknown");
    assert(!classified.byte_derived);

    // A printable run with no line structure is a binary field, not a file.
    const auto banner = literal("VERSION 1", 32U);
    const auto banner_classified = gdspaces::ResourceClassifier::classify(
        "slot_0002.bin", std::span<const std::byte>{banner}, false);
    assert(banner_classified.format == "unknown");
    assert(!banner_classified.byte_derived);

    // A lone lead byte with no valid trail is not Shift-JIS text.
    const auto truncated = literal("ok\r\n\x83", 16U);
    const auto truncated_classified = gdspaces::ResourceClassifier::classify(
        "slot_0003.bin", std::span<const std::byte>{truncated}, false);
    assert(truncated_classified.format == "unknown");
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
    authoring_text_records_are_read_as_text();
    shift_jis_comments_do_not_make_a_record_binary();
    the_padding_is_reported_apart_from_the_text();
    a_named_text_file_keeps_its_own_extension();
    a_binary_record_is_never_called_text();
    a_zero_offset_slot_is_absent_not_broken();
    return 0;
}
