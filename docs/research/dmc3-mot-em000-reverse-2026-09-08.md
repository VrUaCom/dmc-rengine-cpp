# DMC3 HD MOT — full em000 motion-corpus reverse checkpoint (2026-09-08)

**Branch:** `reverse/mot-em000-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Primary conclusion

Every populated child inspected under the three top-level em000 motion PACs is a member of the **MOT binary resource family**, including children that the extraction layer named `.bin`.

Corpus:

```text
em000_035.pac -> 71 populated children
em000_036.pac ->  1 populated child
em000_037.pac -> 10 populated children
                         --
                         82 total
```

For all `82 / 82` payloads:

```text
bytes +0x04..+0x07 == "MOT\0"
```

Only ten children were emitted with a `.mot` suffix; seventy-two were emitted as `.bin`. Therefore **the extractor suffix is not semantic format authority** for this family.

Correct identity rule for this corpus:

```text
MOT\0 structural payload + motion-container context -> MOT candidate/confirmed format
.bin suffix alone                                  -> no format claim
```

The canonical executable independently registers `.mot` in the motion/control dispatcher at `0x1402E01A0`, so MOT identity is **EXE_AND_CORPUS_CONFIRMED**.

## Two observed header envelope sizes

The first u32 is a header-size field in every one of the 82 samples:

```text
0x50 -> 77 files
0x30 ->  5 files
```

Observed common prefix:

```text
+0x00 u32 header_size       # 0x30 or 0x50 in this corpus
+0x04 char[4] "MOT\0"
+0x08 u32 zero              # 82/82 in current corpus
+0x0C f32 raw_time_a
+0x10 f32 raw_time_b
+0x14 f32 raw_time_c
... raw header bytes until header_size
```

Across `82 / 82` samples:

```text
raw_time_a == raw_time_c
```

These values are finite and integer-like in the bound corpus, with examples such as `10`, `19`, `41`, `48`, `53`, `60`, `99`, `109`, `114`, `120`, `124`, `127`, `141` and higher values.

`raw_time_b` is zero in most samples but is non-zero in six files. Examples include:

```text
raw_time_b = 36   while raw_time_a/raw_time_c = 53
raw_time_b = 24   while raw_time_a/raw_time_c = 26
raw_time_b = 125  while raw_time_a/raw_time_c = 237
raw_time_b = 78   while raw_time_a/raw_time_c = 141
raw_time_b = 297  while raw_time_a/raw_time_c = 298
raw_time_b = 349  while raw_time_a/raw_time_c = 350
```

A loop/segment-start interpretation is plausible but **not promoted**. The canonical field names remain raw until the CMotion consumer is bound.

## Record stream grammar

At file offset `header_size`:

```text
u32 record_count
```

Immediately after it, records are serialized sequentially. Each record begins with:

```text
u16 record_span
```

Advancing exactly by `record_span` for `record_count` records reaches the physical tail correctly in **82 / 82** samples.

Observed properties:

- record sizes are variable;
- many spans are multiples of 8;
- observed spans include `0x28`, `0x30`, `0x38`, `0x40`, `0x48` and larger values;
- after the final record the only remaining bytes are zero padding;
- observed tail padding is `0`, `4`, `8`, or `12` bytes;
- every inspected resource size is 16-byte aligned.

This establishes a useful read-only envelope without claiming record opcodes or channel semantics.

## Representative samples

### 0x50-header variant

A representative child from `em000_035.pac`:

```text
physical size       2608
+0x00 header_size   0x50
+0x04 magic         MOT\0
+0x0C raw_time_a    10.0
+0x14 raw_time_c    10.0
+0x50 record_count  63
+0x54 first span    0x28
```

All 63 variable-length records fit the file envelope and terminate before zero alignment padding.

### 0x30-header variant

The sole child under `em000_036.pac` is an extractor-labeled `.bin`, but structurally:

```text
physical size       688
+0x00 header_size   0x30
+0x04 magic         MOT\0
+0x0C raw_time_a    49.0
+0x14 raw_time_c    49.0
+0x30 record_count  9
+0x34 first record
```

It therefore belongs to the same MOT family despite the `.bin` presentation label.

## Required C++20 module boundary

MOT must be a dedicated binary module:

```text
include/dmc_rengine/formats/mot.hpp
src/formats/mot.cpp
analysis/mot/...              # semantic CMotion binding later
tests/mot_tests.cpp
```

Initial `formats.mot-structural-v1` requirements:

1. identify by evidenced binary structure, not suffix alone;
2. validate `header_size` before any record access;
3. retain the complete raw header bytes;
4. expose the three raw f32 fields without semantic renaming;
5. read `record_count` at `header_size`;
6. bounds-check every `u16 record_span` and preserve each raw record payload;
7. preserve/report the zero alignment tail;
8. accept both observed `0x30` and `0x50` envelopes;
9. fail closed on zero spans, overflow, truncation or records that cross EOF;
10. remain read-only.

## Relationship to MOD

MOT is not a subtype of MOD. The architectural relationship is runtime/dataflow:

```text
MOT / CMotion evaluation
        -> pose / transform channels
        -> MOD/EFM node-domain runtime transforms
        -> world matrices
        -> skinned geometry
```

The exact record-to-node/channel binding is still a reverse target. Do not map record indices to bones by assumption.

## Open reverse gates

- trace extension-dispatch index `0` to concrete MOT acquisition/parse code;
- recover the runtime meaning of the three f32 header fields;
- decode record headers beyond `record_span`;
- identify channel IDs, interpolation encodings, key counts and key payloads;
- bind MOT records to MOD motion groups and node indices in machine code;
- distinguish MOT/MOT2/... revision or logical-family naming from physical header variants;
- compare em000 against player, weapon, boss and stage animation corpora;
- prove replay and only then design authoring/writer stages.

## Evidence status

| Claim | Status |
|---|---|
| `.mot` runtime family exists | `EXE_CONFIRMED` |
| all 82 bound motion-container children expose `MOT\0` | `CORPUS_CONFIRMED` |
| `.bin` extractor suffix can contain MOT | `CORPUS_CONFIRMED` |
| header-size values 0x30/0x50 | `CORPUS_CONFIRMED` |
| record count at `header_size` | `CORPUS_CONFIRMED` |
| variable records begin with u16 span | `CORPUS_CONFIRMED` |
| f32 high-level timing semantics | `PRESERVED_UNDECODED` |
| record/channel semantics | `PRESERVED_UNDECODED` |
| writer authority | `NOT AUTHORIZED` |
