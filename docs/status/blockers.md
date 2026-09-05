# Current Blockers

**Snapshot date:** 2026-09-05  
**Canonical base reviewed:** `main@ee08b388cbc5448a0e1a5d02231d9aaf7e01587d`  
**Active proof/integration slice:** PR #287  
**Completion rule:** original-DMC3 claims require the authority appropriate to the claim; synthetic CI alone is never original-game equivalence.

The current proof execution order is [DMC Rengine Roadmap](../roadmap.md) plus [GDSpaces Proof Roadmap](../gdspaces/proof-roadmap-2026-09-05.md).

## P0 — GDSpaces L1 completion blockers

### B-L1-01 — Direct-retail representative provenance

**Status:** ❌ TRUSTED ORIGINAL/PROTECTED-PROCESS RECEIPT REQUIRED

Product acquisition tooling exists, but it does not prove what the original protected process selected. A valid receipt must preserve:

- protected executable authority;
- observed successful mount topology;
- resolver-selected provider/volume/archive/member identity;
- archive SHA/size;
- central-entry metadata;
- materialized SHA/size;
- transform and ByteProvenance;
- trusted observer identity and zero-loss trace state where runtime observation is used.

`obj\em000.pac` remains a high-value request, not a predeclared archive member.

### B-L1-02 — Exact retail representation classification

**Status:** ❌ DEPENDS ON B-L1-01

Classify the exact selected bytes. Do not infer writer authority from transformed DDS/TM2/runtime evidence or a filename alone. Stop if the real representation lies outside an evidenced writer domain.

### B-L1-03 — Representative real edit/rebuild/rematerialization receipt

**Status:** ❌ REAL PROTECTED-INSTALL RECEIPT REQUIRED

Current product code supports top-level/nested PAC/PNST authoring, next-volume NBZ creation and canonical product rematerialization. Acceptance still requires one same-lineage real receipt:

```text
original-selected member
 -> supported bounded edit
 -> bottom-up rebuild
 -> untouched sibling validation
 -> next-contiguous authored NBZ
 -> original resolver selects authored higher volume
 -> exact rebuilt member bytes are materialized
 -> exact authored child is reached
```

### B-L1-04 — Original DMC3 Level-E consumption + rollback

**Status:** ❌ FINAL EXTERNAL ACCEPTANCE REQUIRED

Canonical tracking: issue #209.

A generated overlay must be SHA-verified, selected by the original runtime through a deterministic path, produce an observable effect attributable to the authored bytes, and then be removed while original retail artifacts remain byte-identical. A crash-free launch is insufficient.

### B-L1-05 — Final L1 cross-stack audit

**Status:** ❌ OPEN / DEPENDS ON B-L1-01..04

Before `L1 COMPLETE / 100%`:

- trusted original selection provenance exists;
- representation classification is explicit;
- real edit/rebuild/rematerialization receipt exists;
- original-game consumer observation exists;
- rollback proves retail immutability;
- exact-head Windows + Ubuntu CI is green;
- code/docs/issues/evidence agree;
- no unresolved contradiction changes the claimed scope.

## Layer 2 blockers

### B-L2-01 — Retail `0x0E` collision census

**Status:** ✅ CLOSED FOR EXACT `dmc3-0.nbz` / ❌ WIDER SCOPE OPEN

Bound receipt:

- archive SHA-256 `2c2302cef5251d9a2499be728d81427e9689d0b9c3ceaeef10d9786260fd13df`;
- central surface SHA-256 `0616683ed1280e80421b5680725d258fe78e41f939ba994a433eadc0f99650af`;
- files-only: 4,333 keys / 4,333 unique / 0 collisions;
- all central entries: 4,334 / 4,334 unique / 0 collisions.

Receipt: `data/reverse/dmc3-nbz-archive-key-census-20260903.json`.

Every additional volume in a wider resolver claim still requires its own census, plus cross-volume normalized-key analysis.

### B-L2-02 — Discovery vs successful mount topology product correction

**Status:** ⚠️ REVERSE CLOSED / IMPLEMENTED ON PR #287 / PROMOTION PENDING

Canonical reverse proves:

```text
filename discovery / registration attempt
!=
successful linked runtime mount
```

PR #287 semantically ports this onto current-main architecture:

- discovery carries no success claim;
- successful topology contains only explicitly successful linked providers;
- sparse successful archive registration is representable;
- resolver traverses only the successful topology;
- discovered-but-failed archives are absent, not synthetic misses;
- failed physical registration yields no physical provider probe;
- product acquisition receipt explicitly says original-process topology is not proven.

Promotion requires final exact-head Ubuntu + Windows CI and review.

### B-L2-03 — Real protected-distribution runtime RVA mapping receipt

**Status:** ❌ TOOLING INTEGRATED BY #219 / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Authority split:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution execution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

Canonical analysis VAs/RVAs may not be applied to the protected process without independent mapping evidence. Required real packet: `OpenGameResource` plus at least two approved type-0 anchors from one exact process/module session.

### B-L2-04 — Trusted original-process selected-provider identity

**Status:** ❌ #221 TOOLING INTEGRATED / BLOCKED BY REAL B-L2-03 + TRUSTED PUBLISHER

