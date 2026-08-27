# GDSpaces Layer 1 Roadmap

**Status:** **INCOMPLETE / NOT 100%**  
**Snapshot date:** 2026-08-27  
**Canonical base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Primary tracking:** #100, #182, #209  
**Layer-boundary authority:** `layer-boundary-status-reconciliation-2026-08-27.md`  
**Materialization reverse authority:** `materialization-completion-boundary-pass-2026-08-26.md` + `materialization-completion-dependency-pass2-2026-08-26.md`

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

The product implementation is advanced and contains a representative authoring/materialization path, but Layer 1 itself is **not complete**. Two independent classes of gates remain: an unresolved original materialization-completion dependency and real-retail/original-game acceptance receipts.

No canonical status may describe L1 as `COMPLETE`, `100%`, or as waiting only on external receipts.

## 1. Canonical L1 boundary

L1 begins after L2 has established a usable selected provider/member identity and ends when exact materialized bytes have been published at the recovered `state 1 -> 2` boundary. Product authoring/rebuild/rematerialization also belongs to L1.

```text
[L2] selected provider/member identity
 -> [L1] backend/FileSlot byte acquisition
 -> sync/async byte transport needed for materialization
 -> STORE/raw-DEFLATE or other evidenced transforms
 -> caller-owned destination population
 -> packed OR loose-list representation materialization
 -> nested PAC/PNST/.lst byte construction
 -> materialization terminal success/error dependency
 -> completion eligibility / failed-work suppression
 -> normal 0x1401B8DC0 dispatch
 -> state 1 -> 2 / exact materialized bytes
 -> ByteProvenance + editable identity
 -> bounded edit / rebuild / repack / publication
 -> reopen / rematerialization
 ===== END L1 =====
 -> [L3] typed post-load / state 2 -> 3 / ready ownership and lifecycle
```

Important ownership corrections:

- selected-byte FileSlot/ReadRequest transport is L1;
- transport completion/status used to decide materialization success is L1;
- `0x1402EF4D0` materialization submission/job behavior is an L1 boundary;
- normal `0x1401B8DC0` `state1 -> state2` publication is the end of L1;
- `.lst` packed-first-vs-loose choice is L1 representation materialization, not L2 resource selection;
- L3 begins from completed materialized state2 for typed-ready/ownership/lifecycle semantics.

L3 cancellation policy may act on unfinished state1/state2 records; that policy remains L3, while the underlying byte-terminal mechanism remains L1.

## 2. Current product capabilities

Current `main` already provides major L1 product capabilities:

- classic NBZ/ZIP bounded indexing and member acquisition;
- STORE and raw-DEFLATE method-8 materialization;
- CRC/size/SHA and ByteProvenance;
- artifact-bound archive/member observations;
- PAC/PNST sparse/empty/alias-preserving parsing and recursive expansion;
- size-changing relative-slot reflow;
- nested root-to-leaf PAC/PNST slot-path authoring;
- byte-exact untouched sibling preservation;
- verified immutable NBZ copy rebuild;
- deterministic next-contiguous STORE NBZ overlay authoring;
- staged canonical NBZ reopen and exact member verification;
- protected distribution executable preflight;
- resolver-based retail acquisition and authored rematerialization closure tooling;
- atomic/no-replace publication seams;
- Windows + Ubuntu CI for promoted product paths.

These are closed/advanced **product implementation slices**. They do not close the unresolved original completion dependency or the real acceptance gates below.

## 3. Mandatory L1 gates

### L1-M — original materialization terminal dependency

**OPEN / MANDATORY BEFORE L1 COMPLETE**

Current proven chain:

```text
0x1401B8CA0 materialization dispatch
 -> 0x1402EF4D0 submission/job creation
 -> lower whole-file/FileSlot transport
 -> UNKNOWN exact terminal dependency
 -> queued 0x1401B8DC0(record-relative-context)
 -> state 2
```

Merged evidence proves normal `0x1401B8DC0` receives only one u32 registry-relative context. It does not receive raw transport status, error flag, byte count, FileSlot handle, child count or outstanding-work metadata. Therefore success/failure eligibility must already be resolved before normal dispatch, or the queued completion must be suppressed/removed.

No generic fan-in counter is evidenced.

Required closure order:

1. close exact `0x1402EF4D0` queued job identity/type and inherited load-context consumer;
2. identify the corresponding `0x1402EF790` dispatch case and persistence/re-poll/retirement rule;
3. reacquire `0x1400333E0` pending/success/error semantics;
4. reacquire `0x140033390` terminal cleanup/release ordering;
5. bind `0x1400335A0` transport completion writes into that state;
6. prove what blocks/suppresses normal `0x1401B8DC0` on failed or incomplete transfer;
7. recover relevant `0x1402EF460` pending scheduler-entry clear/rollback behavior;
8. apply the confirmed mechanism to `.lst` child/recursive failure ordering where required.

Until this gate closes, L1 is not 100% even if a representative product path succeeds.

### L1-A — publication integrity

**CLOSED / CANONICAL**

Atomic/no-replace staged publication is canonical for the promoted authoring seams.

### L1-B — artifact-stable retail acquisition implementation

**CLOSED / CANONICAL IMPLEMENTATION**

Archive/member/hash observations are artifact-bound and stale snapshot substitution is rejected.

### L1-C — direct-retail representative provenance

**IMPLEMENTATION READY / REAL RECEIPT OPEN**

Run:

