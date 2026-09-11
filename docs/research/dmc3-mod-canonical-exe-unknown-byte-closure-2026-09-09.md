# DMC3 HD MOD — canonical EXE unknown-byte closure index (2026-09-09)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Architecture:** PE32+ x86-64  
**Bounded MOD corpus:** 38 unique MODs / 166 objects / 180 meshes / 20,976 vertices / 285 transforms  
**Evidence sync follow-up:** 2026-09-11

## Evidence vocabulary and boundary

Only canonical statuses are used:

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

Reader-side materialization is also not serialized-ABI evidence. Declared count, serialized source field/offset, materialized vector and writer-owned output stream remain distinct.

MOD mesh `+0x48` is the separate generated topology count. It is not `align16(6*(vertex_count-2))`; that physical workspace span belongs to the workspace associated with `+0x40`.

## Closure table

| Target | Evidence-safe status | Canonical result |
|---|---|---|
| `BLENDINDICES.x` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | CPU/DXBC runtime non-use closed; all 8 DXBC signatures read Y/Z/W but not X |
| transform `+0x1C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | whole-image transform-runtime behavior closed as dormant/no effect |
| mesh `+0x0C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | six direct source-mesh derivation funcs + 34 runtime-mesh roots; no canonical source `+0x0C` consumer |
| mesh `+0x38` | `EXE_AND_CORPUS_CONFIRMED` | MOD-specific auxiliary stream role closed; EFM homologous slot is COLOR0 |
| mesh `+0x4C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | no canonical `+0x4C` consumer; neighboring `+0x48` remains live positive control |
| header `+0x14` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | copied to manager `+0xE4`; whole recursive downstream census finds no consumer |
| source flag `0x00100000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` | technical GS TEST/ZBUF semantic closed |
| source flag `0x00200000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | carried/restored but uninterpreted in canonical typed runtime surface |
| object `+0x04..07` | `EXE_CONFIRMED` + `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` | whole-image retained/direct-source census closes canonical runtime effect as dormant/no effect |
| object `+0x14..17` | same | dormant/no effect; source bytes still preserved |
| object `+0x20..2F` | same | dormant/no effect; source bytes still preserved |

## Writer-preservation contracts

Canonical runtime dormancy/non-use never authorizes zero-normalization.

The current preservation obligations include:

- `BLENDINDICES.x` exact source byte;
- transform `+0x1C` exact source bits;
- mesh `+0x0C/+0x38/+0x4C` exact source bytes;
- header `+0x14` exact source `u32`;
- source flags including `0x00100000` and `0x00200000` exactly;
- object secondary regions `+0x04..07`, `+0x14..17`, `+0x20..2F` exactly.

`include/dmc_rengine/analysis/mod/secondary_serialized.hpp` now includes a compile-time synthetic non-zero preservation probe for all three object-secondary regions.

## Transform `+0x1C`

Whole-image manager transform fingerprint `+0x20/+0xEA/+0x188` yields the source materializer, MOD/EFM builder, SCM control and two CMotion materializers. No canonical consumer gives serialized `+0x1C` a transform effect. Runtime behavior is `EXE_CONFIRMED` dormant; serialized semantic remains `PRESERVED_UNDECODED`.

## Header `+0x14`

`0x1402F95C2 -> 0x1402F95C5` copies serialized `+0x14` to model manager `+0xE4`. Whole-image raw classification plus accessor-rooted recursive dataflow, wrapper escapes and exact CMotion RTTI/vtable closure find no downstream consumer. The old universal decimal identity formula remains `REJECTED`.

## Source flag `0x00200000` — whole-image root correction

The exact runtime-object fingerprint is:

```text
manager +0xE8 count
manager +0x100 object array
runtime_object = manager +0x100 + index*0x380
```

The earlier model-core pass counted 31 derivation functions. Whole-image follow-up adds `0x14029F0B0`, which calls model-manager accessor `0x140089DE0`, validates `+0xE8` and derives the same `+0x100/index*0x380` object. The canonical count is therefore **32**.

