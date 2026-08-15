# Canonical Project Decisions

This document records current project law until explicitly superseded through evidence/governance. Historical corrections remain visible rather than being silently erased.

For completion terminology, `docs/status/completion-and-evidence-policy.md` is authoritative.

## CD-001 — GDSpaces is the single product resource authority

**Status:** confirmed, active

All product tools consume resources through GDSpaces contracts. GDSpaces owns product source/resource identity, lookup/resolution, materialization, provenance, classification, container expansion, resource graph/diagnostics, routing and WorkingCopy resource boundaries.

GDSpaces does **not** own reconstructed original DMC3 factories, caches, lifetimes, collision runtime or other original-game code.

## CD-002 — Containers are internal resource layers

**Status:** confirmed, active

PAC, PNST, NBZ and AFS are resource/container layers inside GDSpaces, not top-level editor products. Stage and non-Stage containers use the same generic resource principles; semantic Stage meaning is layered above stable resource identity.

Legacy PAC Editor/PAC Manager top-level architecture is rejected.

## CD-003 — No second resolver

**Status:** confirmed, active

Stage Ops, Semantic Graph, ModViz, Binary Inspector, Item/HUD editors and other tools may not create independent path/container/resource resolution systems.

Direct path/file-handle state is not canonical resource identity.

## CD-004 — Identity is layered

**Status:** confirmed, active

Distinguish source identity, logical path, container/slot lineage, immutable source byte span, canonical resource ID, presentation labels and evidence-backed executable/semantic identities.

Display names never replace canonical IDs.

## CD-005 — Evidence precedes Canon

**Status:** confirmed, active; vocabulary extended by CD-015

Reverse conclusions require explicit evidence/status and reproducibility. Corrections/rejections preserve history.

Agent/model consensus is not evidence.

## CD-006 — Recovered C++ is durable reconstruction, not EXE-Editor-owned game code

**Status:** corrected, active

Earlier wording treated EXE Editor as the owner of recovered source. The corrected ownership is:

- reconstructed original DMC3 game code lives in the **Recovered Game Source Tree**;
- Reverse Core owns generic reconstruction/evidence identities;
- EXE Editor is a frontend/editor over those shared authorities.

Readable or compilable recovered C++ is an evidence-backed executable specification, not automatically original Capcom source or behaviorally equivalent code.

## CD-007 — Binary Inspector is an evidence/structure consumer

**Status:** confirmed, active

Binary Inspector receives exact byte lineage, stable resource/artifact identity, structures, ownership, diagnostics and evidence. It does not independently resolve sources.

For edited data it must track the exact active WorkingCopy revision/byte view.

## CD-008 — Stage Ops owns product-side stage/scene assembly

**Status:** corrected/expanded, active

Earlier wording described Stage Ops mainly as a consumer of typed bundles. The current responsibility is broader:

- Stage Ops consumes selected/materialized GDSpaces Stage data plus recovered-runtime links;
- it owns `StageAssemblyWorkspace`, product scene/domain assembly and operational state;
- it coordinates edit/reanalysis/invalidation/validation state;
- it exposes stable Stage state to Semantic Graph and ModViz.

Stage Ops does not resolve resources independently and does not own original DMC3 runtime code.

## CD-009 — ModViz is an editor consumer, not scene authority

**Status:** confirmed/corrected, active

ModViz provides Scene/Model and Menu/HUD editing workflows over shared Stage Ops/resource/evidence state. It does not build a competing Stage scene model or perform direct source/EXE writes.

## CD-010 — Writes are explicit and guarded

**Status:** confirmed, active foundation / full production reintegration incomplete

Original source bytes remain immutable by default. Edits require WorkingCopy/revision identity, validation, conflict/capacity checks, manifests, explicit copied-output/export targets and rollback.

Executable modifications additionally require exact artifact identity, expected bytes/ranges, address mapping and runtime validation appropriate to the change.

## CD-011 — Public repository remains content-clean

**Status:** confirmed, active

The public repository contains original implementation, documentation, sanitized evidence metadata and synthetic fixtures, not proprietary game files, leaked source or redistributed assets.

## CD-012 — The C++ repository is not a blind legacy port

**Status:** confirmed, active

Legacy/private functionality is migrated only when ownership, evidence, architecture fit and tests are understood. Existing code is not canonical merely because it already exists.

## CD-013 — Knowledge systems have distinct responsibilities

**Status:** confirmed, active workflow

