# Architecture

## Mission

DMC Rengine is a C++20 evidence-backed reverse-engineering, reconstruction, editing, validation and progressive recompilation framework for Devil May Cry 3 HD.

Architecture describes ownership and dependency direction. It does **not** imply subsystem completion. See `docs/status/completion-and-evidence-policy.md`.

## Canonical authority model

```text
canonical DMC3 executable / runtime evidence
        │
        ├── Reverse Core
        │     artifact/function/type/evidence/claim/reconstruction/validation identities
        │
        ├── Recovered Game Source Tree
        │     reconstructed original DMC3 runtime behavior
        │
        └── executable resource / Stage authority
                    │
                    ▼
                 GDSpaces
        resource identity / resolution / bytes / provenance / containers
                    │
                    ▼
                 Stage Ops
        product stage/scene assembly / operational state
             ┌──────┴──────┐
             ▼             ▼
      Semantic Graph      ModViz
      derived index       editor consumer
```

Binary Inspector and EXE Editor are inspection/editing frontends over these shared authorities; they do not become alternate resource, scene or reverse-truth owners.

## Hard ownership boundaries

### GDSpaces

GDSpaces is the **single product resource authority**.

It owns:

- source mounting and lookup;
- resource identity;
- runtime-equivalent path/source resolution policies where evidenced;
- byte acquisition/materialization;
- ByteProvenance;
- format classification;
- container expansion;
- resource graph and resource-facing diagnostics;
- WorkingCopy substrate at the resource boundary.

It does **not** own reconstructed original DMC3 factories, caches, object lifetimes, collision runtime, scene-manager behavior or other original-game code.

### Recovered Game Source Tree

`recovered-game/...` owns evidence-backed reconstruction of original DMC3 runtime functions/types/behavior.

Examples include:

- original resource lifecycle and typed post-load behavior;
- factories/consumers/cache/lifetime when recovered;
- original collision runtime ABI and geometry/query behavior;
- Stage/runtime consumer behavior;
- other executable-derived original systems.

Recovered C++ is an executable specification backed by evidence. It is not automatically original Capcom source and is not behaviorally equivalent merely because it compiles.

### Reverse Core

Reverse Core owns **generic reverse infrastructure**, including identities such as artifacts, address ranges, functions, data objects, recovered types, evidence, hypotheses, experiments, task claims, reconstructions and validation receipts.

It must remain reusable outside DMC3. DMC-specific runtime code belongs in the recovered game tree.

Parallel agents coordinate ownership/claims through this layer; agent agreement itself is not evidence.

### Stage Ops

Stage Ops is the **product-side owner of stage/scene assembly and operational workspace state**.

It consumes already resolved/materialized GDSpaces resources and recovered-runtime contracts; it does not resolve game files again and does not copy original-game runtime ownership into product code.

It owns:

- `StageAssemblyWorkspace`;
- resource/domain assembly state;
- partial/unresolved state;
- WorkingCopy/edit/reanalysis orchestration across stage resources;
- dependency invalidation/validation state;
- stable scene state for consumers.

Product assembly success is not the same as original DMC3 state-3/game-ready equivalence.

### Stage Semantic Graph

The Semantic Graph is a **derived representation/index over Stage Ops state**.

It may represent Stage/Room, geometry, collision, lighting, camera, doors, scripts, events, effects, audio, enemies, runtime/evidence links, shared resources and unknown nodes as evidence permits.

It does not resolve resources, traverse archives, create original runtime objects or assemble a competing scene.

### ModViz

ModViz owns interactive 3D/asset/HUD editing views over Stage Ops state.

It must not:

- resolve GDSpaces resources independently;
- create a competing Stage model;
- reconstruct original runtime behavior locally;
- directly mutate original executable/resource files.

### Binary Inspector

Binary Inspector owns byte/structure/ownership/evidence inspection over supplied byte lineage and stable resource/artifact identity.

It does not reopen paths or become a game-source resolver.

### EXE Editor

EXE Editor is the executable/reconstruction editing frontend. It consumes Reverse Core evidence/reconstruction identities and recovered-source units, plus PE/address/byte evidence.

It does not own a separate decompilation truth disconnected from Reverse Core/recovered-game.

### Build & Test Lab / guarded modification

Validation and guarded modification own:

- expected-byte/hash gates;
- validation plans;
- copied-output execution;
- rollback;
- behavioral comparison receipts;
- release/reintegration gates when eventually implemented.

## Stage identity

Stage architecture is not based on `st001` or a filename template.

Current Wave-2 executable authority distinguishes:

- 110 Bank-A observed descriptors;
- 79 Bank-B observed descriptors;
- 189 observed descriptors total;
- 193 selector entries;
- 10 group-base pointers;
- numeric `stageId / 100` and `stageId % 100` indirection.

Keep separate:

1. resource-set/catalog-entry identity;
2. numeric Stage identity;
3. semantic/gameplay Stage identity only when separately evidenced.

`st001` is regression/compatibility data only. 189 descriptors are not automatically 189 gameplay stages.

## Container policy

PAC, PNST, NBZ and AFS are resource/container layers consumed through GDSpaces. Legacy PAC Editor/PAC Manager top-level architecture is rejected.

Parsers consume supplied byte spans and do not perform independent source discovery.

## Mutation and safety policy

- original source bytes are immutable by default;
- edits occur through revisioned WorkingCopies;
- Binary Documents/parser results must track exact byte lineage/revision;
- executable changes require exact target identity and expected-byte guards;
- product writers are not original-builder equivalence claims unless separately proven;
- export/reintegration requires validation and explicit output ownership;
- no major behavior-equivalence claim without the applicable ValidationReceipt gate.

## Completion rule

Architecture completion is not inferred from implementation density.

A parser can be complete for a bounded schema while its runtime system remains incomplete. A wrapper ABI can be bounded-closed while the collision subsystem remains incomplete. A compiled recovered target can be tested while original-game equivalence remains not proven.

The project-wide completion state is maintained in `docs/status/current.md` and `docs/status/canonical-status.json`.
