# Roadmap

**Snapshot date:** 2026-08-15

The roadmap prioritizes evidence-backed vertical completion over UI breadth. Legacy PAC Editor/PAC Manager logic remains excluded; container formats stay internal to GDSpaces. `st001` is a regression fixture only and never an architecture milestone.

## Foundation — maintained

- C++20/CMake and Windows/Ubuntu CI;
- artifact SHA-256 identity and Evidence Packets;
- clean-room/public-data policy;
- revisioned WorkingCopies and guarded mutation;
- copied-output/rollback and source/build provenance;
- GDSpaces-only product resource resolution.

## Milestone A — Close high-value raw reverse evidence

Primary collision target: `0x14005E7A0`.

Close exact ABI, no-hit initialization, candidate metrics, static/dynamic arbitration, equality/tie-break behavior, and caller-visible result. Follow with `0x14005B460`, `0x14005FEC0`, and `0x1400601E0`.

**Exit gate:** collision candidate production/arbitration is represented by evidence-backed recovered contracts rather than inferred product behavior.

## Milestone B — First recovered-game compile + validation proof

Use one bounded subsystem in the Recovered Game Source Tree:

```text
binary artifact/range/function/type identity
  -> evidence
  -> reconstruction
  -> recovered C++
  -> isolated compile
  -> behavioral comparison
  -> ValidationReceipt
```

**Exit gate:** one real DMC3 subsystem receives an accepted/corrected/rejected receipt. Readable C++ alone does not satisfy this milestone.

## Milestone C — Resource runtime lifecycle closure

Complete issue #55/#88 evidence from request through readiness and teardown:

- source/archive registration and priority;
- lookup and `.lst` fallback;
- byte acquisition/transformation;
- recursive PAC/PNST traversal;
- MOD/EFM/SCM/SHW typed post-load behavior where evidence is conflict-free;
- factory/construction;
- cache/reuse/ownership;
- consumer handoff;
- state-4 cleanup, transition/restart/menu/shutdown.

**Exit gate:** representative resources reach the appropriate Level C/D/E validation without equating product materialization with original state-3 readiness.

## Milestone D — Representative full-catalog Stage validation

Use the executable-derived Stage/resource authority:

- 189 resource descriptors: Bank A 110 + Bank B 79;
- separate 193-entry selector space;
- 10 group-base pointers;
- independent `resource_set_id/catalog_entry_id`, numeric Stage ID, and semantic Stage ID.

Validate representative Bank-A, Bank-B, shared/alias, fallback/selector, and partial/unresolved cases.

**Exit gate:** Stage Ops consumes representative executable-derived resource sets without second resolution and emits evidence-bearing assembly/validation receipts.

## Milestone E — Stage Ops operational assembly hardening

PR #91 establishes the central implementation:

`StageRuntimeLoadReport -> StageOpsIngress -> StageAssemblyWorkspace -> StageOperationsSession -> domain workspaces/runtime links -> Semantic Graph / ModViz`.

Next hardening:

- finish migration from compatibility/project-only paths;
- explicit Binary Inspector byte-source/revision lineage;
- broaden evidence-backed domain relations/runtime links;
- preserve partial/unresolved requirements;
- prove edit/reparse/invalidation/refresh across size-changing WorkingCopy edits;
- keep `game_ready_equivalent` evidence-gated.

**Exit gate:** one representative stage slice survives load, edit, refresh, validation, semantic projection, and ModViz consumption from the same Stage Ops authority.

## Milestone F — Stage domain breadth

Add evidence-backed domain assembly for:

- geometry/model/texture;
- collision;
- lighting;
- camera;
- doors/transitions;
- StageSet/script/events;
- effects;
- audio;
- enemy/spawn;
- positions/markers;
- shared and unknown resources;
- recovered factory/lifecycle links.

Semantic Graph remains a disposable representation/index over Stage Ops, not a scene assembler.

## Milestone G — Reverse Core v0.1 acceptance

Complete shared generic identities, TaskClaims, reconstruction revisioning, bridges, and ValidationReceipts.

TaskClaims prevent mutation races but do not constitute evidence or semantic code ownership.

**Exit gate:** the Milestone-B subsystem is represented end-to-end in Reverse Core without DMC-specific concepts leaking into the reusable core.

## Milestone H — ModViz vertical editing

Finish the Red Orb HUD counter as the first Menu Editor proof:

- GDSpaces ResourceId;
- HUD hierarchy/digit meshes/UV atlas/layout;
- WorkingCopy edits;
- EXE/Reverse Core formatting constraints;
- guarded patch request where required;
- representative preview;
- deterministic validation/export.

**Exit gate:** a visual edit travels through shared resource/stage/evidence/write contracts without a second resolver or direct EXE write path.

## Milestone I — Integration-spine promotion

Before calling the active stack project-wide truth:

- compose/review stacked branches deliberately;
- preserve #74/#91/recovered-runtime fixes;
- resolve duplicate/stale documentation and evidence;
- run whole-stack Windows/Ubuntu CI;
- record included PR/commit heads and superseded branches;
- regenerate status against the promoted commit.

## Milestone J — Controlled recompilation

Only after multiple behavior-tested recovered modules:

- replacement/rebinding boundaries;
- deterministic composite builds;
- source-to-output address provenance;
- runtime regression receipts;
- first rebuilt executable claim only when evidence supports behavioral equivalence.

## Parallel work

- HITS real-corpus/runtime validation and source2 research;
- Binary Inspector / EXE Editor Reverse Core bridges;
- narrow evidence-backed recovered-source promotion;
- SCM conflict closure before post-load promotion;
- Red Orb vertical slice after shared contracts stay intact.

## Deferred behind evidence gates

- broad UI-first expansion;
- production archive writer/repack suite;
- broad automatic decompilation at scale;
- public release claims before deterministic validation/signing.
