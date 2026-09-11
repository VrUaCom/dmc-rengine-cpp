# DMC3 HD MOD — canonical EXE unknown-byte closure index (2026-09-09)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Architecture:** PE32+ x86-64  
**Bounded MOD corpus:** 38 unique MODs / 166 objects / 180 meshes / 20,976 vertices / 285 transforms  
**Evidence sync follow-up:** 2026-09-11

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

MOD mesh `+0x48` is the separate serialized/generated `generated_topology_count`; it is not physical topology-workspace capacity. The observed `align16(6*(vertex_count-2))` rule belongs to the workspace associated with `+0x40`.

Android/native-reader test success is regression/build evidence, not semantic file-format proof.

## Canonical executable verification

The retail executable used for direct reverse is 6,356,432 bytes and hashes exactly to the canonical SHA-256 above. All direct VAs refer to that binary.

## Closure table

| Target | Current evidence-safe status | Key direct result | Canonical note |
|---|---|---|---|
| `BLENDINDICES.x` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | canonical runtime non-use closed: serialized `+0x28` -> runtime `+0x140` -> draw descriptor `+0x30`; all 8 compiled DXBC BLENDINDICES signatures have `Mask=0xF`, `ReadWriteMask=0xE`; CPU census has no X consumer | `dmc3-mod-blendindices-x-reverse-2026-09-09.md` |
| transform `+0x1C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | whole-image manager-transform fingerprint census closes canonical runtime behavior as dormant/no effect; MOD/EFM matrix path and two CMotion transfer paths do not consume `+0x1C`; SCM control explicitly overwrites homologous W scratch | `dmc3-mod-transform-1c-reverse-2026-09-09.md` |
| mesh `+0x0C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | whole-image closure: 6 exact direct `mesh_table + index*0x50` derivations plus 34 runtime-mesh derivations; 43 reachable typed funcs / 20 typed calls / 0 source-pointer escapes; 21 live source-field reads consume neighbors but never `+0x0C` | `dmc3-mod-mesh-0c-reverse-2026-09-09.md` |
| mesh `+0x38` | `EXE_AND_CORPUS_CONFIRMED` for MOD-specific runtime role | EFM homologous slot is live COLOR0; canonical MOD disables corresponding runtime auxiliary stream | `dmc3-mod-mesh-38-reverse-2026-09-09.md` |
| mesh `+0x4C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | same whole-image direct/backreference closure; live post-build source reads consume `+0x48` five times but never adjacent `+0x4C`; raw `+0x4C` hits are unrelated layouts | `dmc3-mod-mesh-4c-reverse-2026-09-09.md` |
| header `+0x14` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | serialized raw `u32` copied to manager `+0xE4`; old decimal identity rejected; whole-image raw `+0xE4` census plus accessor-rooted recursive dataflow, wrapper escapes and exact CMotion RTTI/vtable census find no downstream manager `+0xE4` consumer | `dmc3-mod-header-14-reverse-2026-09-09.md` |
| source flag `0x00100000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED`; artistic category `PRESERVED_UNDECODED` | technical semantic closed: GS `TEST_1.AREF` 0 vs 16 and `ZBUF_1.ZMSK` 1 vs 0 through per-mesh A+D descriptors and generic backend | `dmc3-mod-source-flag-00100000-reverse-2026-09-09.md` |
| source flag `0x00200000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | canonical typed runtime consumption closed as carried/restored but uninterpreted: 31 exact object-derivation functions, 61 direct object-passing calls, 39 helper targets and a 45-state/19-edge recursive census expose no bit21 semantic decoder or typed indirect escape | `dmc3-mod-source-flag-00200000-reverse-2026-09-09.md` |

## Writer-preservation contracts

The evidence layer separates serialized bytes from runtime semantics.

- `BLENDINDICES.x` is preserved exactly even though canonical CPU/DXBC consumption is closed as non-use.
- transform `+0x1C` remains raw/preserved even though canonical transform-runtime behavior is closed as dormant.
- mesh `+0x0C` and `+0x4C` remain raw/preserved even though whole-image typed runtime consumption is closed as dormant/no effect.
- mesh `+0x38` remains source-preserved despite its MOD-specific runtime role being understood relative to EFM COLOR0.
- header `+0x14` remains raw/preserved even though canonical manager `+0xE4` downstream non-consumption is closed.
- source flag `0x00100000` has a technical GS-state semantic, but the source bit is preserved exactly.
- source flag `0x00200000` is preserved exactly even though canonical typed runtime consumption is closed as carried but uninterpreted.

Canonical runtime dormancy/non-use never authorizes zero-normalizing unresolved serialized bytes or source bits.

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

