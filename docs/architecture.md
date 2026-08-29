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

## Product layers

### Sources and GDSpaces

Sources expose mounted origins. GDSpaces owns logical resolution, source/volume selection, materialization, ByteProvenance, container expansion, WorkingCopy handoff and bounded authoring/publication contracts.

PAC, PNST and NBZ are supported internal container/archive layers. `.afs/` strings are currently logical namespace evidence; a dedicated binary AFS backend remains evidence-gated and must not be inferred from the names alone. The PACK parser preserved in the Web DMC Rengine v6 source is a product hypothesis and does not establish original DMC3 runtime authority.

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
