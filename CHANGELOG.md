# Changelog

All notable changes to the clean C++ generation of DMC Rengine are documented here.

The project is pre-1.0. Historical reverse research is preserved in `docs/history/`, public Evidence Packets and Google Drive pass records. A historical record is not automatically current authority after explicit correction/supersession.

## [Unreleased]

### 2026-08-15 — Completion, evidence and ownership reconciliation

#### Added

- canonical `docs/status/completion-and-evidence-policy.md`;
- machine-readable status schema v2 with explicit `no-major-end-to-end-subsystem-complete` state;
- exact bounded statuses including `EXE CONFIRMED`, `BOUNDED CLOSED`, `VALIDATED`, `NOT PROVEN`, `CORRECTED`, `REJECTED` and strictly gated `COMPLETE`;
- explicit truth-layer separation between raw artifact/runtime evidence, Evidence Packets, recovered-game reconstruction, active PR heads, merged `main` and Drive research history;
- PR/evidence-submission template requirements for exact bounded claim, still-open scope, evidence provenance, fresh-artifact statement, truth-layer/promotion state and ValidationReceipt when equivalence is claimed;
- Google Drive canonical Completion & Evidence Policy and `Current Canon 2026-08-15` sheet in the reverse Knowledge Base.

#### Changed

- Constitution amended to **1.1.0**;
- governance/contribution/reverse rules now prohibit completion inflation and agent-consensus-as-evidence;
- root README, documentation index, architecture, cross-tool integration, GDSpaces contract, StageBundle assembly, roadmap, phase map, blockers, risks and canonical decisions reconciled to the live ownership model;
- `docs/stage/dmc3-stage-resource-plan.md` is now explicitly a historical `st001` compatibility slice rather than Stage architecture authority;
- Stage authority is catalog/selector driven: 110 Bank-A + 79 Bank-B = 189 observed descriptors, separate 193 selectors and 10 group-base pointers;
- resource-set/catalog identity, numeric Stage identity and semantic/gameplay identity remain separate;
- `st001` is regression/compatibility data only and is not an architectural or acceptance gate;
- Stage Ops is documented as product-side scene/stage assembly + operational authority; Semantic Graph is a derived projection; ModViz is an editor consumer;
- reconstructed original DMC3 runtime code ownership moved explicitly to the Recovered Game Source Tree; EXE Editor is a reconstruction/executable frontend and Reverse Core remains generic infrastructure;
- immutable `ResourceId` source identity is explicitly separated from mutable WorkingCopy byte size/revision and Binary Document/parser lineage.

#### HITS reverse correction

Later Pass-10 evidence in PR #85 supersedes older Pass-8/Pass-9 wording that the full upper P0 wrapper set remained open:

- `0x14005E7A0` bounded-closed at the combined-query wrapper contract;
- `0x14005B460` reclassified into the separate dynamic-world update pipeline;
- `0x14005FEC0` and `0x1400601E0` bounded-closed at their stated top-level contracts;
- later contact-normal semantics and primitive-descriptor ownership slices validated.

This does **not** make HITS/collision complete. Deeper primitive producers/helpers, type-5 source path, source2 backing/ownership/lifetime/live selection, controlled runtime comparison, modified-topology validation and Capcom offline-builder equivalence remain open.

#### Canonical executable evidence correction

- canonical project Evidence Packets are being corrected in PR #92 to pair SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082` with size `6,356,432` bytes;
- stale `3,735,552` pairing for the same SHA is corrected/rejected;
- a fresh independent re-hash/disassembly claim still requires the raw artifact to be actually mounted in that pass.

#### HITS format authority correction

PR #93 corrects current product metadata to:

- four-byte `HITS` magic;
- reject obsolete `HITS$` classification;
- reject `0x18060001` as a universal record marker;
- describe the deterministic DMC Rengine writer without claiming Capcom offline-builder equivalence.

#### Project-wide completion statement

As of 2026-08-15 **no major DMC Rengine end-to-end subsystem is `COMPLETE` or proven behaviorally equivalent to the original DMC3 runtime**.

This statement does not mean nothing works. Many bounded components are implemented, tested, EXE-confirmed, bounded-closed or validated.

### Existing implemented foundations

#### Core / evidence / executable

- C++20/CMake core library and CLI;
- Windows/Ubuntu CI;
- SHA-256 artifact identity;
- bounded binary/PE32/PE32+ readers and RVA/VA/file-offset mapping;
- Evidence Registry / Evidence Packets / strict JSON import-export;
- Artifact, executable workspace and guarded patch infrastructure;
- copied-output execution and rollback lineage;
- source modification / Integration Project / Custom Build lineage foundations.

#### GDSpaces / project integration

- `ResourceId`, `ResourceRef`, `ResourcePayload`, diagnostics and source registry;
- product path/format classification, resource graph and routing;
- generic container contracts and production-oriented container/source work in active stacks;
- revisioned WorkingCopy with guarded variable-size edits and undo/reset;
- ProjectWorkspace / events / manifests;
- Stage resource matching/materialization foundations.

#### Binary Inspector

- regions, fields, ownership, annotations and evidence links;
- selection context, coverage/unknown gaps and conflicts;
- deterministic manifests;
- byte diff and entropy analysis;
- structural format adapters.

#### HITS

- header-driven `HITS` parsing;
- exact `0x38` raw-flags + triangle/plane records;
- spatial grids/reference lists;
- source0/member3 and source1/member6 identity evidence;
- deterministic DMC Rengine spatial writer;
- topology-safe editing and spatial comparison/report tooling;
- substantial EXE-backed runtime query/source/primitive reverse evidence.

#### Stage / recovered runtime / Stage Ops

Active stacked PRs add substantially newer implementation than `main`, including:

- full 189-descriptor / 193-selector Stage authority and runtime resolution;
- recovered resource/Stage runtime units;
- StageAssemblyWorkspace / StageOpsIngress / StageOperationsSession;
- domain workspaces, runtime links and invalidation;
- Semantic Graph and ModViz Stage Ops projections;
- edit -> reanalysis -> derived-refresh regression paths.

These remain branch-scoped until deliberately promoted and do not establish vanilla game-ready equivalence.

#### Item / save / other bounded slices

- guarded Item runtime request/patch foundations;
- PC-save Pass 31/32 structural/checksum envelopes;
- format/parser/editor integration slices for HITS, DCA, LIG2, Stage TXT and other active areas.

### Current research/completion boundaries

- full GDSpaces DMC3 request-to-unload runtime equivalence remains open;
- typed post-load/factory/cache/ownership/lifetime/transition/unload closure remains open;
- representative Bank-A/Bank-B/shared/alias/partial Stage validation remains open;
- Stage Ops complete domain + vanilla lifecycle/game-ready equivalence remains open;
- Recovered Game broad behavioral equivalence and subsystem ValidationReceipts remain open;
- Reverse Core mature generic claim/reconstruction validation program remains open;
- HITS deeper primitive/source2/runtime/original-builder equivalence remains open;
- SCM `mesh+0x28` reconciliation remains open;
- Binary Inspector/EXE Editor/ModViz/Item-HUD complete product workflows remain open;
- production reintegration/export/release pipeline remains open;
- full DMC3 decompilation and behaviorally equivalent rebuilt executable do not exist today.

## [0.1.0] — 2026-08-02

### Added

- initial C++20/CMake foundation;
- minimal CLI;
- initial `ResourceId` model;
- cross-platform build workflow;
- initial architecture, roadmap, reverse-engineering rules and README;
- proprietary-data exclusions.
