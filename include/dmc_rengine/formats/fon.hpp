#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

/**
 * FON: the game's bitmap fonts (`font\euro28.fon`, `china20m.fon`,
 * `china20z.fon`, `wksong_glim.fon`). No tag, no header; the layout is read
 * from the four retail files, where every byte is accounted for:
 *
 * - +0x000, 256 bytes: one byte per high byte of a UTF-16 code unit — the
 *   number of the page that maps that block, or 0 when no character in it
 *   has a glyph. The pages are numbered 1..P, each used once.
 * - +0x100, P pages of 256 u16: for each low byte, the glyph's number
 *   (1-based), or 0. Page p is at 0x100 + (p - 1) * 0x200.
 * - +0x100 + P * 0x200: the glyphs, numbers 1..G in order, each the same
 *   size: rows of 1-bit pixels, most significant bit first, a whole number
 *   of bytes per row.
 *
 * The cell's size is not stored. It is the one the glyph's byte count
 * allows with the narrowest whole-byte row at least as wide as it is tall:
 * 112 bytes are 28 rows of 4 (32 x 28, euro28), 60 bytes 20 rows of 3
 * (24 x 20, the three 20-pixel fonts) — the size each file's name gives.
 * euro28 maps 197 Latin-1, Latin Extended and geometric characters;
 * china20m 21 193 and china20z 6 962 CJK; wksong_glim 2 445 Hangul.
 */
namespace dmc::rengine::formats::fon {

inline constexpr std::size_t k_lead_table = 0x100U;
inline constexpr std::size_t k_page_size = 0x200U;

struct Glyph final {
    /// The UTF-16 code unit this glyph draws.
    std::uint16_t code{};
    /// 1-based glyph number.
    std::uint16_t number{};
};

struct Document final {
    std::uint32_t pages{};
    std::uint32_t glyph_count{};
    std::uint32_t glyph_bytes{};
    std::uint32_t row_bytes{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint64_t glyph_offset{};
    /// Every mapped character, in code order.
    std::vector<Glyph> characters;
    /// High bytes with a page, in order.
    std::vector<std::uint8_t> blocks;
};

struct ReadResult final {
    bool recognized{false};
    std::optional<Document> document;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept { return document.has_value(); }
};

/// The layout above, or a refusal saying which part of it the bytes break.
[[nodiscard]] ReadResult read(std::span<const std::byte> bytes);

/// Cheap structural identity: the layout reads with every byte accounted for.
[[nodiscard]] bool looks_like_fon(std::span<const std::byte> bytes) noexcept;

/// Glyph `number`'s pixels, one byte per pixel (0 or 1), `width` x `height`.
[[nodiscard]] std::vector<std::uint8_t> glyph_pixels(
    const Document& document, std::span<const std::byte> bytes, std::uint32_t number);

/// The columns a glyph's ink spans: [first, last], or nothing for a blank glyph.
struct InkSpan final {
    std::uint32_t first{};
    std::uint32_t last{};
};
[[nodiscard]] std::optional<InkSpan> ink_span(
    const Document& document, std::span<const std::byte> bytes, std::uint32_t number);

/// A sheet of glyphs in code order, ink dark on light, RGBA8.
struct Sheet final {
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t columns{};
    /// Characters drawn; fewer than mapped when the pixel budget ran out.
    std::uint32_t shown{};
    std::vector<std::uint8_t> rgba8;
};

/// The tallest sheet drawn: a shell scales anything taller down, and the glyphs
/// are only legible at their own size.
inline constexpr std::uint32_t k_sheet_max_side = 4096U;

/// A sheet of at most `max_pixels` and `k_sheet_max_side` tall, `columns`
/// cells wide (1 px gaps).
[[nodiscard]] Sheet render_sheet(const Document& document, std::span<const std::byte> bytes,
                                 std::uint32_t columns, std::uint64_t max_pixels);

/// The sheet's size for these bounds, without drawing it.
[[nodiscard]] Sheet sheet_size(const Document& document, std::uint32_t columns, std::uint64_t max_pixels);

} // namespace dmc::rengine::formats::fon
