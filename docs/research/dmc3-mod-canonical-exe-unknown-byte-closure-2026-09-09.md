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
| header `+0x08..0F` | `EXE_CONFIRMED` + `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` | whole-image source-pointer provenance finds zero canonical model-manager reads/writes |
| header `+0x18..1F` | same | whole-image source-pointer provenance finds zero canonical model-manager reads/writes |
| header `+0x28..3F` | same | whole-image source-pointer provenance finds zero canonical model-manager reads/writes |
| node-domain `+0x10..1F` | `EXE_CONFIRMED` + `RESERVED_OBSERVED_ZERO` + `PRESERVED_UNDECODED` | `+0x10` has one canonical dead read; `+0x11..1F` has no model-domain consumer; shell has no runtime effect |

## Writer-preservation contracts

Canonical runtime dormancy/non-use never authorizes zero-normalization.

The preservation obligations include:

- `BLENDINDICES.x` exact source byte;
- transform `+0x1C` exact source bits;
- mesh `+0x0C/+0x38/+0x4C` exact source bytes;
- header `+0x14` exact source `u32`;
- secondary header `+0x08..0F/+0x18..1F/+0x28..3F` exact source bytes;
- node-domain `+0x10..1F` exact source bytes;
- source flags including `0x00100000` and `0x00200000` exactly;
- object secondary regions `+0x04..07`, `+0x14..17`, `+0x20..2F` exactly.

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

Whole-image follow-up adds `0x14029F0B0` to the previously bounded 31 roots, so the canonical count is **32**. Existing recursive typed census still finds zero bit21 semantic decoders.

## Mesh `+0x0C/+0x4C`

Whole-image direct serialized-mesh derivation occurs in exactly six functions. Runtime-mesh derivation occurs in 34 functions; typed propagation reaches 43 funcs / 20 direct calls with zero source-pointer escapes or indirect source calls.

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

`+0x0C` and `+0x4C` have zero reads/writes. `+0x48` is the positive control.

## Object secondary regions — whole-image closure

`0x140302AAA` stores the complete serialized 0x40-byte object pointer at runtime object `+0x18`. Whole-image propagation from all 32 exact `0x380` runtime-object roots reaches 61 typed states / 49 direct pointer-tagged calls.

Only five paths load runtime object `+0x18`; three proceed only through serialized object `+0x08` into the mesh domain and two save the pointer to dead locals. An independent direct-source-header reconstruction census also finds no target-region use.

Therefore object `+0x04..07/+0x14..17/+0x20..2F` are canonically dormant/no-effect while remaining `PRESERVED_UNDECODED`.

## Secondary header shell — final whole-image closure

Detailed receipt:

`data/reverse/dmc3-mod-header-node-shell-closure-20260911.json`

The canonical executable has 12,235 `.pdata` runtime functions. Conservative source-pointer discovery finds:

```text
116 functions with non-stack owner +0x108 -> source-like loads
234 source-like load sites
53 raw header-target candidate functions
138 raw target-range read/write operations
14 direct calls to model-family classifier 0x1402FD650
```

Raw offset equality deliberately over-collects unrelated owners. After model-manager provenance classification:

```text
header +0x08..0F canonical model reads/writes = 0
header +0x18..1F canonical model reads/writes = 0
header +0x28..3F canonical model reads/writes = 0
```

A separate leaf-code pass does not revive a model-domain consumer. The two leaf `+0x108` source-like loads at `0x140165B23/0x140165B46` belong to a nested transform-like owner and are rejected.

The complete secondary header shell is therefore `EXE_CONFIRMED` dormant/no-effect, but still `PRESERVED_UNDECODED` on disk.

## Node-domain secondary shell — final whole-image closure

The binder `0x1402F1DB0` resolves only node-domain relative dwords `+0x00/+0x04/+0x08/+0x0C`.

Whole-image source/node propagation produces one canonical secondary-shell read in layout planner `0x1402FD9C0`:

```text
0x1402FD9F1  manager +0x108 -> source
0x1402FDA00  source +0x20   -> node-domain
0x1402FDA0C  read byte [node +0x10]
0x1402FDA10  store byte -> [rbp+0x18]
```

That local has zero later reads. Therefore `+0x10` is a canonical dead read, not an unread byte. `+0x11..+0x1F` has zero provenance-confirmed model-domain reads/writes, and the raw shell pointer does not escape.

The full node shell is `EXE_CONFIRMED` dormant/no-effect and `PRESERVED_UNDECODED`.

## Direct-EXE phase result

The following direct-EXE gates are closed:

1. `BLENDINDICES.x` canonical CPU/GPU consumption;
2. transform `+0x1C` canonical transform-runtime consumption;
3. mesh `+0x0C` canonical typed runtime consumption;
4. mesh `+0x38` MOD-specific auxiliary-stream role;
5. mesh `+0x4C` canonical typed runtime consumption;
6. header `+0x14 -> manager +0xE4` downstream consumer census;
7. source flag `0x00100000` technical renderer semantic;
8. source flag `0x00200000` canonical typed-runtime consumption;
9. object secondary regions `+0x04..07/+0x14..17/+0x20..2F` runtime-consumption census;
10. secondary header regions `+0x08..0F/+0x18..1F/+0x28..3F` whole-program census;
11. node-domain `+0x10..1F` whole-program census.

**There is no remaining direct-EXE unknown-field consumer gate in the current canonical MOD contract.**

This statement does not invent semantic names for dormant bytes. Their correct serialized status remains `PRESERVED_UNDECODED`.

## Next separate gate

The next phase is not more unknown-byte guessing. It is writer authority:

1. promote a byte-preserving MOD writer;
2. prove exact no-edit parse -> write round-trip;
3. only then permit edited output;
4. verify edited assets in the original canonical `dmc3.exe`.
