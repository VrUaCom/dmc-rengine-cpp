# GDSpaces hypothetical reverse-progress scale

Status snapshot: 2026-08-20.

This is an **operational reverse-coverage index**, not a behavioral-equivalence percentage and not a completion claim. Canonical layer boundaries remain those in `decompilation-layer-classification.md`.

## 100% rule

A layer may be marked `100% / COMPLETE` only when:

1. every mandatory gate is closed or removed from scope by evidence;
2. closing implementation/evidence is promoted to canonical `main`;
3. representative real-corpus validation exists for the layer boundaries;
4. required whole-head Windows + Ubuntu validation is green;
5. architecture-changing contradictions are zero;
6. the completion wording is no broader than the evidence.

`99%` may still mean **NOT COMPLETE** when one mandatory artifact or receipt is missing. Synthetic CI alone cannot produce 100%.

## Coverage bands

| Index | Meaning |
|---:|---|
| 0-19% | discovery / largely unknown |
| 20-39% | major structures identified |
| 40-59% | bounded reverse + partial implementation |
| 60-79% | major path reconstructed; representative gaps remain |
| 80-94% | broad evidence closure; narrow but material gates remain |
| 95-99% | final mandatory gates only; still NOT COMPLETE |
| 100% | mandatory gates zero + canonical completion criteria satisfied |

## Current hypothetical scale

| Layer | Index | Status | Main remaining reason |
|---|---:|---|---|
| **L1 Resource Materialization** | **88%** | ACTIVE / NOT COMPLETE | Real `.lst` receipt, representative real child-to-slot/intrinsic-byte authority, real size-changing round-trip/game-consumption receipts, and no-loss retail-NBZ metadata/repack tier remain. |
| **L2 Resource Resolution** | **94%** | HIGH / NOT COMPLETE | Narrow path/open-surface edges, broader build/profile validation, and final representative end-to-end receipts remain. |
| **L3 Original Runtime / Lifecycle** | **72%** | ADVANCED / NOT COMPLETE | FileSlot/AsyncIO/scheduler ownership, unresolved mode semantics, dynamic lifecycle and complete release/unload/shutdown receipts remain. |
| **V Validation** | **60%** | SUPPORTING / NOT A LAYER | Many bounded hashes/corpus/CI receipts exist, but exact-build authored-resource and whole-path behavioral receipts remain incomplete. |

The numbers are deliberately hypothetical and may move down when stronger reverse evidence reveals a larger surface.

## L1 closed strongly in canonical code

L1 currently includes strong canonical coverage for:

- NBZ classic-ZIP source/indexing and STORE/raw-DEFLATE materialization;
- ByteProvenance source-vs-materialized coordinate separation;
- PAC and PNST structural parsing with sparse/empty/alias identity preservation;
- recursive PAC/PNST expansion and bounded parse reuse;
- editable WorkingCopy with parser validation where required;
- same-size layout-preserving PAC/PNST writing;
- same-size nested reintegration with alias-conflict arbitration;
- runtime-synth size-changing PAC/PNST writing using recovered `.lst` 64-byte layout;
- typed verified runtime-synth complete-image child authority for multi-level size-changing composition;
- deterministic STORE-only next-volume NBZ overlay authoring and canonical reopen;
- authored-byte receipts kept separate from source ByteProvenance.

## Mandatory L1 gates before 100%

### Real `.lst` validation

Reacquire a representative legal real `.lst` corpus and run the canonical parser/materializer. Strong EXE layout evidence does not replace this corpus receipt.

### Real child-to-slot and intrinsic-byte authority

For representative size-changing nested resources, prove which exact intrinsic child image belongs to which physical slot. Synthetic `slot_NNNN.bin` identities and parser-inferred parent extents are not acceptable substitutes.

### Real size-changing A-to-Z round-trip

Demonstrate a representative legal path:

`original source/member -> materialize -> nested edit -> size-changing bottom-up rebuild -> root -> generated NBZ -> reopen -> canonical reparse/compare`.

### Original-game consumption receipt

At least one representative authored output must be consumed successfully through the original DMC3 path under a controlled receipt. Product reparse success alone is not game compatibility evidence.

### No-loss retail NBZ tier

The current writer intentionally creates deterministic STORE-only mod overlays. It does not preserve the complete original retail ZIP metadata envelope or claim byte-identical retail repacking.

The first preservation substep is the bounded on-demand [`NbzZipSerializationScanner`](dmc3-nbz-retail-serialization.md), which records raw central/local/EOCD framing plus source spans without turning ordinary materialization into writer ownership. That scanner does **not** close this gate by itself.

The remaining retail-NBZ tier must artifact-bind the preserved spans, implement metadata-preserving unchanged/changed entry serialization, rebuild offsets, reopen the result, compare materialization + serialization properties, and obtain representative game-backed validation.

## Non-blocking evidence-gated families

- Binary AFS backend does not block DMC3 HD L1 unless new direct raw/backend evidence proves it is required; current evidence establishes `.afs/` namespaces.
- PACK does not block DMC3 HD L1 absent new runtime/raw-format evidence.
- Original factory/cache/lifecycle beyond the byte-materialization contract is L3, not L1.
- Stage Ops/ModViz semantics do not count toward L1 completion.

## Update policy

After a material reverse/promotion pass, reconcile GitHub issue/PR evidence, Drive evidence, historical implementation and current `main`; update the index only when the known surface or mandatory gates changed. Never convert a percentage into COMPLETE automatically. When L1 finally closes, set `100% / COMPLETE` in the same promotion that records the final mandatory receipts.
