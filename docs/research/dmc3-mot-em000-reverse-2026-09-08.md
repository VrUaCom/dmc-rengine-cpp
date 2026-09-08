# DMC3 HD MOT — full em000 motion-corpus reverse checkpoint (2026-09-08)

**Branch:** `reverse/mot-em000-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Primary identity result

All 82 populated children under `em000_035.pac`, `em000_036.pac` and `em000_037.pac` are MOT-family payloads. Ten were extracted with `.mot`; seventy-two were extracted with `.bin`; all 82 have `MOT\0` at `+0x04`.

The canonical executable independently registers `.mot` in the motion/control extension dispatcher at `0x1402E01A0`, so MOT family identity is **EXE_AND_CORPUS_CONFIRMED**.

## Header ABI — now structurally closed for em000

Observed common header:

```text
+0x00 u32 header_size
+0x04 char[4] "MOT\0"
+0x08 u32 zero                    # 82/82
+0x0C f32 raw_time_a
+0x10 f32 raw_time_b
+0x14 f32 raw_time_c
+0x18 u16 raw_header_flag
+0x1A u16 raw_header_selector
+0x1C u16 channel_domain_count
+0x1E u16 channel_mask[channel_domain_count]
... zero/preserved header tail until header_size
```

Observed domain counts:

```text
22 -> 77 files
 3 ->  4 files
 4 ->  1 file
```

The physical header-size formula is exact for **82/82** files:

```text
header_size = align16(0x1E + 2 * channel_domain_count)
```

Therefore the observed `0x50` and `0x30` headers are not unrelated revisions: they are the aligned envelope produced by different channel-domain counts.

Observed header-size census:

```text
0x50 -> 77 files
0x30 ->  5 files
```

`raw_time_a == raw_time_c` in 82/82 files. `raw_time_b` is zero in 76/82 and non-zero in six. High-level timing names remain unpromoted until the canonical executable consumer is rebound.

`raw_header_flag` is 0 or 1 in em000. `raw_header_selector` is either 0 or equal to the channel-domain count in this corpus. Their meanings remain raw.

## Channel masks and exact record-count invariant

At `header_size`:

```text
u32 record_count
```

For **82/82** payloads:

```text
record_count == sum(popcount(channel_mask[i]))
```

Observed masks:

```text
0x000
0x007
0x038
0x03F
0x1C0
0x1F8
```

The masks are nine-bit channel selectors grouped into three 3-bit triplets:

```text
bits 0..2 -> low triplet
bits 3..5 -> middle triplet
bits 6..8 -> high triplet
```

Independent public DMC3 animation-importer source corroborates the longstanding community mapping:

```text
bit 8 translation X
bit 7 translation Y
bit 6 translation Z
bit 5 rotation X
bit 4 rotation Y
bit 3 rotation Z
bit 2 scale X
bit 1 scale Y
bit 0 scale Z
```

This mapping is also strongly supported by the em000 numeric distributions: middle-triplet tracks are radian-scale, high-triplet tracks include large positional ranges, and low-triplet tracks cluster around scale-like values near 1.0. However, because this checkpoint does not yet contain the canonical dmc3.exe track-evaluation consumer, the high-level channel names are recorded as **SEMANTIC_CANDIDATE with independent external corroboration**, not `EXE_CONFIRMED`.

## Track ordering

Tracks are serialized in channel-domain order and then in descending semantic mask order used by the known mapping above: translation X/Y/Z, rotation X/Y/Z, scale X/Y/Z for every set bit.

The sum-popcount invariant reconstructs the complete record stream with no orphan records in 82/82 files.

## Track record ABI

Every record starts with the same 8-byte prefix:

```text
+0x00 u16 span
+0x02 u16 key_count
+0x04 u16 compression_type
+0x06 u16 start_time_raw
```

`start_time_raw == 0` in all **5,118** em000 tracks.

Observed compression types:

```text
2 -> 156 tracks
3 -> 4,962 tracks
```

No type 0/1 track appears in this corpus.

Independent public DMC3 importer source labels these values:

```text
0 LINEAR_FLOAT32
1 HERMITE_FLOAT32
2 LINEAR_INT16
3 HERMITE_INT16
```

The current corpus independently proves the structural layouts for 2 and 3.

### Compression 2 — linear int16 envelope

Exact for **156/156** type-2 tracks:

```text
span = 0x10 + 4 * key_count
```

Layout:

```text
+0x08 f32 value_min_or_bias
+0x0C f32 value_range_or_scale
+0x10 key[key_count]
```

Each key is 4 bytes:

```text
u16 time_control
u16 quantized_value
```

All type-2 key times are monotonic and bit15 of `time_control` is clear in all 459 observed keys.

### Compression 3 — Hermite int16 envelope

Exact for **4,962/4,962** type-3 tracks:

```text
span = 0x20 + 8 * key_count
```

Layout:

```text
+0x08 f32 value_min_or_bias
+0x0C f32 value_range_or_scale
+0x10 f32 in_tangent_min_or_bias
+0x14 f32 in_tangent_range_or_scale
+0x18 f32 out_tangent_min_or_bias
+0x1C f32 out_tangent_range_or_scale
+0x20 key[key_count]
```

Each key is 8 bytes:

```text
u16 time_control
u16 quantized_value
u16 quantized_in_tangent
u16 quantized_out_tangent
```

Masking time as:

```text
time_index = time_control & 0x7FFF
flag       = time_control >> 15
```

produces monotonic key times for **4,962/4,962** tracks.

Across 94,416 type-3 keys:

```text
flag = 1 -> 94,410
flag = 0 ->      6
```

The meaning of that high bit remains unresolved.

## Quantization

Independent public importer code uses the transform:

```text
value = quantized * range * (1 / 65535) + min
```

and the same form for incoming/outgoing Hermite tangents.

The em000 corpus strongly corroborates unsigned 16-bit normalized quantization: frequently observed quantized values include `0x0000`, `0x3FFF`, `0x7FFF`, `0xBFFF`, `0xFFFE`, and `0xFFFF`.

Until the canonical executable evaluator is directly rebound, the decode formula is treated as **externally corroborated + corpus-consistent**, not yet executable-promoted.

## Complete corpus record census

```text
files                  82
tracks               5,118
compression 2          156
compression 3        4,962
```

Record-count values:

```text
66 -> 64 files
69 ->  9 files
63 ->  4 files
 3 ->  4 files
 9 ->  1 file
