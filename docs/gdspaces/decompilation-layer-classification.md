# GDSpaces Decompilation-Layer Classification

**Canonical reconciliation:** 2026-08-24  
**Snapshot base:** `main@c4920c8602dd7492b6c89e9fc8ecf8a6d8397ee0`

This document keeps GDSpaces/resource-runtime work separated by ownership and acceptance layer. The Layer-1 execution authority is [L1 roadmap](l1-roadmap.md).

## Canonical tags

### [L1] Resource Materialization

```text
physical/container bytes
 -> bounded acquisition
 -> transform/decompression
 -> exact materialized bytes
 -> nested extraction
 -> exact editable child authority
 -> WorkingCopy/edit
 -> rebuild/repack/publication
 -> reopen/rematerialization
```

L1 is not closed by lookup, enumeration, structural parsing or synthetic composition alone.

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
FileSlot / AsyncIO / callbacks
 -> LoadedResource states
 -> typed post-load
 -> claims/cache/factory handoff
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
| archive/member provenance stability | L1 + V | OPEN for provenance-grade acquisition |
| atomic/no-replace artifact publication | L1 product safety | OPEN cross-stack correction |
| PAC/PNST relative-slot parsing | L1 | strong canonical |
| PNST structural classification | L1 | current-main hardened |
| recursive PAC/PNST expansion | L1 | strong canonical |
| exact intrinsic editable-child identity | L1 | representation-specific; never inferred from parent extent alone |
| bounded PAC/PNST reflow/reintegration | L1 | canonical at evidenced writer scope |
| transformed DDS texture writer | L1 | canonical safe subset; retail provenance not yet established |
| texture runtime relocation compatibility | L1 support from original EXE | canonical bounded composition |
| `.lst` synthesized bytes | L1 | structurally recovered; real loose corpus/dynamic edge behavior separate |
| `.lst` packed-first selection | L2 | recovered representation-selection behavior |
| `DMC3-N.nbz` bootstrap/first-gap/precedence | L2 | strong canonical |
| request basename candidates | L2 | strong recovered boundary |
| archive normalization/index/qsort/bsearch | L2 | strong recovered boundary |
| type-0 physical final filename/open behavior | L2 | OPEN exact Win32 semantics after recovered 0x0C normalization |
| `LocalDirectorySource` physical lookup | L2 product policy | not original-equivalent final-open proof |
| FileSlot pool/AsyncIO ownership | L3 | substantial recovered static spine |
| LoadedResource 0/1/2/3/4 lifecycle | L3 | substantial recovered static spine |
| typed MOD/EFM/SCM/SHW post-load | L3 | bounded family authority; exhaustive family semantics open |
| loader-node claims/reset/release | L3 | substantial recovered authority |
| dynamic transition/reload/shutdown receipts | L3 + V | OPEN representative Level-E coverage |
| `.index` manifests | OUTSIDE | metadata, not original lookup authority on recovered path |
| binary AFS backend | evidence-gated | not established by `.afs/` namespace strings |
| PACK original runtime use | evidence-gated | historical product parser is not original-runtime proof |

## GDS-relevant EXE function-boundary matrix

### Strong / do not restart without contradiction

- resource bootstrap / numbered-volume registration family around `0x14002E930`;
- mounted-source resolver family around `0x140327430`;
- basename-oriented `OpenGameResource` request path around `0x14002FCA0`;
- normalization family including archive `0x0E` and physical `0x0C` behavior;
- archive central index/sort/search family (`qsort`/`bsearch` architecture);
- `ZipEntryRead 0x140328F50` direct-vs-inflated branch;
- `InflateRead 0x140328820` raw-DEFLATE streaming behavior at the recovered scope;
- whole-file selected-backend materialization spine at the bounded recovered scope;
- LoadedResource `0 -> 1 -> 2 -> typed post-load -> 3` boundary;
- PAC/PNST recursive typed traversal;
- major `.lst` packed-first/synthesis structure.

These boundaries may still have open error/lifetime edges; that does not justify restarting the already recovered core behavior.

### Bounded open reverse targets relevant to GDS

1. **Type-0 physical provider final open** — exact Win32 filename construction/comparison/case behavior, open flags and failure semantics after `0x0C` normalization.
2. **`0x140328540` ZIP entry/stream initializer** — complete body, allocation/lifetime/error behavior.
3. **`0x140328FE0` compressed seek/reset** — complete reset + reinflate/discard/error behavior.
4. **Malformed/partial-read exact error behavior** — only where required for a claimed compatibility boundary.
5. **Dynamic `.lst` ownership/lifetime/error/cycle behavior** — only if direct real loose-container acceptance depends on it.
6. **Representative dynamic original-process lifecycle** — load/reload/transition/release/shutdown receipts after L1 reaches game execution.

## Current PR / promotion classification

Historical numbered PR lists are not maintained here as the primary truth because rapid clean respins supersede stacked branches. Current status is read from `main` plus active PRs/issues.

Important current state:

- #183 supersedes stale #167 synthetic nested A-to-Z validation;
- #192 supersedes stale #168 PNST classification promotion;
- #184-#189 represent the current clean texture/overlay/game-preflight progression now reflected in current main at this snapshot;
- #191 is an active L1 acquisition seam and remains **DO NOT PROMOTE** until publication, artifact-stability and retail-tree-output blockers close;
- #190 is EXE Editor/recovered-source-tree work and is branch truth until merged.

## Cross-boundary rules

- Physical provider selection is L2; materializing the selected bytes is L1.
- `.lst` representation choice is L2; synthesized container bytes are L1.
- FileSlot can support L1 byte-read reconstruction while original pool/scheduler/callback ownership remains L3.
- Product hardening never becomes original-game acceptance behavior automatically.
- Writer compatibility with read/runtime contracts does not prove Capcom offline-writer equivalence.
- Direct-retail member identity must come from the canonical resolver winner, not a pre-guessed archive path.
- Evidence-grade archive/member receipts require artifact stability across index/member/hash observation.

## Current priority accounting

Primary execution accounting remains L1. The roadmap sequence is:

```text
atomic publication
 -> artifact-stable acquisition
 -> direct-retail provenance
 -> representation classification
 -> real edit/rebuild
 -> next-volume reopen/rematerialization
 -> original-game consumption
 -> final L1 audit
```

L2/L3 work remains valid and may proceed as supporting evidence, but it must not be counted as L1 completion unless it closes a mandatory L1 gate.