# Phase Map

**Snapshot date:** 2026-08-15  
**Policy:** phase gates describe bounded maturity; they do not imply that the containing major subsystem or the whole project is complete.

Read `completion-and-evidence-policy.md` first.

## Phase 0 — Public C++ foundation

**Status:** foundation gate satisfied / maintained, project still incomplete.

Implemented:

- C++20/CMake core and CLI;
- Windows/Ubuntu CI;
- governance, security, clean-room policy;
- evidence/status vocabulary;
- repository and build foundations.

This is a bounded platform-foundation gate only.

## Phase 1 — Evidence and resource contracts

**Status:** foundation gate satisfied / expanding.

Implemented foundation includes artifact identity, Evidence records/packets, strict import/export, `ResourceId`/`ResourceRef`/`ResourcePayload`, diagnostics, source registry, graph and routing contracts.

Still open at project level: global content-addressed evidence consistency, broader reverse coverage, subsystem receipts and lifecycle-backed promotion.

## Phase 2 — Read-only platform and EXE inspection

**Status:** substantial partial implementation.

Implemented includes bounded readers, PE32/PE32+ parsing, offset/RVA/VA conversion, target recognition and executable evidence/workspace contracts.

Still open: broad function/type recovery, complete EXE Editor workflows, full decompilation and behavior-tested reconstructed subsystems.

## Phase 3 — Production resource/container runtime

**Status:** active / not complete.

Implemented across active stacks includes generic containers, production-oriented source/provenance work, NBZ ZIP indexing/materialization slices, path normalization, source lookup, runtime resolver composition and recursive expansion.

Still required for DMC3 runtime equivalence:

- complete PAC/PNST/NBZ/AFS evidence-bounded behavior;
- complete request/source/fallback semantics;
- typed post-load/factory/cache/lifetime/unload closure;
- representative game-backed ValidationReceipts.

## Phase 4 — Stage Catalog and identity

**Status:** strong partial reverse/implementation / not complete.

Current Wave-2 authority:

- Bank A: 110 observed descriptors;
- Bank B: 79 observed descriptors;
- 189 observed descriptors total;
- 193 selector entries;
- 10 group-base pointers;
- numeric `stageId / 100` and `% 100` selector/group indirection.

Identity axes remain separate: resource-set/catalog identity, numeric Stage identity, and separately evidenced semantic/gameplay identity.

`st001` is a regression fixture, not the architectural stage target.

Exit still requires representative Bank-A/Bank-B/shared/alias/partial validation plus lifecycle linkage and semantic evidence where claimed.

## Phase 5 — Binary Inspector

**Status:** substantial domain implementation / not complete.

Implemented: regions, fields, ownership, annotations, evidence links, selection, coverage/conflicts, diff, entropy, manifests and format adapters.

Still open: persistent cache, broader diagnostics/templates/unknown analysis, complete EXE/patch bridges and final native interaction/UI.

## Phase 6 — Working copy, guarded modification and source/build lineage

**Status:** bounded safety foundations implemented / full production flow not complete.

Implemented: immutable source + revisioned WorkingCopy, expected-byte guards, guarded patch plans, copied-output execution, rollback, provenance, source modification packages and custom-build lineage models.

Still open: complete production output/reintegration, container repack, full editor export and release validation.

## Phase 7 — Recovered Game Source Tree

**Status:** selected executable slices implemented/tested / not complete.

Selected recovered units compile and run deterministic tests in active branches. Evidence levels differ per unit: direct reconstructed, disassembly-complete/corpus-pending, executable candidate, or research required.

Still open: broad ABI/body coverage, ownership/lifetime closure, controlled original-vs-reconstruction behavioral comparisons and subsystem ValidationReceipts.

## Phase 8 — Stage Ops / Semantic Graph

**Status:** substantial branch-scoped implementation / not complete.

PR #91 implements Stage assembly/operations, shared WorkingCopy/parser lineage, domain projections, Semantic Graph projection and ModViz projection.

Still open: complete domain assembly, recovered runtime factory/lifecycle bridges, representative catalog validation, vanilla-ready equivalence and deliberate promotion.

Semantic Graph remains a derived representation, not scene assembly authority.

## Phase 9 — ModViz and editors

**Status:** partial / not complete.

Editor integration slices and shared projections exist. Complete scene/model/HUD workflows, validated runtime behavior, deterministic export and broad UX remain open.

## Phase 10 — HITS/collision reconstruction

**Status:** strong bounded reverse/implementation / not complete.

Implemented/validated slices include HITS format structure, spatial reconstruction, deterministic DMC Rengine writer, source0/source1 ownership and multiple EXE-backed runtime contracts.

Pass-10 correction: the older statement that `0x14005E7A0`, `0x14005FEC0` and `0x1400601E0` top-level contracts are wholly open is superseded by PR #85 bounded closures/reclassification.

Still open: deeper primitive producer/helper reconstruction, source2 backing/lifetime/live semantics, full gameplay meaning, controlled runtime comparison and original Capcom builder equivalence.

## Phase 11 — Reverse Core

**Status:** architecture/foundation active / not complete.

Target flow:

`artifact -> range/function/type -> evidence -> hypothesis -> experiment -> reconstruction -> behavioral validation -> ValidationReceipt`

plus claim/ownership coordination for parallel agents.

Exit requires multiple bounded subsystem proofs and stable generic infrastructure, not only DMC-specific records.

## Phase 12 — Decompilation and recompilation frontier

**Status:** long-term / not complete.

Build-lineage architecture and selected recovered modules exist. A fully decompiled, behaviorally equivalent, rebuilt DMC3 executable does not exist.

## Current sequence

```text
exact artifact/evidence authority
  -> resource lifecycle/factory/cache closure
  -> evidence-bounded recovered runtime reconstruction
  -> representative Stage Catalog + lifecycle validation
  -> Stage Ops complete domain/runtime bridging
  -> behavioral ValidationReceipts
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

Parallel HITS work continues below the wrapper ABI toward source-equivalent primitive/contact behavior and runtime validation.