```

These values are fully explained by channel-mask popcounts rather than by a hardcoded `three records per bone` rule. That distinction matters for masks such as `0x1F8`, `0x03F`, zero masks, and the small-domain MOT variants.

## External corroboration boundary

A public Blender importer for DMC3 HD independently contains the same:

- MOT header fields;
- channel-mask table;
- nine channel bits;
- compression enum 0..3;
- int16 linear/Hermite layouts;
- `time & 0x7FFF` plus high-bit flag;
- `/65535` quantization;
- Hermite interpolation.

This source is valuable corroboration and gave names to structures already recovered from em000 bytes. It is **not substituted for canonical EXE authority**. Final semantic promotion still requires direct `dmc3.exe` consumer evidence.

## ADR-0003 implementation target

MOT must follow modular architecture:

```text
include/dmc_rengine/formats/mot/
    abi.hpp
    ir.hpp
    parser.hpp
src/formats/mot/
    parser.cpp
include/dmc_rengine/analysis/mot/
    channel_semantics.hpp      # only after executable promotion
    evaluation.hpp             # only after evaluator recovery
    binding.hpp                # MOD/EFM/CMotion binding
```

A `formats/mot.hpp` facade may aggregate headers but must not own parser/runtime logic.

## Relationship to MOD/EFM

```text
MOT serialized tracks
    -> channel mask / compressed curves
    -> CMotion evaluation
    -> per-node animated local transforms
    -> MOD/EFM currentWorld
    -> inverseRest * currentWorld
    -> skin palette
```

The channel table now gives a concrete route to bind animation records to model-node indices without guessing from record ordinal alone.

## Next executable gates

1. trace extension-dispatch index `0` to the MOT loader and parser;
2. find the consumer of the 9-bit channel mask table;
3. prove channel-bit semantics in canonical machine code;
4. find the compression switch for values `0/1/2/3`;
5. prove the `/65535` decode and Hermite evaluator;
6. resolve `raw_time_a/raw_time_b/raw_time_c`;
7. resolve `raw_header_flag`, `raw_header_selector`, `start_time_raw`, and key high-bit semantics;
8. bind decoded tracks to `CMotionJoint`, `motion_group`, MOD node indices and currentWorld;
9. compare with player/weapon/boss MOT corpora before writer work.

## Evidence status

| Claim | Status |
|---|---|
| MOT family identity | `EXE_AND_CORPUS_CONFIRMED` |
| header-size formula | `CORPUS_CONFIRMED 82/82` |
| channel-domain count + mask table | `CORPUS_CONFIRMED 82/82` |
| `record_count == sum(popcount(mask))` | `CORPUS_CONFIRMED 82/82` |
| channel bit names translation/rotation/scale | `SEMANTIC_CANDIDATE`, externally corroborated |
| record prefix span/key_count/compression/start_time | `CORPUS_CONFIRMED` |
| compression 2 size/layout | `CORPUS_CONFIRMED 156/156` |
| compression 3 size/layout | `CORPUS_CONFIRMED 4962/4962` |
| compression 2/3 names linear/Hermite int16 | `SEMANTIC_CANDIDATE`, externally corroborated |
| key time lower-15-bit rule | `CORPUS_CONFIRMED` |
| key high-bit meaning | `PRESERVED_UNDECODED` |
| `/65535` quantization decode | `SEMANTIC_CANDIDATE`, externally corroborated/corpus-consistent |
| exact timing-header semantics | `PRESERVED_UNDECODED` |
| writer authority | `NOT AUTHORIZED` |
