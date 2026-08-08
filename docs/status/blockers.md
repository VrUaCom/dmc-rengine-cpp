# Current Blockers

**Snapshot date:** 2026-08-08  
**Reviewed `main` baseline before this documentation branch:** `6eb6a07975753e2bbe9414893a76e13c946fa78e`

This register distinguishes implementation blockers, research-required gates, and external infrastructure concerns. Planned work is not represented as implemented.

## Resolved foundation blockers

The following are no longer project-foundation blockers:

- cross-platform Windows/Ubuntu CI visibility;
- SHA-256 artifact identity;
- Evidence Packet serialization and strict untrusted import;
- bounded PE32/PE32+ inspection and offset/RVA/VA conversion;
- Binary Inspector regions/fields/ownership/annotations/range context;
- Binary Inspector Wave 1 diff and entropy domain support;
- revisioned working copies and guarded patch foundations;
- copied-output execution and verified rollback;
- source modification / Custom Build lineage models;
- selected HITS and PC-save Pass 31/32 promotion.

## Active blockers

### B-012 — Production read-only container source layer incomplete

**Priority:** P0  
**Status:** open  
**Tracking:** issue #3

The generic container foundation is implemented, but production evidence-bounded PAC/PNST/NBZ/AFS exposure through GDSpaces remains incomplete.

Required:

- production read-only PAC/PNST subset;
- NBZ/AFS source exposure;
- nested child classification and diagnostics;
- `.index` metadata linking without treating it as runtime truth;
- sanitized local corpus reports;
- deterministic integration commands.

Do not add broad production writer/repack behavior before the read path and validation contracts stabilize.

### B-013 — `st001` StageBundle is not game-backed end-to-end

**Priority:** P0  
**Status:** open  
**Tracking:** issue #4

Stage descriptors, matching, assembly, and workspace contracts exist, but one legal local `st001` bundle has not yet been demonstrated through production sources with canonical identity reuse across all consumers.

### B-023 — Reverse Core canonical object contract is not implemented

**Priority:** P0  
**Status:** architecture accepted / implementation pending

The project currently has many required ingredients but lacks one durable shared contract spanning executable bytes, functions, data, types, hypotheses, reconstruction revisions, and validation receipts.

Required v0.1 objects:

`BinaryArtifact`, `AddressRange`, `Function`, `DataObject`, `RecoveredType`, `EvidenceRecord`, `Hypothesis`, `Experiment`, `TaskClaim`, `Reconstruction`, `ValidationReceipt`, `Subsystem`.

Without this contract, EXE Editor, Binary Inspector, automation, and recovered-source work can drift into parallel identity stores.

### B-024 — Parallel-agent ownership/claim protocol is absent

**Priority:** P0  
**Status:** open

Multiple agents can analyze the same bytes, but canonical reconstruction mutation needs negotiated ownership. The MCP/Kanban workflow does not yet expose a formal `TaskClaim` contract for function/range/type/subsystem ownership.

Required:

- claim identity and scope;
- owner/task/spec linkage;
- conflict detection;
- release/supersession/block states;
- deterministic audit history;
- explicit distinction between work ownership and Evidence status.

### B-025 — Recovered-source tree/export contract is absent

**Priority:** P0  
**Status:** open

Recovered source exists as research input and selected promoted units, but there is no canonical tree contract that an agent can populate and export as a normal VS Code/CMake-ready project while preserving per-unit provenance.

Required:

- subsystem-oriented source layout;
- stable reconstruction IDs;
- binary/range provenance;
- ABI/lifetime assumptions;
- evidence links;
- tests and ValidationReceipts;
- deterministic export manifest.

### B-026 — First complete reconstruction validation loop is absent

**Priority:** P0  
**Status:** open

No real DMC3 subsystem has yet completed the full loop:

`binary -> stable reverse identities -> recovered C++ -> isolated build -> behavioral comparison -> ValidationReceipt`.

This is the principal gate before mass decompilation.

### B-015 — Reverse research promotion coverage is incomplete

**Priority:** P1  
**Status:** open

