# DMC3 HD MOD — canonical EXE unknown-byte closure index (2026-09-09)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Architecture:** PE32+ x86-64  
**Bounded MOD corpus:** 38 unique MODs / 166 objects / 180 meshes / 20,976 vertices / 285 transforms  
**Evidence sync follow-up:** 2026-09-10

## Evidence vocabulary

This note uses only the project-wide canonical statuses:

```text
EXE_CONFIRMED
CORPUS_CONFIRMED
EXE_AND_CORPUS_CONFIRMED
STRUCTURAL_CONFIRMED
SEMANTIC_CANDIDATE
PRESERVED_UNDECODED
RESERVED_OBSERVED_ZERO
REJECTED
```

Zero in the corpus is never padding proof. Blender/importer behavior is not format authority. Same offset across MOD/EFM/SCM is not semantic proof.

## Canonical serialized/materialized boundary

Reader-side state is not serialized-ABI evidence. The following remain distinct:

```text
declared element count
serialized stream offset / serialized field
materialized reader vector
writer-owned output stream
```

A reader may resize a vector from a declared count before proving a valid serialized source stream exists. Therefore reader vector size cannot establish serialized stream presence.

MOD mesh `+0x48` is the separate serialized `generated_topology_count`; it is not physical topology-workspace capacity. The observed `align16(6*(vertex_count-2))` rule belongs to the workspace associated with `+0x40`.

Android/native-reader test success is regression/build evidence, not semantic file-format proof.

## Canonical executable verification

The retail executable used for direct reverse is 6,356,432 bytes and hashes exactly to the canonical SHA-256 above. All direct VAs refer to that binary.

## Closure table

| Target | Current evidence-safe status | Key direct result | Canonical note |
|---|---|---|---|
| `BLENDINDICES.x` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | canonical runtime non-use closed: serialized `+0x28` -> runtime `+0x140` -> draw descriptor `+0x30`; all 8 compiled DXBC BLENDINDICES signatures have `Mask=0xF`, `ReadWriteMask=0xE`; CPU census has no X consumer | `dmc3-mod-blendindices-x-reverse-2026-09-09.md` |
| transform `+0x1C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | whole-image manager-transform fingerprint census closes canonical runtime behavior as dormant/no effect; MOD/EFM matrix path and two CMotion transfer paths do not consume `+0x1C`; SCM control explicitly overwrites homologous W scratch | `dmc3-mod-transform-1c-reverse-2026-09-09.md` |
| mesh `+0x0C` | `STRUCTURAL_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | raw `u32`; no positive consumer in audited load/build/material paths; whole-EXE provenance closure still required | `dmc3-mod-mesh-0c-reverse-2026-09-09.md` |
| mesh `+0x38` | `EXE_AND_CORPUS_CONFIRMED` for MOD-specific runtime role | EFM homologous slot is live COLOR0; canonical MOD disables corresponding runtime auxiliary stream | `dmc3-mod-mesh-38-reverse-2026-09-09.md` |
| mesh `+0x4C` | `STRUCTURAL_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | `+0x48` is separate generated topology count; no `+0x4C` companion transfer established; whole-EXE provenance closure still required | `dmc3-mod-mesh-4c-reverse-2026-09-09.md` |
| header `+0x14` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | serialized raw `u32` copied to manager `+0xE4`; old decimal identity rejected; whole-image raw `+0xE4` census plus accessor-rooted recursive dataflow, wrapper escapes and exact CMotion RTTI/vtable census find no downstream manager `+0xE4` consumer | `dmc3-mod-header-14-reverse-2026-09-09.md` |
| source flag `0x00100000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED`; artistic category `PRESERVED_UNDECODED` | technical semantic closed: GS `TEST_1.AREF` 0 vs 16 and `ZBUF_1.ZMSK` 1 vs 0 through per-mesh A+D descriptors and generic backend | `dmc3-mod-source-flag-00100000-reverse-2026-09-09.md` |
| source flag `0x00200000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | canonical typed runtime consumption closed as carried/restored but uninterpreted: 31 exact object-derivation functions, 61 direct object-passing calls, 39 helper targets and a 45-state/19-edge recursive census expose no bit21 semantic decoder or typed indirect escape | `dmc3-mod-source-flag-00200000-reverse-2026-09-09.md` |

