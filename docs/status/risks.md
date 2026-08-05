# Architecture and Project Risks

**Snapshot date:** 2026-08-05  
**Snapshot base:** `main` at `1f77e2076a79216e015a3ddc83b1d1ed89c121c8`

## R-001 — Second resolver regression

**Severity:** critical

A subsystem may reintroduce direct path or container loading for convenience.

Mitigation:

- public tool contracts accept `ResourceRef`, `ResourcePayload`, shared workspaces, or typed bundles;
- parsers consume supplied byte spans rather than opening paths;
- architecture tests and review checklist;
- no filesystem handles in editor identity;
- GDSpaces-backed CLI loading;
- no tool-specific archive mount path.

## R-002 — Historical claim inflation

**Severity:** high

Old research summaries or recovered source may be interpreted as implemented, fully confirmed, or original Capcom source.

Mitigation:

- GitHub `main` is implementation truth;
- Drive is newer research input;
- explicit confidence and implementation states;
- Evidence Packets tied to artifact hashes and locations;
- corrections and rejected models remain visible;
- recovered C++ is described as independent evidence-backed reconstruction.

## R-003 — Parser vulnerabilities

**Severity:** high

Malformed binary or JSON inputs may trigger out-of-bounds reads, integer overflows, path traversal, excessive allocation, recursion abuse, or inconsistent partial state.

Mitigation:

- bounded binary reader;
- checked arithmetic and ranges;
- root-containment guards;
- strict JSON size/depth/count limits;
- deterministic diagnostics;
- synthetic malformed corpora;
- fuzzing after production parser interfaces stabilize.

## R-004 — Premature production write support

**Severity:** high

Adding archive or executable write-back before production source, validation, export, backup, and rollback contracts stabilize may corrupt user files.

Mitigation:

- immutable source payloads;
- revisioned working copies;
- copied-output execution only in the current patch path;
- exact source hash and expected-byte guards;
- verified rollback before accepting a copied output;
- no archive writer accepted without an evidence-backed specification and game validation.

## R-005 — Decompiled-code false confidence

**Severity:** high

Readable recovered C++ may hide wrong ABI, types, ownership, lifetimes, side effects, or behavior.

Mitigation:

- promote recovered units incrementally rather than bulk-importing snapshots;
- source units reference Evidence records and artifact identities;
- ABI, ownership, and lifetime assumptions remain explicit;
- compile isolation and behavioral comparison tests;
- correction/rejection history;
- no recompilation milestone without runtime evidence.

## R-006 — Public repository content contamination

**Severity:** high

Contributors may accidentally commit game binaries, extracted assets, saves, executable bytes, or leaked source.

Mitigation:

- `.gitignore` exclusions;
- clean-room and repository policies;
- synthetic public fixtures;
- user-supplied legal local files only;
- PR and issue confirmations;
- future artifact scanning in CI;
- immediate removal procedure.

## R-007 — Architecture monolith

**Severity:** medium

GDSpaces or Project Workspace could become oversized because they coordinate broad resource and provenance responsibilities.

Mitigation:

- modular internal services for sources, identity, classification, containers, graph, routing, diagnostics, working copies, events, and manifests;
- small typed public contracts;
- domain services remain independent consumers;
- no UI-specific state in core resource services.

## R-008 — UI drives domain design

**Severity:** medium

Visual workflows may redefine resource identity, ownership, write policy, or project state around widgets.

Mitigation:

- domain APIs and CLI vertical slices precede complete GUI implementation;
- Stage Ops and ModViz consume shared Stage Workspace state;
- Binary Inspector UI must consume the existing document model;
- Item UI produces runtime requests rather than direct patches.

## R-009 — AI-generated churn

**Severity:** medium

High-volume generated code or documentation may create duplicated abstractions, stale claims, unsupported names, or accidental scope expansion.

Mitigation:

- generate → triage → test → correct → review → Canon;
- narrow branches and PRs;
- explicit non-goals and evidence boundaries;
- machine-readable status and deterministic manifests;
- human review before promotion.

## R-010 — GitHub/Drive/status drift

**Severity:** high

GitHub code, GitHub status documents, Drive research, recovered-source snapshots, issues, and implementation receipts may diverge.

