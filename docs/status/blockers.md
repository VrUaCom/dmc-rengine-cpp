# Current Blockers

**Snapshot date:** 2026-08-15  
**Rule:** a bounded slice may be implemented/validated while the major subsystem remains blocked from `COMPLETE`.

Read `completion-and-evidence-policy.md` before interpreting blocker closure.

## P0 — Resource runtime lifecycle equivalence

**Status:** OPEN  
**Tracking:** #55 and recovered-runtime workstreams

Current product-side materialization and selected recovered lifecycle/post-load slices are not the full DMC3 resource runtime.

Still required:

- exact request/source/fallback semantics where unresolved;
- typed post-load dispatcher/factory handoff;
- cache/reuse/ownership semantics;
- state-4 and unload/shutdown behavior;
- room/stage transition retention/release behavior;
- representative game-backed lifecycle ValidationReceipts.

`StageBundle` materialization must not be used as a game-ready/state-3 completion claim.

## P0 — Representative Stage Catalog validation

**Status:** OPEN  
**Tracking:** #4

The active Wave-2 model supports 189 observed descriptors, 193 selectors and 10 group-base pointers, but completion still requires representative real validation across:

- Bank A;
- Bank B;
- shared/repeated resources;
- selector alias/fallback cases where evidenced;
- partial/unresolved cases;
- semantic/gameplay mapping only where separately proven;
- lifecycle linkage to #55.

`st001` is only a fixture and is not an exit gate.

## P0 — Stage Ops vanilla lifecycle/domain completeness

**Status:** OPEN  
**Tracking:** #90 / PR #91

PR #91 provides substantial Stage Ops assembly/operations implementation, but completion still requires:

- complete evidence-backed domain coverage;
- recovered typed-postload/factory/lifecycle bridges;
- representative Stage Catalog receipts;
- deterministic reload/transition validation;
- complete ModViz/Semantic Graph consumption without parallel authority;
- deliberate promotion after current branch-scoped validation.

## P0 — Recovered Game behavioral equivalence

**Status:** OPEN

Selected recovered units compile/pass deterministic tests. That is not enough for original-game equivalence.

Required:

- direct artifact identity/evidence for each reconstructed unit;
- exact ABI/ownership/lifetime boundaries;
- controlled original-vs-reconstruction behavioral comparison;
- correction/rejection handling;
- subsystem-level ValidationReceipts.

## P0/P1 — HITS/collision deeper reconstruction

**Status:** OPEN  
**Tracking:** #25 / PR #85 / Drive Pass 10

The older Pass-8/Pass-9 blocker list that treated the entire top-level `0x14005E7A0 / 0x14005B460 / 0x14005FEC0 / 0x1400601E0` layer as unknown is superseded by later Pass-10 bounded closure/reclassification.

Do not restart those wrapper questions without contradictory direct evidence.

Later Pass-10 also validates/corrects:

- primitive-shape layer separation and several parser->descriptor->runtime mappings;
- common contact normal at metadata `+0x28/+0x2C/+0x30`;
- manager `+0x108` entry table, manager `+0x110` descriptor table, `0x04` entry stride, `u16 @ entry+0x02` descriptor index, `0x50` descriptor stride;
- runtime `+0x118` primitive descriptor vs runtime `+0x20` transform pointer;
- runtime type 0 one-point and type 1 two-endpoint structural representations;
- Stage-CFG PAC provenance for collision tables.

### Concrete current type-5 target

Slice 12 corrects the earlier abstract-blob model. For observed Stage-CFG PAC layouts:

- modern: slot 38 related source block, slot 39 C260 entry table, **slot 40 primitive descriptor table**;
- legacy observed: slot 21 related source block, slot 22 entry table, **slot 23 primitive descriptor table**.

Therefore the next evidence target is not an unbounded type-5/global scan. It is the concrete Stage-CFG primitive descriptor table at modern slot 40 / legacy slot 23 across representative real stage resources, with C8D0 as preferred runtime observation when raw slot data is unavailable.

Remaining HITS/collision blockers:

- identify actually referenced type-5 descriptors and upstream authored/population path from those concrete tables;
- deeper primitive-specific geometry/contact producer/helper reconstruction;
- remaining fourth-component semantics where still unresolved;
- source2 backing resource, ownership, lifetime and live-selection semantics;
- controlled canonical runtime traces vs reconstructed behavior;
- modified-topology game validation;
- original Capcom offline-builder equivalence.

HITS/collision as a whole is NOT COMPLETE.

## P1 — SCM post-load conflict

**Status:** RESEARCH REQUIRED

SCM remains gated until the `mesh +0x28` behavior is reconciled against representative real SCM resources and stale fixed-stride assumptions are explicitly superseded/qualified.

## P1 — Evidence authority consistency

**Status:** OPEN

Same-artifact-ID conflicting metadata is fail-closed in current import paths, but a stronger global normalized-SHA immutable-metadata invariant across distinct artifact IDs remains incomplete.

Canonical executable metadata correction in PR #92 must be deliberately promoted after review.

## P1 — Raw canonical artifact reacquisition

**Status:** OPEN WHEN NEW BYTE-LEVEL CLAIMS ARE REQUIRED

The project has sanitized executable evidence/reconstructed results, but independent fresh byte-level verification requires the canonical raw artifact or a sanitized disassembly/xref packet materialized for the target.

Do not claim a fresh independent re-hash/disassembly when the raw executable was not actually available in that pass.

## P1 — Reverse Core acceptance program

**Status:** OPEN

Generic evidence/reconstruction/claim infrastructure is not complete until bounded subsystem reconstructions can be claimed, compiled, behaviorally compared and promoted with ValidationReceipts while parallel-agent ownership conflicts are prevented.

## P1 — Binary Inspector / EXE Editor product completion

**Status:** OPEN

Strong domain foundations exist, but the complete native interaction/editor/reconstruction workflow is unfinished. Remaining work includes persistent analysis/cache, broad diagnostics/templates, complete EXE bridges, reconstruction editing, behavioral-validation integration and product UI.

## P1 — ModViz / Item / HUD editor completion

**Status:** OPEN

Bounded editor slices exist. Complete scene/model/HUD workflows, representative runtime validation, deterministic export/reintegration and full product UX remain open.

## P1 / long-term — Full decompilation and recompilation

**Status:** OPEN

No fully decompiled, behaviorally equivalent, rebuilt DMC3 executable exists today.

Required progression:

1. evidence-backed recovered unit;
2. isolated compile;
3. behavioral comparison;
4. ValidationReceipt;
5. replacement/rebinding boundary;
6. progressive composite builds;
7. controlled game validation;
8. only then larger recompilation milestones.

## P2 — Production export/release/reintegration

**Status:** OPEN

Working-copy and guarded copied-output foundations exist, but complete container repack/reintegration, release artifact validation, signing/attestation, packaging and public binary release are not complete.

## Current critical path

```text
artifact/evidence integrity
  -> resource runtime lifecycle/factory/cache closure
  -> evidence-backed recovered runtime slices + behavioral receipts
  -> representative Stage Catalog validation
  -> Stage Ops full domain/lifecycle bridge
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

Parallel HITS work continues below the already-closed wrapper layer, starting from concrete Stage-CFG descriptor-table provenance rather than global scanning.
