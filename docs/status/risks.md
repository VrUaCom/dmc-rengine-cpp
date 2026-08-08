# Architecture and Project Risks

**Snapshot date:** 2026-08-08  
**Reviewed `main` baseline before this documentation branch:** `6eb6a07975753e2bbe9414893a76e13c946fa78e`

## R-001 — Second resolver regression

**Severity:** critical

A subsystem may reintroduce direct path/container loading for convenience.

Mitigation: public tool contracts use GDSpaces identities/payloads/workspaces; parsers consume supplied byte spans; no tool-specific archive mount path; architecture review/tests enforce one resource authority.

## R-002 — Historical claim inflation

**Severity:** high

Research summaries or recovered source may be misrepresented as implemented, confirmed, or original Capcom source.

Mitigation: explicit implementation/evidence states, Evidence Packets, correction/rejection history, exact artifact provenance, and controlled promotion into reviewed C++.

## R-003 — Parser vulnerabilities

**Severity:** high

Malformed binary/JSON inputs may trigger bounds, overflow, traversal, allocation, recursion, or inconsistent-state bugs.

Mitigation: bounded readers, checked arithmetic, containment guards, parser limits, deterministic diagnostics, malformed corpora, and later fuzzing.

## R-004 — Premature production write support

**Severity:** high

Archive/executable write-back before read/validation/export/rollback contracts stabilize can corrupt user files.

Mitigation: immutable source payloads, working copies, copied-output execution, expected-byte/source-hash guards, manifests, rollback, and evidence-backed writer specifications.

## R-005 — Decompiled-code false confidence

**Severity:** high

Readable C++ may still have wrong ABI, types, ownership, lifetime, side effects, or behavior.

Mitigation: per-unit provenance, explicit assumptions, isolated builds, behavioral comparisons, validation receipts, and correction/rejection history.

## R-006 — Public repository content contamination

**Severity:** high

Game binaries/assets/saves/extracted proprietary bytes or leaked source may be committed accidentally.

Mitigation: clean-room rules, synthetic fixtures, ignore/scanning policy, local user-supplied legal resources, and removal procedure.

## R-007 — Architecture monolith

**Severity:** high

Triangle Forge, Reverse Core, GDSpaces, or Project Workspace could absorb unrelated domain responsibilities and become an unmaintainable monolith.

Mitigation: explicit layering; Reverse Core remains game-agnostic; GDSpaces remains resource-specific; DMC semantics remain in DMC Rengine; UI state stays outside core identity/evidence services.

## R-008 — UI drives domain design

**Severity:** medium

Visual workflows may redefine resource identity, ownership, write policy, or reconstruction state around widgets.

Mitigation: domain/CLI contracts precede full GUI; Stage Ops/ModViz share workspaces; Binary Inspector consumes the document model; Item/ModViz produce typed requests rather than direct unguarded patches.

## R-009 — AI-generated churn

**Severity:** high

High-volume generated code/docs may introduce duplicated abstractions, stale claims, invented compatibility, or scope expansion.

Mitigation: generate -> triage -> evidence/test -> correct -> review -> Canon; narrow specs/PRs; machine-readable status; deterministic manifests; human-reviewed promotion.

## R-010 — GitHub/research/status drift

**Severity:** high

Reviewed code, status docs, issues, Drive research, recovered-source snapshots, and implementation receipts can diverge.

Mitigation: GitHub `main` is implementation truth; research sources remain explicitly research-ready/promotion-pending; status refresh after major merges; append-only correction records; issue reconciliation.

## R-011 — Scope dispersion

**Severity:** high

Parallel work on containers, stages, EXE recovery, HITS, Item, ModViz, save semantics, Reverse Core, and recompilation may prevent a complete vertical proof.

Mitigation: maintain one critical path and exit gates; prioritize Resource Identity/game-backed stage plus first validated recovered subsystem; keep research promotion narrow.

## R-012 — HITS compatibility overclaim

**Severity:** high

A structurally valid DMC Rengine SAT writer may be described as equivalent to Capcom's unknown offline builder.

Mitigation: preserve `RESEARCH REQUIRED`, compare real corpus ownership, publish differential receipts without copyrighted bytes, and perform controlled runtime/restart/reload tests.

## R-013 — Rejected HITS model regression

**Severity:** high

Stale notes may reintroduce rejected `HITS$`, universal-marker, guard-dword, or false spatial interpretations.

Mitigation: corrected header-driven model in code/tests/docs; no parallel HITS parser; explicit corrected/rejected Canon records.

## R-014 — Executable-build mismatch

**Severity:** high

Known VAs/bytes may be applied to the wrong executable build.

Mitigation: exact SHA-256/size/PE metadata, artifact-scoped evidence locations, expected-byte guards, excluded-build records, and no implicit rebasing.

## R-015 — Generic container foundation overclaim

