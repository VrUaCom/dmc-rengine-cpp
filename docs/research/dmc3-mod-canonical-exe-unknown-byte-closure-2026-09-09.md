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

Older labels such as `EXE_CONFIRMED_UNUSED`, `MULTI_CORPUS_OBSERVED_ZERO`, `ALIGNMENT_OR_RESERVED_CANDIDATE`, or compound ad-hoc status strings are superseded and must not be used as authority.

Zero in the corpus is never padding proof. Blender/importer behavior is not format authority. Same offset across MOD/EFM/SCM is not semantic proof.

## Canonical serialized/materialized boundary

Reader-side state is not serialized-ABI evidence.

The following must remain distinct in parser, analysis and writer documentation:

```text
declared element count
serialized stream offset / serialized field
materialized reader vector
writer-owned output stream
```

A reader may resize a vector from a declared count before proving that a valid serialized source stream exists. Therefore `vector.empty()` / `vector.size()` behavior cannot establish presence or absence of a serialized stream.

Likewise, MOD mesh `+0x48` is the separate serialized `generated_topology_count` field. It is not the physical topology-workspace capacity. The current physical workspace span follows the independently observed `align16(6*(vertex_count-2))` rule; that rule belongs to the workspace associated with `+0x40`, while serialized `+0x48` remains separately observed as zero in the current MOD corpus.

Android/native-reader regression results are build/regression evidence only and are not semantic proof of the file format.

## Canonical executable verification

The retail executable used for the direct reverse pass is 6,356,432 bytes and hashes exactly to the canonical SHA-256 above. All direct VAs below refer to that binary.

## Closure table

| Target | Current evidence-safe status | Key direct result | Canonical note |
|---|---|---|---|
| `BLENDINDICES.x` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | canonical runtime non-use closed: serialized `+0x28` -> runtime `+0x140` -> draw descriptor `+0x30`; all 8 compiled DXBC BLENDINDICES signatures have `Mask=0xF`, `ReadWriteMask=0xE`, so Y/Z/W are read and X is not; CPU census has no X consumer | `dmc3-mod-blendindices-x-reverse-2026-09-09.md` |
| transform `+0x1C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | `0x1402FA080` physically transfers the fourth rotation lane; `0x140330450` consumes XYZ only; CMotion path skips the fourth scalar; whole-EXE semantic closure still remains | `dmc3-mod-transform-1c-reverse-2026-09-09.md` |
| mesh `+0x0C` | `STRUCTURAL_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | raw `u32`; no positive consumer in audited load/build/material paths; no padding promotion | `dmc3-mod-mesh-0c-reverse-2026-09-09.md` |
| mesh `+0x38` | `EXE_AND_CORPUS_CONFIRMED` for MOD-specific runtime role | EFM homologous slot is live COLOR0; canonical MOD does not forward it and disables corresponding runtime auxiliary stream | `dmc3-mod-mesh-38-reverse-2026-09-09.md` |
| mesh `+0x4C` | `STRUCTURAL_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | `+0x48` is a separate generated-topology-count positive control; no `+0x4C` companion transfer established | `dmc3-mod-mesh-4c-reverse-2026-09-09.md` |
| header `+0x14` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | `0x1402F95C2 -> 0x1402F95C5` copies raw `u32` to manager `+0xE4`; old decimal identity hypothesis rejected; typed parent `+0x80/+0x50` accessors plus 67 unique first-hop helpers yield zero provenance-confirmed manager-`+0xE4` reads; recursive forwarding remains open | `dmc3-mod-header-14-reverse-2026-09-09.md` |
| source flag `0x00100000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED`; artistic category `PRESERVED_UNDECODED` | technical semantic closed: selects GS `TEST_1.AREF` 0 vs 16 and `ZBUF_1.ZMSK` 1 vs 0, then reaches per-mesh A+D descriptors and generic backend | `dmc3-mod-source-flag-00100000-reverse-2026-09-09.md` |
| source flag `0x00200000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | whole source flag word is copied to runtime object `+0x10/+0x14`; audited GS/material helpers do not interpret bit21; equal-mask manager `+0xE0` and runtime `+0x304` candidates are `REJECTED` as direct consumers | `dmc3-mod-source-flag-00200000-reverse-2026-09-09.md` |

