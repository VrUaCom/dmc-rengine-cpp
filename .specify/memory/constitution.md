# DMC Rengine Constitution

**Version:** 1.0.0  
**Ratified:** 2026-08-02

## Article I — One Resource Law

GDSpaces is the single public resource-access architecture.

All tools receive stable resource identities, references, payloads, diagnostics, graphs, and typed bundles through GDSpaces contracts. No editor may independently resolve game folders, NBZ, AFS, PAC, PNST, or nested resource identity.

## Article II — Evidence Before Canon

Every reverse-engineering claim must declare confidence and provenance. Confirmed findings require reproducibility against an identified artifact. Corrections preserve prior history and explicitly supersede earlier claims.

AI output, decompiler output, intuition, and naming convenience are inputs to investigation—not evidence by themselves.

## Article III — Read-Only First

New source and format support begins read-only. Write capability requires a separate specification covering working copy, validation, capacity, conflicts, backup, manifest, rollback, and tests.

Original game files must never be silently modified.

## Article IV — Guarded Executable Changes

Every executable patch requires:

- artifact hash;
- source bytes;
- target bytes;
- file offset and/or RVA/VA mapping;
- semantic purpose;
- supported versions;
- dependencies and conflicts;
- rollback data;
- runtime validation.

## Article V — Clean Public Repository

The repository contains original implementation, public documentation, sanitized evidence metadata, and synthetic fixtures. It does not contain proprietary game binaries/assets, leaked source, credentials, or unauthorized distributions.

## Article VI — C++ Source Recovery

C++20 is the durable implementation language for the clean generation. Recovered source is independent reconstruction. It is not accepted as equivalent behavior until interfaces, ABI, ownership, lifetime, and tests support it.

## Article VII — Responsibility Boundaries

- GDSpaces owns resource access and identity.
- EXE Editor owns executable evidence, recovered units, and guarded patch planning.
- Binary Inspector owns structural/ownership views over supplied bytes and regions.
- Stage Ops owns stage workflows over typed bundles.
- ModViz owns scene/model and menu/HUD visual editing.
- Format editors own typed interpretation and edits, not source resolution.
- Build & Test owns reproducibility and validation.

## Article VIII — Specifications and Tests

Non-trivial features require a specification with problem, scope, non-goals, architecture, risks, acceptance criteria, and tests.

A feature is not complete because code exists. Completion requires passing tests, synchronized documentation/status, and compliance with this Constitution.

## Article IX — Correctness Over Compatibility With Legacy Mistakes

The clean repository may break compatibility with undocumented legacy internals when required to preserve the canonical architecture. Proven behavior may be migrated; deprecated ownership and resolver patterns may not.

## Article X — Lore Is Optional

The Order of the Inverted Triangle, Monks of Reverse, and Sect of Neuroslop are fictional brand language. They cannot define permissions, replace governance, or obscure technical communication.

## Amendment Process

Amendments require a dedicated pull request explaining motivation, compatibility impact, migration, and consequences for existing specifications. The Constitution version must be updated.