## Writer-preservation contracts

The evidence layer separates serialized bytes from runtime semantics.

- `BLENDINDICES.x` is preserved exactly even though canonical CPU/DXBC consumption is closed as non-use.
- transform `+0x1C` remains raw/preserved even though canonical transform-runtime behavior is closed as dormant.
- header `+0x14` remains raw/preserved even though canonical manager `+0xE4` downstream non-consumption is closed.
- mesh `+0x0C/+0x38/+0x4C` retain raw preservation projections.
- source flag `0x00100000` has a technical GS-state semantic, but the source bit is preserved exactly.
- source flag `0x00200000` is preserved exactly even though canonical typed runtime consumption is closed as carried but uninterpreted.

Canonical runtime non-use never authorizes zero-normalizing unresolved serialized bytes or source bits.

## Transform `+0x1C` closure follow-up

The manager transform domain uses `+0x20` transform pointer, `+0xEA` count and `+0x188` runtime workspace. A whole-image joint-fingerprint census finds exactly five functions: source-pointer materializer `0x1402F1DB0`, MOD/EFM transform builder `0x1402FA080`, SCM control `0x1402FA360`, and CMotion materializers `0x14030F850` / `0x14030FAD0`.

Every canonical consumer either constructs the pointer or gives serialized `+0x1C` no transform effect. Canonical runtime behavior is therefore closed as `EXE_CONFIRMED` dormant; serialized semantics remain `PRESERVED_UNDECODED`.

## Header `+0x14` recursive typed closure follow-up

Owner-to-manager accessors:

```text
0x140089DE0 -> parent +0x80
0x140089DF0 -> parent +0x50
```

Immediate and 67 first-hop helper census yields zero manager `+0xE4` reads. Recursive `.pdata`-bounded pointer dataflow reaches 70 taint states across 49 functions, again with zero manager `+0xE4` reads, and identifies three stored-pointer escapes.

Two wrapper escapes (`+0x08/+0x10`) were followed through wrapper methods and helper family `0x1402F7350..0x1402F75D0`; they consume other manager state, not `+0xE4`.

The third escape stores the manager to `CMotion +0xE8` in both CMotion materializers. Exact RTTI identifies `.?AVCMotion@@`, CompleteObjectLocator `0x140520018`, vtable `0x140507938`, 45 virtual methods. None accesses `this+0xE8`, and the wider motion-family span `0x14030E000..0x140311500` contains zero non-stack qword reads of `[*+0xE8]`.

A whole-image non-stack qword `+0xE8` census has only six readers. Four are RTTI-confirmed `CComEm000/005/006/008`; the remaining two do not read or forward pointee `+0xE4`. Therefore no hidden backreference route reaches the header-derived manager field.

Canonical downstream non-consumption of manager `+0xE4` is closed as `EXE_CONFIRMED`. This is not permission to rename the serialized field: it stays `PRESERVED_UNDECODED` and writer-preserved.

## Source flag `0x00200000` typed-runtime closure follow-up

The serialized source word is copied intact into runtime baseline/effective flags:

```text
0x140302AB2  read serialized object +0x10
0x140302ABF  -> runtime effective +0x14
0x140302AC9  -> runtime baseline +0x10
```

The state machine preserves or reintroduces bit21, but the recovered semantic decoders use other bits only:

```text
0x140302640  -> masks 0xF, 0x00010000, 0x00100000
0x1402F9890  -> masks 0x00004000
0x1402F28E0  -> BTS bit17 and writes the complete effective word back
```

The table-address use in `0x1402F17C0` is driven by a low-mode value already reduced with `AND 0xF`; source bit21 cannot contribute to the table index.

A structural whole-executable pass derives each runtime object through:

