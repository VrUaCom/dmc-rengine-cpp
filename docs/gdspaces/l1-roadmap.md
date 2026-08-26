# GDSpaces Layer 1 Roadmap

**Status:** INTERNAL PRODUCT PATH CLOSED / EXTERNAL ACCEPTANCE OPEN  
**Snapshot date:** 2026-08-26  
**Canonical implementation base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Primary tracking:** #100, #182, #209  
**Final pre-Level-E audit:** `l1-final-audit-2026-08-25.md`  
**Canonical EXE boundary review:** `l1-exe-boundary-review-2026-08-26.md`  
**L1/L3 handoff follow-up:** `l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md`

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

L1 answers:

> Can DMC Rengine obtain the exact bytes selected for a DMC3 resource, preserve their provenance, safely edit an evidenced representation, rebuild the required nested container/archive stack, reopen/rematerialize the authored result, and prove that the original protected game consumed those authored bytes?

Synthetic tests, a working resolver, a valid parser or a crash-free game launch are not sufficient for `L1 COMPLETE`.

## 1. Canonical L1 boundary

```text
L2 selected logical/provider identity
 -> L1 physical/container byte acquisition
 -> exact member/span acquisition
 -> read/seek/decompression mechanics
 -> exact materialized byte buffer + ByteProvenance
 -> PAC/PNST nested expansion
 -> exact editable child identity
 -> bounded edit
 -> bottom-up container rebuild
 -> next-volume NBZ publication
 -> canonical resolver/reopen/rematerialization
 -> product closure receipt
 -> original DMC3 vertical acceptance
```

The canonical raw-L3 audit now controls the runtime boundary after exact materialized bytes:

```text
L1 exact materialized bytes
 -> [L3] FileSlot/async request ownership and scheduling
 -> [L3] LoadedResource acquisition/state 0->1
 -> [L3] completion/state 1->2
 -> [L3] typed post-load/state 2->3
 -> [L3] consumer lifecycle/release/reset/teardown
```

FileSlot is a boundary subsystem: byte-read mechanics can support L1, while request ownership/scheduling/completion/cancellation/close lifetime are L3.

`0x1401B8CA0` is an explicit semantic seam: representation/materialization mechanics are L1-relevant, while success gates L3 state1 publication.

L2 may supply exact selected identity and L3 may supply original consumer/lifecycle evidence. Their broader completion is not required for L1 unless a concrete L1 acceptance path activates one of their unresolved boundaries.

## 2. Canonical product capabilities on current main

Current `main` includes:

- classic NBZ/ZIP indexing and bounded central/member acquisition;
- STORE and raw-DEFLATE method-8 materialization;
- CRC, size, SHA and explicit `ByteProvenance`;
- artifact-bound serialization/member observation preventing stale snapshot receipts;
- recovered contiguous numbered-volume / first-gap runtime namespace;
- canonical runtime resolver composition and higher-volume precedence;
- atomic/no-replace artifact publication with staged validation;
- direct-retail resolver-based member extraction + provenance sidecar;
- PAC and PNST sparse/empty/alias-preserving relative-slot parsing;
- recursive PAC/PNST expansion;
- size-changing relative-slot reflow with byte-exact untouched physical spans;
- nested root-to-leaf slot-path authoring, e.g. `0/2/1`, with bottom-up parent reflow and chained SHA receipts;
- verified immutable NBZ copy rebuild;
- deterministic next-contiguous STORE NBZ overlay authoring;
- staged NBZ reopen and exact member verification before publication;
- protected distribution executable preflight;
- protected retail authoring closure orchestration through resolver rematerialization;
- type-0 physical-provider post-`0x0C` static contract promoted through #215;
- protected-runtime RVA mapping acquisition tooling promoted through #219;
- guarded canonical-analysis EXE window acquisition infrastructure;
- Windows + Ubuntu CI for promoted product paths.

These close the known mandatory **internal implementation** work for the current representative DMC3-HD L1 acceptance scope.

## 3. Gate status

### L1-A — publication integrity

**CLOSED / CANONICAL**

Closed by #194 and all subsequent authoring/acquisition seams using the shared final no-replace publication contract.

### L1-B — artifact-stable retail acquisition

**CLOSED / CANONICAL**

Closed by #195–#197 and consumed by the direct-retail acquisition command.

### L1-C — direct-retail representative provenance

