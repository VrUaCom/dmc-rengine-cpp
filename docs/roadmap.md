# Roadmap

**Status rule:** roadmap phases describe workstreams and bounded gates. They do not imply that a major subsystem is complete unless `docs/status/completion-and-evidence-policy.md` is satisfied.

## Phase 0 — Foundation

- C++20 and CMake baseline
- cross-platform CI
- stable module/ownership boundaries
- evidence and status vocabulary
- no proprietary game data in the repository

Foundation gates can be satisfied while the project remains incomplete.

## Phase 1 — GDSpaces resource authority

- stable resource/source identities
- read-only source mounting
- provider normalization and lookup
- container-chain representation
- byte provenance
- format classification
- resource graph and diagnostics
- OpenRouter contracts
- runtime-equivalent source/lookup policies where evidenced
- typed post-load/factory/cache/lifetime reconstruction bridge
- representative request-to-unload validation receipts

**Exit:** do not call GDSpaces/DMC3 resource runtime complete until representative Level-E lifecycle equivalence is validated.

## Phase 2 — Evidence / Reverse Core / EXE tooling

- artifact SHA/size/PE authority
- binary regions and ownership
- RVA/VA/file-offset mapping
- evidence packets and reconciliation
- function/type/reconstruction identities
- task claims for parallel agents
- source hash and expected-byte guards
- recovered-source compile pipeline
- controlled original-vs-reconstruction behavioral comparison
- ValidationReceipts

**Exit:** one or more bounded reconstructed subsystems pass evidence + compile + behavioral comparison without relying on agent consensus.

## Phase 3 — Full Stage Catalog and Stage Ops

- EXE-backed descriptor/selector identity
- Bank A + Bank B descriptor universe
- numeric selector/group indirection
- keep resource-set, numeric and semantic identities separate
- arbitrary catalog-entry resolution through GDSpaces
- StageAssemblyWorkspace
- domain assembly and unresolved-state handling
- recovered-runtime lifecycle/factory links
- Stage Semantic Graph projection
- ModViz projection
- representative Bank-A/Bank-B/shared/alias/partial receipts

`st001` is only a regression fixture and is never the architectural exit gate.

## Phase 4 — HITS / collision reconstruction

- verified HITS file structure
- spatial reconstruction and deterministic product writer
- source0/source1 ownership
- bounded query ABI reconstruction
- primitive descriptor/producer/helper reconstruction
- source2 backing/lifetime closure
- runtime trace comparison
- modified-topology game validation
- original-builder equivalence only if actually proven

Pass-10 wrapper closure does not equal collision completion.

## Phase 5 — Editors

- Binary Inspector complete interaction/workflow integration
- EXE Editor reconstruction views
- Stage format editors
- Item/HUD editor slices
- ModViz Scene/Model Editor
- ModViz Menu/HUD Editor
- evidence/source navigation
- revision-guarded edits through shared WorkingCopies

Editors must never create second resource/scene/reconstruction authorities.

## Phase 6 — Safe export and reintegration

- WorkingCopies
- validation
- guarded patch plans
- deterministic manifests
- rollback
- container reintegration/repack only after evidence gates
- mod package generation
- original-file protection
- representative game validation

## Phase 7 — Progressive decompilation and recompilation

- evidence-backed recovered C++ units
- ABI/ownership/lifetime recovery
- isolated replacement modules
- behavioral comparison harness
- subsystem ValidationReceipts
- replacement/rebinding boundaries
- deterministic composite builds
- progressive recompilation toward a working executable
- controlled full-game validation

A fully decompiled, behaviorally equivalent rebuilt DMC3 executable is a long-term exit gate and does not exist today.

## Current execution priority

```text
artifact/evidence integrity
  -> resource lifecycle/factory/cache closure
  -> recovered-game bounded behavioral receipts
  -> representative Stage Catalog validation
  -> Stage Ops complete domain/runtime bridge
  -> HITS deeper primitive/source2/runtime validation
  -> validated editor/export verticals
  -> progressive recompilation milestones
```
