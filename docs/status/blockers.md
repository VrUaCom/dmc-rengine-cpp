# Current Blockers

**Snapshot date:** 2026-08-15  
**Rule:** `st001` is a regression fixture only. Blockers are expressed against the executable-derived catalog/runtime model, not one filename family.

## P0 — Runtime and reconstruction blockers

### B-030 — Original resource state-2 -> state-3 readiness is incomplete

**Status:** open  
**Tracking:** #55, #88 and recovered-runtime work

Product materialization/expansion exists, but the complete original runtime boundary remains open:

```text
request -> lookup -> bytes -> transform -> typed post-load
        -> factory -> cache/ownership -> consumer -> teardown
```

Required evidence includes source/archive registration and priority, `.lst` fallback, MOD/EFM/SCM/SHW post-load behavior, recursive typed dispatch, factory/construction, reuse/cache, state-4 cleanup, transitions/restart/menu/shutdown.

`game_ready_equivalent` must remain false until this is behaviorally validated.

### B-031 — Collision arbitration ABI remains incomplete

**Status:** research required  
**Tracking:** #25

Highest-value targets:

- `0x14005E7A0` — exact ABI, no-hit initialization, static/dynamic metrics, arbitration and tie-break, caller-visible output;
- `0x14005B460` — category-list ABI, ownership/lifetime, candidate production, return semantics;
- `0x14005FEC0` — source-1 output ABI;
- `0x1400601E0` — in/out structure, fourth component, accumulation behavior.

### B-032 — First complete recovered-game ValidationReceipt is absent

**Status:** open  
**Tracking:** #51

No bounded real DMC3 subsystem has yet closed the entire chain:

```text
binary identity -> evidence -> reconstruction -> recovered C++
-> isolated build -> behavioral comparison -> ValidationReceipt
```

This remains the principal gate before scaling mass reconstruction/decompilation.

### B-033 — Reverse Core shared mutation/identity infrastructure is incomplete

**Status:** open  
**Tracking:** #51

Required shared objects/flows include `BinaryArtifact`, ranges/functions/data/types, evidence/hypotheses/experiments, `TaskClaim`, reconstruction revisions, `ValidationReceipt`, and subsystem membership.

TaskClaims must prevent canonical reconstruction races without being mistaken for evidence or semantic ownership.

### B-034 — Active integration stack is not yet promoted as one reviewed project state

**Status:** open

`main`, the GDSpaces integration spine, recovered-runtime branches, Stage Ops PR #91, and research authority contain different generations of truth. Green feature-branch CI does not by itself make a branch project-wide canon.

Required:

- explicit integration-spine composition;
- overlap/supersession review;
- whole-stack Windows/Ubuntu CI;
- promotion receipt listing included PR heads;
- status/docs regenerated against the promoted commit.

## P1 — Validation and domain blockers

### B-035 — Representative full Stage catalog validation is incomplete

**Status:** open  
**Tracking:** #4, #55, #90

The active model includes 189 descriptors (Bank A 110 + Bank B 79), a separate 193-entry selector space, and 10 group-base pointers. Validation must cover more than `st001`:

- Bank A representative cases;
- Bank B representative cases;
- shared/aliased resource sets;
- selector/fallback cases as semantics are recovered;
- partial/unresolved stages;
- transition/reload/unload behavior.

### B-036 — Stage domain breadth is incomplete

**Status:** open  
**Tracking:** #53, #90

PR #91 establishes the Stage Ops assembly/domain/graph architecture, but evidence-backed domain coverage remains incomplete for geometry/model/texture, camera, doors/transitions, effects/audio, enemy/spawn, events/demo, positions, and unknown runtime-linked domains.

Stage Semantic Graph must continue to project Stage Ops state rather than becoming a second assembler.

### B-037 — SCM post-load conflict remains gated

**Status:** research required

Do not promote SCM post-load/game-ready behavior while the `mesh+0x28` interpretation conflict is unresolved.

### B-038 — Collision source2 backing/lifetime is unknown

**Status:** research required  
**Tracking:** #25

Source0/source1 ownership is substantially reconstructed; source2 remains external/global and still needs backing resource, lifetime, and selection-source evidence.

### B-039 — HITS original runtime/offline-builder equivalence remains unproven

**Status:** research required

Parser/writer, `0x38` records, spatial reconstruction, inclusive 13-axis SAT, deterministic topology rebuild, and corpus checks are strong, but Capcom builder/runtime equivalence still requires controlled game-backed receipts.

### B-040 — ModViz Red Orb vertical slice incomplete

**Status:** open  
**Tracking:** #52

Remaining work includes hierarchy editing, screen-space canvas/layout, transforms/scale, UV atlas regions, draw order, digit clone/insert/remove, representative preview, runtime formatting constraints, deterministic validation/export.

## P2 / downstream blockers

### B-041 — Production output/release pipeline incomplete

Deterministic output packaging, release validation, signing/attestation, and public binary distribution remain downstream of evidence and runtime gates.

### B-042 — Complete desktop UI deferred

UI breadth remains downstream of domain/runtime correctness. New panels must not redefine resource identity, Stage assembly, reverse truth, or write policy.

## External infrastructure concern

The Triangle Forge/MCP desktop bootstrap/logging/signing environment can affect coordination and distribution, but it does not block static DMC3 reverse work or reviewed C++ library work. Keep that infrastructure state separate from DMC3 evidence status.

## Current critical path

```text
B-031 high-value collision reverse
  + B-032 first recovered compile/validation proof
  -> B-030 resource lifecycle closure
  -> B-035 representative catalog validation
  -> B-036 broader Stage Ops domain/runtime links
  -> B-033 Reverse Core validated shared workflow
  -> B-034 integration-stack promotion
  -> controlled recompilation milestones
```
