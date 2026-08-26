# GDSpaces Decompilation-Layer Classification

**Canonical reconciliation:** 2026-08-26  
**Snapshot base:** `main@c147facb310d32ef084c56ba82d1e4b6b9b1b496`  
**Detailed L1 EXE review:** [l1-exe-boundary-review-2026-08-26.md](l1-exe-boundary-review-2026-08-26.md)

This document keeps GDSpaces/resource-runtime work separated by ownership and acceptance layer. The Layer-1 execution authority is [L1 roadmap](l1-roadmap.md), and cross-layer execution follows [master-roadmap.md](master-roadmap.md).

## Canonical tags

### [L1] Resource Materialization

```text
selected physical/container identity
 -> whole-file/FileSlot byte transport
 -> exact member/span acquisition
 -> STORE/raw-DEFLATE transform
 -> caller-owned materialized bytes
 -> packed representation OR .lst synthesis
 -> nested extraction
 -> exact editable child authority
 -> WorkingCopy/edit
 -> rebuild/repack/publication
 -> reopen/rematerialization
 -> resource materialization completion / state 1 -> 2 handoff
```

L1 is not closed by lookup, enumeration, structural parsing or synthetic composition alone.

`FileSlot`/AsyncIO is L1 **where it transports the selected bytes**. FileSlot pool ownership, cancellation, release/reset and broader runtime lifecycle are L3. Functions crossing the boundary are classified by behavior, not assigned wholesale to one layer.

### [L2] Resource Resolution

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume selection
 -> duplicate/ambiguity behavior
 -> exact ResourceRef identity
```

### [L3] Original Runtime / Lifecycle

```text
materialized-byte state2 input
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3 / ready visibility
 -> claims/cache/family ownership
 -> cancellation/reset/release/unload/shutdown
