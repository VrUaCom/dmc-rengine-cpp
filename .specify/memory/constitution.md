# DMC Rengine Constitution

**Version:** 1.1.0  
**Ratified:** 2026-08-02  
**Amended:** 2026-08-15 via completion/evidence reconciliation PR #94

## Article I — One Product Resource Law

GDSpaces is the single product-side resource-access, identity, resolution, materialization and provenance authority.

All product tools receive stable resource identities/references/payloads/diagnostics/graphs and selected Stage resource state through GDSpaces contracts. No editor, Stage Ops, Semantic Graph, ModViz, Binary Inspector or recovered-source tool may independently create a second game-folder/archive/path/container resolver.

GDSpaces does **not** own reconstructed original DMC3 runtime functions, factories, caches, object lifetimes, collision behavior or other original-game code.

## Article II — Evidence Before Canon

Every reverse-engineering claim must declare exact scope, evidence status and provenance. Direct executable/runtime/corpus evidence is tied to exact artifact identity. Corrections preserve prior history and explicitly supersede earlier claims.

AI output, decompiler output, agent consensus, intuition, naming convenience and synthetic-test success are investigation inputs, not evidence by themselves.

If raw artifact bytes were not actually available in a pass, prior Evidence Packets/summaries must not be described as a fresh independent re-hash/re-disassembly.

## Article III — Bounded Closure Before Completion

DMC Rengine distinguishes bounded technical closure from whole-subsystem completion.

Accepted statuses include, as applicable:

- `HYPOTHESIS` / `CANDIDATE` / confidence grades;
- `EXE CONFIRMED` / `DERIVED FROM VERIFIED RUNTIME`;
- `IMPLEMENTED` / `TESTED`;
- `BOUNDED CLOSED` / `VALIDATED`;
- `RESEARCH REQUIRED` / `NOT PROVEN`;
- `CORRECTED` / `REJECTED`;
- `COMPLETE` only after the full applicable subsystem gate.

A parser, writer, wrapper ABI, recovered unit, Stage Ops slice or green CI run may be fully valid for its bounded scope while the containing major subsystem remains incomplete.

A major subsystem may be called `COMPLETE` only after its defined scope, artifact/evidence coverage, implementation, representative corpus/runtime behavior, applicable lifecycle, cross-platform implementation tests, deterministic ValidationReceipt and deliberate canonical promotion gates are satisfied.

As of the 2026-08-15 amendment, no major DMC Rengine end-to-end subsystem is `COMPLETE` or proven behaviorally equivalent to the original DMC3 runtime.

## Article IV — Read-Only / Immutable Source First

New source and format support begins read-only. Original source payloads remain immutable by default.

Write capability requires explicit WorkingCopy/revision, validation, capacity/conflict rules, output ownership, backup/manifest/rollback and tests.

A `ResourceId` identifies immutable source identity. A WorkingCopy may legally change active byte length/revision; parser/Binary Document/derived state must track the exact active byte lineage rather than mutating source identity.

Original game files must never be silently modified.

## Article V — Guarded Executable Changes

Every executable patch requires, as applicable:

- exact artifact SHA/build identity and size;
- source/expected bytes;
- target bytes;
- file offset and/or RVA/VA mapping;
- semantic purpose;
- supported version/build scope;
- dependencies/conflicts;
- rollback data;
- runtime validation appropriate to the changed behavior.

No patch or address claim may be silently rebased onto a different executable build.

## Article VI — Clean Public Repository

The repository contains original implementation, public documentation, sanitized evidence metadata and synthetic fixtures. It does not contain proprietary game binaries/assets, extracted copyrighted payloads, leaked source, credentials or unauthorized distributions.

Legally obtained game data stays local.

## Article VII — Recovered Game Source Ownership

C++20 is the durable reconstruction language, but **reconstructed original DMC3 game code belongs in the Recovered Game Source Tree**, not in GDSpaces, Stage Ops, ModViz, Binary Inspector or Reverse Core.

Recovered source is independent evidence-backed reconstruction. It is not accepted as original source or equivalent behavior because it is readable or compiles.

Behavioral equivalence requires exact artifact evidence, ABI/ownership/lifetime boundaries, controlled original-vs-reconstruction comparison and a reproducible ValidationReceipt for the claimed scope.

