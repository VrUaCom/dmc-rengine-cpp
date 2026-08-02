# Current Blockers

**Snapshot date:** 2026-08-02

## Resolved foundation blockers

### B-001 — Cross-platform CI visibility and validation

**Status:** resolved

GitHub Actions validation is fully visible through PR #1. Final Build #170 passed configure, build, and all tests on both Windows and Ubuntu.

Resolved findings included:

- corrected escaped-newline Evidence JSON test expectation;
- identified Release `NDEBUG` removal of side-effectful `assert` expressions on Windows;
- rejected the unreliable forced-include assertion workaround;
- enforced `/UNDEBUG` / `-UNDEBUG` for every test target.

### B-002 — No serialized evidence packet schema

**Status:** resolved for export; import remains a separate blocker

Implemented:

- versioned `EvidencePacket`;
- artifact identities;
- validation;
- deterministic JSON export;
- public canonical target Evidence Packet.

Remaining work is strict untrusted JSON import, tracked by issue #2.

### B-003 — Artifact hashing absent

**Status:** resolved

Implemented SHA-256, known-vector tests, CLI `hash`, artifact identity, guarded-patch hash checks, and known executable target matching.

### B-004 — Read-only PE inspection absent

**Status:** resolved at foundation level

Implemented generic PE32/PE32+ parsing, bounds checks, sections, image base, entry point, subsystem, checked offset/RVA/VA conversion, synthetic fixtures, CLI inspection, and DMC3 target recognition.

### B-006 — Working-copy and patch safety absent

**Status:** resolved at foundation level

Implemented revisioned `WorkingCopy`, expected-byte operations, undo/reset, source hash identity, and atomic fixed-size `GuardedPatchPlan`. Export/manifests remain future work.

### B-007 — Legacy evidence not normalized

**Status:** partially resolved

The first canonical DMC3 HD target packet now records the executable hash, PE identity, entry point, `dmc3_main`, stage resource table, and StageSet classifier metadata. Other historical findings still require packet-by-packet migration.

## Active blockers

### B-011 — Strict Evidence Packet JSON import

**Priority:** P0  
**Status:** open  
**Tracking:** issue #2

The exporter and schema model exist, but public evidence files cannot yet be parsed as untrusted input.

Required:

- strict parser diagnostics;
- size/depth/count limits;
- duplicate and reference validation;
- schema migration policy;
- round-trip and malformed-input tests;
- CLI evidence validation.

### B-012 — No read-only container source layer

**Priority:** P0  
**Status:** open  
**Tracking:** issue #3

Local directory sources work, but NBZ/AFS/PAC/PNST children are not yet exposed through generic GDSpaces container contracts.

Required:

- generic parser/result interfaces;
- stable slot/container child identity;
- synthetic fixtures;
- partial-failure diagnostics;
- parent/child graph integration;
- no writer support in this phase.

### B-013 — `st001` StageBundle is not game-backed yet

**Priority:** P1  
**Status:** open  
**Tracking:** issue #4

`StageBundle` and `StageBundleAssembler` are implemented, but the four EXE-backed `st001` resource roles are not yet resolved through container sources.

### B-014 — Binary Inspector field/annotation layer absent

**Priority:** P1  
**Status:** open  
**Tracking:** issue #5

Regions, ownership, coverage, unknown gaps, and conflicts exist. Typed fields, nested structures, annotations, owner lookup, and manifest export remain.

### B-015 — Evidence migration coverage is incomplete

**Priority:** P1  
**Status:** open

TXT parser helpers, Door/Box candidates, format schemas, Item Editor runtime findings, and EXE phases 13–16 remain prose/history rather than validated public packets.

Resolution: migrate one independently reproducible subsystem per packet and link it to code/tests.

### B-016 — No public synthetic container/format corpus

**Priority:** P1  
**Status:** open

Synthetic PE and lower-level binary fixtures exist, but PAC/PNST/NBZ/AFS and stage-format corpora are not yet implemented.

### B-017 — Export and release pipeline absent

**Priority:** P2  
**Status:** open

No production file exporter, patch manifest format, signed release artifact, or release automation exists. This remains intentionally deferred until container and validation contracts stabilize.

### B-018 — UI technology decision remains deferred

**Priority:** P2  
**Status:** deferred by ADR-0001

Qt/ImGui/rendering decisions remain blocked on stable container, Binary Inspector, and first StageBundle vertical slices. CLI/domain-first development continues.

## Current critical path

```text
Evidence import
  + container parser/source foundation
  → game-backed st001 StageBundle
  → Binary Inspector field/evidence integration
  → Stage Ops/ModViz migration
  → validated export pipeline
```