The content-candidate/normalizer/artifact binder does not make editable JSON original evidence. A real R3 promotion requires:

1. valid real protected-process mapping;
2. observer/publisher attached to that exact process without changing selection semantics;
3. `trace_complete=true` and `dropped_event_count=0`;
4. exact observer artifact SHA;
5. exact mounted NBZ artifact SHA/size binding;
6. trusted capture origin that cannot be asserted by editable content.

Archive lookup hit followed by wrapper/open failure at `0x140328290` remains a terminal provider/backend failure, not a clean lower-volume miss.

### B-L2-05 — Direct-retail original resolver identity receipt

**Status:** ❌ MEMBER SURFACE UNBLOCKED FOR `dmc3-0.nbz` / ORIGINAL OBSERVATION REQUIRED

The bound `dmc3-0.nbz` key surface is clean. What remains is the original resolver winner itself: selected provider/volume/member from a trusted protected-process observation.

### B-L2-06 — Final L2 audit

**Status:** ❌ OPEN

Requires the claimed collision scope, promoted successful-mount topology product model, real protected-runtime mapping, trusted selected identity, exact-head CI and reconciled code/docs/evidence.

## Layer 3 blockers

### B-L3-01 — Current-main R1 contradiction-gated closure

**Status:** ⚠️ RESEARCH CONCLUSION EXISTS / CANONICAL PROMOTION OPEN

Historical/reconciliation research concluded the bounded LoadedResource state-writer census can be contradiction-gated closed. It must be rechecked against current main and promoted semantically before the canonical roadmap receives a green closure mark.

### B-L3-02 — R2 family/backing ownership

**Status:** ❌ OPEN

Close family/group ownership and lifecycle ordering for `+0x08`, `+0x10` where applicable, `+0x18`, `+0x20`, `+0x28` and stable adjacent fields without conflating SCM mesh `+0x28` with LoadedResource fields.

### B-L3-03 — Materialization scheduler terminal dependency

**Status:** ❌ RAW CANONICAL PASS REQUIRED

The bounded seam already proves materialization-dispatch success precedes state1 and normal callback `0x1401B8DC0` publishes state2. The still-open question is the exact lower scheduler/transport condition that allows or suppresses that normal completion after failed/incomplete transport.

Fresh raw targets:

- `0x1402EF4D0`;
- `0x1402EF790`;
- `0x1400333E0`;
- `0x140033390`;
- `0x1400335A0`;
- `0x1402EF460`;
- regression anchor `0x1401B8DC0`.

Do not promote historical helper labels without fresh canonical bytes.

### B-L3-04 — V1–V7 original-process receipts

**Status:** ❌ OPEN

Required dynamic breadth remains initial load, transition, reload, full reset/menu, in-flight cancellation, shutdown and family/build aggregation.

### B-L3-05 — Final L3 audit

**Status:** ❌ OPEN

Depends on static promotion/ownership closure plus accepted original-process lifecycle receipts.

## Closed product/reverse foundations — do not reopen absent contradiction

- ➖ atomic/no-replace publication — product safety policy;
- ➖ artifact-bound SHA / ByteProvenance — product evidence policy;
- ✅ numbered-volume / first-gap bootstrap bounded behavior;
- ✅ six-prefix `OpenGameResource` bounded direct-call policy;
- ✅ archive `0x0E` / physical `0x0C` normalization;
- ✅ type-0 physical final-open/miss bounded contract;
- ✅ PAC/PNST typed traversal and PAC slot-0 traversal;
- ✅ LoadedResource state1-after-materialization-success;
- ✅ normal `1 -> 2`, typed post-load -> callback -> state3 bounded path;
- ✅ cancellation `1|2 -> 4` and quiescence `{0,3}` bounded rules;
- ✅ `dmc3-0.nbz` zero-collision receipt for that exact artifact;
- ✅ `.index` rejected as recovered original resolver/materialization authority on the canonical path; packed/`.lst` selection is EXE-confirmed.

## Bounded reverse gaps — activate only when the claim requires them

- complete ZIP stream initializer `0x140328540` body/lifetime;
- complete compressed seek/reset/reinflate `0x140328FE0` behavior;
- exhaustive malformed/partial-read original error equivalence;
- dynamic `.lst` allocation/free/error/cycle behavior and real `.lst` corpus breadth.

## Evidence-gated freezes / non-blockers

- Binary AFS is not inferred from `.afs/` namespace strings.
- Historical PACK parsing does not establish original DMC3 PACK runtime authority.
- Capcom offline writer equivalence is not required for DMC Rengine authoring acceptance.
- Stage Ops/ModViz and unrelated format progress do not substitute for L1/L2/L3 proof gates.

## Current access boundary

The connected environment currently lacks the trusted protected-process/install observations required for B-L1-01..04 and B-L2-03..05. During the 2026-09-05 proof pass the exact raw canonical `e454...` executable blob was also not located in the connected Library, so B-L3-03 is intentionally left open rather than filled from prior hypotheses.

The `dmc3-0.nbz` collision census is already closed for its exact bound artifact and must not be listed as an access blocker anymore.
