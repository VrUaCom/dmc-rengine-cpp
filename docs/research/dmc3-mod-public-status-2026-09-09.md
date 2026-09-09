# DMC3 HD MOD — current evidence status (2026-09-09)

**Working branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

This document is the public-facing synthesis for the current MOD reverse. It does not replace the machine receipts or the individual reverse notes; it exists to stop older intermediate hypotheses from leaking into the discovery site and learning material after later evidence has narrowed or rejected them.

## Current bounded corpus

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
```

The corpus is no longer enemy-only. It includes the recursive `em000` population, two independent `pl000` model resources including the cloth-associated MOD, and `id100` HUD/model data.

A corpus observation is still not a universal format law. All writer-facing unknown bytes remain source-preserved unless a separate authoring contract proves otherwise.

## Stable structural/runtime core

The current reverse supports the following evidence-backed model:

- `0x40` document header, `0x40` outer object record and `0x50` inner mesh record;
- position, normal, fixed-point UV, `BLENDINDICES` and packed weight/topology streams;
- texture-slot and legacy GS CLAMP state;
- node-domain parent/order/motion-group/transform organization;
- XYZ-radian local transforms, hierarchical world propagation and inverse-rest/current-world skin palette construction;
- topology generation into the mesh-relative runtime workspace;
- external texture-companion/runtime-descriptor relationship;
- source-byte preservation for unresolved serialized state.

Reader/runtime evidence does **not** automatically promote unrestricted writer/edit support.

## 2026-09-09 closures and corrections

### Header `+0x14`

The raw `u32` transfer into manager `+0xE4` remains `EXE_CONFIRMED`.

The old universal decimal interpretation

```text
family * 100000 + model_set * 100 + sub_index
```

is **REJECTED for MOD** after the expanded corpus exposed values such as `217` and `1000000`. The safe C++/documentation name is `runtime_metadata_u32`; the high-level semantic remains `PRESERVED_UNDECODED`. Do not import SCM `LegacyResourceCode` semantics and do not derive this field from a filename.

Authority: `dmc3-mod-header-14-reverse-2026-09-09.md`.

### `BLENDINDICES.x`

The complete serialized field remains a `u8x4` ABI. Canonical runtime propagation into the draw/input descriptor is confirmed.

Across eight compiled DXBC input signatures that consume `BLENDINDICES`:

```text
Mask          = 0xF
ReadWriteMask = 0xE
```

The compiled shader path therefore reads Y/Z/W and not X. A provenance-aware direct CPU census also found the known Y positive control but no direct X consumer. The bounded corpus has X equal to zero on `20,976 / 20,976` vertices.

This is **not permission to call X padding**. Its semantic label remains `PRESERVED_UNDECODED`, and a writer must preserve the raw source byte.

Authority: `dmc3-mod-blendindices-x-reverse-2026-09-09.md`.

### Source flag `0x00100000`

The technical renderer semantic is closed for the canonical active low-mode path.

```text
bit clear -> TEST_1.AREF = 0,  ZBUF_1.ZMSK = 1
bit set   -> TEST_1.AREF = 16, ZBUF_1.ZMSK = 0
```

Alpha test remains `GREATER`; Z test remains `GEQUAL`.

The chain is traced from serialized object flags through baseline/effective runtime state, the MOD state helper, legacy GS A+D descriptor construction, register-identified `TEST_1` / `ZBUF_1` values and the generic render-command/backend path. The technical semantic is therefore `EXE_CONFIRMED`.

The bit still has **no evidence-backed artistic/material category** such as “cloth”, “transparent” or “alpha material”.

Authority: `dmc3-mod-source-flag-00100000-reverse-2026-09-09.md`.

### Source flag `0x00200000`

The source bit is real retail state and is `EXE_CONFIRMED` as part of the baseline/effective runtime flag words and their restoration logic.

The confirmed local GS packet helper does not interpret this bit. Equal `0x00200000` masks found in manager/object state were rejected as direct consumers where provenance did not connect them to the serialized source bit.

Its distinct terminal semantic remains `PRESERVED_UNDECODED`.

Authority: `dmc3-mod-source-flag-00200000-reverse-2026-09-09.md`.

### Mesh `+0x38`

The same physical slot must not receive one semantic name across every model family.

- canonical MOD path: slot is not forwarded and the corresponding runtime auxiliary stream is disabled;
- bounded MOD corpus: `180 / 180` values are zero;
- homologous EFM path: slot is live and reaches COLOR0-facing runtime state.

Therefore the MOD-specific runtime role is confirmed inactive in the audited path, while the raw serialized slot remains source-preserved. EFM COLOR0 semantics must not be copied into MOD.

Authority: `dmc3-mod-mesh-38-reverse-2026-09-09.md`.

### Mesh `+0x0C` and `+0x4C`

Both physical fields are bounded zero in the current multi-corpus population and lack a positive semantic consumer in the audited MOD paths. Synthetic non-zero preservation regressions exist so they cannot silently become zero-normalized padding.

Current status: `PRESERVED_UNDECODED`.

Authorities: `dmc3-mod-mesh-0c-reverse-2026-09-09.md`, `dmc3-mod-mesh-4c-reverse-2026-09-09.md`.

### Transform `+0x1C`

The bounded corpus reports zero on `285 / 285` MOD transforms, with an additional bound EFM positive comparison population also zero.

The canonical local rotation construction copies the 0x20 record but consumes only serialized X/Y/Z rotation scalars; the fourth scalar is not read by that local-matrix path. This is executable-backed negative evidence, not a global semantic name.

Current status: `PRESERVED_UNDECODED`; writer preserves the source float exactly.

Authority: `dmc3-mod-transform-1c-reverse-2026-09-09.md`.

## Evidence discipline

The current MOD pass deliberately distinguishes:

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

Important rules:

- a zero histogram is not padding proof;
- negative evidence from one runtime path is not proof of global non-use;
- the same serialized offset in MOD/EFM/SCM is not semantic equivalence;
- a reversible arithmetic decomposition is not automatically a semantic field partition;
- a rendered model is not writer acceptance;
- unknown bytes remain an asset to preserve, not noise to normalize.

## Current authoring boundary

The reverse is substantially stronger than the earlier single-corpus state, but production MOD writing is still a separate promotion gate.

Before unrestricted edited MOD output can be advertised, the project still needs evidence for the full authoring chain, including field dependencies, layout planning, mutation constraints, reopen/reparse, container reintegration, original-game acceptance and rollback integrity.

No public page should claim “full MOD writer”, “100% MOD reverse” or original-game-safe arbitrary editing from the reader/reverse evidence alone.

## Primary detailed notes

```text
docs/research/dmc3-mod-multicorpus-unknown-field-closure-2026-09-08.md
docs/research/dmc3-mod-blendindices-x-reverse-2026-09-09.md
docs/research/dmc3-mod-header-14-reverse-2026-09-09.md
docs/research/dmc3-mod-mesh-0c-reverse-2026-09-09.md
docs/research/dmc3-mod-mesh-38-reverse-2026-09-09.md
docs/research/dmc3-mod-mesh-4c-reverse-2026-09-09.md
docs/research/dmc3-mod-transform-1c-reverse-2026-09-09.md
docs/research/dmc3-mod-source-flag-00100000-reverse-2026-09-09.md
docs/research/dmc3-mod-source-flag-00200000-reverse-2026-09-09.md
```