Human documentation, machine-readable evidence/graphs, long-term memory, SDD/specifications, Drive research and Git implementation remain distinct layers with explicit synchronization/supersession rather than silent overwrite.

## CD-014 — Brand lore never overrides engineering language

**Status:** confirmed, active

Lore/community names are presentation. APIs, schemas, status, evidence, permissions, governance and security use direct technical identities.

## CD-015 — Bounded closure does not imply subsystem completion

**Status:** confirmed, active

Use the narrowest truthful state: hypothesis/candidate/confidence, EXE confirmed, runtime-derived, implemented, tested, bounded closed, validated, research required/not proven, corrected/rejected.

`COMPLETE` for a major subsystem is reserved for the full applicable completion gate, including representative evidence/runtime/lifecycle validation and a deterministic ValidationReceipt where equivalence is claimed.

Green CI, parser/writer round trips, compiled recovered C++, or one closed ABI do not establish whole-subsystem completion.

As of 2026-08-15 no major DMC Rengine end-to-end subsystem is `COMPLETE` or proven equivalent to the original DMC3 runtime.

## CD-016 — Recovered Game Source Tree owns original-runtime reconstruction

**Status:** confirmed, active

Original DMC3 runtime functions/types/behavior reconstructed from executable evidence belong under `recovered-game/...`.

This includes, when recovered, resource typed post-load/factories/cache/lifetime, collision runtime, Stage consumers and other original systems.

Tool consumption does not transfer ownership of those functions into GDSpaces, Stage Ops, ModViz or Binary Inspector.

## CD-017 — Reverse Core is generic reverse infrastructure

**Status:** confirmed, active direction

Reverse Core owns generic identities/workflows such as artifact, address range, function, data object, recovered type, evidence, hypothesis, experiment, task claim, reconstruction and ValidationReceipt.

It must remain reusable across games/projects and must not become a DMC3-specific gameplay-code tree.

Parallel agents use claims/coordination to avoid racing on the same function/type/file. Consensus is not evidence.

## CD-018 — Semantic Graph is a projection of Stage Ops

**Status:** confirmed, active

Stage Semantic Graph represents/indexes assembled Stage Ops state and evidence relationships. It does not resolve resources, traverse archives, instantiate original runtime objects or assemble a second scene.

## CD-019 — Stage identity has three independent axes

**Status:** confirmed, active

The current Wave-2 Stage authority distinguishes:

- 110 Bank-A observed descriptors;
- 79 Bank-B observed descriptors;
- 189 observed descriptors total;
- separate 193-entry selector space;
- separate 10-pointer group-base table;
- numeric group/remainder selector indirection.

Keep separate:

1. resource-set/catalog-entry identity;
2. numeric Stage identity;
3. semantic/gameplay Stage identity only when separately evidenced.

189 descriptors are not automatically 189 gameplay stages. `st001` is regression/compatibility data only and is not the Stage architecture or completion gate.

## CD-020 — Materialized Stage data is not original game-ready state

**Status:** confirmed, active

GDSpaces materialization/container expansion and product `StageBundle`/Stage Ops assembly must remain distinct from original DMC3 typed post-load, factory construction, cache/ownership/lifetime and consumer-ready state.

`game_ready_equivalent` cannot become true solely because bytes were found/parsed/assembled.

## CD-021 — Source identity is immutable; WorkingCopy byte lineage is revisioned

**Status:** confirmed, active

`ResourceId` represents immutable source identity/span. A WorkingCopy may change active byte size/revision while retaining that source identity.

Parser/Binary Document/derived Stage state must track the active WorkingCopy byte source and revision rather than requiring immutable source size equality after insert/delete edits.

## CD-022 — Truth layers remain separate

**Status:** confirmed, active

Do not collapse:

- raw canonical artifact/runtime evidence;
- sanitized Evidence Packets/reconciliation;
- recovered-game reconstruction;
- branch-scoped PR implementation;
- merged GitHub `main` implementation;
- Google Drive research history.

A green active PR is not merged-main truth. A historical Drive pass is not current authority after explicit supersession. Raw direct evidence outranks summaries for the exact artifact.

## CD-023 — Historical reverse records are preserved through supersession

**Status:** confirmed, active

When later evidence closes/corrects/rejects an older target:

- preserve the historical record;
- add correction/supersession links/notices;
- update current coordination/status surfaces;
- do not restart the old target merely because an older document still lists it as open.

Reopening a bounded-closed claim requires contradictory/new evidence, not status drift.
