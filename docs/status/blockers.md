# Current Blockers

**Snapshot date:** 2026-08-26  
**Canonical base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Latest L2 tooling promotion:** PR #219  
**Latest L3 raw authority:** `l3-boundary-audit-2026-08-26.md` + `l3-raw-exe-pass-2026-08-26.md`  
**Focused handoff review:** [l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md](../gdspaces/l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md)

The canonical Layer-1 execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md). The cross-layer dependency order is [GDSpaces Master Roadmap](../gdspaces/master-roadmap.md).

## P0 — GDSpaces L1 completion blockers

There is no known mandatory internal product-code blocker for the current representative DMC3-HD L1 acceptance scope.

The remaining P0 gates are evidence executions.

### B-L1-01 — Direct-retail representative provenance

**Status:** EXTERNAL REAL-RETAIL RECEIPT REQUIRED

Run the canonical direct-retail acquisition command against a protected DMC3 installation and preserve:

- protected executable authority;
- observed numbered-volume topology;
- resolver-selected volume/archive/member identity;
- archive SHA/size;
- central-entry metadata;
- materialized SHA/size;
- compression transform and ByteProvenance.

`obj\em000.pac` is a high-value request, not a predeclared archive member. Another representative request is acceptable if it gives a stronger deterministic authoring/consumer receipt.

### B-L1-02 — Exact retail representation classification

**Status:** EXTERNAL EVIDENCE REQUIRED

Classify the exact bytes from B-L1-01. Do not infer retail writer authority from transformed DDS/TM2/runtime evidence alone.

If the representation is outside current supported authoring domains, stop and create a new bounded evidence gate.

### B-L1-03 — Representative real edit/rebuild/rematerialization receipt

**Status:** EXTERNAL VALIDATION REQUIRED

Current product code supports top-level and nested PAC/PNST size-changing authoring, next-volume NBZ creation and canonical rematerialization.

The remaining requirement is one real protected-install receipt:

```text
retail-selected member
 -> supported bounded edit
 -> top-level or nested bottom-up rebuild
 -> byte-exact untouched sibling validation
 -> next-contiguous NBZ
 -> canonical resolver higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

### B-L1-04 — Original DMC3 Level-E consumption + rollback

**Status:** FINAL EXTERNAL ACCEPTANCE REQUIRED

Canonical tracking: issue #209.

The generated exact overlay must be copied into the protected installation under controlled conditions, its SHA verified, consumed through a deterministic original-game path, then removed without changing original retail artifacts.

A crash-free launch alone is insufficient.

### B-L1-05 — Final L1 cross-stack audit

**Status:** OPEN / DEPENDS ON B-L1-01..04

Before `L1 COMPLETE / 100%`:

- real acquisition provenance exists;
- real representation classification exists;
- real edit/rebuild/rematerialization receipt exists;
- original-game consumer observation exists;
- rollback proves original retail immutability;
- exact-head Windows + Ubuntu CI is green;
- #100, #182, #209, code and current documentation agree;
- no unresolved contradiction alters the declared supported L1 scope.

## Supporting L1/L3 handoff reverse gaps — bounded, not automatic L1 blockers

The canonical raw EXE is available and has already closed major L3 state/lifecycle ordering. The remaining seam is not "raw EXE unavailable"; it is a narrower cross-layer handoff/body problem.

### B-X-R1 — `0x1402EF4D0` exact body/callees/load-context semantics

**Status:** STATIC REVERSE BREADTH OPEN

Safe current label: **resource materialization submission/scheduling wrapper**.

Open exact questions:

- which lower byte/materialization path(s) it calls;
- whether it directly uses the higher scheduler ring or delegates to another wrapper;
- first concrete consumer and numeric domain of inherited load-context/mode parameter;
- exact failure return conditions.

### B-X-R2 — L1 byte-materialization -> L3 lifecycle ownership handoff

**Status:** STATIC REVERSE BREADTH OPEN

Raw-L3 evidence now closes the high-level seam:

```text
0x1401B8CA0 representation/materialization mechanics
 -> boolean success
 -> only on success 0x1401B84E0 publishes state1 and schedules 0x1401B8DC0