```text
runtime_object = manager +0x100 + object_index * 0x380
```

The direct typed surface contains 31 exact derivation functions, 61 direct object-passing call sites and 39 unique helper targets. The only direct effective `+0x14` read on that exact surface is `0x1402F9ED9`, which forwards the word to `0x1402F9890`; that helper reduces it to `0x4000`.

A conservative recursive pass seeded by all 39 helpers covers 45 typed states and 19 propagation edges. It finds zero tagged indirect/vtable calls and zero new bit21 semantic decoders. Candidate object-pointer stores/returns produced by a deliberately path-insensitive escape pass were manually classified and rejected as scalar state, draw/mesh pointers, allocation-array returns or packet values rather than exported `0x380` runtime-object pointers.

The numerically equal bit21 domains remain separate:

```text
manager +0xE0 bit21    generated from different serialized source bits 0x200/0x400
runtime object +0x304  independently initialized/mutated; bit21 set in its own path
```

Neither domain has a provenance edge from source-carried `+0x10/+0x14`, so both remain `REJECTED` as consumers of serialized source bit `0x00200000`.

The canonical DMC3 HD typed-runtime consumption gate is therefore closed:

```text
serialized presence                    CORPUS_CONFIRMED
source -> baseline/effective           EXE_CONFIRMED
carriage/restoration                   EXE_CONFIRMED
canonical typed bit21 decoder          EXE_CONFIRMED: none
canonical typed runtime effect         EXE_CONFIRMED: dormant/uninterpreted
serialized high-level semantic         PRESERVED_UNDECODED
writer policy                          preserve exact source bit
```

This closure does not establish an artistic category and does not authorize renaming the source bit as padding/reserved.

## Corrected `0x00200000` domain collision

Historical `dmc3-mod-source-flag-00200000-chain-20260909.json` describes manager `+0xE0` bit `0x00200000`, raised through a different source path. It remains `REJECTED` as a serialized source-bit receipt. Its evidence lives in:

```text
data/reverse/dmc3-mod-manager-bit21-runtime-vector-20260909.json
```

## Machine-readable receipts

```text
data/reverse/dmc3-mod-blendindices-x-20260909.json
data/reverse/dmc3-mod-transform-1c-20260909.json
data/reverse/dmc3-mod-mesh-0c-20260909.json
data/reverse/dmc3-mod-mesh-38-20260909.json
data/reverse/dmc3-mod-mesh-4c-20260909.json
data/reverse/dmc3-mod-header-14-20260909.json
data/reverse/dmc3-mod-source-flag-00100000-20260909.json
data/reverse/dmc3-mod-source-flag-00200000-20260909.json
data/reverse/dmc3-mod-manager-bit21-runtime-vector-20260909.json
```

Aggregate machine index:

```text
data/reverse/dmc3-mod-canonical-exe-unknown-byte-closure-20260909.json
```

## Closed direct-EXE gates

The following are no longer blockers for the unknown-field phase:

1. `BLENDINDICES.x` canonical CPU/GPU consumption;
2. transform `+0x1C` canonical transform-runtime consumption;
3. mesh `+0x38` MOD-specific auxiliary-stream role;
4. header `+0x14 -> manager +0xE4` canonical downstream consumer census;
5. source flag `0x00100000` technical renderer semantic;
6. source flag `0x00200000` canonical typed-runtime consumption.

Their serialized preservation requirements remain in force where stated.

## Remaining direct-EXE gates

1. Complete provenance-aware whole-EXE closure for mesh `+0x0C/+0x4C` outside known loaders/builders.
2. Classify retained-pointer consumers for secondary object regions `+0x04..+0x07`, `+0x14..+0x17`, `+0x20..+0x2F`.
3. Census secondary header and node-domain zero regions without promoting zero observations to padding.
4. After writer-critical preservation contracts are complete, promote the MOD byte-preserving writer and prove exact no-edit round-trip before edited writer authority.

Until a remaining field gains stronger evidence, `PRESERVED_UNDECODED` is the correct writer-safe semantic status.
