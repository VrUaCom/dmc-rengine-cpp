# DMC3 Reverse Data Authority and Synchronization Architecture

## Status

Canonical development rule for the DMC3 reverse-engineering stream.

Target executable:

```text
dmc3.exe
SHA-256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
```

Reconciliation state on 2026-08-03:

- Drive and Sheets authority roles are defined;
- Process Control and Knowledge Base late-row schema drift is corrected;
- historical Pass 31 Git receipt `784b06b7...` is superseded;
- reviewed replacement merge is `76ed3b7a02ee83a6285834495ac0e4c9f84845e3`;
- Windows and Ubuntu each passed 58 of 58 tests for the migrated Pass 31 package;
- Drive and Sheets readback is recorded in Process Control.

## Why this exists

The DMC Rengine Drive contains several mature but overlapping systems:

- a process-control spreadsheet;
- a normalized research knowledge base through Wide Pass 31;
- an older reverse registry;
- full reports and eight-step pass documentation;
- raw pass archives and source evidence;
- recovered C/C++ source snapshots;
- the active GitHub C++ product repository.

These are not interchangeable copies. Each system has a different authority role. Treating all of them as writable sources of truth causes stale pass state, duplicated findings, unverifiable Git receipts and recovered source that cannot be reproduced from evidence.

## Canonical authority split

### 1. DMC3_Reverse_Process_Control

Role: workflow state and operational control.

It owns:

- the required eight reverse-pass steps;
- pass status and closure state;
- runtime-test planning and results;
- decisions and corrections;
- sync and snapshot receipts;
- artifact completeness.

It does not prove binary semantics by itself. A Process Control row may reference a finding, but confirmation must resolve to the Knowledge Base and evidence artifacts.

### 2. DMC3_Reverse_Knowledge_Base_through_Pass31

Role: canonical structured index of current research knowledge.

It owns:

- pass summaries;
- findings and confidence/status;
- open gaps;
- RTTI and inheritance relationships;
- globals and registries;
- artifact indexes and overview metrics.

A Knowledge Base row is an index, not raw proof. A confirmed row must resolve to one or more of:

- exact EXE/file locations;
- a full pass report;
- a raw or derived evidence artifact;
- a machine-readable Evidence Packet;
- a reviewed recovered source unit and tests;
- an existing Git commit when a Git receipt is declared.

The Passes sheet now has explicit Date, Recovered Source, Drive Sync, Git Receipt and Repository Paths columns for late passes. Pass 29-31 rows were normalized without deleting their historical meaning.

### 3. DMC3_Reverse_Registry

Role: historical pre-Pass-23 registry.

Status: archived, read-only.

It must not receive new findings and must not be used by product code as the current knowledge base. Historical rows remain useful for provenance and correction tracking.

### 4. Pass Artifacts and Source Evidence

Role: human-readable and raw evidence store.

It owns:

- complete pass reports;
- the eight required pass documents;
- verified ZIP/raw packages;
- runtime samples;
- analysis outputs;
- evidence indexes.

Drive presence alone is not sufficient for product promotion. Hashes, sizes and exact source relationships must be recorded. Artifact Checklist now distinguishes a present/readable file from a hash-verified immutable artifact.

### 5. Recovered C_CPP Source Tree

Role: historical recovered-source input.

The Drive tree preserves pass-specific recovered source and source skeleton versions. It is not automatically equivalent to reviewed product source.

Promotion into `VrUaCom/dmc-rengine-cpp` requires:

1. an exact target identity;
2. a supporting Evidence Packet;
3. conservative names for unresolved fields;
4. isolated tests;
5. cross-platform CI;
6. a Git commit and pull-request receipt;
7. Drive synchronization and readback.

### 6. VrUaCom/dmc-rengine-cpp

Role: canonical machine-readable product output.

It owns:

- reviewed C++ implementation;
- tests and validation tools;
- machine-readable evidence;
- schemas and manifests;
- source/build lineage;
- CI and merge receipts.

GitHub must not invent findings that are absent from the evidence system. Drive must not declare a Git receipt that cannot be resolved in the repository.

## Correct synchronization pipeline

```text
Scope and target identity
→ static analysis journal
→ evidence validation and confidence
→ Knowledge Base / architecture update
→ open gaps and runtime-test plan
→ artifact manifest
→ reviewed GitHub evidence + C++ + tests
→ CI receipt
→ Drive sync and readback
→ Process Control closure
```

