# DMC3 HD em000 — active reverse gates (2026-09-08)

**Branch:** `research/enemy-architecture-em000-20260908`

This document turns the current unresolved-format inventory into an active work program. It does not create new format identities; it points each open question back to the already-owned format branches.

## Priority 1 — gameplay -> root effect selection

Goal:

```text
CEm / attack / damage / death / action
    -> root effect id
    -> G/V or direct P/E
    -> downstream effect graph
```

Current known boundary:

- G/V routing graph is recovered in `reverse/effect-pack-em000-20260908`;
- G/V do not reach every P/E record, so direct P/E roots or another upstream selector must exist;
- no canonical runtime field/function is yet promoted as the root-effect selector.

Required evidence:

1. locate `CEm*`/enemy action code that requests an effect;
2. identify the selected manifest-domain id and kind;
3. prove whether roots can be G, V, P, E, or a separate table;
4. bind at least one concrete em000 gameplay action to one effect graph;
5. record exact EXE addresses and observable effect.

Status: `OPEN / highest priority`.

## Priority 2 — MOT evaluator -> CMotion -> MOD

Owned by: `reverse/mot-em000-20260908`.

Already structural:

- 82 MOT resources;
- channel-domain table;
- exact record-count/popcount invariant;
- 5,118 tracks;
- compression 2/3 envelopes;
- lower-15-bit key-time rule;
- strong channel-semantic candidate mapping.

Still required:

1. canonical executable compression switch;
2. exact `/65535` quantization path;
3. Hermite evaluator;
4. channel-bit meanings in machine code;
5. header timing-field semantics;
6. key control-bit semantics;
7. exact track -> CMotionJoint -> MOD node binding;
8. wider player/weapon/boss corpus;
9. writer only after evaluation parity.

Status: `PARTIAL / high priority`.

## Priority 3 — SO working family identity and consumers

Owned by: `reverse/so-em000-identity-20260908`.

Known:

```text
038 -> graph/control structure
039 -> 24 x 4-byte compact records
040 -> 23 x 0x50 spatial records
```

Cross-resource correlation:

```text
23 link-domain records
23 spatial records
23 MOD transform selectors
```

Still required:

1. original resource name/extension, if any;
2. graph owner/consumer;
3. semantics of the four-byte link records;
4. switch/consumer for spatial type values 2 and 4;
5. exact node/transform selector binding;
6. only then gameplay labels such as collision/hit/contact.

Status: `STRUCTURALLY STRONG / SEMANTIC IDENTITY OPEN`.

## CLT

Owned by: `reverse/clt-em000-20260908`.

Known:

- real `.clt` family;
- text serialization with NUL padding;
- cloth/deformation subsystem evidence;
- `Bone` entries map into associated local MOD/EFM node domains in em000;
- `WindParent` can point outside that local domain.

Still required:

- exact runtime meaning of `Gravity`, `SpringForce`, `MaxSpeed`, `Stiffness`, `Wind`, `WindLocal`, `WindParent`, `WindType`, `LimitLength`;
- `Y/NY/Z` axis-token semantics;
- exact external parent-domain ownership;
- multiple-cloth-record grammar outside current samples;
- executable parser field destinations;
- writer.

## TSC

Owned by: `reverse/tsc-em000-20260908`.

Known:

- real `.tsc` family;
- dedicated text DSL;
- `TexNo` correlates with the EFM/model texture-slot domain and available PTX slot in the bound actor cluster.

Still required:

- semantics of `ScrlType`;
- `DirUV` token semantics;
- `TimeUV` math;
- `MinimumUV` math;
- `RELATIVE` versus `ABSOLUTE` behavior;
- `<Finish>` semantics;
- exact runtime state receiving each field;
- broader TSC corpus;
- writer.

## EFM

Owned by: `reverse/efm-em000-20260908`.

Known:

- related model-family shell;
- real mesh-bearing EFM payload;
- six source streams;
- mesh `+0x38` is the EFM extra per-vertex `COLOR0` stream in the bound layout.

Still required:

- exact COLOR0 normalization/alpha semantics;
- whether skin packing is MOD-identical across variants;
- texture companion/runtime descriptor ownership;
- EFM-specific meanings of header `+0x13/+0x14`;
- complete node-domain semantics from the EFM consumer, not copied from MOD;
- revision/corpus diversity;
- writer.

## Effect pack G/V/E/P/T/A/M

Owned by: `reverse/effect-pack-em000-20260908`.

Known graph:

```text
G/V -> G/V/P/E
E subtype 1/2 -> T + optional A
E subtype 5   -> M -> MOD + optional companion
P variant 02/03 -> T + A -> same T
A -> T and 33 x 0x0A grid entries
T -> 0x70 DMC wrapper + DDS
```

Still required:

### G/V
- semantic difference between G and V;
- high-byte selector/modifier meaning;
- remaining record fields;
- upstream root selection.

### E
- semantic meaning of subtype 1, 2, 5;
- raw modifiers;
- rest of 0x220 record behavior.

### P
- semantics of 0x130 primary block;
- semantics of 0xB0 auxiliary blocks;
- variants `01/01` and `00/04`;
- meaning of internal names;
- runtime consumer.

### A
- meaning of `+0x02/+0x03`;
- `raw_flags`;
- active-entry count/mode relation;
- EXE proof for UV/atlas semantics.

### T
- remaining 0x70 wrapper field semantics;
- exact effect-side texture materialization path.

### M
- meaning of the optional 16-byte companion;
- meaning of constant first u32 `0x31` in populated companions;
- whether optionality is semantic or storage-driven.

## MOD

Owned by: `reverse/mod-completion-20260907`.

Still open:

- header `+0x14` semantic closure;
- transform `+0x1C`;
- mesh `+0x0C/+0x38/+0x4C`;
- `BLENDINDICES.x`;
- remaining source-flag semantics;
- writer and original-game edited acceptance.

## PTX / wrapped DDS

PTX identity and em000 binding are strong. Remaining work is mostly descriptor semantics and authoring/reintegration. Effect `T` wrapped-DDS slots are a separate framing and must not be conflated with PTX bundles.

## Definition of current work

The next reverse passes are considered successful only when they convert an item above from `OPEN/PRESERVED_UNDECODED/SEMANTIC_CANDIDATE` into a bounded evidence claim with:

1. exact byte offset/width or runtime field;
2. direct producer/consumer evidence;
3. corpus statistics where applicable;
4. explicit non-claims;
5. modular C++ implementation if promotable;
6. regression;
7. machine-readable receipt;
8. exact-head CI before merge.

No writer work is allowed to outrun semantic/runtime closure.
