# Architecture

## Mission

DMC Rengine is a C++20 evidence-backed reverse-engineering, decompilation, editing and progressive-recompilation framework for Devil May Cry 3 HD.

## Canonical authority flow

```text
Evidence / exact artifacts
 -> Reverse Core identities
 -> Recovered Game Source Tree
 -> GDSpaces resource authority
 -> Stage Ops assembly/orchestration
 -> Stage Semantic Graph
 -> ModViz/editor consumers
 -> Runtime host layer
```

These layers cooperate but do not collapse ownership.

## Core rules

1. **GDSpaces is the only product resource resolver/materializer/provenance authority.**
2. **Recovered original DMC3 functions/types/lifecycle code belong to the Recovered Game Source Tree.**
3. **Stage Ops owns product-side stage/scene assembly and operational workspace state.**
4. **Stage Semantic Graph represents Stage Ops state; it does not load/assemble independently.**
5. **ModViz consumes Stage Ops/Semantic Graph state; it does not create a second scene/resource truth.**
6. **Binary Inspector consumes bytes/regions/evidence; it is not a source resolver.**
7. **EXE Editor is a frontend over executable/recovered-source evidence, not a second reverse authority database.**
8. **The Runtime host layer owns platform, frame loop and rendering device only; it reads resources through GDSpaces and mirrors Stage Ops scene state.**

## Product layers

### Sources and GDSpaces

Sources expose mounted origins. GDSpaces owns logical resolution, source/volume selection, materialization, ByteProvenance, container expansion, WorkingCopy handoff and bounded authoring/publication contracts.

PAC, PNST and NBZ are supported internal container/archive layers. `.afs/` strings are currently logical namespace evidence; a dedicated binary AFS backend remains evidence-gated and must not be inferred from the names alone. Historical PACK product parsing likewise does not establish original DMC3 runtime authority.

### Reverse Core and Recovered Game Source Tree

Reverse Core owns generic artifact/range/function/type/claim/reconstruction/validation identities. Recovered Game Source Tree owns reconstructed DMC3 code and behavior, including resource runtime, scene lifecycle, HITS runtime and other original-game subsystems.

GDSpaces may reproduce confirmed behavior as safe product policy without moving original runtime ownership into product code.

### Stage Ops

Stage Ops consumes canonical resolved/materialized resources and executable-backed Stage descriptor/selector authority. It produces one `StageAssemblyWorkspace`/operational scene state, preserving technical resource-set identity, numeric Stage identity and semantic gameplay identity separately.

### Stage Semantic Graph

A deterministic evidence-aware representation/index over Stage Ops state. It does not resolve resources, parse archives or own scene orchestration.

### ModViz

Editor/visualization consumer over Stage Ops and Semantic Graph state. Edits flow back through revision/WorkingCopy/validation contracts.

### EXE Editor

Owns executable navigation/editing UX over exact artifact mappings, recovered-source identities, evidence and guarded patch/rebuild requests. Recovered source is not promoted from readability or compile success alone.

### Runtime host layer

Owns the playable-target host: surface lifetime, host lifecycle, the fixed-step frame loop and the rendering device abstraction. It exists so DMC Rengine has somewhere to *run* resources, on Android first and on desktop and further platforms after.

Both authority doors are closed by construction. `ResourceBridge` holds a `SourceRegistry` reference and is the only unit in the layer that reads bytes, so the runtime cannot become a second resolver. `StageHost` mirrors a Stage Ops `StageBundle` in bundle order and replaces it wholesale on rebind, so it cannot become a second scene truth.

The layer makes no behavioral claim about DMC3. It draws a stage member only when an owner supplies geometry for it, and every rendering backend except the reference `null` device is declared rather than implemented — creating one fails closed instead of silently substituting.

The runtime is a separate CMake target (`DMCRengine::Runtime`) on its own C++ standard, so the core library keeps its C++20 baseline. See [Specification 010](../specs/010-runtime-platform-foundation/spec.md).

## Current primary dependency

The current project critical path is [GDSpaces Layer 1](gdspaces/l1-roadmap.md):

```text
physical bytes -> exact materialization -> bounded edit/rebuild -> NBZ publication -> canonical reopen -> original-game consumption
```

Stage/editor feature work must not create private resource paths or displace mandatory L1 closure work without an explicit evidence dependency.

## Safety and publication

- source game files are immutable by default;
- WorkingCopy separates edits from source bytes;
- authored output uses explicit export/publication contracts;
- no-clobber means the final publication operation itself cannot replace an existing destination;
- evidence-grade archive/member receipts require artifact-stability binding across index/member/hash observation;
- outputs must not be published into a measured retail source tree by acquisition/evidence commands;
- product hardening is kept distinct from claims about original DMC3 malformed-input acceptance.

## Completion policy

A bounded parser, writer, recovered function, synthetic test or successful build may be complete at its own scope without making the containing subsystem complete. Formal subsystem completion requires its explicit acceptance gate and behavioral/evidence receipts.