## Writer-preservation contracts now enforced

The current C++ evidence layer deliberately separates serialized bytes from runtime semantics.

- `BLENDINDICES.x` is not called padding/reserved and must be preserved exactly even though canonical CPU/DXBC consumption is closed as unused.
- transform `+0x1C` is retained byte-for-byte and is not described as padding/alignment.
- mesh `+0x0C/+0x38/+0x4C` have a raw preservation projection.
- synthetic non-zero mesh values (`DEADBEEF`, `0x0123456789ABCDEF`, `A5A55A5A`) are required to survive that projection.
- a synthetic non-zero transform `+0x1C` must survive parsing while remaining outside the currently proven XYZ local-matrix semantic.
- source flag `0x00100000` has an executable-confirmed technical GS-state projection; the source bit itself remains preserved.
- source flag `0x00200000` has an exact whole-word carry projection and remains semantically unnamed.

None of these contracts authorizes a writer to zero-normalize unresolved source bytes.

## Corrected `0x00200000` domain collision

A historical receipt named `dmc3-mod-source-flag-00200000-chain-20260909.json` actually described **manager `+0xE0` bit `0x00200000`**, raised by serialized source flags `0x00000200/0x00000400`. That filename is marked `REJECTED` as a source-flag artifact. Its evidence lives in:

```text
data/reverse/dmc3-mod-manager-bit21-runtime-vector-20260909.json
```

The serialized object source flag `0x00200000` has its own independent receipt and provenance chain.

## Header `+0x14` typed-escape follow-up

The 2026-09-10 direct-EXE pass identified two owner-to-manager accessors:

```text
0x140089DE0 -> parent +0x80   (2695 whole-EXE calls)
0x140089DF0 -> parent +0x50   (131 whole-EXE calls)
```

Immediate uses of their returned pointers contain zero direct manager `+0xE4` reads. Across 67 unique first-hop helpers, the only raw `+0xE4` operand occurs in `0x14030F850`; provenance reconstruction proves that `0x14030F8AE` writes `+0xE4` on a separate first-argument CMotion destination while the manager is the second argument in `RDX/R15`. That hit is therefore `REJECTED` as a manager-metadata consumer.

This closes the immediate and first-hop typed census, not recursive whole-program forwarding. Header `+0x14` therefore remains `PRESERVED_UNDECODED`.

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

The aggregate machine index is:

```text
data/reverse/dmc3-mod-canonical-exe-unknown-byte-closure-20260909.json
```

## Remaining direct-EXE gates

The unknown-field phase is not globally complete. `BLENDINDICES.x` canonical runtime consumption and source flag `0x00100000` technical renderer semantics are no longer blockers. Remaining evidence gates are:

1. complete provenance-aware whole-EXE closure for transform `+0x1C` outside `0x1402FA080 -> 0x140330450` and `0x14030F850`;
2. recursively follow typed manager escapes beyond the 67 first-hop helpers and close header `+0x14 -> manager +0xE4` downstream use;
3. find a distinct terminal semantic consumer of source bit `0x00200000`, if one exists outside the audited object-state/material paths;
4. complete provenance-aware whole-EXE closure for mesh `+0x0C/+0x4C` outside known loaders/builders;
5. classify retained-pointer consumers for secondary object regions `+0x04..+0x07`, `+0x14..+0x17`, `+0x20..+0x2F`;
6. census secondary header and node-domain zero regions without promoting zero observations to padding;
7. only after writer-critical preservation contracts are complete, promote the MOD byte-preserving writer and prove exact no-edit round-trip before edited writer authority.

Until those gates close, `PRESERVED_UNDECODED` remains the correct writer policy where stated above.
