# DMC3 HD MOD — current evidence status (2026-09-09)

**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Canonical reviewed main:** `d8534badbbe52cae1610d624822874431f581fa9`

This document is the public-facing synthesis for the current MOD reverse and bounded authoring state. Detailed receipts and C++ modules remain the technical authority.

## Current bounded corpus

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
```

The corpus covers `em000`, independent `pl000` model resources and `id100`. Corpus invariants are not automatically universal format laws.

## Stable structural/runtime core

Canonical evidence covers:

- `0x40` document header, `0x40` object record and `0x50` mesh record;
- position, normal, fixed-point UV, blend-index and packed skin/topology streams;
- texture slot and legacy GS CLAMP state;
- node hierarchy, local/world transforms and inverse-rest/current-world skin palette construction;
- post-load relocation/topology generation;
- external texture-companion/runtime-descriptor relationship;
- source-byte preservation for unresolved serialized state.

Important 2026-09-09 semantic boundaries remain unchanged:

- header `+0x14` is a raw runtime-carried `u32`; the old universal decimal family/model/sub-index interpretation is rejected for MOD;
- `BLENDINDICES.x` remains preserved undecoded even though the audited compiled shader path reads Y/Z/W;
- source flag `0x00100000` is EXE-confirmed as the bounded legacy GS TEST/ZBUF selector, without an invented artistic category;
- source flag `0x00200000` is runtime-carried/restored but its distinct terminal semantic remains preserved undecoded;
- mesh `+0x38` is inactive on the audited MOD path while the homologous EFM slot is live;
- mesh `+0x0C/+0x4C` and transform `+0x1C` remain source-preserved undecoded fields.

## Bounded writer authority

### Preserve-Layout Writer Gate 1 — PR #365

The canonical writer is source-bound and preservation-first. It reparses an immutable source image, starts from original serialized bytes, refuses structural reflow and unauthorized domains, permits only promoted fixed-size edits for object bounds and existing position/normal/UV streams, verifies exact authorized byte spans, reparses output and emits source/output preservation receipts.

This is bounded writer authority, not a rebuild-from-scratch serializer.

### Retail no-op parity — PR #368

```text
files                   38 / 38
source bytes             882,736
parse                    38 / 38
preserve-layout write    38 / 38
exact byte equality      38 / 38
canonical reopen         38 / 38
modified bytes           0
failures                 0
```

### Controlled real-retail edit — PR #369

`em000_021.mod` `object[0].bounding_radius` was changed from `0.6208532452583313` to `0.625`. Serialized span `[124,128)` changed at exactly offsets `[124,125,126]`; every unauthorized byte remained unchanged; disk SHA/reread/reopen and independent raw diff passed.

### Writer receipt -> authored child bridge — PR #369

`ModAuthoredChildBridge` validates the writer receipt before producing the generic `AuthoredChildImage`. Synthetic PAC reintegration/reopen is regression-proven through the existing `NestedRelativeSlotReintegrator`.

### Provenance-bound real retail PNST reintegration — PR #372

The container gate is now stronger than the earlier synthetic-only statement:

```text
retail parent            m20_s00_012.pac
representation           PNST
physical slots           33
target MOD slot           23
target offset             129280
target size               1888
parent size               346272
```

After the controlled MOD edit and same-size reintegration:

- the parent size is unchanged;
- the PNST slot table is unchanged;
- only the three expected authored child bytes change in the complete parent image;
- canonical parent reparse/re-expand returns the exact MOD writer output;
- the requested edited radius survives canonical MOD reopen.

This is a **provenance-bound real retail PNST reintegration receipt**. It is not proof that the original game selected or consumed an authored overlay.

### Synthetic MOD -> container -> NBZ overlay -> reopen — PR #372

The existing `NbzStoreOverlayWriter`, `NBZZipSource` and production overlay path are reused. A generated next-volume NBZ reopens, its root member equals the reintegrated container, and the authored MOD reopens with the requested value.

This is a synthetic product/reopen gate. It is **not** provenance-bound retail NBZ acceptance and not original `dmc3.exe` acceptance.

## Current authoring boundary

Still open before a **full production MOD writer** or “arbitrary safe editing” claim:

- canonical layout synthesis/reflow and typed-IR-only rebuild;
- transform authoring;
- skin/blend-index authoring;
- source-flag/material/texture-binding authoring;
- texture-companion rewriting/coherence;
- broader mutation authority for preserved-undecoded fields;
- provenance-bound retail NBZ overlay acceptance for the authored MOD lineage;
- original `dmc3.exe` no-op rebuilt-MOD acceptance;
- original `dmc3.exe` edited-MOD acceptance;
- complete animation/current-pose ownership and complete TIM2 authoring where those claims are required;
- rollback-backed original-game authoring acceptance.

No public page may claim “full MOD writer”, “100% MOD reverse” or original-game-safe arbitrary editing from the current bounded evidence.

## Primary authorities

```text
docs/research/dmc3-mod-multicorpus-unknown-field-closure-2026-09-08.md
docs/research/dmc3-mod-preserve-layout-writer-gate-2026-09-09.md
docs/research/dmc3-mod-pac-reintegration-gate-2026-09-09.md
docs/research/dmc3-mod-nbz-overlay-reopen-gate-2026-09-09.md
docs/research/dmc3-model-formats-unified-frontier-2026-09-09.md
data/reverse/dmc3-mod-preserve-layout-writer-gate-20260909.json
data/reverse/dmc3-mod-controlled-retail-edit-attestation-20260909.json
data/reverse/dmc3-mod-retail-pnst-reintegration-attestation-20260909.json
data/reverse/dmc3-mod-nbz-overlay-reopen-gate-20260909.json
```
