# FON: the game's bitmap fonts

2026-10-10. Four retail fonts analysed (not committed): `font\euro28.fon`
(23 856 bytes), `china20m.fon` (1 316 380), `china20z.fon` (461 496) and
`wksong_glim.fon` (169 996). Reader: `formats/fon.hpp`; structure view in
`integration/resource_structure.cpp`; registry maturity `structural`.

## Layout

No tag, no header. Every byte of all four files is accounted for by:

| offset | size | content |
|---|---|---|
| 0x000 | 256 | one byte per UTF-16 high byte: the number of the page that maps that block, 0 when none |
| 0x100 | P × 0x200 | page p at 0x100 + (p − 1) × 0x200: 256 u16, the glyph number for each low byte, 0 when none |
| 0x100 + P × 0x200 | G × N | glyphs 1..G in order, each N bytes: rows of 1-bit pixels, MSB first |

Pages are numbered 1..P, each used once; glyph numbers 1..G, each used once,
so the glyph count is the number of mapped characters and the data size
divides by it exactly.

| file | pages | glyphs | bytes/glyph | cell | blocks |
|---|---|---|---|---|---|
| euro28 | 3 | 197 | 112 | 32 × 28 | U+00xx, U+01xx (Œ œ), U+25xx |
| china20m | 87 | 21 193 | 60 | 24 × 20 | Latin, U+03xx, U+20xx, U+30xx, CJK U+4Exx.. |
| china20z | 85 | 6 962 | 60 | 24 × 20 | Latin, U+20xx, U+30xx, CJK |
| wksong_glim | 45 | 2 445 | 60 | 24 × 20 | Latin, Hangul U+ACxx.. |

## Cell size

Not stored. The reader takes the narrowest whole-byte row at least as wide as
the glyph is tall: 112 = 28 rows × 4 bytes (32 px), 60 = 20 rows × 3 bytes
(24 px). Both match the number in each file's name, and the glyphs drawn this
way read correctly (Latin with accents, 中文魔鬼泣, 한국어).

## Open

- Advance widths: not in the file. The reader measures each glyph's ink span;
  how the game spaces characters (fixed cell or measured ink) is the loader's
  question.
- Baseline and the loader that draws them.
