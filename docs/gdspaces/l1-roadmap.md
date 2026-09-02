# GDSpaces Layer 1 Roadmap

**Status:** **INCOMPLETE / NOT 100%**  
**Snapshot date:** 2026-08-27  
**Canonical base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Primary tracking:** #100, #182, #209  
**Boundary authority:** `layer-boundary-status-reconciliation-2026-08-27.md`  
**Focused byte-exactness authority:** PR #244 / `l1-byte-exactness-gap-pass-2026-08-27.md`

This is the canonical execution roadmap for **GDSpaces Layer 1 — Resource Materialization**.

The product implementation is advanced and contains a representative authoring/materialization path. **Layer 1 itself is not complete and its original EXE materialization reverse is not exhaustive.**

No canonical status may describe L1 as `COMPLETE`, `100%`, or as waiting only on external receipts.

## 1. Canonical L1 boundary

L1 begins after L2 establishes a usable selected provider/member identity. For original-runtime reverse accounting, L1 ends when the exact selected-resource byte representation and materializer success/error result are established through `0x1401B8CA0`.

```text
[L2] usable selected provider/member identity
 -> [L1] materialized-size authority
 -> destination capacity/allocation
 -> selected byte/span acquisition semantics
 -> final-chunk / EOF / short-read / progress semantics
 -> transform/decompression
 -> exact caller-owned destination bytes
 -> packed OR .lst synthesized representation
 -> nested PAC/PNST/.lst byte construction
 -> terminal materializer success/error
 -> 0x1401B8CA0 materialization result
 ===== END L1 ORIGINAL BYTE-MATERIALIZATION CUT =====
 -> [L3] scheduler/completion ownership
 -> LoadedResource state 1 -> 2
 -> typed post-load / state 2 -> 3 / lifecycle
```

Product-side provenance, edit, rebuild/repack, publication and reopen/rematerialization remain L1 responsibilities.

Important ownership rules:

- L2 owns selection identity, not selected-byte production;
- L1 owns byte size/extent/capacity/transform/exact representation semantics;
- FileSlot/ReadRequest byte-result semantics can be L1 while their queue/object/callback lifetime remains L3;
- `0x1402EF4D0` is a seam until its exact byte-producing job and scheduling behavior are separated;
- `state1 -> state2` is L3 lifecycle publication, not the L1 endpoint;
- `.lst` packed-first-vs-loose is L1 representation materialization.

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

These are advanced **product implementation slices**, not proof of exhaustive original L1 reverse.

## 3. Mandatory original L1 reverse gates

### L1-R1 — logical/materialized size authority

**OPEN / P0**

Close `0x14002F9F0 -> 0x140048E20`:

- physical vs ZIP logical size;
- STORE/compressed uncompressed-size authority;
- signed/unsigned width and error sentinels;
- zero-length resource semantics;
- missing vs size-query failure vs valid zero size.

### L1-R2 — rounded transfer versus exact byte extent

**OPEN / P0**

Recovered whole-file requests use `ceil(totalBytes/0x800)` chunks. Close:

- lower final-chunk clamp;
- physical and ZIP EOF behavior;
- short read vs progress vs terminal success/failure;
- loaded-byte completion condition;
- whether rounded tail bytes can ever reach destination memory.

Primary window: `0x140033390..0x1400335A0` plus the actual backend read dispatch.

### L1-R3 — destination capacity / allocation / initialization

**OPEN / P0**

Close `0x1401B7B90` and its allocation callees:

- required logical size vs rounded capacity;
- 64-byte alignment;
- integer-width/overflow behavior;
- allocation failure propagation;
- initial backing byte state;
- relationship between 0x800 transfer granularity and allocated capacity.

### L1-R4 — `.lst` representation planner/writer equivalence

**OPEN / P0**

Close `0x1401B79E0`, `0x1401B7FD0`, `0x1401B85C0`:

- packed/loose positive-size checks;
- planner and writer child-size authority;
- nested recursive size propagation;
- missing/zero child behavior;
- child return/failure propagation;
- temporary buffer cleanup;
- partial parent image state on failure.

### L1-R5 — synthesized padding / alignment bytes

**OPEN / P0 FOR BYTE-EQUIVALENCE CLAIM**

64-byte placement is known. Original padding contents are not yet proven. Determine whether gaps are zeroed, explicitly initialized, copied, or left unspecified.

Product zero-fill policy must not be promoted as original equivalence without evidence.

### L1-R6 — exact byte-producing ingress behind `0x1402EF4D0`

**OPEN / P0**

Keep the safe label `resource materialization submission/scheduling wrapper` until exact evidence separates:

- L1 byte-producing ingress and inherited materialization context;
- L3 queued-job ownership, persistence and callback lifetime.

Prove which lower whole-file/FileSlot path actually produces bytes and its success contract.

### L1-R7 — partial-read / transform terminal composition

**OPEN / P0**

Close composition for:

- STORE short reads;
- InflateRead partial production;
- early stream-end;
- no-progress handling;
- resubmission/progress behavior;
- malformed/truncated input exposing partially written destinations.