```

Still open is the exact dependency bridge between lower byte/materialization work and the L3 scheduler lifecycle.

A generic child/outstanding-work **fan-in counter is not evidenced**. Safe wording is **materialization-to-lifecycle completion ordering / dependency barrier**.

### B-X-R3 — Scheduler rollback and running-transport interaction

**Status:** STATIC REVERSE BREADTH OPEN

Known:

- `0x1402EF580` = scheduler enqueue;
- `0x1402EF790` = scheduler worker/callback execution;
- `0x1402EF460` = pending scheduled-entry clear/rollback;
- canonical cancellation writer moves only state1/state2 records to state4 and queues deferred cleanup.

Open:

- exact scheduler-entry matching/clear rules;
- already-executing callback behavior;
- interaction with an already-running FileSlot/ReadRequest/backend operation.

`0x1402EF460` is **not** OS AsyncIO cancellation authority.

### B-X-R4 — Transport failure -> lifecycle acquisition/cancellation mapping

**Status:** STATIC REVERSE BREADTH OPEN

`0x1400335A0` is lower whole-file transfer progress/status. Exact propagation from transfer failure into `0x1401B84E0`/scheduler/cancellation behavior remains to be closed.

### B-X-R5 — `.lst` child-population failure and temporary-buffer cleanup

**Status:** STATIC REVERSE BREADTH OPEN

Grammar/layout/recursive synthesis are strong. Remaining handoff breadth:

- child failure/completion ordering relative to L3 lifecycle advancement;
- exact temporary-list allocation/free identity;
- malformed/truncated/recursion failure cleanup;
- partial parent-buffer lifetime on failure.

No generic child counter is promoted without direct evidence.

Focused acquisition authority: `data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json`.

## Layer 2 evidence blockers

These are L2 closure gates. They are not substitutes for the L1 Level-E acceptance sequence.

### B-L2-01 — Real-retail `0x0E` collision census

**Status:** EXTERNAL ARTIFACT ACCESS REQUIRED

The exact `dmc3-0.nbz` artifact is approximately 960 MB and cannot currently be transferred through the connected Drive channel. No exact central-directory/member-list derivative is available in the connected corpus.

Required evidence is an exact member-name/central-directory surface cryptographically bound to the retail archive, followed by the canonical `0x0E` normalized-key collision census.

### B-L2-02 — Protected-distribution runtime RVA mapping receipt

**Status:** TOOLING MERGED IN #219 / REAL ORIGINAL-PROCESS RECEIPT REQUIRED

Authority split:

- canonical analysis executable: SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution execution candidate: SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent mapping evidence.

Merged #219 provides bounded live main-module acquisition by explicit PID + RVA, protected-image SHA/size gating, metadata-only receipts and a multi-anchor mapping validator. The remaining blocker is a real protected-process packet covering the approved anchors.

### B-L2-03 — Original-process selected-provider identity

**Status:** BLOCKED BY REAL B-L2-02 RECEIPT

After bounded runtime mapping is proven, instrument the mapped resolver path and preserve the original-process request, candidate order and exact selected provider/source/member identity.

### B-L2-04 — Final L2 audit

**Status:** OPEN / DEPENDS ON B-L2-01..03

Layer 2 remains incomplete until real-retail collision evidence, protected-runtime mapping, original-process selected identity, exact-head CI and canonical code/docs agree.

## L3 completion blockers

Canonical raw acquisition is **not** a blocker anymore. Static raw EXE evidence is available.

Still mandatory for L3 completion:

- alias-aware whole-image census of every possible LoadedResource state writer/caller context;
- family-complete ownership census for `+0x08/+0x18/+0x20/+0x28` and stable adjacent fields;
- external typed/factory/dependency failures outside the central best-effort dispatcher;
- SCM `mesh +0x28` reconciliation;
- shared-owner coordination breadth;
- original-process V1–V7 lifecycle receipts;
- cross-build/profile differences;
- final contradiction-free L3 audit.

## Closed former L1 blockers

Do not reopen these absent contradictory direct evidence:

- atomic/no-replace publication — #194;
- artifact-bound archive/member stability — #195;
- direct-retail acquisition implementation — #196;
- raw-DEFLATE artifact-bound regression — #197;
- first-gap retail-read behavior — #198;
- verified immutable NBZ copy rebuild — #199;
- PAC/PNST user-facing size-changing rebuild — #201;
- protected retail product closure orchestration — #208;
- nested PAC/PNST root-to-leaf slot-path authoring — #213;
- NBZ STORE/raw-DEFLATE product materialization;
- PAC/PNST sparse/empty/alias-preserving parsing;
- recursive PAC/PNST expansion;
- ByteProvenance;
- next-volume STORE overlay generation and resolver selection composition.

## Closed former L2 blockers/tooling gates

Do not reopen absent contradictory direct evidence:

- exact type-0 physical-provider post-`0x0C` Win32 final path/open/miss semantics — static reverse closed by #215/#204;
- product physical native-path model + controlled hit/miss/archive→physical fallback receipts — #215;
- protected-runtime RVA mapping acquisition/validation tooling — #219; real process evidence remains separate.

## Explicit superseded reverse shorthand

Do not reintroduce:

- `L1 ends at LoadedResource state1->2`;
- `FileSlot/AsyncIO is wholly L1`;
- `FileSlot/AsyncIO is wholly L3` without distinguishing byte mechanics from request lifecycle;
- `0x1402EF4D0 == packed-file reader/exact-path resolver/final backend open`;
- `0x1401B8DC0 == raw I/O callback`;
- `.lst synchronous temporary load == 0x1402EF920`;
- `materialization fan-in == evidenced generic child/outstanding-work counter`;
- `0x1402EF460 == OS AsyncIO cancellation`;
- `type-0 physical final-open semantics still open` after #215;
- `canonical raw EXE unavailable` for current L3 static work.

## Evidence-gated freezes / non-blockers

- Binary AFS is not inferred from `.afs/` logical namespace strings.
- Historical PACK parsing does not establish original DMC3 PACK runtime authority.
- Capcom offline writer equivalence is not required for DMC Rengine L1 product authoring acceptance.
- Stage Ops, ModViz and unrelated HITS semantics do not count as L1/L3 closure.

## Environment blocker

The connected automation environment does not currently expose all exact **protected-install/original-process** artifacts required to execute the real L1 receipts, retail DMC3 collision census or protected-process runtime mapping here.

This does not mean the canonical analysis EXE is unavailable project-wide: the raw `e454...` EXE has already been reacquired and used by the canonical L3 static audits.

This external execution/corpus limitation must not be hidden by synthetic CI or converted into a weaker completion criterion.