The third escape stores the manager to `CMotion +0xE8` in both CMotion materializers. Exact RTTI identifies `.?AVCMotion@@`, CompleteObjectLocator `0x140520018`, vtable `0x140507938`, 45 virtual methods. None accesses `this+0xE8`, and the wider motion-family span contains no relevant backreference consumer.

Canonical downstream non-consumption of manager `+0xE4` is closed as `EXE_CONFIRMED`. This does not rename the serialized field: it stays `PRESERVED_UNDECODED` and writer-preserved.

## Source flag `0x00200000` typed-runtime closure follow-up

The serialized source word is copied intact into runtime baseline/effective flags:

```text
0x140302AB2  read serialized object +0x10
0x140302ABF  -> runtime effective +0x14
0x140302AC9  -> runtime baseline +0x10
```

The recovered semantic decoders use other bits only:

```text
0x140302640  -> masks 0xF, 0x00010000, 0x00100000
0x1402F9890  -> masks 0x00004000
0x1402F28E0  -> BTS bit17 and writes the complete effective word back
```

The direct typed surface contains 31 exact runtime-object derivation functions, 61 direct object-passing call sites and 39 unique helper targets. Conservative recursive propagation reaches 45 typed states / 19 edges with zero tagged indirect/vtable calls and zero bit21 semantic decoders.

The canonical typed-runtime gate is closed as carried/restored but uninterpreted. The source bit remains `PRESERVED_UNDECODED` and writer-preserved.

## Mesh `+0x0C/+0x4C` whole-image closure follow-up

Two independent pointer-provenance surfaces were closed.

### Direct serialized mesh table

Whole-image function census for:

```text
serialized_mesh = object.mesh_table(+0x08) + mesh_index*0x50
```

finds exactly six functions:

```text
0x1402F7A90
0x1402F7D60
0x1402FDB40
0x1402FDD10
0x1402FE3B0
0x1402FE6A0
```

None reads or writes `+0x0C` or `+0x4C`. Neighboring fields are actively consumed, including generated topology count `+0x48`.

### Runtime mesh source backreference

MOD runtime builder stores the serialized source pointer at `runtime mesh +0x10` (`0x1402FE713`). A whole-image census of exact runtime-mesh derivation:

```text
runtime_mesh = object.runtime_mesh_array(+0x20) + mesh_index*0x1A0
```

finds **34 functions**. Conservative typed propagation from those roots reaches **43 functions** and **20 direct typed calls**, with:

```text
serialized-source pointer escapes    0
serialized-source indirect calls     0
```

Two roots outside the earlier local window, `0x14030D9B0` and `0x14030DA80`, feed `0x14030D8B0`; that helper operates on runtime stream buffers and never follows `runtime +0x10`.

Live source-backreference consumers make 21 provenance-confirmed serialized reads:

```text
+0x00  6
+0x02  2
+0x04  2
+0x06  1
+0x08  1
+0x0A  1
+0x40  3
+0x48  5
```

There are zero reads/writes of source `+0x0C` and zero reads/writes of source `+0x4C`.

`+0x48` is a particularly strong positive control: canonical build code writes/forwards it, and post-build consumer `0x140309C60` follows the source backreference and compares `+0x48` against `0xA2` and `0x16`, while never touching adjacent `+0x4C`.

Raw model-region `+0x4C` candidates (`0x1402F78DE`, `0x14030134F`, `0x140305FAF`, `0x14030806E`, `0x1403086A6`) were provenance-classified as unrelated statistics/control layouts, not serialized mesh records.

Therefore canonical runtime consumption is closed for both fields:

```text
mesh +0x0C runtime effect   EXE_CONFIRMED dormant/no effect
mesh +0x4C runtime effect   EXE_CONFIRMED dormant/no effect
serialized semantics        PRESERVED_UNDECODED
writer policy               preserve exact source u32
```

Dormancy is behavioral evidence only. Neither field is renamed padding/reserved.

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
3. mesh `+0x0C` canonical typed runtime consumption;
4. mesh `+0x38` MOD-specific auxiliary-stream role;
5. mesh `+0x4C` canonical typed runtime consumption;
6. header `+0x14 -> manager +0xE4` canonical downstream consumer census;
7. source flag `0x00100000` technical renderer semantic;
8. source flag `0x00200000` canonical typed-runtime consumption.

Their serialized preservation requirements remain in force where stated.

## Remaining direct-EXE gates

1. Classify retained-pointer consumers for secondary object regions `+0x04..+0x07`, `+0x14..+0x17`, `+0x20..+0x2F`.
2. Census secondary header and node-domain zero regions without promoting zero observations to padding.
3. After writer-critical preservation contracts are complete, promote the MOD byte-preserving writer and prove exact no-edit round-trip before edited writer authority.

Until a remaining field gains stronger evidence, `PRESERVED_UNDECODED` is the correct writer-safe semantic status.
