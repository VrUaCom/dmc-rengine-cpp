# ADR-0003 — Stage Ops owns stage/scene assembly and operations

## Status

Accepted for the active DMC3 reconstruction stack.

## Context

The project accumulated several partially overlapping stage representations:

- `GDSpaces::StageBundle` grouped materialized resources;
- `ProjectWorkspace` stored per-resource sessions, working copies, evidence and events;
- `stageops::StageWorkspaceView` projected stage-tagged project sessions;
- ModViz independently projected the same `ProjectWorkspace` into visual resources;
- Stage Semantic Graph planning drifted toward becoming the effective scene assembler.

That left no single product-side owner for the coherent stage/scene between resource materialization and editor/graph consumers. It also created a risk that tool convenience would be mistaken for recovered vanilla runtime behavior.

The recovered executable model now makes the missing boundary explicit: product materialization is earlier than the original game's typed post-load/factory/lifecycle ready state. A durable ownership decision is therefore required before more stage, camera, collision, script, enemy, effect or runtime reconstruction is integrated.

## Decision

### 1. Stage Ops is the stage/scene aggregate authority

Stage Ops owns the product-side `StageAssemblyWorkspace` and the operations performed over that aggregate.

Its responsibilities include:

- selected Stage/resource-set identity;
- exact descriptor/catalog provenance;
- direct required roots, including unresolved roots;
- indirect/nested resources and parent relationships;
- shared ResourceId relationships without duplicate resource ownership;
- parsed/typed stage-domain objects as they become available;
- dependency and stale-state tracking;
- stage-scoped edit/reload/rebuild/validation orchestration;
- one stable scene state for all stage consumers.

`StageWorkspaceView` is a projection of Stage Ops state, not Stage Ops itself.

### 2. GDSpaces remains the only product resource authority

GDSpaces owns:

- source mounting;
- resource lookup/resolution;
- canonical `ResourceId`;
- byte materialization and `ByteProvenance`;
- bounded container expansion;
- source/write-policy boundaries.

Stage Ops may request or consume these services through established APIs, but it must not introduce a second resolver, archive walker or hidden byte store.

`StageBundle` is a materialized resource grouping/input milestone. It is not the complete stage scene and is not an original Capcom runtime object.

### 3. ProjectWorkspace is the cross-tool resource-session substrate

`ProjectWorkspace` remains authoritative for per-resource sessions, Binary Documents, WorkingCopies, evidence links, graph synchronization and mutation events.

Stage Ops owns the **stage-scoped aggregate and orchestration** over those resource sessions; it does not duplicate WorkingCopy bytes or bypass `ProjectWorkspace` mutation/event APIs.

Therefore:

```text
ProjectWorkspace = per-resource operational substrate
StageAssemblyWorkspace = stage-scoped aggregate authority
Stage Ops controller/session = stage-scoped orchestration over both
```

### 4. Recovered Game Source Tree owns vanilla runtime truth

Recovered game functions, types, loaders, post-load fixups, factories, caches, ownership and lifetime logic belong in `recovered-game`.

Stage Ops consumes evidence-backed contracts/results from that reconstruction. It must not copy convenient approximations of Capcom runtime behavior into tool code.

Product materialization and original-game readiness are separate gates:

```text
GDSpaces materialized resources
  -> Stage Ops product assembly
  -> recovered typed post-load/factory/lifecycle evidence
  -> original-game-ready equivalence (only when proven)
```

### 5. Stage Semantic Graph is a derived semantic/evidence projection

Stage Semantic Graph represents relationships already present in or evidenced for the Stage Ops assembly.

It may index:

- stage/room/domain relationships;
- resource containment and sharing;
- script/event/door/camera/collision/effect/audio relationships;
- evidence/confidence;
- Reverse Core and recovered-runtime identities;
- unresolved/unknown relationships.

It must not:

- resolve resources;
- traverse archives independently;
- assemble the scene;
- own mutable stage state;
- schedule vanilla runtime behavior;
- infer gameplay semantics from filenames.

The graph is deterministically rebuildable from Stage Ops state plus linked evidence.

