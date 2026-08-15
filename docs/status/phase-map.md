# Phase Map

**Snapshot date:** 2026-08-15  
**Scope rule:** phase status must distinguish reviewed `main` from active stacked implementation. `st001` is a regression fixture, never a phase gate.

## Phase 0 — Public C++ foundation

**Status:** complete / maintained

C++20/CMake, Windows/Ubuntu CI, governance, clean-room rules, evidence contracts, working-copy safety, and public documentation foundations are established.

## Phase 1 — Evidence, identity, and guarded mutation

**Status:** substantially implemented / expanding

Artifact identity, Evidence Packets, strict import/export, resource identities, revisioned WorkingCopies, guarded patches, copied-output execution, rollback, source/build provenance, and project workspaces are implemented foundations.

## Phase 2 — EXE inspection and reverse identities

**Status:** active

PE32+ inspection, RVA/VA/file-offset conversion, executable target identity, evidence address resolution, and selected promoted executable findings are established.

Open: broad function/type reconstruction under one Reverse Core identity model and behavior-tested recovered subsystems.

## Phase 3 — GDSpaces production resource path

**Status:** active, significantly beyond the old 110-row/st001 model

Active integration work includes production resource resolution/materialization/provenance/expansion and executable-derived Stage authority:

- 189 descriptors: Bank A 110 + Bank B 79;
- independent 193-entry selector space;
- 10 group-base pointers;
- four-role descriptor planning;
- independent resource-set, numeric Stage, and semantic Stage identities;
- partial-result preservation and recursive expansion.

Open: complete request/lookup/source priority, `.lst` fallback, typed post-load/factory/cache/lifetime/unload equivalence.

**Exit gate:** representative Bank-A, Bank-B, shared/alias/fallback/partial cases complete the appropriate resource-runtime validation levels without any tool-local resolver.

## Phase 4 — Stage Ops assembly and operations

**Status:** central architecture implemented on active PR #91; representative game validation open

The old milestone “build one st001 StageBundle and then invent Stage Ops” is superseded.

Active implementation includes:

- `StageRuntimeLoadReport -> StageOpsIngress -> StageAssemblyWorkspace`;
- `StageOperationsSession` over shared WorkingCopies/events;
- canonical typed parser-result reuse;
- Stage domain workspaces/source spans/relations;
- explicit recovered-runtime links;
- scene snapshots and invalidation;
- deterministic Stage Semantic Graph projection;
- ModViz Stage Ops projection/edit plumbing.

**Exit gate:** representative executable-derived resource sets validate partial/complete assembly, edit/refresh/invalidation, runtime-link evidence, and lifecycle readiness without second resolution or invented semantics.

## Phase 5 — Binary Inspector

**Status:** domain foundation implemented / reverse integration active

Regions, typed fields, ownership, annotations, selection/range context, coverage/unknown/conflict analysis, deterministic manifests, diff, entropy, and format adapters are established.

Current integration priority is explicit byte-source/revision lineage and durable Reverse Core bridge identities, not a second reverse database.

## Phase 6 — Recovered Game Source Tree

**Status:** physical/reconstruction work active; complete validated subsystem still open

Original DMC3 runtime reconstruction belongs under the recovered game source tree rather than editor/tool modules.

The required per-unit chain is:

```text
binary identity -> evidence -> reconstruction -> C++ -> isolated build -> behavioral comparison -> ValidationReceipt
```

**Exit gate:** one bounded real subsystem completes that chain and can be promoted/corrected/rejected deterministically.

## Phase 7 — Reverse Core v0.1

**Status:** architecture accepted / shared infrastructure incomplete

Required generic objects remain:

`BinaryArtifact`, `AddressRange`, `Function`, `DataObject`, `RecoveredType`, `EvidenceRecord`, `Hypothesis`, `Experiment`, `TaskClaim`, `Reconstruction`, `ValidationReceipt`, `Subsystem`.

TaskClaims coordinate mutation ownership; they do not create evidence or semantic game ownership.

**Exit gate:** one real DMC subsystem uses shared identities/claims/reconstruction/validation end-to-end without DMC-specific concepts leaking into Reverse Core.

## Phase 8 — Stage Semantic Graph

**Status:** deterministic Stage Ops-derived representation implemented on active PR #91; domain breadth/evidence expansion open

The graph represents/indexes Stage Ops state. It never resolves resources, owns WorkingCopy bytes, assembles a parallel scene, or manufactures gameplay identity from filenames.

Open domain breadth includes evidence-backed geometry/model/texture, camera, doors/transitions, effects/audio, enemy/spawn, events/demo, positions, and unresolved domains.

## Phase 9 — ModViz vertical editing

**Status:** shared Stage Ops projection/edit architecture active; product vertical slices incomplete

Scene/Model editing consumes `StageAssemblyWorkspace`/Stage Ops projections. The first Menu Editor proof remains the Red Orb HUD counter through shared resource identity, WorkingCopy, EXE/reverse constraints, guarded change, preview, validation, and export.

## Phase 10 — Runtime reconstruction closure

**Status:** research/implementation active

Current high-value gates include:

- resource state-2 -> typed post-load/factory -> state-3 readiness;
- cache/reuse/ownership/state-4 teardown/lifecycle;
- collision arbitration around `0x14005E7A0` and supporting functions;
- source2 collision backing/lifetime;
- SCM `mesh+0x28` conflict closure.

## Phase 11 — Controlled recompilation frontier

**Status:** long-term; executable milestone not achieved

Requires multiple behavior-tested recovered modules, controlled replacement/rebinding, deterministic composite builds, source-to-output provenance, and runtime regression receipts.

No behaviorally equivalent recompiled DMC3 executable is currently claimed.

## Current critical sequence

```text
high-value reverse evidence + recovered compile proof
  -> resource lifecycle closure
  -> representative StageAssemblyWorkspace validation
  -> Semantic Graph / ModViz over the same Stage Ops authority
  -> Reverse Core validated subsystem receipt
  -> broader reconstruction and controlled recompilation
```

Parallel work includes HITS/collision runtime reverse, representative catalog validation, Red Orb vertical editing, Binary Inspector/EXE bridges, and narrow evidence-backed recovered-source promotion.