Wide Pass 33, most Recovered Source Skeleton v1.8 units, and substantial Phase 12–17 runtime/resource research remain outside reviewed product source.

Policy remains narrow promotion:

`immutable artifact -> Evidence Packet -> reviewed C++ -> tests -> CI -> provenance receipt`.

Do not bulk-import recovered source snapshots.

### B-016 — Real public-safe corpus validation is incomplete

**Priority:** P1  
**Status:** open

Synthetic fixtures are strong, but broader sanitized reports from legally supplied local resources remain limited for production containers, stages, and runtime behavior.

### B-019 — HITS original-builder/runtime equivalence is unproven

**Priority:** P1  
**Status:** research required

The corrected header-driven parser, runtime grid behavior, safe edits, deterministic SAT writer, round trips, and spatial comparison tooling are implemented.

Still required:

- real source 0/source 1 corpus comparison;
- explanation of original/candidate cell ownership differences;
- controlled room-transition/restart/reload tests;
- runtime validation manifests.

Capcom offline-builder equivalence must remain `RESEARCH REQUIRED` until this evidence exists.

### B-020 — Wide Pass 33 is not promoted into reviewed C++

**Priority:** P1  
**Status:** product-promotion-pending

Requires immutable authority artifacts, strict Evidence Packet, conservative C++ structures, focused tests, Windows/Ubuntu CI, and provenance reconciliation.

### B-021 — Recovered-source coverage remains partial

**Priority:** P1  
**Status:** open

Each promoted recovered unit still requires per-unit authority, ABI/ownership/lifetime review, compile isolation, behavioral comparison, and explicit correction/rejection handling.

### B-022 — Full recompilation frontier remains open

**Priority:** P1 / long-term  
**Status:** open

The repository does not contain a complete behaviorally equivalent rebuilt DMC3 executable. Controlled replacement/rebinding and deterministic composite-build milestones remain downstream of the first validated subsystem island.

### B-027 — Stage Semantic Graph is not assembled from game-backed data

**Priority:** P1  
**Status:** planned / blocked by B-012 and B-013

The target semantic graph links stage, room, geometry, collision, lighting, camera, transitions, events, effects, audio, and runtime references. Existing typed format/workspace pieces are not yet assembled into one game-backed semantic graph with evidence-bearing edges.

### B-028 — ModViz Red Orb vertical slice is not complete

**Priority:** P1/P2  
**Status:** planned

The Menu Editor architecture is accepted, but the first end-to-end Red Orb counter workflow has not yet demonstrated resource edit + EXE evidence/guarded runtime request + preview + validation + export through shared contracts.

### B-017 — Production export and public release pipeline absent

**Priority:** P2  
**Status:** open

Missing production output-file export, container repack integration, release attestation/signing automation, public binary packaging, and reproducible release validation.

### B-018 — Complete desktop UI remains deferred

**Priority:** P2  
**Status:** deferred

Complete Binary Inspector, Stage Ops, ModViz, and Item desktop interfaces remain downstream of domain, evidence, and vertical-slice gates.

## External platform infrastructure concern

### B-029 — MCP desktop bootstrap logging and signed installer key dependency

**Priority:** external / infrastructure  
**Status:** open outside the DMC3 reverse core

The desktop bootstrap path can start the MCP server without the same stdout/stderr redirection used by the PowerShell launcher, reducing diagnostic visibility. Correcting that path requires rebuilding/reinstalling the native bootstrapper. The release pipeline also depends on access to the private RSA signing key used for signed installer production.

This does **not** block static DMC3 reverse engineering or reviewed C++ library work. It can block reliable distribution/reinstallation and diagnostics for the broader local Triangle Forge/MCP coordination environment, so it is tracked here only as an external dependency.

## Current critical path

```text
B-012 production GDSpaces read path
  -> B-013 game-backed st001
  -> B-023 Reverse Core schema
  -> B-024 TaskClaim ownership
  -> B-025 recovered-source tree
  -> Binary Inspector / EXE Editor bridges
  -> B-026 first complete reconstruction validation loop
  -> broader semantic/runtime validation
  -> controlled recompilation milestones
```