**Severity:** medium to high

Synthetic generic container contracts may be mistaken for production PAC/PNST/NBZ/AFS compatibility.

Mitigation: status separates foundation from production source implementations; real support requires evidence-bounded parsers and local reports; writers stay downstream.

## R-016 — Recovered-source bulk import

**Severity:** high

Wholesale import of recovered-source snapshots may place speculative or ABI-incorrect units into active product source.

Mitigation: immutable package identity, one-subsystem-at-a-time promotion, Evidence Packets, compile/test/behavior gates, and provenance receipts.

## R-017 — Recursive source discovery hides accidental inclusion

**Severity:** medium

Recursive build discovery may compile newly added source unintentionally.

Mitigation: changed-file review, explicit module docs/test registration, and later explicit source lists for mature subsystems.

## R-018 — Incomplete release/distribution boundary

**Severity:** medium

Custom-build identity or copied-output patching may be mistaken for a production release system.

Mitigation: no release claim without deterministic output, package validation, signing/attestation, rollback, and runtime testing.

## R-019 — Brand/community confusion

**Severity:** low to medium

Lore or Triangle Forge language could be mistaken for engineering identities or imply affiliation/completion claims.

Mitigation: technical names remain authoritative; fictional brand language never overrides APIs, evidence status, permissions, or legal statements.

## R-020 — Reverse Core becomes a second DMC Rengine

**Severity:** critical

If Reverse Core absorbs DMC-specific concepts, it stops being reusable and duplicates domain services.

Mitigation:

- Reverse Core schema only covers generic binary/reverse objects;
- DMC resources/gameplay semantics remain in DMC Rengine;
- GDSpaces handles game resources;
- architecture review rejects Red Orb/stage/HITS/item concepts from the reusable core.

## R-021 — Duplicate reverse identity stores

**Severity:** critical

EXE Editor, Binary Inspector, agents, and recovered-source tooling may each assign independent function/type/evidence identities.

Mitigation:

- one Reverse Core ID contract;
- bridges/adapters instead of duplicated stores;
- deterministic manifests and cross-reference validation;
- no editor-local canonical function database.

## R-022 — Agent race on canonical reconstruction

**Severity:** high

Parallel agents may edit the same function, type, range, or source unit and silently overwrite/cross-contaminate results.

Mitigation:

- `TaskClaim` ownership protocol;
- claim scope/conflict detection;
- release/supersession history;
- explicit independent-analysis mode;
- canonical mutation only through negotiated ownership/review.

## R-023 — Consensus masquerades as evidence

**Severity:** high

Multiple AI agents may converge on the same wrong interpretation and incorrectly increase confidence.

Mitigation:

- work claims and model agreement are never Evidence;
- promotion requires artifact/runtime/test provenance;
- competing hypotheses remain explicit;
- confirmation rules live in specs/acceptance criteria.

## R-024 — Recovered C++ drifts from binary truth

**Severity:** critical

Source can become cleaner and more maintainable while silently losing the behavior/ABI represented by the binary.

Mitigation:

- per-function/range provenance;
- reconstruction revisions linked to exact artifact identity;
- ABI/lifetime assumptions recorded;
- isolated builds and behavioral comparison;
- ValidationReceipt required for promotion;
- correction/rejection when behavior diverges.

## R-025 — Path-based evidence fragmentation

**Severity:** high

The same logical resource encountered through filesystem, NBZ/AFS/PAC/PNST, or extracted copies may accumulate separate identities/evidence.

Mitigation: GDSpaces Resource Identity v1, layered source/container identity, stable `ResourceId`, and consumer reuse across Stage Ops/ModViz/Binary Inspector/Reverse Core links.

## R-026 — Semantic graph inference becomes hidden truth

**Severity:** medium to high

Stage Semantic Graph edges inferred from names or proximity may be mistaken for confirmed runtime relationships.

Mitigation: evidence/confidence on semantic edges, preservation of unknown nodes, deterministic manifests, and explicit distinction between structural containment and inferred runtime semantics.

## R-027 — Vertical-slice bypass in ModViz

**Severity:** medium

Pressure to make the Menu Editor visually impressive may encourage direct archive or EXE manipulation outside shared contracts.

Mitigation: Red Orb slice must use GDSpaces identity, working copy, EXE/Reverse Core evidence, guarded patch requests, validation, and explicit export before feature breadth expands.

## R-028 — External MCP release infrastructure obscures diagnosis

**Severity:** medium / external

Native bootstrap logging differences and unavailable private signing material can make local platform failures hard to diagnose or distribute even while DMC C++ work is healthy.

Mitigation: keep this dependency separate from DMC3 evidence status; restore stdout/stderr capture in the native bootstrap path; document signing-key custody and release requirements; never mark core reverse work blocked solely because installer packaging is unavailable.
