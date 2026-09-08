# DMC3 HD MOT — reconciliation of the historical `st001` sample (2026-09-08)

**Branch:** `reverse/mot-em000-20260908`  
**Purpose:** re-evaluate old PR #254 MOT conclusions against the 82-payload em000 grammar.  
**Canonical executable identity used by the historical pass:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Why this pass matters

PR #254 contained one real 63,440-byte MOT payload from `st001.pac`. Its parser was intentionally structural, but because only that one payload was available it generalized two observations too far:

1. every track was assumed to use `span = 0x20 + 8 * key_count`; and
2. the first key word was treated as signed `s16` time, producing `-32768 .. -32118` for a 650-frame motion.

The new 82-file em000 corpus supplies the missing variants. Re-reading the exact historical observations through the new grammar explains the old sample without special cases and identifies which earlier conclusions were variant-specific.

## Historical header bytes now decoded structurally

The old research note recorded the 28 u16 values beginning at `+0x18` as:

```text
0, 24, 24, 0, 448, 56, 56, 56, 56, 56, 56, 56, 56, 56,
56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 0
```

Under the em000-derived ABI this is exactly:

```text
+0x18 raw_u16_18           = 0
+0x1A raw_u16_1A           = 24
+0x1C channel_domain_count = 24
+0x1E channel_mask[24]     =
    node 0  : 0x000
    node 1  : 0x1C0
    node 2..23: 0x038
header alignment tail      = 0x0000
```

The header-size formula closes:

```text
align16(0x1E + 24 * 2)
= align16(0x4E)
= 0x50
```

The old payload's recorded `track_count = 69` is now independently explained by the masks:

```text
popcount(0x000)       = 0
popcount(0x1C0)       = 3
22 * popcount(0x038)  = 66
                         --
                         69
```

This is a major cross-corpus validation of the channel-domain grammar: the old sample was not used to derive the new mask model, yet it satisfies the model exactly.

## Historical track layout was one compression variant

PR #254 observed:

```text
69 / 69 tracks:
span = 0x20 + 8 * key_count
u32 at +0x04 = 3
```

The em000 corpus now proves that the old `u32 kind == 3` interpretation combined two adjacent u16 fields:

```text
+0x04 u16 compression = 3
+0x06 u16 start_time_raw = 0
```

and that compression value `3` has exactly the historical physical layout:

```text
0x20-byte prefix
+ key_count * 8-byte keys
```

In em000 this holds for **4,962 / 4,962** compression-3 tracks.

The old parser's size identity was therefore correct **for compression 3**, but incorrect as a universal MOT-track identity because em000 also contains **156 compression-2 tracks** with:

```text
span = 0x10 + 4 * key_count
```

Safe correction:

```text
old universal rule                       -> REJECTED
compression-3-specific physical rule     -> CORPUS_CONFIRMED
compression-2-specific physical rule     -> CORPUS_CONFIRMED
```

## Signed time-stamp interpretation corrected

The historical sample recorded first/last raw key words as signed values:

```text
-32768 .. -32118
```

with a difference of 650.

Those exact bit patterns are:

```text
0x8000 .. 0x828A
```

The em000 corpus and independent DMC3 importer corroboration instead split the word as:

```text
time_index = raw & 0x7FFF
high_flag  = raw >> 15
```

Applying that split to the historical values gives:

```text
0 .. 650
```

which directly matches the historical header value `650.0f` at both `+0x0C` and `+0x14`.

Therefore:

```text
signed s16 timeline interpretation -> REJECTED
lower-15-bit time index             -> CORPUS_CONFIRMED / cross-corpus corroborated
bit15 semantic meaning              -> PRESERVED_UNDECODED
```

This also explains why the historical parser found a constant apparent negative offset: bit15 was being sign-extended instead of separated from the time index.

## Historical four-byte terminator reclassified

PR #254 required exactly one zero dword after the last track. The em000 corpus shows post-track zero padding of 0, 4, 8 or 12 bytes depending on final alignment.

The old sample's four zero bytes are therefore better classified as an observed zero alignment tail, not a universal semantic terminator.

Safe correction:

```text
exact 4-byte semantic terminator -> REJECTED as universal
zero-valued post-track alignment -> CORPUS_CONFIRMED
```

## Cross-corpus status after reconciliation

The MOT grammar is now supported by:

```text
82 em000 payloads
+1 historical st001 payload
----------------------------
83 structurally consistent payload observations
```

The historical raw file is not available in the current workspace for a fresh SHA-bound byte replay, so the `st001` contribution is **historical evidence reconstructed from the PR #254 byte/statistics record**, not a substitute for fresh raw-corpus possession.

Still, three independent old observations are exactly predicted by the new grammar:

1. 0x50 header size from 24 channel-domain entries;
2. 69 tracks from channel-mask popcounts;
3. 650-frame timeline after masking bit15.

## Architectural consequence

Do not salvage the old PR #254 MOT parser wholesale. Salvage only its evidence and independently revalidated facts.

Canonical candidate implementation remains the ADR-0003 modular branch:

```text
formats/mot/abi.hpp
formats/mot/ir.hpp
formats/mot/parser.hpp
src/formats/mot/parser.cpp
```

The old fixed-size `MotContract`, signed-stamp model and exact four-byte terminator must not be reintroduced.
