# DMC3 HD em000 effect pack — grouped-record reverse checkpoint (2026-09-08)

**Branch:** `reverse/effect-pack-em000-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Target:** `em000_041.pnst`  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Primary correction

The current canonical `EffectPackParser` was intentionally bounded to the earlier `st001_effect.pac` / `st114_effect.pac` evidence and assumes:

```text
one accepted manifest line == one populated payload in records PNST
```

`em000_041.pnst` proves that this is **not a universal DMC3 effect-pack rule**.

The em000 structure is:

```text
outer PNST
  slot 0 -> ASCII manifest
  slot 1 -> inner PNST record storage
```

but the manifest describes **logical record groups**, and at least kind `M` consumes two physical slots.

The parser and stored-name evidence layer must therefore evolve from:

```text
ManifestLine -> PhysicalPayload
```

to:

```text
ManifestEntry -> PhysicalRecordGroup[1..N]
```

without weakening physical slot identity.

## Manifest census

The em000 manifest contains 173 semantic entries:

| Kind | Entries |
|---|---:|
| `G` | 12 |
| `V` | 50 |
| `E` | 45 |
| `P` | 34 |
| `T` | 8 |
| `A` | 11 |
| `M` | 13 |
| **Total** | **173** |

`G` and `M` were not represented in the previous five-kind contract (`V/E/P/T/A`). They are direct corpus facts and must no longer be rejected merely because the previous smaller corpus did not contain them.

No semantic expansion of the letters is invented in this checkpoint.

## Physical inner PNST census

The inner record PNST exposes physical slot indices `0..185`.

Observed:

```text
declared physical domain: 186 slots
populated payloads:        183
empty slots:               3
empty indices:             161, 163, 167
```

The first 160 manifest entries (`G/V/E/P/T/A`) map one-to-one to physical slots `0..159`.

The 13 `M` entries then consume paired physical slots:

```text
M[0]  -> 160,161
M[1]  -> 162,163
M[2]  -> 164,165
M[3]  -> 166,167
...
M[12] -> 184,185
```

For every pair:

- first slot is a valid `MOD ` resource;
- second slot is an optional companion;
- three second slots are physically empty (`161`, `163`, `167`);
- ten second slots contain an identical 16-byte payload.

Observed 16-byte companion bytes:

```hex
31 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
```

Safe interpretation:

> `M` is a grouped model-bearing effect record whose observed physical representation is `MOD + optional 16-byte companion`.

The meaning of `0x31` is unresolved and must remain raw.

## Why the old one-to-one invariant fails

Manifest entries:

```text
173
```

Populated physical payloads:

```text
183
```

Difference:

```text
10
```

That difference is exactly the number of populated optional `M` companion slots.

Therefore the current `manifest_line_count == populated_record_count` acceptance rule is contradicted by a valid retail em000 effect pack and must be treated as a **variant-specific earlier-corpus invariant**, not a universal format law.

## Record-kind extents

### `G`

```text
12 records
96 bytes each
```

### `V`

```text
50 records
368 bytes each
```

### `E`

```text
45 records
544 bytes each
```

### `P`

`P` is **not fixed at 704 bytes** in em000. Observed sizes include:

```text
336
528
704
896
```

with multiple records at 336 and 528 and smaller counts at 704/896.

Therefore the existing fixed-extent expectation for `P` can only remain as historical corpus corroboration, not a universal parser requirement.

### `T`

Eight texture-bearing resources are present.

Each is a DMC texture-slot wrapper:

```text
0x70-byte DMC descriptor
-> embedded "DDS " at +0x70
-> DDS body
```

Observed physical sizes include one larger 87,648-byte resource and seven 22,112-byte resources in this corpus.

These are **wrapped DDS resources**, not plain DDS beginning at byte zero and not full PTX bundles.

### `A`

```text
11 records
336 bytes each
```

### `M`

```text
13 logical manifest entries
13 MOD primary slots
10 populated 16-byte companions
3 empty companion slots
```

## Format-identity boundary

The letters `G`, `V`, `E`, `P`, `T`, `A`, `M` are currently **effect-pack record kinds**. They are not automatically standalone filename extensions.

Do not create invented `.g`, `.v`, `.e`, `.p`, `.a` file formats from these letters until executable consumers prove independent resource-family identity.

Current safe modular boundary:

```text
formats/effect_pack
    manifest parser
    grouped record mapper
    kind-specific structural adapters
        G raw-96
        V raw-368
        E raw-544
        P variable-size raw structure
        T wrapped-DDS adapter
        A raw-336
        M MOD + optional companion group
```

A kind-specific adapter can exist without pretending that its kind is a public extension.

## Required correction to canonical architecture

The future parser should expose concepts equivalent to:

```text
EffectManifestEntry
    kind
    identifier
    source_line
    raw_label

EffectPhysicalMember
    physical_slot
    offset
    extent
    byte identity

EffectRecordGroup
    manifest entry
    one or more physical members
    group variant
```

For the currently observed em000 grammar:

```text
G/V/E/P/T/A -> primary member only
M           -> primary MOD member + optional companion member
```

This arity table must remain profile/corpus evidence, not an unversioned global assumption.

## Stored-name evidence implications

`EffectStoredNameEvidenceBuilder` currently seals one manifest label directly to one physical child. That is insufficient for `M`.

The revised evidence model must preserve both facts:

1. the manifest entry names the **logical group**;
2. every physical member retains its own immutable `ResourceId`, slot index, byte range and SHA-256.

The name layer must never collapse the paired slots into one physical resource.

A safe model is:

```text
logical group evidence
  authority = enclosing effect pack + exact manifest line
  members[] = exact physical child identities
```

Presentation may show a grouped resource, while extraction/rebuild continues to preserve the two physical slots.

## Relationship to existing formats

### PNST

PNST remains the physical container envelope. The effect-pack grammar is a semantic profile layered above PNST and must not be baked into the generic PNST parser.

### MOD

`M` primary members are genuine MOD resources and should route to the canonical MOD module after group materialization.

### DDS/PTX

`T` members are descriptor-plus-DDS wrapped texture slots. They should route through the canonical texture-slot framing logic as `wrapped_dds`, not be reimplemented inside effect code.

## Open reverse gates

- trace the producer/consumer of the effect manifest and grouped record storage;
- determine semantic meaning of `G/V/E/P/A/M` letters from executable consumers;
- recover the `M` 16-byte companion meaning and whether it is optional by design or by value;
- decode variable `P` layouts and determine whether size selects a subtype;
- bind `T` resources to effect texture consumers;
- acquire additional enemy/effect packs and infer grouping variants without overfitting em000;
- redesign stored-name evidence to support one logical entry -> multiple physical members;
- add real-corpus regression tests before changing canonical main behavior.

## Evidence status

| Claim | Status |
|---|---|
| outer two-slot PNST + manifest + inner PNST | `CORPUS_CONFIRMED` |
| em000 manifest has G/V/E/P/T/A/M | `CORPUS_CONFIRMED` |
| G/V/E/P/T/A first 160 entries map 1:1 to slots 0..159 | `CORPUS_CONFIRMED` |
| M maps to paired slots with primary MOD | `CORPUS_CONFIRMED` |
| M companion is optional in current corpus | `CORPUS_CONFIRMED` |
| 16-byte companion semantics | `PRESERVED_UNDECODED` |
| P has one universal fixed extent | `REJECTED` |
| manifest line universally equals one populated payload | `REJECTED` |
| letters are standalone file extensions | `NOT PROVEN` |
| writer authority | `NOT AUTHORIZED` |
