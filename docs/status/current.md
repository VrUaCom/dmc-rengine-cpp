# Current Project Status

**Snapshot date:** 2026-08-15  
**Repository:** `VrUaCom/dmc-rengine-cpp`  
**Merged product baseline:** `main` at `25bd70092503cc6ca3be51f05582dcd33af1523d` (merge PR #98)  
**Synchronization cutoff:** after merge PR #98 and creation of PR #99  
**Overall completion status:** **NO MAJOR END-TO-END SUBSYSTEM IS COMPLETE**

Read `completion-and-evidence-policy.md` before interpreting any status below.

## Executive truth

DMC Rengine contains a large amount of real, compiled/tested implementation plus substantial reverse evidence. That does **not** mean the major systems are finished.

> Individual parsers, ABIs, runtime slices, writers, workspace flows and integration branches may be implemented/tested/EXE-confirmed/bounded-closed/validated. The containing major subsystem remains incomplete until its evidence, integration, representative runtime validation, lifecycle behavior and ValidationReceipt gates are satisfied.

A green CI job proves the tested branch/build contract only. It does not prove original DMC3 behavioral equivalence.

## Truth layers

### GitHub `main`

`main` is merged product implementation truth. It currently includes PR #98 generic executable byte-window acquisition and still lags active Stage/GDSpaces/recovered-runtime/HITS research and integration stacks.

PR #98 adds exact expected-SHA-gated read-only PE VA byte-window acquisition through the canonical resource path, deterministic metadata receipts, fail-closed VA/RVA/file-backed mapping/range validation and optional explicit local-only hex. An acquisition receipt proves artifact/range/byte identity only; it does not prove function boundary, ABI, ownership, gameplay/runtime semantics, recovered-source correctness or behavioral equivalence.

Old `main` files that still reference a `st001`-centric compatibility path are not current Stage architecture authority.

### Active implementation/reverse stacks

Important draft branches/PRs include:

- PR #74 — active DMC3 Stage descriptor/resource materialization stack;
- PR #85 — HITS Pass 10 evidence/reverse base stack through validated Slice 14;
- PR #89 — executable Recovered Game Source Tree Wave-3 slices;
- PR #91 — Stage Ops scene assembly/operations authority;
- PR #92 — canonical executable SHA/size metadata correction;
- PR #93 — HITS classifier/format-authority correction;
- PR #94 — completion/evidence/ownership documentation-governance reconciliation;
- PR #96 — HITS Pass-10 Slice 15 referenced Stage-CFG descriptor census, bounded-complete at implementation/evidence-tooling scope with green Windows/Ubuntu CI;
- PR #97 — HITS Pass-10 Slice 16 Stage-CFG transform-source provenance, `RESEARCH REQUIRED`;
- PR #99 — production-oriented read-only PAC structural parser on current `main`.

PR #95 is closed unmerged because its temporary Slice-13 identity collided with already-existing Slice 13/14 numbering; the retained implementation continues as Slice 15 in #96. PR #98 is already merged and is therefore `main` truth, not active-PR truth.

These open PRs are branch-scoped truth until deliberately promoted. They must not be described as merged-main completion.

## Current subsystem status

### GDSpaces — NOT COMPLETE

Implemented/tested slices include resource identities, source registry, lookup policies, provenance, container expansion, Stage resource planning/materialization, WorkingCopy and branch-scoped runtime-resolution work.

PR #99 now adds the current-generation production-oriented structural `PAC\0` parser path into the shared `ContainerDocument` / `ContainerEntry` contracts. The parser preserves exact declared slot space and empty slots, infers populated extents from the next greater distinct populated offset, preserves duplicate non-zero slot identities without inventing semantic aliases, and fails closed on malformed/truncated/out-of-range structure.

PR #99 does **not** establish full game-validated PAC compatibility and does not complete issue #3. Representative legal real-PAC validation across multiple resource families remains required. PNST/NBZ/AFS, recursive expansion policy on this new generation, `.lst` behavior, source priority, write/repack/export and original-runtime equivalence remain separate gates.

Still open for original-game runtime equivalence:

- full request -> lookup -> bytes -> typed post-load -> factory -> cache -> consumer -> unload chain;
- duplicate/source-priority semantics where unresolved;
- complete `.lst` grammar/recursion/ownership behavior;
- original factory/cache/lifetime/transition/shutdown behavior;
- representative game-backed ValidationReceipts.

A materialized `StageBundle` is not an original DMC3 state-3/game-ready object.

### Stage Catalog / identity — STRONG PARTIAL, NOT COMPLETE

Current Wave-2 authority distinguishes:

- Bank A: 110 observed descriptors;
- Bank B: 79 observed descriptors;
- total: 189 observed descriptors;
- separate 193-entry selector space;
- separate 10-pointer group-base table;
- numeric selection via `stageId / 100` and `stageId % 100` indirection.

Keep separate:

1. `resource_set_id / catalog_entry_id`;
2. `numeric_stage_id`;
3. semantic/gameplay Stage identity only when separately evidenced.

`st001` is a regression/compatibility fixture only. 189 descriptors are not automatically 189 gameplay stages.

Remaining gates include representative Bank-A/Bank-B/shared/alias/partial validation, semantic mapping, selector fallback behavior, `.lst` behavior and lifecycle linkage.

### Stage Ops — IMPLEMENTED ON ACTIVE BRANCH, NOT COMPLETE

PR #91 contains substantial product-side assembly/operations implementation, including Stage assembly workspace, ingress, operations, shared WorkingCopy/parser lineage, domain projections, Semantic Graph and ModViz projections.

Final current-head branch receipt: head `b0994436457a7ae26e3083a4a13461f50db6e76d`, Actions `31877176748`, Ubuntu 106/106 and Windows 106/106. This proves the tested PR #91 implementation scope only. Stage Ops still lacks complete domain/runtime coverage and vanilla lifecycle equivalence. It is not merged-main completion and must not be called game-ready.

### Stage Semantic Graph — IMPLEMENTED PROJECTION SLICES, NOT COMPLETE

Semantic Graph is a derived representation/index over Stage Ops state. It does not own resource resolution or scene assembly. Current projections do not prove complete scene semantics or original runtime behavior.

### ModViz — PARTIAL, NOT COMPLETE

ModViz consumes Stage Ops state in the target architecture. Scene/HUD/editor slices exist, but complete scene/asset/HUD editing, validated export and original-game behavioral equivalence are open.

### Recovered Game Source Tree — COMPILES IN SELECTED SLICES, NOT COMPLETE

Selected recovered DMC3 runtime units compile and execute deterministic tests in active branches. Recovered C++ is an evidence-backed executable specification, not automatically original Capcom source.

Still required:

- broader direct instruction/ABI coverage;
- exact ownership/lifetime semantics;
- controlled original-vs-reconstruction behavioral comparison;
- subsystem-level ValidationReceipts;
- eventual integration/rebinding milestones.

### Reverse Core — FOUNDATION/DIRECTION ACTIVE, NOT COMPLETE

Reverse Core owns generic evidence/reconstruction/claim/validation infrastructure, not DMC3 gameplay code. Its acceptance gate requires bounded reconstructed subsystems with compile + behavioral comparison + ValidationReceipt, plus robust multi-agent claim/evidence workflows.

Merged PR #98 is a concrete Reverse Core/EXE acquisition primitive, not Reverse Core completion.

### HITS / collision — STRONG PARTIAL, NOT COMPLETE

Merged/product and active research contain substantial HITS work: header-driven format parsing, exact `0x38` records, spatial reconstruction, deterministic DMC Rengine writer, source0/source1 ownership evidence, runtime query evidence and deep Pass-10 reconstruction.

Important correction: do **not** repeat the older statement that the top-level P0 wrapper set remains open. PR #85 / Drive Pass 10 records bounded closure/reclassification for:

- `0x14005E7A0` combined point-query wrapper ABI/precedence;
- `0x14005FEC0` and `0x1400601E0` top-level contracts;
- `0x14005B460` reclassified into the dynamic-world update pipeline rather than an `E7A0` candidate producer.

Later Pass-10 slices additionally validate/correct:

- parser-source shape / descriptor type / runtime primitive type are distinct layers;
- sphere `0->2->2`, box `1->3->3`, cylinder `2->6->6`, capsule `3->4->4` mappings for the evidenced path;
- runtime type 4 also has a swept-sphere/moving-sphere origin;
- type 5 is structurally a three-vertex face representation while authored vocabulary remains unresolved;
- common contact normal at metadata `+0x28/+0x2C/+0x30`;
- manager `+0x108` entry table and `+0x110` primitive descriptor table;
- entry stride `0x04`, descriptor index `u16 @ entry+0x02`, descriptor stride `0x50`;
- runtime `+0x118` primitive descriptor and runtime `+0x20` transform pointer;
- runtime type 0 one-point and type 1 two-endpoint structural representations;
- Slice 13 serialized `0x40 / 0x04 / 0x50` collision-triplet view, including independent `em000.pac` data-side validation and the negative control that slot numbers are not global schemas;
- Slice 14 Stage-CFG entry/primitive-descriptor view using modern slots 39/40 and legacy observed 22/23 while transform-selector bounds remain unavailable;
- Slice 15 referenced Stage-CFG descriptor census tooling, which counts only descriptors actually referenced by bounded entry rows and keeps raw type semantics bounded.

#### Latest validated HITS slice — Slice 15

PR #96 is bounded-complete at implementation/evidence-tooling scope. Final evidence/documentation head: `a4be42d5ea73c9e120febd8a9b1b0654d5858dbc`; Actions `31886670409`: Ubuntu and Windows success, failed jobs 0.

The real-corpus state remains open: current accessible sources do not establish a representative referenced-descriptor census for real `room/stXXXcfg.pac`, so neither real type-5 presence nor real type-5 absence is claimed.

#### Active HITS reverse frontier — Slice 16

PR #97 is `RESEARCH REQUIRED`. The current question is the provenance of the raw dynamic-collision transform selector `entry+0x01` and the exact transform pointer passed as C8D0 stack arg5 and stored at runtime `+0x20`.

Current direct-evidence boundaries:

- modern Stage-CFG slots 39/40 remain the entry/primitive-descriptor tables;
- legacy observed slots 22/23 remain the entry/primitive-descriptor tables;
- modern slot38 is consumed by `0x1400594B0` and has its own relative-offset structure;
- slot38 is **NOT proven** to be the C740-style `0x40` transform table;
- `transform_selector_bounds_available() == false` remains a hard freeze;
- no Stage-CFG slot38 transform parser or three-table adapter is justified without direct evidence.

Current acquisition/reverse targets:

1. full `0x1400594B0` body + callers + slot38 dataflow;
2. modern Stage-CFG route around C260 callsite `0x14009823F` through downstream builder/C8D0;
3. legacy observed route around `0x1400B6483`;
4. complete C630/C740 caller census classified by manager/source identity;
5. exact producer/base/object, bounds/count and lifecycle of C8D0 stack arg5.

PR #98 may be used to reacquire exact known/probe windows when the legal canonical executable is locally available. It does not remove the need for actual raw bytes and does not convert a discovery window into a proved function body.

This does **not** mean collision is complete. Still open include deeper primitive geometry/contact producers/helpers, source2 backing/lifetime/live semantics, remaining fourth-component semantics where unresolved, controlled runtime comparison, modified-topology game validation and original-builder equivalence.

The deterministic HITS writer is a DMC Rengine product writer. Capcom offline-builder equivalence remains NOT PROVEN.

### Binary Inspector — SUBSTANTIAL DOMAIN IMPLEMENTATION, NOT COMPLETE

Regions, fields, ownership, annotations, selection, coverage/conflicts, diff, entropy, manifests and format adapters exist. Full native product interaction, persistent analysis cache, broader templates/diagnostics, complete EXE bridges and final editor UX are still open.

### EXE Editor / decompilation / recompilation — NOT COMPLETE

PE/address/evidence infrastructure, merged exact byte-window acquisition and selected recovered-source/build-lineage work exist. Full DMC3 decompilation, full source recovery, behaviorally equivalent recompilation and a validated rebuilt executable do not exist today.

### Item / HUD / other editors — PARTIAL, NOT COMPLETE

Guarded request/patch infrastructure and bounded item/editor slices exist. Complete production editor workflows, representative runtime validation and final export/release behavior remain open.

## Canonical executable identity

Current project evidence uses SHA-256:

`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

with corrected project-evidence size `6,356,432` bytes in PR #92. The former `3,735,552` pairing is stale/corrected for this SHA.

PR #92 final correction scope has green Ubuntu/Windows validation in Actions `31877266101`, but the correction remains branch-scoped until deliberately merged. Current `main` at the synchronization cutoff contains PR #98, not an implied merge of #92.

This project-evidence tuple must still be distinguished from an independent fresh hash/read performed in any specific session when the raw executable is not mounted.

## What may be called finished

Only bounded scopes whose exact exit gate is satisfied, for example a specific parser contract, tested product primitive, exact ABI slice or validated reverse claim.

Do not extrapolate that to the containing subsystem.

## Explicitly NOT COMPLETE

- full GDSpaces DMC3 resource runtime equivalence;
- fully game-validated PAC/PNST/NBZ/AFS compatibility and reintegration;
- complete game-backed Stage Catalog semantics/lifecycle validation;
- complete Stage Ops scene/lifecycle equivalence;
- complete Semantic Graph domain coverage;
- complete ModViz editor;
- complete HITS/collision runtime/original-builder equivalence;
- Stage-CFG transform provider/bounds/lifecycle closure for Slice 16;
- source2 collision backing/lifetime semantics;
- complete Recovered Game Source Tree;
- complete Reverse Core acceptance program;
- complete Binary Inspector product UI/integration;
- complete Item/HUD/editor/export stack;
- full DMC3 decompilation;
- behaviorally equivalent rebuilt DMC3 executable;
- complete public release/reintegration pipeline.

## Current critical path

```text
preserve exact artifact/evidence identity
  -> use merged hash-gated acquisition when new exact EXE windows are required
  -> close remaining resource-runtime lifecycle/factory/cache semantics
  -> continue evidence-bounded recovered-game reconstruction
  -> representative Stage Catalog + runtime lifecycle validation
  -> complete Stage Ops domain/runtime bridges
  -> controlled original-vs-reconstruction behavioral receipts
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

HITS proceeds in parallel below its closed wrapper layer from validated Slice 15 into Slice 16 transform-source provenance: `0x1400594B0` + slot38 dataflow + exact C8D0 transform-source/bounds/lifecycle, while representative real Stage-CFG descriptor census and PAC production-path validation continue separately.
