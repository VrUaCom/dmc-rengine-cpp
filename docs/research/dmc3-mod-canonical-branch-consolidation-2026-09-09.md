# DMC3 HD MOD — canonical branch consolidation and no-repeat frontier

**Date:** 2026-09-09  
**Canonical working branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Decision

All future `.MOD` reverse work uses **one working branch only**:

```text
reverse/mod-completion-20260907
```

All other MOD-related refs listed below are historical evidence sources, merged integration slices, stale mixed branches, or presentation/infrastructure branches. They are **not independent sources of current truth** and must not be used as starting points for new MOD reverse work.

At the consolidation checkpoint immediately after PR #360 was merged, `main` and `reverse/mod-completion-20260907` were identical at:

```text
913bfc0c06816e1d8b909ebda62f5f13d7855f39
```

No stale branch history was merged wholesale.

## Why this consolidation exists

The repository accumulated many short-lived MOD branches while individual evidence slices were being recovered: skinning, transforms, texture state, object flags, runtime post-load, motion groups, model-family integration, site lessons, and follow-up unknown-field closure.

Several stale branches still report commits `ahead` of current `main` because their PRs were squash-merged, semantically salvaged, or superseded by later canonical work. **Ahead-count alone is not evidence of missing work.** Before any salvage, current canonical blobs, merged PRs and explicit supersession records must be checked.

PR #305 already established this rule for the first large consolidation: salvage evidence-backed deltas only; do not merge stale `model-family*`, `mod-skin*` or world-transform history wholesale.

## Full MOD-related branch audit

### A. Canonical active branch

| Branch | Status | Action |
|---|---|---|
| `reverse/mod-completion-20260907` | **CANONICAL_ACTIVE** | Only branch for new MOD reverse. Keep synchronized with `main`; open bounded PRs from here. |

### B. Fully subsumed / no branch-only MOD work

These refs reported `ahead=0` against the audited current main, or are exact integration ancestors. Do not re-open them for research.

| Branch | Classification |
|---|---|
| `mod-skin` | `FULLY_SUBSUMED` |
| `mod-texture-state` | `FULLY_SUBSUMED` |
| `ada/real-mod-shw-payload-binding-20260901` | `FULLY_SUBSUMED` |
| `agent/source-modification-integration` | `FULLY_SUBSUMED_RELATED_INFRASTRUCTURE` |
| `fix/mod-shared-mesh-fields` | `FULLY_SUBSUMED` |
| `integrate/mod-animation-binding-20260907` | `FULLY_SUBSUMED` |
| `integrate/mod-canonical-consolidation-20260906` | `FULLY_SUBSUMED` |
| `integrate/mod-header-core-20260907` | `FULLY_SUBSUMED` |
| `integrate/mod-mesh-preservation-skin-abi-20260907` | `FULLY_SUBSUMED` |
| `integrate/mod-motion-group-em000-20260907` | `FULLY_SUBSUMED` |
| `integrate/mod-object-core-20260907` | `FULLY_SUBSUMED` |
| `integrate/mod-object-runtime-projection-20260907` | `FULLY_SUBSUMED` |
| `integrate/mod-runtime-postload-20260906` | `FULLY_SUBSUMED` |
| `integrate/mod-skin-palette-20260907` | `FULLY_SUBSUMED` |
| `integrate/mod-texture-binding-20260907` | `FULLY_SUBSUMED` |
| `integrate/model-family-native-reader-v2` | `FULLY_SUBSUMED` |
| `integrate/model-texture-companion-20260907` | `FULLY_SUBSUMED` |
| `integrate/model-texture-runtime-descriptor-20260907` | `FULLY_SUBSUMED` |
| `reverse/mod-header-0x14` | `FULLY_SUBSUMED` |
| `tmp-rebase-mod-default-joint-index` | `FULLY_SUBSUMED_TEMP` |

### C. Stale/squash/mixed refs — apparent ahead commits are not missing canonical MOD work

| Branch | Audit result | Action |
|---|---|---|
| `mod-skin-reverse` | stale transform/skin/world history; evidence-backed delta already salvaged by PR #305 | `SUPERSEDED_DO_NOT_RESEARCH_FROM` |
| `research/mod-world-transform-exe-proof` | ancestor/parallel form of the same transform/skin evidence package | `SUPERSEDED_DO_NOT_RESEARCH_FROM` |
| `research/mod-world-transform-exe-proof-v2` | explicitly identified by PR #305 as subsumed through `mod-skin-reverse` | `SUPERSEDED_DO_NOT_RESEARCH_FROM` |
| `model-family` | mixed SCM/MOD/model-family history, far behind current main; not a safe MOD integration source | `MIXED_STALE_DO_NOT_MERGE` |
| `model-family-reader-hardening` | PR #284 explicitly closed as superseded by canonical main + PR #305 | `SUPERSEDED_DO_NOT_MERGE` |
| `integrate/mod-default-joint-index-20260907` | squash-history commit remains apparent, but canonical research blob is byte-identical in `main` | `PATCH_EQUIVALENT_SUBSUMED` |
| `integrate/mod-motion-group-em000-20260907-v2` | merged motion-group promotion; apparent ahead commit is integration history, not a new research frontier | `PATCH_EQUIVALENT_SUBSUMED` |
| `integrate/model-family-native-reader` | current reported branch-only diff is empty `README.tmp`; no MOD semantic value | `STALE_JUNK_DO_NOT_SALVAGE` |
| `architecture/modular-format-architecture-20260907` | ADR/policy branch; accepted ADR is already in `main` | `RELATED_POLICY_ONLY` |
| `site/mod-native-lessons` | site/build presentation delta only | `RELATED_PRESENTATION_ONLY` |
| `site/mod-lesson-live-acceptance` | Pages/site CI presentation delta only | `RELATED_PRESENTATION_ONLY` |
| `refactor/native-reader-modular-main` | current branch delta is Native Reader/DDS/PTX infrastructure, not MOD ABI/reverse | `RELATED_INFRASTRUCTURE_ONLY` |

