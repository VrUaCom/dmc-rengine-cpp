# Current Blockers

**Snapshot date:** 2026-08-05  
**Snapshot base:** `main` at `1f77e2076a79216e015a3ddc83b1d1ed89c121c8`

This file lists blockers against the current reviewed product tree. Historical research gaps are not described as implementation gaps unless they still block a tested product path.

## Resolved blockers

### B-001 — Cross-platform CI visibility and validation

**Status:** resolved

Windows and Ubuntu validation is established. The latest reviewed Pass 32 promotion recorded 67/67 tests per platform and a final immutable review run before merge.

### B-002 — Evidence Packet serialization and strict import

**Status:** resolved

Implemented:

- versioned Evidence Packets;
- deterministic JSON export;
- strict untrusted JSON import;
- input/depth/count limits;
- duplicate-key and duplicate-ID rejection;
- artifact/location cross-reference validation;
- CLI `validate-evidence`;
- round-trip and malformed-input tests.

Issue #2 is closed as completed.

### B-003 — Artifact hashing absent

**Status:** resolved

SHA-256, known-vector tests, artifact identity, CLI hashing, target matching, guarded-patch hash checks, and build/reopen lineage are implemented.

### B-004 — Read-only PE inspection absent

**Status:** resolved at platform-foundation level

Generic PE32/PE32+ parsing, sections, checked file offset/RVA/VA conversion, target recognition, diagnostics, and executable workspace manifests are implemented.

### B-005 — Binary Inspector fields and selection context absent

**Status:** resolved at domain-foundation level

Typed fields, parent-child structures, annotations, owner lookup, selection context, conflict analysis, and deterministic manifests are implemented. Issue #5 is closed.

The full desktop hex/visual UI remains product work, not a blocker on the domain model.

### B-006 — Working-copy and patch safety absent

**Status:** resolved at integrated-workflow level

Implemented:

- revisioned `WorkingCopy`;
- exact expected-byte guards;
- source SHA-256 guards;
- overlap/range validation;
- evidence-gated patch compilation;
- in-memory copy execution;
- output hashing;
- verified rollback;
- provenance manifests stating that original-file writes were not performed.

### B-007 — No source-integration or rebuilt-output lineage model

**Status:** resolved as architecture/model

`SourceModificationPackage`, `IntegrationProject`, `CustomBuildIdentity`, `CustomBuildRecord`, source-to-binary mappings, test gates, distribution identity, and executable reopen lineage are implemented.

A real rebuilt DMC3 executable remains an open long-term milestone.

## Active blockers

### B-012 — Production read-only container source layer incomplete

**Priority:** P0  
**Status:** open  
**Tracking:** issue #3

The generic parser/source foundation is implemented with synthetic legal fixtures, stable child identity, empty-slot preservation, partial-failure diagnostics, and graph integration.

Still required:

- evidence-bounded production read-only PAC/PNST subset;
- NBZ/AFS source exposure through GDSpaces;
- nested child classification on real user-supplied resources;
- `.index` metadata linking without treating it as a runtime asset;
- sanitized malformed and corpus reports;
- deterministic local integration commands.

No writer support should be added before the read path and validation contracts stabilize.

### B-013 — `st001` StageBundle is not game-backed end-to-end

**Priority:** P0  
**Status:** open  
**Tracking:** issue #4

Implemented:

- confirmed 110 × 4 stage-table descriptor;
- `st001` role plan;
- path normalization and resource matcher;
- typed bundle assembler;
- Stage Workspace builder and manifests.

Still required:

- resolve the four roles from legally supplied local game data through production container sources;
- preserve partial failures and unknown children;
- generate a deterministic local report;
- prove one canonical resource identity is reused by all consumers.

### B-015 — Reverse research promotion coverage is incomplete

**Priority:** P1  
**Status:** open

The active product tree contains reviewed packets and code for selected systems, including canonical EXE evidence, Item runtime, HITS, and PC-save Pass 31/32.

Still outside reviewed product source:

- Wide Pass 33 payload semantics;
- most of Recovered Source Skeleton v1.8;
- much of the Phase 12–17 supporting reverse branch;
- additional TXT, Door/Box, texture, runtime ownership, and subsystem findings.