### L1-R8 — packed-child intrinsic-size boundary

**OPEN SCOPE LIMIT / MUST REMAIN EXPLICIT**

Relative slot starts do not prove a universal original child-size field. Keep these scopes separate:

1. layout-preserving patch against original packed bytes;
2. synthesized/reflowed representation only where exact child bytes/extent have independent evidence.

Do not claim a universal size-changing retail PAC/PNST packer from next-greater-offset extraction policy alone.

## 4. Cross-layer completion bridge

After direct byte-terminal semantics are exact, reconcile the seam that prevents L3 normal completion from publishing state2 on failed/incomplete L1 work.

Known:

- `0x1401B8DC0` receives only one u32 registry-relative context;
- it cannot inspect raw transport status/error/byte count;
- FIFO alone is not a proven barrier;
- no generic fan-in counter is evidenced.

Required seam closure:

1. identify `0x1402EF4D0` queued job identity/type and inherited context;
2. identify matching `0x1402EF790` dispatch and persistence/re-poll/retirement behavior;
3. bind terminal status from `0x1400333E0/0x140033390/0x1400335A0` after fresh reacquisition;
4. prove normal `0x1401B8DC0` suppression/removal on incomplete/failure;
5. recover relevant `0x1402EF460` pending-entry clear/rollback semantics;
6. apply the confirmed model to `.lst` recursive failure ordering.

This is an **L1/L3 seam**, not authority for moving LoadedResource `state1 -> state2` into L1.

## 5. Real acceptance gates

### L1-C — direct-retail representative provenance

**IMPLEMENTATION READY / REAL RECEIPT OPEN**

Preserve actual resolver winner, selected archive/member identity, archive/member hashes, central metadata, materialized SHA/size and transform/provenance.

### L1-D — exact retail representation classification

**REAL RECEIPT OPEN**

Classify exact selected bytes before choosing a writer. Unsupported representation opens a bounded evidence gate.

### L1-E — bounded real edit + rebuild

**PRODUCT IMPLEMENTATION READY / REAL-RETAIL RECEIPT OPEN**

Use only an evidenced writer domain and prove untouched spans exact.

### L1-F — next-volume publication + canonical reopen/rematerialization

**PRODUCT IMPLEMENTATION READY / REAL-RETAIL RECEIPT OPEN**

```text
rebuilt member
 -> next contiguous DMC3-N.nbz
 -> staged reopen
 -> authored higher-volume winner
 -> exact rebuilt-member rematerialization
 -> exact authored-child verification
```

### L1-G — original DMC3 consumption + rollback

**OPEN / EXTERNAL LEVEL-E / issue #209**

Require an attributable original-game effect and clean rollback. Crash-free launch alone is insufficient.

### L1-H — final contradiction-free audit

**OPEN**

Before `L1 COMPLETE / 100%`:

- declared original L1 reverse scope has no mandatory byte-exactness gap;
- cross-layer terminal/completion seam is reconciled sufficiently for the claimed scope;
- direct-retail selected-member provenance exists;
- exact retail representation is classified;
- real edit/rebuild/rematerialization receipt exists;
- original DMC3 consumption is attributable;
- rollback proves retail immutability;
- exact-head Windows + Ubuntu validation is green;
- #100, #182, #209, code, evidence and canonical docs agree;
- no unresolved contradiction changes the claimed scope.

## 6. Current work order

```text
1. close L1-R1 size/zero/error semantics
2. close L1-R2 final-chunk/EOF/short-read behavior
3. close lower physical/ZIP clamp behavior
4. close L1-R3 capacity/allocation/initial byte state
5. close L1-R4 planner/writer equivalence
6. close L1-R5 synthesized padding contents
7. close L1-R6 exact byte-producing ingress behind 0x1402EF4D0
8. close L1-R7 partial-read/transform terminal composition
9. reconcile the L1-terminal -> L3 state2 completion seam
10. preserve L1-R8 packed-child scope limit or replace it only with new evidence
11. obtain real-retail provenance and classify representation
12. run supported edit/rebuild/rematerialization
13. execute #209 original-game consumption + rollback
14. run final L1 audit
15. mark L1 100% / COMPLETE only if every mandatory gate is valid
```

## 7. Cross-layer dependencies

- **L2 supplies identity:** it does not own exact selected-byte production.
- **L3 supplies lifecycle:** scheduler/queue/callback/state publication remains L3 even when it must wait on an L1 terminal result.
- **V supplies promotion evidence:** hashes, CI and original-process receipts cannot replace missing static reverse.
- **DOMAIN consumes authority:** Stage Ops/ModViz do not define L1 truth.

## 8. Current completion labels

- `L1 product implementation`: advanced / representative-path implementation-ready at bounded scope;
- `L1 original EXE materialization reverse`: **NOT EXHAUSTIVE**;
- `L1 overall`: **INCOMPLETE / NOT 100%**.

Percentage estimates are planning aids only; gate closure is the authority.