**IMPLEMENTATION CLOSED / REAL RECEIPT OPEN**

Use:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

The request, not a pre-guessed archive path, is the input authority. The receipt must preserve the actual resolver winner, selected volume/archive identity, central-entry metadata, materialized SHA/size and byte transform.

`obj\em000.pac` remains a high-value target but is not mandatory if another representative resource gives a stronger deterministic authoring/consumer receipt.

### L1-D — retail representation classification

**REAL RECEIPT OPEN**

Classify the exact bytes obtained in L1-C. Only use an existing writer if the observed retail representation is inside its evidenced domain.

If not, stop and create a new bounded evidence gate; do not force the bytes through a convenient writer.

### L1-E — bounded real edit + bottom-up rebuild

**PRODUCT IMPLEMENTATION CLOSED / REAL-RETAIL RECEIPT OPEN**

Current writers support top-level and nested PAC/PNST slot paths, size-changing reflow, sparse/alias preservation and exact untouched-sibling validation.

Relevant promotion includes #201 and #213.

### L1-F — next-volume publication + canonical reopen

**PRODUCT IMPLEMENTATION CLOSED / REAL-RETAIL RECEIPT OPEN**

Current product path supports:

```text
rebuilt member
 -> next contiguous DMC3-N.nbz
 -> staged canonical reopen
 -> atomic/no-replace publication
 -> RuntimeResourceResolver higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

`verify-dmc3-l1-authoring` composes the protected preflight, retail acquisition, top-level PAC/PNST authoring, next-volume overlay and resolver/rematerialization checks into one product closure receipt. Nested authoring is also canonical through `rebuild-relative-slot-path`; closure orchestration may be widened to slot paths as a usability refinement, but this is not a correctness prerequisite for a representative acceptance resource that fits the current closure command.

### L1-G — original DMC3 consumption

**OPEN / EXTERNAL LEVEL-E / FINAL VERTICAL ACCEPTANCE**

Canonical tracking: issue #209.

Required controlled run:

```text
product closure receipt
 -> exact generated DMC3-N.nbz
 -> verify destination absent
 -> controlled copy into retail data/dmc3
 -> post-copy SHA == closure overlay SHA
 -> launch protected distribution executable
 -> deterministic request / consumer path
 -> observable effect attributable to authored bytes
 -> clean transition/exit
 -> remove only test overlay
 -> verify original retail artifacts unchanged