Policy: promote one independently reproducible subsystem at a time through Evidence Packet → reviewed C++ → tests → CI → provenance receipt. Do not bulk-import recovered skeleton snapshots.

### B-016 — Real public-safe corpus validation is incomplete

**Priority:** P1  
**Status:** open

Synthetic format and integration fixtures are strong, but broader sanitized reports from user-supplied legal resources remain limited.

Required:

- PAC/PNST/NBZ/AFS malformed and structural corpus;
- stage-level local reports;
- HITS original/candidate corpus comparison receipts;
- artifact hashes and evidence links without publishing copyrighted bytes.

### B-019 — HITS original-builder/runtime equivalence is unproven

**Priority:** P1  
**Status:** research required

The header-driven parser, runtime grid specification, safe editing, deterministic SAT writer, spatial differential validator, and local comparison CLI are implemented.

Still required before stronger compatibility claims:

- compare real source 0/source 1 corpora;
- explain original versus candidate cell ownership differences;
- controlled room-transition, restart, and reload tests;
- game-runtime validation manifests;
- explicit retention of `RESEARCH REQUIRED` for Capcom offline-builder equivalence.

The rejected `HITS$` and fixed record-marker model must not re-enter the project.

### B-020 — Wide Pass 33 is not promoted into reviewed product C++

**Priority:** P1  
**Status:** product-promotion-pending

Drive research records the `0x708` DetailedSlotPayload partition, including two six-record mode banks and the MissionResultMatrix at `+0x66C`.

Required:

- immutable Pass 33 artifact identities and authority checks;
- a strict Evidence Packet;
- conservative public C++ structures with open semantic labels where required;
- focused tests;
- Windows/Ubuntu CI;
- Drive/GitHub reconciliation receipt.

### B-021 — Recovered-source coverage remains partial

**Priority:** P1  
**Status:** open

Recovered Source Skeleton v1.8 exists in Drive, but only selected evidence-backed modules are promoted into active product source.

Required:

- per-unit authority and artifact identity;
- ABI, ownership, and lifetime review;
- compile isolation;
- behavioral comparison tests;
- rejection or correction records for invalid recovered units.

### B-022 — Full behavioral validation and recompilation frontier

**Priority:** P1 / long-term  
**Status:** open

The repository has strong provenance and custom-build models, but it does not yet contain a complete behaviorally equivalent DMC3 executable.

Required milestone order:

1. isolate a recovered subsystem;
2. compile it as reviewed C++;
3. compare behavior against the canonical executable;
4. record ABI/lifetime evidence;
5. establish a replacement/rebinding boundary;
6. only then claim an incremental recompilation milestone.

### B-017 — Production export and public release pipeline absent

**Priority:** P2  
**Status:** open

In-memory guarded copy execution and rollback are implemented. Missing:

- production output-file export contract;
- container repack path;
- release artifact signing/attestation automation;
- public binary packaging;
- reproducible release validation.

This remains intentionally behind container and game-backed integration work.

### B-018 — Complete desktop UI remains deferred

**Priority:** P2  
**Status:** deferred

Domain and shared-view contracts exist, but the complete Binary Inspector, Stage Ops, ModViz, and Item desktop interfaces are not implemented in this repository.

UI work must consume existing domain contracts and must not redefine resource identity, ownership, or write policy.

## Documentation and backlog reconciliation

This documentation refresh resolves the 2026-08-02 status drift for:

- strict Evidence import;
- Binary Inspector fields/annotations/owner lookup;
- integrated stage/item/workspace stack;
- guarded copy execution and rollback;
- source modification and custom-build lineage;
- HITS parser/runtime/writer/comparison work;
- PC-save Pass 31/32 promotion.

Issue #13 requires direct backlog reconciliation because its original body contains the rejected `HITS$` model.

## Current critical path

```text
production read-only PAC/PNST/NBZ/AFS subset
  → legal local game-backed st001 StageBundle
  → real stage/HITS corpus and runtime validation
  → narrow promotion of Pass 33 and other Drive research
  → behavior-tested recovered subsystems
  → controlled recompilation milestones
```

Production export and complete UI remain downstream of these evidence and integration gates.