The added root does not read baseline `+0x10` or effective `+0x14`; it follows runtime `+0x18` to serialized object `+0x08` and mesh data. Bit21 closure is unchanged:

```text
32 exact runtime-object roots
61 direct object-passing call sites
39 direct helper targets
45 conservative typed states / 19 propagation edges
0 tagged indirect/vtable calls
0 bit21 semantic decoders
```

## Mesh `+0x0C/+0x4C`

Whole-image direct serialized-mesh derivation:

```text
serialized_mesh = object.mesh_table(+0x08) + mesh_index*0x50
```

occurs in exactly six functions. Runtime builder also stores the live source mesh pointer at `runtime mesh +0x10`.

Whole-image runtime-mesh derivation occurs in 34 functions; typed propagation reaches 43 funcs / 20 direct calls with zero source-pointer escapes or indirect source calls.

Live source-backreference consumers make 21 serialized reads:

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

`+0x0C` and `+0x4C` have zero reads/writes. `+0x48` is a strong positive control because it is generated, forwarded and consumed post-build.

## Object secondary regions — whole-image closure

`0x140302AAA` stores the complete serialized 0x40-byte object pointer at runtime object `+0x18`. The new whole-image census begins from all **32 exact `0x380` runtime-object roots** and propagates pointer provenance through **61 typed states / 49 direct pointer-tagged calls**.

Only five paths load runtime object `+0x18`:

```text
0x14029F0B0 -> 0x14029F0F0 -> serialized object +0x08
0x1402F7D60 -> 0x1402F7DB4 -> serialized object +0x08   (EFM sibling)
0x1402F8000 -> 0x1402F845E -> saved local, never dereferenced
0x1402FE6A0 -> 0x1402FE6F4 -> serialized object +0x08   (MOD)
0x1402FE930 -> 0x1402FED8E -> saved local, never dereferenced
```

The retained serialized pointer has:

```text
pointer escapes         0
indirect-call escapes   0
target-region reads     0
target-region writes    0
```

A second whole-image control looks for direct reconstruction from `manager/source +0x108`, `index*0x40`, `source_header+0x40`. Exactly five functions match. MOD/EFM/shared planning paths consume known fields only; `0x3C0` runtime-owner paths are separate provenance domains and are not promoted as MOD consumers.

Therefore canonical runtime consumption is closed for:

```text
object +0x04..+0x07  -> EXE_CONFIRMED dormant/no effect
object +0x14..+0x17  -> EXE_CONFIRMED dormant/no effect
object +0x20..+0x2F  -> EXE_CONFIRMED dormant/no effect
```

All three remain `PRESERVED_UNDECODED` serialized regions with exact-byte writer preservation. `RESERVED_OBSERVED_ZERO` remains a corpus-only statement.

## Closed direct-EXE gates

The following no longer block the unknown-field phase:

1. `BLENDINDICES.x` canonical CPU/GPU consumption;
2. transform `+0x1C` canonical transform-runtime consumption;
3. mesh `+0x0C` canonical typed runtime consumption;
4. mesh `+0x38` MOD-specific auxiliary-stream role;
5. mesh `+0x4C` canonical typed runtime consumption;
6. header `+0x14 -> manager +0xE4` downstream consumer census;
7. source flag `0x00100000` technical renderer semantic;
8. source flag `0x00200000` canonical typed-runtime consumption;
9. object secondary regions `+0x04..07/+0x14..17/+0x20..2F` canonical runtime-consumption census.

## Remaining direct-EXE gates

1. Whole-program closure for secondary header regions `+0x08..0x0F`, `+0x18..0x1F`, `+0x28..0x3F`.
2. Whole-program closure for node-domain `+0x10..+0x1F`.
3. After writer-critical preservation contracts are complete, promote the MOD byte-preserving writer and prove exact no-edit round-trip before edited writer authority.

Until stronger evidence exists for remaining regions, source preservation remains mandatory.