The previous eight-step documentation process remains valid. GitHub promotion and CI are explicit between artifact creation and Drive closure.

## Reconciliation results — 2026-08-03

### Confirmed strengths

- The Knowledge Base is normalized into Passes, Findings, Open Gaps, RTTI, Inheritance, Globals & Registries, Artifacts and Overview.
- It records Wide Passes 1-31 and indexes current findings, gaps, RTTI, inheritance, globals/registries and artifacts.
- Process Control provides an explicit eight-step Definition of Done, runtime tests, correction log, snapshots and artifact completeness.
- Drive contains the native Pass 31 report, steps 01-08, complete ZIP and Recovered Source Skeleton v1.6 / 210-file tree.
- Pass 31 is evidence-rich: canonical EXE hash, real save hash, exact file layout, validation function, confirmed offsets and explicit open semantic gaps.

### Resolved corrections

1. **Legacy duplication contained.** `DMC3_Reverse_Registry` is now explicitly archived read-only and is not a live mirror.
2. **Process Control schema drift corrected.** Pass 29-31 control rows and late Artifact Checklist rows were aligned to declared columns. Presence is no longer mislabeled as hash verification.
3. **Knowledge Base schema drift corrected.** Late Passes rows gained explicit columns; actual repository paths and reviewed artifact receipts were added.
4. **Historical Git receipt superseded.** Unresolvable receipt `784b06b7...` and its nonexistent paths were replaced by merge `76ed3b7a02ee83a6285834495ac0e4c9f84845e3` and seven existing repository outputs.
5. **Confirmed Pass 31 source gap closed.** The exact PC save physical layout, 21-block integrity ABI, packed-BCD fields and conservative semantic boundary were promoted into C++, tests and machine-readable evidence.

### Remaining blockers

1. **Recovered source is only partially promoted.** Drive has a 210-file v1.6 tree, but only evidence-reviewed modules should enter active product source.
2. **Immutable Drive artifact hashes are missing.** Pass 29/30 archives and Pass 31 archive, step package and source tree are present but not SHA-256 verified in the artifact checklist.
3. **Pass 32 semantics remain open.** The `0x708` payload, several summary/header fields, trailer states beyond 0/1 and the exact production writer require controlled differential saves and runtime tracing.
4. **Some long-lived Docs have stale opening headers.** Consumers must use the latest explicit canonical-state section, not assume the first header is current.

## Corrected development priorities

### P0 — Keep authority and evidence synchronized

- write findings to the canonical Knowledge Base, not the legacy Registry;
- write pass state and closure receipts to Process Control;
- require actual repository paths and existing commit receipts;
- record binary hashes before marking Drive artifacts Verified;
- require Drive readback after GitHub merge.

### P1 — Pass 32 controlled save research

The next reverse-engineering target is not a speculative save editor. It is controlled differential-save research for:

- header field semantics;
- summary fields `+0x0C`, `+0x10`, `+0x14`;
- semantic segmentation and ownership of the `0x708` payload;
- trailer status values beyond `0/1`;
- the exact integrity writer and update lifecycle.

Do not introduce semantic editing of unresolved regions before evidence closes them.

### P2 — Incremental recovered-source migration

Migrate Drive source units by subsystem and evidence coverage. Do not import the 210-file skeleton as one unreviewed dump.

Each migrated unit must link:

```text
Drive artifact
→ Knowledge Base finding/gap
→ Evidence Packet
→ reviewed C++ source
→ tests
→ Git commit/PR
→ Drive sync receipt
```

### P3 — Continue product infrastructure behind evidence gates

Custom Build Identity, EXE reopen lineage and GDSpaces range-readable sources remain valid directions. They must consume the canonical evidence/source model rather than become parallel registries.

The 173,038,003-byte `DMCL-0.nbz` confirms the need for range-readable GDSpaces sources. This remains a product requirement after the Pass 31 reconciliation.

## Non-negotiable constraints

- DMC HD Vanilla remains isolated from DMC3 Reverse Engineering.
- Unknown fields keep neutral names and open-gap records.
- Corrections supersede older claims; historical evidence is preserved.
- A spreadsheet cell, document sentence or source skeleton name is not independently sufficient proof.
- A Git SHA is valid only when the commit exists in the declared repository.
- Pass closure requires both machine-readable output and Drive readback.
