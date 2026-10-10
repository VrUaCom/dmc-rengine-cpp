#include "dmc_rengine/formats/fon.hpp"

#include <algorithm>
#include <cstring>
#include <string>

namespace dmc::rengine::formats::fon {
namespace {

[[nodiscard]] std::uint16_t u16(std::span<const std::byte> bytes, std::size_t at) noexcept {
    std::uint16_t value = 0U;
    std::memcpy(&value, bytes.data() + at, sizeof value);
    return value;
}

void refuse(ReadResult& result, std::string code, std::string message, std::uint64_t offset) {
    result.diagnostics.push_back(
        {.severity = ParseSeverity::error, .code = std::move(code), .message = std::move(message), .offset = offset});
    result.document.reset();
}

} // namespace

ReadResult read(std::span<const std::byte> bytes) {
    ReadResult result;
    if (bytes.size() < k_lead_table + k_page_size) {
        refuse(result, "fon.size", "Shorter than the lead table and one page.", 0U);
        return result;
    }
    // The lead table numbers its pages 1..P, each once.
    std::vector<bool> seen(257U, false);
    std::uint32_t pages = 0U;
    Document document;
    for (std::size_t high = 0U; high < k_lead_table; ++high) {
        const auto page = std::to_integer<std::uint32_t>(bytes[high]);
        if (page == 0U) continue;
        if (seen[page]) {
            refuse(result, "fon.lead-table", "Two blocks name the same page.", high);
            return result;
        }
        seen[page] = true;
        pages = std::max(pages, page);
        document.blocks.push_back(static_cast<std::uint8_t>(high));
    }
    if (pages == 0U || document.blocks.size() != pages) {
        refuse(result, "fon.lead-table", "The lead table does not number its pages 1..P.", 0U);
        return result;
    }
    const auto glyph_offset = k_lead_table + std::size_t{pages} * k_page_size;
    if (bytes.size() <= glyph_offset) {
        refuse(result, "fon.pages", "The pages run past the end of the file.", k_lead_table);
        return result;
    }
    result.recognized = true;
    // Every glyph number 1..G appears exactly once.
    std::vector<std::uint16_t> owner;
    for (const auto high : document.blocks) {
        const auto page = std::to_integer<std::uint32_t>(bytes[high]);
        const auto base = k_lead_table + std::size_t{page - 1U} * k_page_size;
        for (std::uint32_t low = 0U; low < 256U; ++low) {
            const auto number = u16(bytes, base + low * 2U);
            if (number == 0U) continue;
            document.characters.push_back(
                {.code = static_cast<std::uint16_t>((std::uint32_t{high} << 8U) | low), .number = number});
        }
    }
    std::uint32_t glyphs = 0U;
    for (const auto& character : document.characters) glyphs = std::max<std::uint32_t>(glyphs, character.number);
    std::vector<bool> used(glyphs + 1U, false);
    for (const auto& character : document.characters) {
        if (used[character.number]) {
            refuse(result, "fon.glyph-numbers", "Two characters share glyph " + std::to_string(character.number) + ".",
                   k_lead_table);
            return result;
        }
        used[character.number] = true;
    }
    if (glyphs == 0U || document.characters.size() != glyphs) {
        refuse(result, "fon.glyph-numbers", "The pages do not number their glyphs 1..G.", k_lead_table);
        return result;
    }
    const auto data = bytes.size() - glyph_offset;
    if (data % glyphs != 0U) {
        refuse(result, "fon.glyph-size",
               std::to_string(data) + " bytes of glyphs do not divide into " + std::to_string(glyphs) + " glyphs.",
               glyph_offset);
        return result;
    }
    const auto glyph_bytes = static_cast<std::uint32_t>(data / glyphs);
    // The narrowest whole-byte row at least as wide as the cell is tall.
    std::uint32_t row_bytes = 0U;
    for (std::uint32_t candidate = 1U; candidate <= 16U; ++candidate) {
        if (glyph_bytes % candidate != 0U) continue;
        if (candidate * 8U >= glyph_bytes / candidate) {
            row_bytes = candidate;
            break;
        }
    }
    if (row_bytes == 0U) {
        refuse(result, "fon.glyph-size", "A glyph of " + std::to_string(glyph_bytes) + " bytes has no cell shape.",
               glyph_offset);
        return result;
    }
    document.pages = pages;
    document.glyph_count = glyphs;
    document.glyph_bytes = glyph_bytes;
    document.row_bytes = row_bytes;
    document.width = row_bytes * 8U;
    document.height = glyph_bytes / row_bytes;
    document.glyph_offset = glyph_offset;
    result.document = std::move(document);
    return result;
}

bool looks_like_fon(std::span<const std::byte> bytes) noexcept {
    try {
        return read(bytes).ok();
    } catch (...) {
        return false;
    }
}

std::vector<std::uint8_t> glyph_pixels(const Document& document, std::span<const std::byte> bytes,
                                       std::uint32_t number) {
    std::vector<std::uint8_t> pixels(std::size_t{document.width} * document.height, 0U);
    if (number == 0U || number > document.glyph_count) return pixels;
    const auto base = document.glyph_offset + std::uint64_t{number - 1U} * document.glyph_bytes;
    if (base + document.glyph_bytes > bytes.size()) return pixels;
    for (std::uint32_t y = 0U; y < document.height; ++y) {
        for (std::uint32_t x = 0U; x < document.width; ++x) {
            const auto byte = std::to_integer<std::uint32_t>(bytes[base + y * document.row_bytes + x / 8U]);
            pixels[std::size_t{y} * document.width + x] = static_cast<std::uint8_t>((byte >> (7U - x % 8U)) & 1U);
        }
    }
    return pixels;
}

std::optional<InkSpan> ink_span(const Document& document, std::span<const std::byte> bytes, std::uint32_t number) {
    const auto pixels = glyph_pixels(document, bytes, number);
    std::optional<InkSpan> span;
    for (std::uint32_t x = 0U; x < document.width; ++x) {
        for (std::uint32_t y = 0U; y < document.height; ++y) {
            if (pixels[std::size_t{y} * document.width + x] == 0U) continue;
            if (!span) span = InkSpan{.first = x, .last = x};
            span->last = x;
            break;
        }
    }
    return span;
}

Sheet sheet_size(const Document& document, std::uint32_t columns, std::uint64_t max_pixels) {
    Sheet sheet;
    columns = std::max<std::uint32_t>(1U, std::min<std::uint32_t>(columns, document.glyph_count));
    const auto cell_w = document.width + 1U;
    const auto cell_h = document.height + 1U;
    sheet.columns = columns;
    sheet.width = columns * cell_w + 1U;
    const auto total_rows = (document.glyph_count + columns - 1U) / columns;
    std::uint32_t rows = total_rows;
    while (rows > 0U && (std::uint64_t{sheet.width} * (rows * cell_h + 1U) > max_pixels ||
                         rows * cell_h + 1U > k_sheet_max_side)) {
        --rows;
    }
    sheet.height = rows == 0U ? 0U : rows * cell_h + 1U;
    sheet.shown = std::min<std::uint32_t>(document.glyph_count, rows * columns);
    return sheet;
}

Sheet render_sheet(const Document& document, std::span<const std::byte> bytes, std::uint32_t columns,
                   std::uint64_t max_pixels) {
    auto sheet = sheet_size(document, columns, max_pixels);
    if (sheet.height == 0U) return sheet;
    // Light paper with a faint grid, dark ink: legible in either theme.
    sheet.rgba8.assign(std::size_t{sheet.width} * sheet.height * 4U, 0U);
    for (std::size_t i = 0U; i < sheet.rgba8.size(); i += 4U) {
        sheet.rgba8[i] = 0xD8U;
        sheet.rgba8[i + 1U] = 0xDCU;
        sheet.rgba8[i + 2U] = 0xE2U;
        sheet.rgba8[i + 3U] = 0xFFU;
    }
    const auto cell_w = document.width + 1U;
    const auto cell_h = document.height + 1U;
    for (std::uint32_t index = 0U; index < sheet.shown; ++index) {
        const auto& character = document.characters[index];
        const auto pixels = glyph_pixels(document, bytes, character.number);
        const auto left = 1U + (index % sheet.columns) * cell_w;
        const auto top = 1U + (index / sheet.columns) * cell_h;
        for (std::uint32_t y = 0U; y < document.height; ++y) {
            for (std::uint32_t x = 0U; x < document.width; ++x) {
                const auto at = (std::size_t{top + y} * sheet.width + left + x) * 4U;
                const bool ink = pixels[std::size_t{y} * document.width + x] != 0U;
                const std::uint8_t value = ink ? 0x10U : 0xFFU;
                sheet.rgba8[at] = value;
                sheet.rgba8[at + 1U] = value;
                sheet.rgba8[at + 2U] = ink ? 0x18U : 0xFFU;
            }
        }
    }
    return sheet;
}

} // namespace dmc::rengine::formats::fon