```

A crash-free launch alone fails this gate.

### L1-H — final cross-stack audit

**OPEN / DEPENDS ON L1-C..G REAL RECEIPTS**

Before `L1 COMPLETE`:

- exact executable authority is recorded;
- direct-retail archive/member provenance exists;
- representation classification is explicit;
- real authored replacement/rebuild receipt exists;
- generated overlay identity is exact;
- canonical resolver/rematerialization succeeds on that real artifact;
- original DMC3 consumer observation exists;
- rollback proves retail immutability;
- exact-head Windows + Ubuntu CI is green on final canonical code/docs;
- #100, #182, #209 and current status agree;
- no unresolved contradiction changes the claimed L1 architecture or accepted representation scope.

Only then may the project state **L1 = 100% / COMPLETE**.

## 4. Commands on the closure path

```text
preflight-dmc3-game-test <exe-dir>
extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
rebuild-relative-slot <container-file> <slot-index> <replacement-file> <output-file>
rebuild-relative-slot-path <container-file> <slot/path> <replacement-file> <output-file>
build-nbz-copy <source-nbz> <central-index> <replacement-file> <output-nbz>
build-dmc3-overlay <exe-dir> <game-request> <authored-file> <output-dir>
verify-dmc3-l1-authoring <exe-dir> <game-request> <slot-index> <replacement-file> <workspace-dir>
```

The commands are product authoring/validation seams. They do not claim to reproduce Capcom's offline writer implementation.

## 5. Supporting EXE reverse boundaries

Detailed authorities:

- [L1 EXE Boundary Review](l1-exe-boundary-review-2026-08-26.md)
- [L1/L3 Materialization-to-Lifecycle Handoff Pass](l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md)
- [Canonical L3 Raw-EXE Boundary Audit](l3-boundary-audit-2026-08-26.md)

Do **not** restart without contradictory evidence:

- numbered-volume bootstrap / first-gap behavior;
- generic basename candidate construction and archive-first/physical-second attempt order;
- archive normalized lookup/index behavior;
- type-0 post-`0x0C` physical final-open static contract promoted by #215;
- whole-file caller-owned byte-transfer mechanics;
- FileSlot/ReadRequest byte-read architecture at the L1 support scope;
- ZIP EOCD/central/member identity path;
- `ZipEntryRead 0x140328F50` STORE-vs-compressed split;
- `InflateRead 0x140328820` raw-DEFLATE streaming behavior;
- compressed seek reset+replay architecture and raw seek architecture;
- primary `.lst` packed-first grammar/layout/recursive synthesis structure.

Correct labels/boundaries:

- `0x1402EF4D0` = **resource materialization submission/scheduling wrapper**, not proven exact-path resolver/final provider open/raw reader;
- `0x1401B8CA0` = L1/L3 seam: materialization mechanics + success result gating L3 acquisition;
- `0x1400335A0` = lower transfer progress/status callback, not LoadedResource state2 writer;
- `0x1401B8DC0` = **L3** normal lifecycle completion/state1->2 writer;
- `0x1402EF460` = pending scheduled-entry clear/rollback, not OS AsyncIO cancellation;
- `.lst` synchronous temporary acquisition is not proven equal to `0x1402EF920`;
- no generic child/outstanding-work fan-in counter is currently evidenced.

### Current supporting reverse priority

Use `data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json`:

1. exact `0x1402EF4D0` body/callees and load-context consumer;
2. exact L1 byte-materialization -> L3 request/scheduler handoff;
3. success-side completion ordering/dependency mechanics without assuming a generic fan-in counter;
4. scheduler rollback semantics around `0x1402EF460` and cancellation;
5. lower transfer failure -> L3 acquisition/cancellation mapping;
6. `.lst` child-population failure + temporary-buffer cleanup;
7. FileSlot/ReadRequest partial/error/cancellation breadth where a compatibility claim requires it;
8. complete `0x140328540`/`0x140328FE0` exact-body/error breadth only when activated by acceptance evidence.

These bounded reverse gaps do not automatically block a representative packed-NBZ/PAC/PNST L1 Level-E receipt.

## 6. Explicit non-blockers / freezes

- **Binary AFS:** `.afs/` strings are evidenced logical namespaces, not a binary-backend proof.
- **PACK:** historical product parsing is not original DMC3 runtime authority absent a direct dependency.
- **Capcom offline writer equivalence:** not required for L1 product authoring completion.
- **Stage Ops / ModViz:** downstream consumers; they do not define L1 truth.
- **Exhaustive malformed-input equivalence:** remains separate reverse breadth unless the claimed acceptance scope explicitly includes it.
- **Real `.lst` corpus:** mandatory only for a claim covering real loose-list consumption or when the representative acceptance path actually selects `.lst`.

## 7. Remaining work order

No new synthetic-only feature may displace this path unless real evidence reveals a concrete missing dependency.

```text
1. run direct-retail acquisition on a protected DMC3 installation
2. preserve acquisition/provenance receipt
3. classify exact retail representation
4. perform one bounded real edit through the supported authoring domain
5. generate next-contiguous NBZ and require canonical reopen/resolver/rematerialization
6. preserve product closure receipt
7. execute #209 original-game consumption + rollback
8. run final L1 audit
9. mark L1 100% / COMPLETE only if every mandatory receipt is valid
```

## 8. Environment boundary

The connected automation environment does not expose all exact raw protected-install artifacts needed to execute the protected-install Level-E run here.

Guarded canonical-analysis EXE reacquisition tooling exists on `main`. During the new handoff review a fresh raw canonical `e454...` blob was not exposed through the connected file surface, so the pass records reconciliation plus a focused next acquisition plan rather than claiming a fresh byte-disassembly execution.

## 9. Documentation synchronization

When L1 evidence or cross-layer boundaries change, synchronize:

- this roadmap;
- `l1-exe-boundary-review-2026-08-26.md`;
- `l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md`;
- `l1-final-audit-2026-08-25.md` or its completion successor;
- `l3-boundary-audit-2026-08-26.md` only when new L3 evidence warrants it;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/canonical-status.json`;
- `data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json`;
- issues #100, #88, #182 and #209.

Percentage estimates are planning aids only; mandatory gates remain the completion authority.
