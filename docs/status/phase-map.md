# Phase Map

**Snapshot date:** 2026-08-15  
**Synchronization cutoff:** after merge PR #98 and creation of PR #99  
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

Merged PR #98 adds a production generic exact expected-SHA-gated executable byte-window acquisition primitive through the canonical source path. It validates PE VA/RVA/file-backed mapping across the entire requested interval, fails closed on ambiguous/virtual-only/raw-only/boundary-crossing mappings, produces deterministic metadata receipts, and exposes raw hex only in explicit local mode.

PR #98 closes a tooling/acquisition responsibility only. Its receipt proves artifact/range/byte identity, not function boundaries, ABI, semantics or behavioral equivalence.

Still open: broad function/type recovery, complete EXE Editor workflows, full decompilation and behavior-tested reconstructed subsystems.

## Phase 3 — Production resource/container runtime

**Status:** active / not complete.

Implemented across active stacks includes generic containers, production-oriented source/provenance work, NBZ ZIP indexing/materialization slices, path normalization, source lookup, runtime resolver composition and recursive expansion.

PR #99 adds the current-generation production-oriented structural `PAC\0` decoder into the shared `ContainerDocument` path:

- exact declared slot space;
- empty-slot preservation;
- absolute populated PAC offsets;
- next-greater-distinct-offset bounded extent inference;
- duplicate populated offsets preserved as separate slot identities without semantic alias inference;
- fail-closed malformed/truncated/out-of-range handling.

PR #99 remains branch-scoped and real-corpus-gated. It is not full PAC compatibility and does not close issue #3.

Still required for DMC3 runtime equivalence:

- representative multi-family real-PAC validation for #99;
- complete PAC/PNST/NBZ/AFS evidence-bounded behavior in the promoted current path;
- complete request/source/fallback semantics;
- `.lst` grammar/recursion/ownership behavior;
- typed post-load/factory/cache/lifetime/unload closure;
- representative game-backed ValidationReceipts;
- write/repack/export only after evidence and safety gates.

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

Final current-head branch receipt: `b0994436457a7ae26e3083a4a13461f50db6e76d`, Actions `31877176748`, Ubuntu 106/106 and Windows 106/106.

Still open: complete domain assembly, recovered runtime factory/lifecycle bridges, representative catalog validation, vanilla-ready equivalence and deliberate promotion.

Semantic Graph remains a derived representation, not scene assembly authority.

## Phase 9 — ModViz and editors

**Status:** partial / not complete.

Editor integration slices and shared projections exist. Complete scene/model/HUD workflows, validated runtime behavior, deterministic export and broad UX remain open.

## Phase 10 — HITS/collision reconstruction

**Status:** strong bounded reverse/implementation / not complete.

Implemented/validated slices include HITS format structure, spatial reconstruction, deterministic DMC Rengine writer, source0/source1 ownership and multiple EXE-backed runtime contracts.

Pass-10 correction: the older statement that `0x14005E7A0`, `0x14005B460`, `0x14005FEC0` and `0x1400601E0` remain a wholly open top-level P0 set is superseded by PR #85 bounded closures/reclassification.

Validated evidence progression now includes:

- Slice 7 shape-layer mappings;
- Slice 8 common contact normal;
- Slice 9 primitive-descriptor ownership;
- Slice 10 runtime type0/type1 structural semantics;
- Slice 12 Stage-CFG PAC provenance;
- Slice 13 serialized `0x40 / 0x04 / 0x50` collision-triplet view;
- Slice 14 Stage-CFG entry/primitive-descriptor view with transform bounds intentionally unavailable;
- Slice 15 referenced Stage-CFG descriptor census tooling.

### Latest validated slice: Slice 15 / PR #96

**Status:** bounded-complete implementation/evidence-tooling gate.

Final evidence/documentation head `a4be42d5ea73c9e120febd8a9b1b0654d5858dbc`, Actions `31886670409`, Ubuntu + Windows success.

Real-corpus referenced-descriptor census remains an open data gate. No real type-5 presence/absence claim is permitted until representative Stage-CFG resources are actually censused.

### Active reverse frontier: Slice 16 / PR #97

**Status:** `RESEARCH REQUIRED`.

Current frontier is Stage-CFG transform-source provenance:

- `entry+0x01` is a transform selector;
- modern Stage-CFG slots 39/40 and legacy observed 22/23 are entry/primitive-descriptor tables;
- modern slot38 is consumed by `0x1400594B0` and has its own relative-offset structure;
- slot38 is not proven to be the C740-style `0x40` transform table;
- C8D0 stack arg5 becomes runtime `+0x20` transform pointer;
- `transform_selector_bounds_available() == false` remains mandatory.

Required acquisition/reverse targets:

1. full `0x1400594B0` body/callers/slot38 dataflow;
2. modern route around `0x14009823F`;
3. legacy observed route around `0x1400B6483`;
4. complete C630/C740 caller census by manager/source identity;
5. exact C8D0 stack-arg5 producer/base/object/bounds/count/lifecycle.

Still open after Slice 16: deeper primitive producer/helper reconstruction, source2 backing/lifetime/live semantics, controlled runtime comparison, modified-topology game validation and original Capcom builder equivalence.

## Phase 11 — Reverse Core

**Status:** architecture/foundation active / not complete.

Target flow:

`artifact -> range/function/type -> evidence -> hypothesis -> experiment -> reconstruction -> behavioral validation -> ValidationReceipt`

plus claim/ownership coordination for parallel agents.

Merged PR #98 is now one concrete generic acquisition primitive in this flow. Exit still requires multiple bounded subsystem proofs and stable generic infrastructure, not only DMC-specific records or acquisition tooling.

## Phase 12 — Decompilation and recompilation frontier

**Status:** long-term / not complete.

Build-lineage architecture and selected recovered modules exist. A fully decompiled, behaviorally equivalent, rebuilt DMC3 executable does not exist.

## Current sequence

```text
exact artifact/evidence authority
  -> merged hash-gated EXE acquisition for new byte windows when raw artifact is available
  -> resource lifecycle/factory/cache closure
  -> evidence-bounded recovered runtime reconstruction
  -> representative Stage Catalog + lifecycle validation
  -> Stage Ops complete domain/runtime bridging
  -> behavioral ValidationReceipts
  -> validated editor/export verticals
  -> progressive recompilation milestones
```

Parallel tracks at this cutoff:

- HITS Slice 16 transform-source provenance;
- representative real Stage-CFG Slice-15 census;
- PR #99 representative real-PAC corpus validation.
