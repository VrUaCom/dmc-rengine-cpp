// FON bitmap fonts read from a synthetic font built here in their layout:
// pages by UTF-16 high byte, glyph numbers, 1-bit glyphs. The reader maps
// every character, refuses layouts that do not close, and draws a sheet.

#include "dmc_rengine/formats/fon.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/integration/resource_structure.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace fon = dmc::rengine::formats::fon;

namespace {

// Two pages (U+00xx and U+25xx), three glyphs of 24 x 20 (60 bytes each).
[[nodiscard]] std::vector<std::byte> font() {
    std::vector<std::byte> bytes(0x100U + 2U * 0x200U + 3U * 60U, std::byte{0});
    bytes[0x00] = std::byte{1};
    bytes[0x25] = std::byte{2};
    const auto map = [&](std::size_t page, std::uint8_t low, std::uint16_t glyph) {
        std::memcpy(bytes.data() + 0x100U + (page - 1U) * 0x200U + low * 2U, &glyph, 2U);
    };
    map(1U, 'A', 1U);
    map(1U, 'B', 2U);
    map(2U, 0xA0U, 3U); // U+25A0, a black square
    const auto glyph = [&](std::uint16_t number) { return bytes.data() + 0x500U + (number - 1U) * 60U; };
    // 'A': a vertical bar in column 2 (byte 0, bit 5) on rows 3..16.
    for (int row = 3; row <= 16; ++row) glyph(1)[row * 3] = std::byte{0x20};
    // U+25A0: rows 4..15 full in bytes 0..1 (columns 0..15).
    for (int row = 4; row <= 15; ++row) {
        glyph(3)[row * 3] = std::byte{0xFF};
        glyph(3)[row * 3 + 1] = std::byte{0xFF};
    }
    return bytes;
}

} // namespace

int main() {
    const auto bytes = font();
    const auto result = fon::read(bytes);
    assert(result.ok());
    const auto& doc = *result.document;
    assert(doc.pages == 2U && doc.glyph_count == 3U && doc.glyph_bytes == 60U);
    assert(doc.width == 24U && doc.height == 20U && doc.row_bytes == 3U && doc.glyph_offset == 0x500U);
    assert(doc.characters.size() == 3U);
    assert(doc.characters[0].code == 'A' && doc.characters[2].code == 0x25A0U && doc.characters[2].number == 3U);

    const auto pixels = fon::glyph_pixels(doc, bytes, 1U);
    assert(pixels[3U * 24U + 2U] == 1U && pixels[3U * 24U + 3U] == 0U && pixels[2U * 24U + 2U] == 0U);
    const auto ink = fon::ink_span(doc, bytes, 3U);
    assert(ink && ink->first == 0U && ink->last == 15U);
    assert(!fon::ink_span(doc, bytes, 2U)); // 'B' is blank

    // Layouts that do not close are refused.
    auto shared = bytes;
    shared[0x30] = std::byte{1}; // two blocks name page 1
    assert(!fon::read(shared).ok());
    auto ragged = bytes;
    ragged.push_back(std::byte{0});
    assert(!fon::read(ragged).ok());
    auto duplicate = bytes;
    const std::uint16_t one = 1U;
    std::memcpy(duplicate.data() + 0x100U + 'C' * 2U, &one, 2U); // 'C' reuses glyph 1
    assert(!fon::read(duplicate).ok());
    assert(!fon::read(std::vector<std::byte>(0x400U, std::byte{0})).ok());

    // The sheet: cells with a 1-pixel grid, bounded by the pixel budget.
    const auto sheet = fon::render_sheet(doc, bytes, 32U, 1U << 20U);
    assert(sheet.columns == 3U && sheet.shown == 3U);
    assert(sheet.width == 3U * 25U + 1U && sheet.height == 21U + 1U);
    assert(sheet.rgba8.size() == std::size_t{sheet.width} * sheet.height * 4U);
    const auto ink_at = ((1U + 3U) * sheet.width + 1U + 2U) * 4U; // 'A', row 3, column 2
    assert(sheet.rgba8[ink_at] == 0x10U);
    const auto tight = fon::render_sheet(doc, bytes, 1U, 25U * 30U);
    assert(tight.shown == 1U);

    // The classifier names it by its layout, and the structure view reads it.
    const auto classified = dmc::rengine::gdspaces::ResourceClassifier::classify("font/test.bin", bytes);
    assert(classified.format == "fon");
    std::string detail;
    const auto view = dmc::rengine::integration::read_structure("fon", bytes, "test.fon", detail);
    assert(view && view->format == "fon" && view->sections.size() >= 3U);
    std::cout << "fon_tests: pages, glyphs and sheet read from the layout\n";
    return 0;
}
