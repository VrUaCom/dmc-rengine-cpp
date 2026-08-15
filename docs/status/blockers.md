# Current Blockers

**Snapshot date:** 2026-08-15  
**Synchronization cutoff:** after merge PR #98 and creation of PR #99  
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

PR #91 provides substantial Stage Ops assembly/operations implementation. Final current-head branch receipt: head `b0994436457a7ae26e3083a4a13461f50db6e76d`, Actions `31877176748`, Ubuntu 106/106 and Windows 106/106. This removes the old pending-CI wording but does not close Stage Ops completion.

Completion still requires:

- complete evidence-backed domain coverage;
- recovered typed-postload/factory/lifecycle bridges;
- representative Stage Catalog receipts;
- deterministic reload/transition validation;
- complete ModViz/Semantic Graph consumption without parallel authority;
- deliberate promotion after branch-scoped validation.

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
**Tracking:** #25 / PR #85 / PR #96 / PR #97 / Drive Pass 10

The older Pass-8/Pass-9 blocker list that treated the entire top-level `0x14005E7A0 / 0x14005B460 / 0x14005FEC0 / 0x1400601E0` layer as unknown is superseded by later Pass-10 bounded closure/reclassification.

Do not restart those wrapper questions without contradictory direct evidence.

Validated/bounded Pass-10 work now extends through:

- Slice 7 primitive-shape layer separation and parser->descriptor->runtime mappings;
- Slice 8 common contact normal;
- Slice 9 primitive-descriptor ownership and runtime `+0x118` descriptor vs `+0x20` transform separation;
- Slice 10 runtime type 0 one-point and type 1 two-endpoint structural semantics;
- Slice 12 Stage-CFG PAC provenance;
- Slice 13 serialized `0x40 / 0x04 / 0x50` collision-triplet view with independent data-side validation;
- Slice 14 Stage-CFG entry/primitive-descriptor adapter with transform bounds intentionally unavailable;
- Slice 15 referenced Stage-CFG descriptor census tooling.

### Slice 15 real-corpus gate

**Status:** OPEN DATA GATE  
**Tracking:** PR #96

PR #96 is bounded-complete at implementation/evidence-tooling scope. Final evidence/documentation head `a4be42d5ea73c9e120febd8a9b1b0654d5858dbc`; Actions `31886670409`: Ubuntu + Windows success.

Still required:

- representative legal real `room/stXXXcfg.pac` census selected through catalog/resource provenance;
- resource-set/numeric-stage/source provenance on every census receipt;
- no claim of real type-5 presence or absence until the census actually observes it;
- follow referenced raw type-5 descriptors to their authored/population path only after they are directly observed.

### Slice 16 transform-source provenance

**Status:** RESEARCH REQUIRED  
**Tracking:** PR #97

Current facts:

- `entry+0x01` is a transform selector, not primitive type;
- modern Stage-CFG slots 39/40 are entry/primitive-descriptor tables;
- legacy observed 22/23 are entry/primitive-descriptor tables;
- modern slot38 is consumed by `0x1400594B0` and has its own relative-offset structure;
- slot38 is **NOT proven** to be the C740-style `0x40` transform table;
- C8D0 stack arg5 becomes runtime `+0x20` transform pointer;
- `transform_selector_bounds_available() == false` remains a hard freeze.

Required direct evidence:

1. full `0x1400594B0` body + callers + slot38 dataflow;
2. modern Stage-CFG route around C260 callsite `0x14009823F` through downstream builder/C8D0;
3. legacy observed route around `0x1400B6483`;
4. complete C630/C740 caller census classified by manager/source identity;
5. exact producer/base/object, bounds/count and lifecycle of C8D0 stack arg5.

Forbidden until that evidence exists:

- Stage-CFG slot38 transform parser;
- Stage-CFG three-table adapter;
- assumption that C740 support proves Stage-CFG uses C740;
- original runtime transform construction inside GDSpaces.

### Remaining HITS/collision blockers

- deeper primitive-specific geometry/contact producer/helper reconstruction;
- remaining fourth-component semantics where still unresolved;
- source2 backing resource, ownership, lifetime and live-selection semantics;
- controlled canonical runtime traces vs reconstructed behavior;
- modified-topology game validation;
- original Capcom offline-builder equivalence.

HITS/collision as a whole is NOT COMPLETE.

## P1 — PAC production-path real-corpus validation

**Status:** OPEN  
**Tracking:** #3 / PR #99

PR #99 adds the current-generation production-oriented structural `PAC\0` parser into the shared `ContainerDocument` path. It preserves declared slot identity and empty slots, infers extents from the next greater distinct populated offset, preserves duplicate populated offsets as separate slot identities over the same bounded span, and fails closed on malformed structure.

Still required before stronger PAC compatibility claims:

- representative legal real PAC validation across multiple resource families;
- sanitized slot/index/offset/size/hash receipts;
- prove the structural extent policy against real corpus cases;
- keep PAC slot index schema/container-scoped and never global semantic identity;
- integrate/promote only through the canonical GDSpaces container path.

PR #99 does not close PNST/NBZ/AFS, recursive expansion policy, `.lst`, source priority, write/repack/export, issue #3 as a whole, or original runtime equivalence.

## P1 — SCM post-load conflict

**Status:** RESEARCH REQUIRED

SCM remains gated until the `mesh +0x28` behavior is reconciled against representative real SCM resources and stale fixed-stride assumptions are explicitly superseded/qualified.

## P1 — Evidence authority consistency

**Status:** OPEN

Same-artifact-ID conflicting metadata is fail-closed in current import paths, but a stronger global normalized-SHA immutable-metadata invariant across distinct artifact IDs remains incomplete.

Canonical executable metadata correction in PR #92 has final green Ubuntu/Windows validation in Actions `31877266101` but must still be deliberately promoted. Current `main` at the synchronization cutoff includes PR #98, not an implied merge of #92.

## P1 — Raw canonical artifact availability for new reverse

**Status:** OPEN WHEN NEW BYTE-LEVEL CLAIMS ARE REQUIRED

The tooling blocker is reduced: merged PR #98 provides generic exact expected-SHA-gated executable byte-window acquisition with deterministic receipts.

The evidence blocker remains: the legal canonical raw executable must actually be locally available to acquire new windows. A receipt proves artifact/range/byte identity only and cannot turn a guessed discovery window into a full function body.

Do not claim a fresh independent re-hash/disassembly when the raw executable was not actually available in that pass.

## P1 — Reverse Core acceptance program

**Status:** OPEN

Generic evidence/reconstruction/claim infrastructure is not complete until bounded subsystem reconstructions can be claimed, compiled, behaviorally compared and promoted with ValidationReceipts while parallel-agent ownership conflicts are prevented. Merged PR #98 is one infrastructure primitive, not acceptance-program completion.

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
  -> use merged PR #98 acquisition for exact EXE windows when raw artifact is available
  -> resource runtime lifecycle/factory/cache closure
  -> evidence-backed recovered runtime slices + behavioral receipts
  -> representative Stage Catalog validation
  -> Stage Ops full domain/lifecycle bridge
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

Parallel HITS work is now explicitly split into: representative Slice-15 Stage-CFG census and Slice-16 transform-source provenance. PR #99 real-PAC corpus validation proceeds in parallel through the canonical GDSpaces path.