```text
dmc-rengine extract-dmc3-retail-member <exe-dir> <game-request> <output-file>
```

The receipt must preserve the actual resolver winner, selected volume/archive/member identity, archive/member hashes and materialized transform/provenance.

### L1-D — exact retail representation classification

**REAL RECEIPT OPEN**

Classify the exact bytes from L1-C. If they fall outside an evidenced writer domain, stop and open a bounded evidence gate rather than forcing a convenient serializer.

### L1-E — bounded real edit + bottom-up rebuild

**PRODUCT IMPLEMENTATION READY / REAL-RETAIL RECEIPT OPEN**

Current writers support top-level and nested PAC/PNST size-changing reflow, sparse/alias preservation and exact untouched-span validation.

### L1-F — next-volume publication + canonical reopen/rematerialization

**PRODUCT IMPLEMENTATION READY / REAL-RETAIL RECEIPT OPEN**

Required real path:

```text
rebuilt member
 -> next contiguous DMC3-N.nbz
 -> staged reopen
 -> higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

### L1-G — original DMC3 consumption + rollback

**OPEN / EXTERNAL LEVEL-E**

Canonical tracking: issue #209.

Required controlled run:

```text
product closure receipt
 -> exact generated DMC3-N.nbz
 -> controlled copy into retail data/dmc3
 -> post-copy SHA equality
 -> launch protected distribution executable
 -> deterministic request/consumer path
 -> observable effect attributable to authored bytes
 -> clean transition/exit
 -> remove only the test overlay
 -> verify original retail artifacts unchanged
```

A crash-free launch is insufficient.

### L1-H — final cross-stack audit

**OPEN / DEPENDS ON L1-M AND L1-C..G**

Before `L1 COMPLETE / 100%`:

- L1-M materialization terminal dependency is closed without contradiction;
- exact executable authority is recorded;
- direct-retail selected-member provenance exists;
- retail representation is explicitly classified;
- real authored rebuild/rematerialization receipt exists;
- original DMC3 consumer observation exists;
- rollback proves retail immutability;
- exact-head Windows + Ubuntu validation is green;
- #100, #182, #209, code, evidence and current documentation agree;
- no unresolved contradiction changes the claimed L1 scope.

Only then may the project state **L1 = 100% / COMPLETE**.

## 4. Cross-layer dependencies

### From L2

L1 consumes the successful selected provider/member identity. L2 owns request/candidate/provider/volume/ambiguity/failure semantics; it does not own byte transport or transformation.

### From L3

L3 consumes state2/materialized bytes for typed post-load and ready/lifetime behavior. L3 may supply original-process consumer evidence for Level-E. It does not own the byte-terminal mechanism merely because cancellation/restart can suppress unfinished work.

### Validation

Hashes, CI, corpus receipts and protected-process observations are cross-cutting validation, not a substitute for static reverse.

## 5. Closed/strong boundaries not to restart without contradiction

- numbered-volume bootstrap / first-gap behavior — L2 dependency;
- basename candidate construction and provider ordering — L2 dependency;
- archive normalized lookup/index behavior — L2 dependency;
- bounded FileSlot/ReadRequest transport architecture — L1;
- ZIP stored-vs-inflated path and raw-DEFLATE core — L1;
- PAC/PNST sparse/nested byte structure — L1;
- `.lst` core grammar/layout/recursive synthesis — L1;
- type-0 physical final-open static chain — L2 selection/open boundary.

Exact remaining work must target the terminal completion dependency, not restart strong lower layers.

## 6. Bounded secondary L1 gaps

After L1-M, close these only when required by the declared compatibility scope:

- `.lst` temporary allocation/free/failure cleanup and malformed/recursive failure propagation;
- FileSlot/ReadRequest partial-read/cancellation breadth;
- complete `0x140328540` lazy-realization error breadth;
- complete `0x140328FE0` compressed-seek error breadth;
- exhaustive malformed original error equivalence.

Binary AFS and original-runtime PACK remain evidence-gated. Capcom offline writer equivalence is not required for DMC Rengine product authoring.

## 7. Current work order

```text
1. close L1-M terminal materialization dependency from exact EXE evidence
2. bind direct-resource transport failure to completion suppression/retirement
3. extend the confirmed terminal mechanism to .lst child/recursive failure ordering
4. obtain real retail selected-member provenance
5. classify exact retail representation
6. perform one supported real edit/rebuild/rematerialization
7. execute #209 original-game consumption + rollback
8. run final contradiction-free L1 audit
9. mark L1 100% / COMPLETE only if every mandatory gate is valid
```

L2 protected selected-identity work and L3 lifecycle receipts may proceed in parallel when they directly support this vertical proof; they must not absorb or relabel L1 work.

## 8. Environment boundary

The connected environment still does not expose every protected-install artifact/process needed for real Level-E acceptance. Static L1-M reverse must use guarded canonical-analysis EXE acquisition/disassembly authority rather than substituting synthetic behavior.

## 9. Documentation synchronization

When L1-M or acceptance status changes, synchronize:

- `layer-boundary-status-reconciliation-2026-08-27.md`;
- this roadmap;
- `decompilation-layer-classification.md`;
- `master-roadmap.md`;
- `l1-final-audit-2026-08-25.md` or its successor;
- `l3-boundary-audit-2026-08-26.md` where cross-layer wording changes;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/canonical-status.json`;
- issues #100, #182 and #209.

Percentage estimates are planning aids only; mandatory gates are the completion authority.
