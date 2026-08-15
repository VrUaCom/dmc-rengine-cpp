# Roadmap

**Status rule:** roadmap phases describe workstreams and bounded gates. They do not imply that a major subsystem is complete unless `docs/status/completion-and-evidence-policy.md` is satisfied.

**Current synchronization cutoff:** merged `main` at `25bd70092503cc6ca3be51f05582dcd33af1523d` after PR #98; active work includes #96/#97/#99.

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
- canonical structural PAC path through shared `ContainerDocument`
- representative legal real-PAC corpus validation across multiple resource families
- PNST/NBZ/AFS and recursive container path reconciliation in the current generation
- `.lst` grammar/recursion/ownership recovery
- typed post-load/factory/cache/lifetime reconstruction bridge
- representative request-to-unload validation receipts
- write/repack/export only after structural/runtime evidence gates

PR #99 is the current PAC implementation unit. It is a structural parser slice, not full PAC or issue-#3 completion. PAC slot index is container/schema-scoped evidence and must never become a global semantic ID.

**Exit:** do not call GDSpaces/DMC3 resource runtime complete until representative Level-E lifecycle equivalence is validated.

## Phase 2 — Evidence / Reverse Core / EXE tooling

- artifact SHA/size/PE authority
- binary regions and ownership
- RVA/VA/file-offset mapping
- evidence packets and reconciliation
- function/type/reconstruction identities
- task claims for parallel agents
- source hash and expected-byte guards
- hash-gated exact executable byte-window acquisition
- deterministic acquisition receipts that bind artifact/range/window identity
- recovered-source compile pipeline
- controlled original-vs-reconstruction behavioral comparison
- ValidationReceipts

Merged PR #98 satisfies the bounded acquisition primitive: exact expected-SHA-gated, read-only, deterministic PE VA window extraction. It does not prove function identity, ABI, semantics or behavioral equivalence.

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

PR #91 current-head branch CI is green at 106/106 on Ubuntu and Windows. This validates its tested scope only; Stage Ops still requires broader domain/runtime/lifecycle evidence and deliberate promotion.

`st001` is only a regression fixture and is never the architectural exit gate.

## Phase 4 — HITS / collision reconstruction

Completed/validated bounded layers already include:

- verified HITS file structure
- spatial reconstruction and deterministic product writer
- source0/source1 ownership
- upper wrapper ABI closure/reclassification for E7A0/B460/FEC0/601E0 at bounded scope
- primitive shape/contact/descriptor evidence through Slice 10
- Stage-CFG PAC provenance through Slice 12
- serialized collision-triplet view Slice 13
- Stage-CFG entry/descriptor view Slice 14
- referenced Stage-CFG descriptor census tooling Slice 15

### Current HITS data gate — Slice 15 / PR #96

- run a representative real Stage-CFG referenced-descriptor census through canonical resource/PAC provenance
- preserve resource-set, numeric-stage and source identity in receipts
- do not claim real type-5 presence or absence before actual census evidence
- if referenced type 5 appears, follow that exact descriptor to its authored/population path

### Current HITS EXE reverse gate — Slice 16 / PR #97

- reacquire full `0x1400594B0` body/callers/slot38 dataflow using merged PR #98 when the legal canonical executable is locally available
- trace modern route around `0x14009823F`
- trace legacy observed route around `0x1400B6483`
- complete C630/C740 caller census by manager/source identity
- identify exact producer/base/object/bounds/count/lifecycle of C8D0 stack arg5 -> runtime `+0x20`
- keep `transform_selector_bounds_available() == false` until direct evidence closes it
- do not add a slot38 transform parser or Stage-CFG three-table adapter from inference

After Slice 16:

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
  -> use merged PR #98 acquisition for exact new EXE windows when raw artifact is available
  -> resource lifecycle/factory/cache closure
  -> recovered-game bounded behavioral receipts
  -> representative Stage Catalog validation
  -> Stage Ops complete domain/runtime bridge
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

Run three evidence-heavy tracks in parallel without collapsing their ownership:

1. HITS Slice 16 — `0x1400594B0` / slot38 / C8D0 transform-source provenance;
2. HITS Slice 15 data gate — representative real Stage-CFG referenced-descriptor census;
3. GDSpaces PR #99 — representative real-PAC multi-family validation through the canonical container path.
