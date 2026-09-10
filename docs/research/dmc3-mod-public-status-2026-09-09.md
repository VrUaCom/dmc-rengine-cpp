# DMC3 HD MOD — reverse-complete public status

**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Canonical reverse scope:** DMC3 HD Collection `.MOD` model family  
**Reverse status:** **COMPLETE for the canonical DMC3 HD MOD scope**

This document is the public-facing synthesis for the completed MOD reverse and the still-bounded authoring/integration surface.

## What “reverse complete” means here

The MOD reverse program is closed because every serialized/runtime domain needed by the canonical DMC3 HD MOD contract now has a terminal evidence state. A field does not need an invented artistic name to count as closed.

Terminal states include:

- typed structural/runtime semantics where directly proven;
- `PRESERVED_UNDECODED` where the bytes/flow are known but no stronger high-level name is evidenced;
- `RESERVED_OBSERVED_ZERO` where the bounded corpus is zero but universal padding is not proven;
- `REJECTED` for disproven historical interpretations;
- family-sensitive boundaries where MOD, EFM and SCM deliberately do not share one semantic by offset analogy.

Therefore `PRESERVED_UNDECODED` is now a deliberate terminal preservation classification, not an open MOD reverse blocker.

## Canonical corpus baseline

```text
38 unique MOD
166 objects
180 meshes
20,976 vertices
285 transforms
```

The corpus covers `em000`, independent `pl000` model resources and `id100`. Corpus invariants are still not promoted into universal rules without independent evidence.

## Recovered MOD contract

The completed reverse covers:

- `0x40` document header, `0x40` object records and `0x50` mesh records;
- positions, normals, fixed-point UVs, blend indices and packed skin/topology streams;
- object bounds, alpha/runtime flag projection and legacy GS-facing state;
- texture slot selection and legacy GS CLAMP state;
- node hierarchy, parent/order domain and default joint behavior;
- serialized local transforms, world propagation and inverse-rest/current-world skin palette construction;
- MOD post-load relocation and generated topology workspace behavior;
- external texture-companion ownership, runtime texture descriptor relationship and mesh binding validation;
- motion-group / animation-binding boundary into the separate MOT/CMotion authority;
- exact preservation rules for source bytes that do not have a stronger evidenced semantic.

## Closed semantic boundaries that remain intentionally preservation-first

These are not open reverse tasks:

- header `+0x14` is a raw runtime-carried `u32`; the old universal decimal family/model/sub-index interpretation is rejected for MOD;
- `BLENDINDICES.x` remains a preserved serialized ABI lane while the audited compiled shader path consumes Y/Z/W;
- source flag `0x00200000` is proven as runtime-carried/restored state without an evidence-backed artistic/material label;
- mesh `+0x38` is inactive in the audited MOD path while the homologous EFM slot is live;
- mesh `+0x0C/+0x4C` and transform `+0x1C` retain explicit source-preservation contracts rather than being mislabeled as padding.

This is the evidence-safe end state of the reverse, not missing work.

## Writer and reintegration state — separate from reverse completeness

MOD reverse completeness does **not** imply unrestricted writing.

### Preserve-Layout Writer Gate 1

The canonical writer is source-bound and preservation-first. It reparses an immutable source image, starts from original bytes, refuses structural reflow and unauthorized domains, permits only promoted fixed-size edits for object bounds plus existing position/normal/UV streams, verifies authorized byte spans and reparses the output.

### Proven retail no-op parity

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

### Controlled retail edit

`em000_021.mod` `object[0].bounding_radius` was changed from `0.6208532452583313` to `0.625`. Exactly three bytes changed inside serialized span `[124,128)` and all unauthorized bytes remained unchanged.

### Real retail PNST reintegration

The authored MOD child is provenance-bound through retail `m20_s00_012.pac` / PNST physical slot 23. Parent size and slot table remain unchanged; only the expected three child bytes change; canonical reparse/re-expand returns the exact writer output.

### Synthetic NBZ overlay reopen

The authored MOD also survives the existing synthetic MOD -> container -> next-volume NBZ overlay -> reopen chain.

That is product/reopen evidence, not original-runtime selection/consumption evidence.

## What remains open after MOD reverse completion

Remaining MOD-family work is **authoring/integration/acceptance**, not file-format reverse:

- typed-IR-only layout synthesis/reflow;
- transform authoring;
- skin/blend-index authoring;
- source-flag/material/texture-binding authoring;
- texture-companion rewriting/coherence;
- broader mutation authority for preservation-only fields;
- provenance-bound retail NBZ overlay acceptance;
- original `dmc3.exe` acceptance of rebuilt/edited MOD resources;
- rollback-backed original-game authoring acceptance.

Neighboring systems such as full MOT playback, TIM2 authoring and original-runtime resolver selection keep their own authorities and must not be counted as unfinished MOD reverse.

## Public wording rule

Public pages may now say:

- **MOD reverse complete**;
- **DMC3 HD MOD format fully reverse-engineered within the canonical project scope**;
- **canonical MOD reader/runtime contract complete**.

They must still not say:

- “full MOD writer”;
- “arbitrary MOD editing is safe”;
- “edited MOD is original-game accepted”;
- “Capcom authoring-tool equivalent”.

## Primary authorities

```text
docs/research/dmc3-mod-multicorpus-unknown-field-closure-2026-09-08.md
docs/research/dmc3-mod-canonical-exe-unknown-byte-closure-2026-09-09.md
docs/research/dmc3-mod-preserve-layout-writer-gate-2026-09-09.md
docs/research/dmc3-mod-pac-reintegration-gate-2026-09-09.md
docs/research/dmc3-mod-nbz-overlay-reopen-gate-2026-09-09.md
data/reverse/dmc3-mod-preserve-layout-writer-gate-20260909.json
data/reverse/dmc3-mod-controlled-retail-edit-attestation-20260909.json
data/reverse/dmc3-mod-retail-pnst-reintegration-attestation-20260909.json
data/reverse/dmc3-mod-nbz-overlay-reopen-gate-20260909.json
```
