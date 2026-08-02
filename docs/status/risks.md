# Architecture and Project Risks

## R-001 — Second resolver regression

**Severity:** critical

A subsystem may reintroduce direct path/container loading for convenience.

Mitigation:

- public tool contracts accept `ResourceRef`/`ResourcePayload`/typed bundles;
- architecture tests and review checklist;
- no filesystem handles in editor identity;
- dependency direction documented in every specification.

## R-002 — Historical claim inflation

**Severity:** high

Old research summaries may be interpreted as implemented or fully confirmed.

Mitigation:

- migration ledger;
- explicit confidence and implementation status;
- evidence packets tied to artifact hashes;
- corrections remain visible.

## R-003 — Parser vulnerabilities

**Severity:** high

Malformed binary inputs may trigger out-of-bounds reads, overflows, path traversal, or excessive allocation.

Mitigation:

- bounded reader abstraction;
- checked arithmetic;
- size budgets;
- synthetic malformed corpus;
- fuzzing after parser interfaces stabilize.

## R-004 — Premature write support

**Severity:** high

Adding archive or EXE write-back before working-copy and validation contracts can corrupt user files.

Mitigation: read-only by default; no writer accepted without patch/export specification, backup, rollback, and tests.

## R-005 — Decompiled-code false confidence

**Severity:** high

Readable generated C++ may hide wrong ABI, types, ownership, or behavior.

Mitigation:

- decompilation units reference evidence;
- ABI and lifetime assumptions are explicit;
- behavioral comparison tests;
- source recovery progresses by isolated subsystems.

## R-006 — Public repository content contamination

**Severity:** high

Contributors may accidentally commit game binaries, extracted assets, or leaked source.

Mitigation:

- `.gitignore` exclusions;
- clean-room policy;
- PR/issue confirmations;
- artifact scanning in future CI;
- immediate removal procedure.

## R-007 — Architecture monolith

**Severity:** medium

GDSpaces could become a single oversized class because it owns broad resource responsibilities.

Mitigation: modular internal services—sources, identity, classification, container expansion, graph, router, diagnostics, working copy—behind one public resource law.

## R-008 — UI drives domain design

**Severity:** medium

Hex cells, panels, or visual workflows may define data ownership prematurely.

Mitigation: domain APIs and CLI vertical slices precede GUI implementation.

## R-009 — AI-generated churn

**Severity:** medium

High-volume generated code/docs may create inconsistency, duplicated abstractions, or false claims.

Mitigation: Sect of Neuroslop pipeline—generate, triage, test, correct, review, then Canon.

## R-010 — Memory/document drift

**Severity:** medium

Git, Obsidian, MemPalace, SDD, and status reports may diverge.

Mitigation:

- repository status JSON as public source of implementation truth;
- append-only decision/correction records;
- scheduled reconciliation;
- tools exchange explicit events rather than overwriting each other.

## R-011 — Scope dispersion

**Severity:** medium

Simultaneous work on GDSpaces, EXE, Stage Ops, ModViz, Item Editor, UI, and recompilation may prevent vertical completion.

Mitigation: phase exits and one demonstrable vertical slice per milestone.

## R-012 — Trademark/community confusion

**Severity:** low to medium

Lore or naming may imply affiliation, a real religious group, or a completed game-engine replacement.

Mitigation: explicit fictional-brand and non-affiliation statements; accurate status language.
