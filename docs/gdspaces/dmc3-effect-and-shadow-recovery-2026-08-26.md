# EFM and SHW — the last two handlers — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
**Routines:** `0x1402F7A90` (709 bytes) and `0x1403204C0` (48 bytes), receipts in
`data/reverse/dmc3-type-identification-windows.v1.json`

**Status:** RECOVERED FROM INSTRUCTIONS / NO CORPUS

The type probe named four handlers. `SCM` and `MOD` were read and verified
against real files; these two are read and **not** verified, because the
supplied data contains no `EFM`, `SHW` or `MRP` payload anywhere — a whole-file
search of the stage corpus, the HUD model and the extracted bundle finds none.

That is why both are recorded as recovered rather than corroborated, and why
neither reader has ever been run against a real file.

## 1. EFM is MOD with one more array

Instruction for instruction, `0x1402F7A90` is `0x1402FE3B0` with a single
addition: it also relocates the batch field at `+0x38`. The document shell, the
indexed `0x50` batches, the four arrays, the two-byte control word with its
`0x8000` break bit, the strip rebuild, the cleared flag — all identical.

The extra array's element width is **not** recoverable: the routine relocates
the base and never indexes through it. So the reader bounds the base and
refuses to claim an extent, and the group packing check deliberately leaves
that array out. Guessing a width to make the arithmetic close would be
manufacturing the fact rather than finding it.

One smaller difference: `MOD` tests two bytes of its secondary array when the
document mode byte is 1. This routine has no such branch.

## 2. SHW breaks the pattern

Three handlers sharing a document shell made it look universal. The fourth does
not use it.

| | SCM · MOD · EFM | SHW |
|---|---|---|
| count | byte at `+0x10` | byte at `+0x10` |
| document pointer | `+0x20`, relocated | none |
| table | groups at `+0x40`, stride `0x40` | entries at `+0x30`, stride `0x40` |
| below the table | batches, arrays, strip rebuild | four relocated bases, nothing else |

The whole routine is 48 bytes:

```text
for (i = 0; i < (uint8)p[0x10]; ++i) {
    e = p + 0x30 + i * 0x40;
    e[0x00] += p;  e[0x08] += p;  e[0x10] += p;  e[0x18] += p;
}
```

A reader written on the assumption that the shell is universal would take entry
0 of a shadow document for a group header — the table sits one field earlier,
so it would land inside the header and read plausible nonsense.

No element widths are recoverable here either: four bases are relocated and
never indexed. The reader records where each array starts and says nothing
about how long it is.

## 3. MRP has no routine

The probe assigns `MRP` a type code and calls nothing. There is no handler to
recover, so nothing is claimed about the payload beyond its tag — which is a
finding, not a gap.

## 4. Status of the four

| tag | routine | corpus | reader |
|---|---|---|---|
| `SCM` | `0x1403051B0` | 7 payloads, 2 stages | structural, verified |
| `MOD` | `0x1402FE3B0` | 1 payload | structural, verified |
| `EFM` | `0x1402F7A90` | none | structural, unverified |
| `SHW` | `0x1403204C0` | none | structural, unverified |
| `MRP` | — | none | none, and none possible |

## 5. What would close this

A single real `EFM` or `SHW` payload. Both readers are written to refuse rather
than guess, so the first real file either confirms the layout or fails loudly —
which is the outcome that would be worth having.