## Canonical MOD knowledge already consolidated

The following areas are already represented by current `main` plus the canonical branch and must be treated as **established baselines**, not rediscovery targets:

1. **Physical model shell**
   - `0x40` document header;
   - `0x40` object record;
   - `0x50` mesh record;
   - field-major position/normal/UV/blend-index/packed-weight streams.

2. **Hierarchy and transforms**
   - parent/order node domain;
   - node-indexed `0x20` local transform records;
   - XYZ-radian local transform construction;
   - EXE-backed world propagation;
   - inverse-rest/current-world skin palette.

3. **Skinning/topology**
   - blend-index lane mapping used by skin influences;
   - 5-bit packed weights normalized by 31;
   - topology break state;
   - runtime topology workspace generation;
   - `BLENDINDICES.x` canonical-path non-consumption boundary while preserving the raw byte.

4. **Texture/material path**
   - mesh texture slot;
   - external companion count authority;
   - runtime descriptor relationship;
   - GS CLAMP region-repeat fields;
   - nearest-filter source flag `0x00004000`;
   - MOD mesh `+0x38` inactive in the audited MOD path, with EFM COLOR0 as a separate positive control.

5. **Header/object runtime state**
   - header `+0x13` = MOD default/fallback joint selector;
   - header `+0x14` raw runtime-carried `u32`, old universal decimal semantic rejected for MOD;
   - object alpha/control, source flags, bounding sphere;
   - object `+0x18/+0x1C` live under confirmed source-flag gates, high-level artistic names still open;
   - source flag `0x00100000` technical GS alpha-test-reference/depth-write selector;
   - source flag `0x00200000` carried/restored but still `PRESERVED_UNDECODED` at the terminal semantic layer.

6. **Motion/animation binding**
   - motion-group selector domain;
   - hierarchy + evaluated local pose -> current world -> inverse-rest/current-world palette composition boundary.

7. **Preservation-only secondary state**
   - mesh `+0x0C`, `+0x38`, `+0x4C` writer-preservation contracts;
   - transform `+0x1C` preservation contract;
   - object secondary zero regions `+0x04..07`, `+0x14..17`, `+0x20..2F` remain preservation-only;
   - retained serialized-object pointer at runtime `+0x18` is live; confirmed MOD consumer reaches serialized object `+0x08` mesh table, while the bounded render-command builder stores the pointer but does not dereference it afterward.

## No-Repeat Frontier

The following work is now **forbidden as a standalone research task** unless a contradiction or genuinely new provenance edge appears:

- re-deriving MOD hierarchy/world transforms from `mod-skin-reverse` or world-transform branches;
- re-proving the 0x40/0x40/0x50 shell;
- re-decoding the same skin-weight bit packing;
- repeating the same header `+0x14` decimal decomposition;
- repeating raw `+0x18` or `+0x14` displacement searches without owner/provenance proof;
- repeating the already-audited bit21 GS helper/material-helper paths;
- treating SCM/EFM same-offset fields as MOD consumers without independent provenance;
- repeating zero histograms for already bounded unknown fields without a new corpus family/revision or a writer experiment.

A new reverse pass must satisfy at least one of these conditions:

1. **NEW_CONSUMER** — a previously unclassified provenance-confirmed executable consumer;
2. **NEW_PRODUCER** — a new producer/derivation edge that changes field ownership;
3. **NEW_CORPUS_CLASS** — a genuinely new MOD revision/resource family that contradicts or extends the current 38-MOD corpus;
4. **WRITER_GATE** — evidence required for serialization, mutation, rebuild or reintegration;
5. **GAME_ACCEPTANCE** — original-game behavioral acceptance or rejection;
6. **CONTRADICTION** — evidence that invalidates a currently promoted claim.

“Confirmed the old conclusion again in the same path” is **not** sufficient.

## Real open frontier

Priority order after this consolidation:

1. **Writer/authoring authority** — highest-value gap now.
   - exact no-op parse -> serialize -> byte parity;
   - preservation of every undecoded region;
   - deterministic layout planning/alignment;
   - controlled single-field mutation tests;
   - reopen/reparse validation.

2. **Companion/container reintegration**
   - MOD + texture companion coherence after rebuild;
   - PAC/PNST/NBZ reintegration through canonical GDSpaces/container tooling;
   - slot identity and rollback integrity.

3. **Original `dmc3.exe` acceptance**
   - load/render tests for no-op rebuilt MOD;
   - bounded edited-field tests;
   - failure/rollback receipts.

4. **Remaining provenance-confirmed secondary consumers**
   - continue only from runtime object `+0x18` retained-pointer dataflow/xrefs;
   - do not return to raw global displacement scans;
   - classify node-domain `+0x10..1F` only when raw serialized provenance is established.

5. **Terminal semantics only when they matter**
   - bit `0x00200000`, object `+0x18/+0x1C`, and other preserved fields should receive names only from a new terminal consumer or game experiment; naming them is lower priority than authoring closure.

## Operational rule

For future agents/chats:

```text
START FROM:
  main
  + reverse/mod-completion-20260907
  + current MOD receipts/docs/tests

DO NOT START FROM:
  mod-skin*
  research/mod-world-transform*
  model-family*
  old integrate/mod-* branches
  site/mod-* branches
```

Before opening any new MOD branch, first prove why the existing canonical branch cannot safely contain the work. The default is **no new branch**.