```

Original code here belongs to the Recovered Game Source Tree.

### [V] Validation

Cross-cutting hashes, corpus receipts, CI, original-vs-reconstruction comparison and original-game execution. Validation is not a fourth decompilation layer.

### [OUTSIDE]

Product/extraction metadata or tooling information not established as original DMC3 runtime behavior.

## Current classification matrix

| Area | Layer | Current boundary |
|---|---|---|
| NBZ local/central/EOCD and stored spans | L1 | canonical bounded read/serialization evidence |
| STORE/raw-DEFLATE member materialization | L1 | strong canonical |
| whole-file caller-owned-destination transfer | L1 | strong `0x1400333F0/3C0/500/5A0` spine |
| FileSlot/ReadRequest byte transport | L1 | strong architecture; exact error/cancellation breadth bounded |
| archive/member provenance stability | L1 + V | canonical product support; real-retail receipt still required |
| atomic/no-replace artifact publication | L1 product safety | canonical |
| PAC/PNST relative-slot parsing | L1 | strong canonical |
| recursive PAC/PNST expansion | L1 | strong canonical |
| bounded PAC/PNST reflow/reintegration | L1 | canonical at evidenced writer scope |
| nested root-to-leaf slot-path authoring | L1 | canonical product support |
| transformed DDS texture writer | L1 | canonical safe subset; direct-retail representation provenance still separate |
| `.lst` synthesized bytes | L1 | structurally strong; temp lifetime/error/fan-in breadth open |
| `.lst` packed-first selection | L2 input + L1 representation boundary | selection recovered; synthesis itself L1 |
| `DMC3-N.nbz` bootstrap/first-gap/precedence | L2 | strong canonical |
| request basename candidates | L2 | strong recovered boundary |
| archive normalization/index/qsort/bsearch | L2 | strong recovered boundary |
| type-0 physical post-`0x0C` final path/open/miss | L2 | CLOSED bounded static reverse + product model via #215/#204 |
| `LocalDirectorySource` physical lookup | L2 product policy | native product model remains distinct from original-process evidence |
| LoadedResource state2 typed finalization / state3 | L3 | substantial recovered static spine |
| typed MOD/EFM/SCM/SHW post-load | L3 | bounded family authority; exhaustive family semantics open |
| loader-node claims/cache/reset/release | L3 | substantial recovered authority |
| dynamic transition/reload/shutdown receipts | L3 + V | OPEN representative Level-E coverage |
| `.index` manifests | OUTSIDE | metadata, not original lookup authority on recovered path |
| binary AFS backend | evidence-gated | not established by `.afs/` namespace strings |
| PACK original runtime use | evidence-gated | historical product parser is not original-runtime proof |

## GDS-relevant EXE function-boundary matrix

### Strong / do not restart without contradiction

- resource bootstrap / numbered-volume registration family around `0x14002E930`;
- mounted-source resolver family around `0x140327430` and the #215 type-0 physical final-open contract;
- basename-oriented `OpenGameResource` request path around `0x14002FCA0`;
- normalization family including archive `0x0E` and physical `0x0C` behavior;
- archive central index/sort/search family;
- whole-file transfer family `0x1400333F0 / 0x1400333C0 / 0x140033500 / 0x1400335A0`;
- FileSlot/ReadRequest transport architecture;
- `ZipEntryRead 0x140328F50` direct-vs-inflated branch;
- `InflateRead 0x140328820` raw-DEFLATE streaming behavior;
- compressed seek reset+replay architecture and raw seek architecture;
- ZIP member teardown ownership;
- `.lst` packed-first grammar/layout/recursive in-place synthesis;
- LoadedResource `state 1 -> 2` resource-level completion boundary;
- L3 `state 2 -> typed post-load -> 3` boundary.

These boundaries may still have open error/lifetime edges; that does not justify restarting the recovered core behavior.

## Corrected labels after the three-pass L1 review

### `0x1402EF4D0`

Safe label:

> **resource materialization submission/scheduling wrapper**

Do not call it an exact-path resolver, final provider open, synchronous file reader or raw OS-read wrapper without new direct evidence.

### `0x1400335A0`

Transport/whole-file completion callback with `(ticketId,userContext,errorFlag,bytesRead)` semantics. This is the raw transfer-completion layer.

### `0x1401B8DC0`

Resource scheduler/materialization completion handoff registered through `0x1402EF580`; normal branch publishes `state 1 -> 2`. It is **not** the raw I/O callback.

### `0x1401B84E0`

Cross-layer acquisition constructor. Allocation/destination/materialization start belongs to L1. State publication/scheduler/lifecycle behavior reaches the L1/L3 boundary.

### `.lst` temporary load

The `.lst` text is synchronously loaded into aligned temporary storage before bounded parsing. There is no direct stored caller/callee edge proving this loader is the synchronous-style wrapper around `0x1402EF920`.

## Bounded open reverse targets relevant to GDS

Current L1 priority:

1. **materialization fan-in/completion semantics** between child/direct submissions and `0x1401B8DC0` state2 publication;
2. **transport error -> resource scheduler/materialization error mapping**;
3. **`.lst` temporary allocation/free identity and failure cleanup**;
4. `.lst` malformed/truncated/recursion failure propagation when real loose-list acceptance activates it;
5. FileSlot/ReadRequest partial-read/error/cancellation breadth where a claimed compatibility boundary requires it;
6. complete `0x140328540` initializer and `0x140328FE0` compressed-seek exact-body/error breadth only when acceptance requires those details.

Current L2/L3 evidence targets remain governed by their own audits and the master roadmap.

## Current promotion / branch classification

Historical stacked L1 PRs are evidence history, not current branch truth. In particular, older open PRs such as #162, #170, #175, #176, #178 and #181 must not be treated as current implementation authority merely because they remain open. Their relevant promoted/corrected behavior is read from `main` and current canonical docs.

Current active work at this reconciliation:

- `main@c147facb...` is canonical implementation truth;
- #215 is merged and closes the bounded type-0 physical-provider static/product slice;
- #219 is active L2 protected-runtime mapping tooling/evidence work;
- #218 is active L3 lifecycle trace validation work and must not be counted as L1 completion;
- the fresh L1 EXE boundary reconciliation branch/PR is documentation/data truth until merged.

Historical branches are intentionally not force-rebased. If a historical PR is later revived for promotion, it must respin cleanly from current `main` and reconcile against the canonical layer boundary first.

## Cross-boundary rules

- Physical provider selection is L2; materializing selected bytes is L1.
- `.lst` representation choice is a selection boundary; synthesized container bytes and child materialization are L1.
- FileSlot transport can be L1 while pool/lifecycle ownership is L3.
- Product hardening never becomes original-game acceptance behavior automatically.
- Writer compatibility with read/runtime contracts does not prove Capcom offline-writer equivalence.
- Direct-retail member identity must come from the canonical resolver winner, not a pre-guessed archive path.
- Evidence-grade archive/member receipts require artifact stability across index/member/hash observation.
- `state 1 -> 2` is the L1 materialized-byte handoff; `state 2 -> 3` is L3 typed-ready progression.

## Current priority accounting

Primary L1 acceptance sequence remains:

```text
direct-retail provenance
 -> representation classification
 -> real edit/rebuild
 -> next-volume reopen/rematerialization
 -> original-game consumption
 -> final L1 audit
```

While external acceptance is unavailable, supporting reverse is permitted only when it closes the bounded L1 handoff/error gaps listed above. L2/L3 work remains valid and may proceed as supporting evidence, but it must not be counted as L1 completion unless it closes a mandatory L1 gate.