## Article VIII — Reverse Core Is Generic Infrastructure

Reverse Core owns reusable reverse-engineering identities/workflows such as artifact, range, function, data object, recovered type, evidence, hypothesis, experiment, TaskClaim, reconstruction, subsystem and ValidationReceipt.

Reverse Core must remain game-agnostic. DMC3 gameplay/resource/collision runtime code belongs in the Recovered Game Source Tree.

Parallel agents must claim/narrow overlapping file/function/type/subsystem scopes so they do not race or create duplicate authorities. Task ownership or agent consensus is not evidence.

## Article IX — Responsibility Boundaries

- **GDSpaces** owns product resource access/identity/resolution/materialization/provenance.
- **Recovered Game Source Tree** owns reconstruction of original DMC3 runtime code/behavior.
- **Reverse Core** owns generic reverse/evidence/reconstruction/claim/validation infrastructure.
- **EXE Editor** is an executable/reconstruction editing frontend over shared Reverse Core/recovered-game/evidence authority.
- **Binary Inspector** owns structural/ownership/evidence views over exact supplied byte lineage.
- **Stage Ops** owns product-side Stage/scene assembly and operational workspace state.
- **Stage Semantic Graph** is a derived representation/index over Stage Ops state, not scene assembly authority.
- **ModViz** owns interactive scene/asset/HUD editing views over Stage Ops state, not resource or scene resolution.
- **Format editors** own typed interpretation/edit commands, not source discovery.
- **Build & Test** owns validation orchestration, guarded modification checks, behavioral comparison receipts and release gates.

Tool consumption never transfers semantic ownership of original-game functions.

## Article X — Stage Identity Is Catalog/Selector Based

Stage architecture may not be defined by `st001`, `stNNN` filename templates, or by assuming one resource descriptor equals one gameplay Stage.

The current Wave-2 authority distinguishes 189 observed resource descriptors (110 Bank A + 79 Bank B), a separate 193-entry selector space and a separate 10-pointer group-base table.

The project must preserve independently:

1. resource-set/catalog-entry identity;
2. numeric Stage selector identity;
3. semantic/gameplay Stage/room/variant identity only when separately evidenced.

`st001` is regression/compatibility data only and is never the architectural Stage or completion gate.

## Article XI — Product Materialization Is Not Vanilla Game-Ready State

Resolved/materialized/parsed Stage resources or a StageBundle do not automatically equal original DMC3 typed-postload/factory/cache/consumer-ready state.

Stage Ops may assemble a valid product workspace while `game_ready_equivalent` remains false.

Original-game ready/lifecycle claims require recovered-runtime evidence and representative behavioral validation.

## Article XII — Truth Layers Must Remain Distinct

Do not collapse:

- raw canonical artifact/runtime observations;
- sanitized Evidence Packets/reconciliation records;
- recovered-game reconstructions;
- active PR/branch implementation truth;
- merged GitHub `main` implementation truth;
- Google Drive historical/newer research material.

Green active PRs are not merged-main truth. Historical documents are not current authority after explicit supersession. Raw direct evidence outranks summaries for the exact artifact.

## Article XIII — Specifications and Tests

Non-trivial work requires a specification/issue/claim with problem, scope, non-goals, ownership, evidence boundary, risks, acceptance criteria and tests.

Code existence is never a sufficient completion argument. Tests validate the tested contract; original-game behavior requires the appropriate behavioral receipt.

Documentation/status/issues must be synchronized when evidence or implementation changes current authority.

## Article XIV — Correctness Over Compatibility With Legacy Mistakes

The clean repository may break compatibility with undocumented/stale internals when required to preserve canonical architecture and evidence.

Proven behavior may be migrated. Deprecated resolver, ownership, identity or overclaim patterns may not.

Historical research is preserved through `CORRECTED` / `REJECTED` / supersession records instead of silent deletion.

## Article XV — Lore Is Optional

Lore/community language cannot define permissions, replace governance, override evidence status, obscure ownership boundaries or imply affiliation/original-source provenance.

## Amendment Process

Amendments require a dedicated reviewed pull request explaining motivation, compatibility impact, migration and consequences for existing specifications/status. The Constitution version and amendment date must be updated.
