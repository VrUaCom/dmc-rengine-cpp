# DMC3 HD MOD header `+0x14` closure audit — 2026-09-07

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Question

What does serialized MOD header `u32 +0x14` mean, and is there enough evidence to promote a MOD-specific semantic name?

## Confirmed serialized/runtime flow

The common model-manager initializer directly copies the field:

```text
0x1402F95BE  read u32 resource +0x14
0x1402F95C5  write u32 manager +0xE4
```

Therefore the field is runtime-carried and is not merely offline metadata.

Current typed storage:

```cpp
Header::runtime_metadata_u32
```

Current safe state:

```text
flow: EXE_CONFIRMED
MOD semantic meaning: PRESERVED_UNDECODED
```

## Fresh repository-wide evidence audit

The current canonical repository contains the following evidence classes around this field:

1. shared document-core layout records `+0x14` as the common runtime-carried u32;
2. MOD documentation explicitly preserves it and rejects transfer of SCM semantics;
3. the previous MOD default-joint pass performed a bounded model-subsystem census and did not establish a MOD-specific downstream read of manager `+0xE4`;
4. SCM corpus work has a stable six-decimal-digit decomposition for the homologous field, but that is format-specific corpus evidence rather than MOD proof;
5. no current MOD analysis module exposes a semantic accessor for `+0x14`, unlike `+0x13` which has direct `default_joint_index` consumers.

This audit therefore finds **no basis for semantic promotion** on current canonical evidence.

## Important rejection

The following shortcut remains invalid:

```text
SCM +0x14 has a legacy resource/provenance interpretation
therefore
MOD +0x14 has the same meaning
```

Shared offset and shared transport to manager `+0xE4` prove common ABI/flow, not common higher-level semantics.

## Evidence needed to close the field

### Pass 1 — canonical executable xref census

Acquire a fresh raw-byte/disassembly view of the canonical executable and enumerate all reads of model-manager displacement `+0xE4`.

For each candidate read:

1. prove the base pointer is the same model-manager type initialized by `0x1402F9570`;
2. identify the family-mask/dispatcher path reaching the consumer;
3. distinguish MOD/EFM/SCM/common behavior;
4. follow the value until a branch, lookup, arithmetic transform, resource request, render packet, motion path or external API boundary gives a stable behavioral meaning;
5. hash-bind every byte window used for promotion.

A displacement-only grep is insufficient because unrelated structures can also have an `+0xE4` member.

### Pass 2 — MOD corpus correlation

For every accessible retail MOD payload, record:

```text
file identity/hash
header +0x14 raw u32
header +0x13 default_joint_index
node count
object count
texture mirror
object source-flag union
motion-group histogram
companion identity/count when known
resource/container lineage
```

Then test whether `+0x14` correlates with:

- actor/model family identity;
- model set / variant index;
- animation grouping;
- texture companion selection;
- parent archive slot/resource ordinal;
- object/render-state patterns.

Correlation alone can produce a `SEMANTIC_CANDIDATE`; it cannot produce `EXE_CONFIRMED` without a downstream executable consumer.

## Current evidence-access boundary for this pass

This pass had access to the canonical repository's hash-bound reverse receipts and prior bounded disassembly findings, but did not receive a complete raw canonical `e454...` executable image through the connected evidence surface for a fresh global xref scan. Therefore it deliberately does not fabricate new `manager+0xE4` read addresses.

This is an acquisition boundary, not evidence that no such consumer exists.

## Result

No semantic promotion.

```text
serialized location      = +0x14 u32
runtime destination      = manager +0xE4
flow                     = EXE_CONFIRMED
MOD-specific semantics   = PRESERVED_UNDECODED
writer mutation authority= NOT_PROMOTED
```

## Next trigger for promotion

Resume this field immediately when either of these becomes available:

- canonical raw executable bytes/disassembly allowing a trustworthy `manager+0xE4` read-xref census; or
- a broader MOD corpus containing discriminating/nontrivial `+0x14` values that can be correlated and then chased into executable consumers.

Until then, future MOD writers must preserve `+0x14` byte-exactly and must not synthesize it from SCM resource-code helpers.
