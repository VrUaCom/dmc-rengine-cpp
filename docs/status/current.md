# Current Project Status

**Snapshot date:** 2026-08-15  
**Repository:** `VrUaCom/dmc-rengine-cpp`  
**Merged product baseline:** `main` at `562e14179723598e09e58c9baded998a2b79e1a3`  
**Overall completion status:** **NO MAJOR END-TO-END SUBSYSTEM IS COMPLETE**

Read `completion-and-evidence-policy.md` before interpreting any status below.

## Executive truth

DMC Rengine contains a large amount of real, compiled and tested implementation plus substantial reverse-engineering evidence. That does **not** mean the major systems are finished.

The current canonical statement is:

> Individual parsers, ABIs, runtime slices, writers, workspace flows and integration branches may be implemented/tested/EXE-confirmed/bounded-closed. The containing major subsystem remains incomplete until its evidence, integration, representative runtime validation, lifecycle behavior and ValidationReceipt gates are satisfied.

A green CI job proves the tested branch/build contract only. It does not prove original DMC3 behavioral equivalence.

## Truth layers

### GitHub `main`

`main` is merged product implementation truth. It currently lags the active Stage/GDSpaces/recovered-runtime/HITS research stacks.

Important consequence: old `main` files that still reference a `st001`-centric compatibility path are **not** current Stage architecture authority.

### Active implementation/reverse stacks

Current important draft branches/PRs include:

- PR #74 — active DMC3 Stage descriptor/resource materialization stack;
- PR #85 — HITS Pass 10 evidence reacquisition and deeper bounded reverse closures;
- PR #89 — executable Recovered Game Source Tree Wave-3 slices;
- PR #91 — Stage Ops scene assembly/operations authority;
- PR #92 — canonical executable SHA/size metadata correction;
- PR #93 — HITS classifier/format-authority correction.

These are branch-scoped truth until deliberately promoted. They must not be described as merged-main completion.

## Current subsystem status

### GDSpaces — NOT COMPLETE

Implemented/tested slices include resource identities, source registry, lookup policies, provenance, container expansion, Stage resource planning/materialization and branch-scoped runtime-resolution work.

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

Remaining gates include representative Bank-A/Bank-B/shared/alias/partial validation, semantic mapping, selector fallback behavior, `.lst` behavior, and lifecycle linkage.

### Stage Ops — IMPLEMENTED ON ACTIVE BRANCH, NOT COMPLETE

PR #91 contains substantial product-side assembly/operations implementation, including Stage assembly workspace, ingress, operations, shared WorkingCopy/parser lineage, domain projections, Semantic Graph and ModViz projections.

The branch has recorded green Windows/Ubuntu CI on its current tested head, but Stage Ops still lacks complete domain/runtime coverage and vanilla lifecycle equivalence. It is not merged-main completion and must not be called game-ready.

### Stage Semantic Graph — IMPLEMENTED PROJECTION SLICES, NOT COMPLETE

Semantic Graph is a derived representation/index over Stage Ops state. It does not own resource resolution or scene assembly. Current projections do not prove complete scene semantics or original runtime behavior.

### ModViz — PARTIAL, NOT COMPLETE

ModViz consumes Stage Ops state in the target architecture. Scene/HUD/editor slices exist, but complete scene/asset/HUD editing, validated export and original-game behavioral equivalence are open.

### Recovered Game Source Tree — COMPILES IN SELECTED SLICES, NOT COMPLETE

Selected recovered DMC3 runtime units compile and execute deterministic tests in active branches. Recovered C++ is an evidence-backed executable specification, not automatically original Capcom source.

Still required across the project:

- broader direct instruction/ABI coverage;
- exact ownership/lifetime semantics;
- controlled original-vs-reconstruction behavioral comparison;
- subsystem-level ValidationReceipts;
- eventual integration/rebinding milestones.

### Reverse Core — FOUNDATION/DIRECTION ACTIVE, NOT COMPLETE

Reverse Core owns generic evidence/reconstruction/claim/validation infrastructure, not DMC3 gameplay code. Its full acceptance gate requires bounded reconstructed subsystems with compile + behavioral comparison + ValidationReceipt, plus robust multi-agent claim/evidence workflows.

### HITS / collision — STRONG PARTIAL, NOT COMPLETE

Merged/product and active research contain substantial HITS work: header-driven format parsing, `0x38` records, spatial reconstruction, deterministic DMC Rengine writer, source0/source1 ownership evidence, runtime query evidence, and deeper Pass-10 findings.

Important 2026-08-15 correction: do **not** repeat the older statement that all top-level P0 wrapper ABI targets are still open. PR #85 records later bounded closure/reclassification for:

- `0x14005E7A0` combined point-query wrapper ABI/precedence;
- `0x14005FEC0` and `0x1400601E0` top-level contracts;
- `0x14005B460` reclassified into the dynamic-world update pipeline rather than an `E7A0` candidate producer;
- common contact-normal semantics;
- primitive descriptor ownership.

This does **not** mean collision is complete. Still open include deeper primitive producer/helper reconstruction, source2 backing/lifetime/live semantics, full gameplay meaning, controlled runtime comparison, and original-builder equivalence.

The deterministic HITS writer is a DMC Rengine product writer. Capcom offline-builder equivalence remains NOT PROVEN.

### Binary Inspector — SUBSTANTIAL DOMAIN IMPLEMENTATION, NOT COMPLETE

Regions, fields, ownership, annotations, selection, coverage/conflicts, diff, entropy, manifests and format adapters exist. Full native product interaction, persistent analysis cache, broader templates/diagnostics, complete EXE bridges and final editor UX are still open.

### EXE Editor / decompilation / recompilation — NOT COMPLETE

PE/address/evidence infrastructure and selected recovered-source/build-lineage work exist. Full DMC3 decompilation, full source recovery, behaviorally equivalent recompilation and a validated rebuilt executable do not exist today.

### Item / HUD / other editors — PARTIAL, NOT COMPLETE

Guarded request/patch infrastructure and bounded item/editor slices exist. Complete production editor workflows, representative runtime validation and final export/release behavior remain open.

## Canonical executable identity

The project currently uses SHA-256:

`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

with the corrected project-evidence size `6,356,432` bytes in PR #92. The former `3,735,552` pairing is stale/corrected for this SHA.

This project-evidence tuple must still be distinguished from an independent fresh hash/read performed in any specific session when the raw executable is not mounted.

## What may be called finished

Only **bounded scopes** whose exact exit gate is satisfied, for example a specific parser contract, tested product primitive, exact ABI slice, or validated reverse claim.

Do not extrapolate that to the containing subsystem.

## Explicitly NOT COMPLETE

- full GDSpaces DMC3 resource runtime equivalence;
- complete game-backed Stage Catalog semantics/lifecycle validation;
- complete Stage Ops scene/lifecycle equivalence;
- complete Semantic Graph domain coverage;
- complete ModViz editor;
- complete HITS/collision runtime/original-builder equivalence;
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
  -> close remaining resource-runtime lifecycle/factory/cache semantics
  -> continue evidence-bounded recovered-game reconstruction
  -> representative Stage Catalog + runtime lifecycle validation
  -> complete Stage Ops domain/runtime bridges
  -> controlled original-vs-reconstruction behavioral receipts
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

HITS proceeds in parallel below its already-closed wrapper layer toward primitive-specific geometry/contact reconstruction, source2/lifecycle closure and controlled runtime comparison.
