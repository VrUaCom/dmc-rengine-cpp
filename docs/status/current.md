# Current Project Status

**Snapshot date:** 2026-08-15  
**Version:** 0.2.0  
**Overall phase:** Evidence-Gated Reconstruction / Runtime Reconstruction / Stage Assembly Consolidation  
**Status authority rule:** distinguish `main`, the active GDSpaces integration spine, active PR stacks, and Drive research evidence. A green feature branch is not automatically `main` truth.

## Executive result

DMC Rengine is an active C++20 reverse-engineering and editing platform. The project has moved beyond a file/parser/editor model toward an evidence-backed reconstruction pipeline:

```text
canonical dmc3.exe
  -> Reverse Core evidence / reconstruction identities
  -> Recovered Game Source Tree
  -> executable-backed resource / selector authority
  -> GDSpaces
  -> Stage Ops
  -> Stage Semantic Graph
  -> ModViz
```

Binary Inspector and EXE Editor are inspection/reconstruction frontends over the same evidence and identity model; they are not independent truth stores.

The project is not yet a complete DMC3 decompilation and does not yet produce a behaviorally equivalent rebuilt game executable.

## Truth and ownership boundaries

- **GDSpaces** is the only product-side game-resource authority: resolution, identity, provenance, materialization, expansion, and product resource access.
- **Recovered Game Source Tree** owns reconstructed original DMC3 runtime behavior. Resource factories, post-load fixups, cache/lifetime code, collision consumers, and other recovered vanilla code do not become GDSpaces/Stage Ops/ModViz implementation merely because those tools consume the findings.
- **Reverse Core** owns generic reverse identities, evidence, hypotheses, experiments, TaskClaims, reconstructions, and ValidationReceipts. Agent consensus is never evidence.
- **Stage Ops** owns complete product-side stage/scene assembly and mutable operational stage state through `StageAssemblyWorkspace` and `StageOperationsSession`.
- **Stage Semantic Graph** is a deterministic representation/index projected from Stage Ops state. It does not resolve resources or assemble a second scene.
- **ModViz** consumes Stage Ops state and semantic/runtime evidence for editing. It does not independently resolve resources or originate scene assembly.
- **Binary Inspector** owns byte/structure/evidence inspection. **EXE Editor** owns function/type/reconstruction editing over shared reverse identities and recovered source.

## GDSpaces and Stage catalog

The active executable-backed model is not `st001`-centric.

Confirmed Stage/resource authority includes:

- 189 EXE-derived resource descriptors;
- Bank A: 110 descriptors;
- Bank B: 79 descriptors;
- a separate 193-entry selector space;
- a separate table of 10 group-base pointers;
- numeric Stage resolution through selector/group logic;
- exact four-role descriptor planning;
- source-bank/source-row/global-row provenance;
- recursive materialization/expansion with partial-result preservation.

The following identity axes remain independent and must never be automatically equated:

```text
resource_set_id / catalog_entry_id
numeric_stage_id
semantic_stage_id
```

`st001` is a regression fixture only. It is not the architecture, catalog universe, or completion gate.

## Resource-runtime readiness

GDSpaces materialization is ahead of full vanilla-runtime reconstruction. The completion model remains:

```text
A — structural read
B — lookup equivalence
C — load-path reconstruction
D — lifetime reconstruction
E — validated runtime model
```

Open evidence still includes source/archive registration and priority, `.lst` fallback details, typed MOD/EFM/SCM/SHW post-load behavior, recursive typed dispatch, factory/construction boundaries, cache/reuse, state-4 cleanup, and lifecycle behavior across transitions/restart/menu/shutdown.

**Materialized bytes are not an original DMC3 state-3/game-ready object.** `game_ready_equivalent` must remain false until recovered-runtime evidence validates post-load/factory/cache/ownership/consumer/lifetime equivalence.

## Stage Ops

PR #91 restores Stage Ops as the product-side stage/scene assembly authority and already implements the central Phase-A architecture on its active branch:

- `StageAssemblyWorkspace`;
- DMC3 `StageOpsIngress` from `StageRuntimeLoadReport` without a second resource resolution pass;
- `StageOperationsSession` over shared WorkingCopies/events;
- retained canonical typed parser results and byte lineage;
- Stage domain workspaces and exact source spans;
- structural domain relations;
- explicit recovered-runtime links and validation;
- deterministic Stage Semantic Graph projection;
- Stage scene snapshots;
- ModViz projection/edit plumbing with revision guards;
- invalidation and validation/receipt plumbing.

This is active PR-stack implementation, not yet equivalent to promotion into `main`.

A size-changing Stage TXT WorkingCopy regression exposed an invalid coupling between immutable `ResourceId::size` and mutable working bytes. The binary-document adapter now sizes itself from the active byte view while retaining source-span identity, with a regression test for insertion/deletion-sized views.

## Stage Semantic Graph

The graph is now a Stage Ops-derived representation, not a scene assembler. It preserves resource, numeric Stage, semantic Stage, domain-object, recovered-runtime, and evidence identities without manufacturing missing semantics.

Structural/product facts, recovered-runtime facts, confirmed semantics, inferred semantics, and unresolved relationships remain distinguishable.

## HITS / collision reverse

HITS remains one of the strongest reverse vertical slices:

- verified deterministic parser/writer;
- `0x38` triangle/plane records;
- reconstructed spatial table behavior;
- exact 13-axis inclusive triangle-box SAT;
- real-corpus verification;
- deterministic topology rebuild;
- source0/source1 separation.

Current runtime ownership evidence includes `StageCollisionWorld` and two embedded static HITS sources. Source2 remains external/global with unresolved backing/lifetime.

Highest-value open reverse target remains `0x14005E7A0`, followed by `0x14005B460`, `0x14005FEC0`, and `0x1400601E0`, to close arbitration, candidate ABI, and caller-visible result semantics.

## Reverse Core and recovered source

The required reconstruction chain is:

```text
dmc3.exe
  -> stable artifact/range/function/type identity
  -> evidence
  -> reconstruction
  -> recovered-game C++
  -> isolated compilation
  -> behavioral comparison
  -> ValidationReceipt
  -> canon promotion / correction / rejection
```

Recovered C++ is an evidence-backed executable specification until behavior and provenance gates justify stronger claims. It is never represented as original Capcom source.

The first acceptance milestone remains one bounded subsystem completing the full chain through a ValidationReceipt.

## ModViz

ModViz remains the 3D scene/asset editor consuming Stage Ops state. Its active Menu Editor vertical slice is the Red Orb HUD counter through GDSpaces identity, hierarchy/mesh/UV editing, WorkingCopy, runtime/EXE evidence, guarded modifications, preview, validation, and export.

## Current blockers

P0/P1 work is now centered on:

1. full resource-runtime lifecycle closure from request to state-3 readiness and teardown;
2. raw reverse evidence for collision arbitration (`0x14005E7A0` family);
3. representative Bank-A, Bank-B, selector/shared/alias/partial validation receipts;
4. first complete recovered-game compile + behavioral-validation receipt;
5. SCM post-load conflict around `mesh+0x28`;
6. source2 collision backing/lifetime;
7. Reverse Core shared TaskClaim/reconstruction infrastructure and bridge completion;
8. integration-stack promotion without losing branch-specific fixes or evidence provenance;
9. no behaviorally equivalent rebuilt DMC3 executable yet.

## Current critical path

```text
close high-value raw reverse evidence
  + recovered-game compile proof
  -> resource state-2 -> state-3 lifecycle closure
  -> StageAssemblyWorkspace representative game validation
  -> Semantic Graph / ModViz consumption of the same Stage Ops state
  -> Reverse Core validated subsystem receipt
  -> controlled broader reconstruction / recompilation
```

UI breadth is deliberately downstream of these evidence and lifecycle gates.