### 6. ModViz is a Stage Ops editor consumer, not a competing assembler

ModViz is the 3D scene/asset editing surface.

It consumes Stage Ops scene state and Semantic Graph relationships. Edits are returned as stage/resource edit commands through Stage Ops and the shared `ProjectWorkspace` mutation path.

ModViz must not independently discover stage membership or maintain a parallel resource/scene authority.

### 7. Dependency direction is enforced

Generic Stage Ops code must not depend on DMC3-specific executable/runtime types.

Profile adapters depend inward on the generic Stage Ops contract:

```text
stageops core
    ^
    |
profiles/dmc3 Stage Ops adapter
```

The same rule applies to future profiles.

## Alternatives considered

- **Semantic Graph as scene owner:** rejected because a graph representation would then perform loading/orchestration and become a second runtime/workspace authority.
- **ProjectWorkspace alone as scene owner:** rejected because it is intentionally resource-session oriented and has no stage-domain aggregate or scene lifecycle.
- **ModViz as scene owner:** rejected because editor/render concerns would own non-visual scene dependencies and duplicate Stage Ops logic.
- **GDSpaces as full scene owner:** rejected because resource materialization and stage-domain orchestration are different responsibilities.
- **Recovered Game Runtime as product scene owner:** rejected because reconstructed vanilla code is evidence/source truth, not the DMC Rengine editor/workspace architecture.

## Consequences

### Positive

- one authoritative product-side stage aggregate;
- no Stage Ops/ModViz/Semantic Graph scene divergence;
- product materialization cannot silently become vanilla-ready truth;
- unresolved resources remain first-class stage state;
- future DMC profiles can reuse generic Stage Ops contracts;
- recovered game code remains cleanly separated from tool architecture;
- editor mutations can be coordinated at stage scope while retaining per-resource WorkingCopy/event authority.

### Negative

- existing Stage Ops and ModViz view builders require migration;
- `StageWorkspaceBuilder` and `StageRuntimeLoader` overlap must be reconciled into one canonical ingress path;
- Stage Ops needs new aggregate/session/controller APIs rather than relying on a small view DTO;
- Semantic Graph work must wait for/track stable Stage Ops domain contracts instead of inventing its own scene model.

## Migration

1. introduce and harden `StageAssemblyWorkspace`;
2. keep DMC3 conversion in a profile adapter, not generic Stage Ops;
3. preserve unresolved descriptor requirements, unique resources, provenance and nested memberships;
4. make existing `StageWorkspaceView` a projection over `StageAssemblyWorkspace`;
5. make ModViz consume Stage Ops assembly state rather than rediscovering stage resources from `ProjectWorkspace`;
6. introduce a Stage Ops session/controller that coordinates StageAssemblyWorkspace with `ProjectWorkspace` resource sessions and mutations;
7. make Stage Semantic Graph a deterministic projection/index of that state;
8. migrate stage-domain assemblers (TXT/StageSet, doors, camera, collision, effects, audio, enemies, events, etc.) into Stage Ops only as evidence permits;
9. bridge recovered-game readiness/factory/lifecycle evidence without moving recovered functions into tool code;
10. retire legacy project-only scene assembly paths once all consumers migrate.

## Validation

Required regression gates:

- structural workspace identity cannot drift from resource-set identity;
- duplicate canonical ResourceIds are rejected in the aggregate;
- memberships must reference existing resources and exact parents;
- unresolved required roots remain visible;
- materialized resources preserve valid ByteProvenance;
- product-complete and original-game-ready gates remain independent;
- generic `stageops` has no DMC3 profile dependency;
- Stage Ops and ModViz consume the same aggregate identities;
- Semantic Graph construction performs no resource resolution;
- edits flow through ProjectWorkspace events/WorkingCopies;
- Windows and Ubuntu CI remain green.

## Review trigger

Reopen this ADR only if executable evidence proves a materially different original stage/runtime boundary, or if a future multi-profile requirement demonstrates that the Stage Ops aggregate contract cannot represent the needed scene without violating the ownership boundaries above.