Observed example:

- the 2026-08-02 status snapshot still described strict Evidence import and Binary Inspector fields as open after those systems were implemented;
- Drive advanced to Wide Pass 33 while GitHub product code remained reviewed through Pass 32;
- issue #13 retained the rejected `HITS$` model after the canonical HITS architecture was merged.

Mitigation:

- GitHub `main` is product implementation truth;
- Drive is newest research truth;
- explicit `research-ready` versus `product-promotion-pending` states;
- status refresh after significant merges;
- Drive/GitHub implementation receipts;
- issue reconciliation when a model is corrected;
- append-only decision and correction records.

## R-011 — Scope dispersion

**Severity:** medium

Simultaneous work on containers, stages, EXE recovery, HITS, Item, ModViz, UI, save semantics, and recompilation may prevent game-backed vertical completion.

Mitigation:

- prioritize production container read path and one game-backed `st001` slice;
- require demonstrable vertical results;
- keep research promotion narrow;
- avoid bulk source-tree migration;
- maintain explicit critical path and exit gates.

## R-012 — HITS compatibility overclaim

**Severity:** high

A structurally valid DMC Rengine SAT writer may be presented as equivalent to Capcom's unknown offline builder.

Mitigation:

- label the writer as a DMC Rengine deterministic policy;
- retain `RESEARCH REQUIRED` for original-builder equivalence;
- compare real corpus spatial ownership;
- emit exact/missing/extra cell and surface reports;
- perform controlled game-runtime, transition, restart, and reload tests;
- separate structural validity from game verification.

## R-013 — Rejected HITS model regression

**Severity:** high

Stale documents or issues may reintroduce `HITS$`, `0x18060001` as a universal marker, or the false AABB interpretation.

Mitigation:

- canonical parser uses `HITS` header-driven topology;
- records are raw flags plus triangle A/B/C, normal, and plane D;
- issue #13 must be reconciled;
- tests and docs preserve corrected assumptions;
- no parallel HITS parser or resolver.

## R-014 — Executable-build mismatch

**Severity:** high

Canonical VAs or expected bytes may be applied to a different executable build.

Mitigation:

- exact executable SHA-256 and size;
- parsed PE metadata checks;
- evidence locations scoped to artifact identity;
- expected-byte guards;
- explicit excluded-build records;
- no automatic rebasing or cross-build patch claim.

## R-015 — Generic container foundation overclaim

**Severity:** medium to high

Synthetic generic container contracts may be mistaken for completed production PAC/PNST/NBZ/AFS compatibility.

Mitigation:

- status distinguishes generic foundation from production source implementations;
- issue #3 remains open;
- real format support requires evidence-bounded parsers and sanitized local reports;
- no writer/repack claim before read paths stabilize.

## R-016 — Recovered-source bulk import

**Severity:** high

Importing Recovered Source Skeleton snapshots wholesale may place stale, speculative, uncompilable, or ABI-incorrect units into active product source.

Mitigation:

- treat snapshots as research inputs;
- hash and identify immutable source packages;
- promote one subsystem at a time;
- require Evidence Packets, code review, tests, CI, and provenance receipts;
- preserve unpromoted units outside the product authority tree.

## R-017 — Recursive source discovery hides accidental production inclusion

**Severity:** medium

Recursive CMake source globbing simplifies integration but may silently compile newly added `.cpp` files.

Mitigation:

- review changed-file lists in every PR;
- require test registration and explicit module documentation;
- consider moving mature subsystems to explicit target source lists when repository scale makes accidental inclusion harder to audit.

## R-018 — Incomplete release and distribution boundary

**Severity:** medium

Custom-build identity and in-memory patch execution may be mistaken for a finished public build/release system.

Mitigation:

- distinguish architecture/model from produced artifacts;
- no release claim without deterministic output, package validation, signing/attestation, rollback, and runtime testing;
- keep public binary release explicitly incomplete.

## R-019 — Trademark and community confusion

**Severity:** low to medium

Lore or naming may imply affiliation, a real religious group, leaked source, or a completed engine replacement.

Mitigation:

- explicit fictional-brand and non-affiliation statements;
- accurate completion language;
- independent-research wording;
- no representation of recovered units as original Capcom source.
