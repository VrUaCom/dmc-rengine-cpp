# GDSpaces L1 Pass 87 — Original PTX envelope geometry — 2026-08-21

## Scope

Layer 1 only. This pass begins recovery of the **original EXE-confirmed PTX/TIM2 representation** after Pass 86 separated it from the transformed DDS-bearing stage-drop profile.

This pass is intentionally read-only and does **not** implement:

- PTX header decoding from raw bytes;
- TIM2 picture-header semantics;
- CLUT or swizzle conversion;
- DDS↔TIM2 conversion;
- PTX/TIM2 serialization;
- original-game texture writeback.

## Canonical EXE evidence

Target executable:

- `dmc3.exe`
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

The reconciled Phase 16 / claim-ledger evidence confirms:

1. the original runtime PTX envelope has a texture count;
2. it has one `blockCount` value per texture;
3. the first TIM2 entry begins at `+0x800`;
4. entry `i+1` advances by `blockCount[i] * 0x800` bytes;
5. the original parser checks the `TM2\0` marker;
6. the runtime `PtxRuntimeBundle` has capacity for up to 64 texture pointers;
7. the original PTX/TIM2 path is distinct from both the transformed DDS-bearing PAC representation and the separate DDS-from-memory loader.

Canonical source records:

- `DMC3 EXE — Complete Consolidation Pass 0-32 — Canonical 2026-08-04`, Phase 16;
- `04_DMC3_EXE_Claim_Reconciliation_Ledger_Pass_0_34_2026-08-05`, renderer/shaders/textures section.

## Missing evidence that remains missing

The preserved Phase 16 recovery index states that the original raw Phase 12–16 ZIP/JSON/annotation packages are no longer present in Drive. The surviving canonical consolidation records do **not** retain the exact byte offsets of:

- `textureCount` inside the first `0x800` bytes;
- `blockCount[]` inside the first `0x800` bytes.

Those offsets are therefore **not inferred** as `+0x00/+0x04` or any other convenient layout.

This is the central safety boundary of Pass 87.

## Implemented authority

Pass 87 adds `OriginalPtxEnvelopeGeometryValidator`.

Input:

- raw candidate PTX bytes;
- an already-decoded `texture_count`;
- an already-decoded `block_counts[]` array supplied by a future evidence-backed header decoder.

Validation:

- decoded count and `block_counts[]` length must agree;
- zero count is fail-closed because no zero-texture original envelope is currently evidenced;
- count must be `<= 64`;
- each block count must be non-zero;
- first physical entry is fixed to `0x800`;
- every entry span is exactly `blockCount * 0x800`;
- every entry begins with `54 4D 32 00` (`TM2\0`);
- all offset arithmetic is overflow-checked;
- every declared physical span must remain within the supplied source bytes;
- trailing source bytes are preserved as an observed quantity rather than rejected or normalized.

The complete first `0x800` bytes remain opaque.

## Regression boundary

The synthetic evidence-backed test covers:

- multi-entry advancement (`2,1,3` blocks);
- exact first-entry and cumulative offsets;
- opaque/random-looking `0x800` header bytes;
- count/array mismatch;
- zero count;
- `>64` count;
- zero block count;
- truncated entry header;
- declared span beyond EOF;
- bad `TM2\0` magic;
- accepted trailing bytes with an explicit receipt.

The test does not claim a synthetic fixture is a retail PTX sample. It validates only the recovered physical relation.

## Relationship to Pass 86

Pass 86 remains authoritative for provenance:

`original PTX/TIM2 != transformed DDS-bearing PAC profile != extracted DDS != runtime GPU texture`.

Pass 87 therefore does not reuse the transformed DDS descriptor (`0x70`) or DDS bundle (`0x800`) serializer as evidence for original PTX/TIM2 layout.

## Next reverse gates

1. recover the exact header read instructions / field offsets for `textureCount` and `blockCount[]` from the canonical EXE or a restored Phase 16 raw artifact;
2. wrap the geometry validator in a real raw-byte PTX envelope parser only after those offsets are proven;
3. recover the TIM2 picture-header fields consumed by the DMC3 runtime;
4. recover CLUT/swizzle/conversion rules;
5. obtain a direct retail PTX-bearing resource sample and run the current C++ parser against it;
6. only then design original PTX/TIM2 authoring and writeback.

## Layer-1 status

Layer 1 remains **NOT COMPLETE**. This pass closes an original-representation structural sub-boundary without claiming original texture serialization.
