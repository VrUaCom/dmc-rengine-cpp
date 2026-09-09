# DMC3 HD MOD — canonical EXE unknown-byte closure index (2026-09-09)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Architecture:** PE32+ x86-64  
**Bounded MOD corpus:** 38 unique MODs / 166 objects / 180 meshes / 20,976 vertices / 285 transforms

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

## Canonical executable verification

The retail executable used for this pass is 6,356,432 bytes and hashes exactly to the canonical SHA-256 above. All direct VAs below refer to that binary.

## Closure table

| Target | Current evidence-safe status | Key direct result | Canonical note |
|---|---|---|---|
| `BLENDINDICES.x` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | audited MOD/EFM skin shaders do not consume X; lane-Y CPU positive control at `0x1402F3D0A`; whole-program lane-X dataflow not fully closed | `dmc3-mod-blendindices-x-reverse-2026-09-09.md` |
| transform `+0x1C` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | rotation helper consumes XYZ only; CMotion transfer copies `+00/+04/+08/+10/+14/+18` then advances by `0x20`, skipping `+0x1C` | `dmc3-mod-transform-1c-reverse-2026-09-09.md` |
| mesh `+0x0C` | `STRUCTURAL_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | raw `u32`; no positive consumer in audited load/build/material paths; no padding promotion | `dmc3-mod-mesh-0c-reverse-2026-09-09.md` |
| mesh `+0x38` | `EXE_AND_CORPUS_CONFIRMED` for MOD-specific runtime role | EFM homologous slot is live COLOR0; canonical MOD does not forward it and disables corresponding runtime auxiliary stream | `dmc3-mod-mesh-38-reverse-2026-09-09.md` |
| mesh `+0x4C` | `STRUCTURAL_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | `+0x48` is generated topology-count positive control; no `+0x4C` companion transfer established | `dmc3-mod-mesh-4c-reverse-2026-09-09.md` |
| header `+0x14` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | `0x1402F95C2 -> 0x1402F95C5` copies raw `u32` to manager `+0xE4`; universal decimal identity hypothesis is `REJECTED` | `dmc3-mod-header-14-reverse-2026-09-09.md` |
| source flag `0x00100000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | `0x140302640` changes runtime packet `+0x08` (`0x5000D`/`0x5010D`) and packet `+0x00` high-state branch | `dmc3-mod-source-flag-00100000-reverse-2026-09-09.md` |
| source flag `0x00200000` | `EXE_CONFIRMED` + `CORPUS_CONFIRMED` + `PRESERVED_UNDECODED` | whole source flag word is copied to runtime object `+0x10/+0x14`; equal-mask manager `+0xE0` and runtime `+0x304` candidates are `REJECTED` as direct consumers | `dmc3-mod-source-flag-00200000-reverse-2026-09-09.md` |

## Writer-preservation contracts now enforced

The current C++ evidence layer deliberately separates serialized bytes from runtime semantics.

- `BLENDINDICES.x` is no longer called reserved/constant.
- transform `+0x1C` is retained byte-for-byte and is not described as padding/alignment.
- mesh `+0x0C/+0x38/+0x4C` have a raw preservation projection.
- synthetic non-zero mesh values (`DEADBEEF`, `0x0123456789ABCDEF`, `A5A55A5A`) are required to survive that projection.
- a synthetic non-zero transform `+0x1C` must survive parsing while remaining outside the currently proven XYZ local-matrix semantic.
- source flag `0x00100000` has an executable-bounded packet projection; source flag `0x00200000` has an exact whole-word carry projection.

None of these contracts authorizes a writer to zero-normalize unresolved source bytes.

## Corrected 0x00200000 domain collision

A historical receipt named `dmc3-mod-source-flag-00200000-chain-20260909.json` actually described **manager `+0xE0` bit `0x00200000`**, raised by serialized source flags `0x00000200/0x00000400`. That filename is now marked `REJECTED` as a source-flag artifact. Its evidence was moved to:

```text
data/reverse/dmc3-mod-manager-bit21-runtime-vector-20260909.json
```

The serialized object source flag `0x00200000` now has its own independent receipt and provenance chain.

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

The unknown-field phase is not yet globally complete. Remaining evidence gates are:

1. follow the `0x00100000` packet changes to the terminal renderer/material/shader interpreter;
2. trace runtime object `+0x10/+0x14` to find a distinct consumer of source bit `0x00200000`, if one exists;
3. complete runtime-stream escape/dataflow closure for `BLENDINDICES.x`;
4. complete a type-aware manager `+0xE4` downstream census for header `+0x14`;
5. complete provenance-aware whole-EXE closure for mesh `+0x0C/+0x4C` outside the known loaders/builders;
6. census the secondary header/object/node zero regions without promoting zero observations to padding.

Until those gates close, `PRESERVED_UNDECODED` remains the correct writer policy where stated above.
