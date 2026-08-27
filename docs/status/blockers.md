# Current Blockers

**Snapshot date:** 2026-08-27  
**Canonical base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Boundary/status authority:** `../gdspaces/layer-boundary-status-reconciliation-2026-08-27.md`  
**Focused L1 gap authority:** PR #244

## P0 — GDSpaces L1 completion blockers

**L1 = INCOMPLETE / NOT 100%.** Product maturity does not close original byte-equivalence or real acceptance.

### B-L1-00 — Materialized-size / zero / error authority

**Status:** MANDATORY STATIC REVERSE OPEN

Close `0x14002F9F0 -> 0x140048E20`: exact logical/uncompressed size, error sentinel domain and zero-size behavior.

### B-L1-01 — Rounded transfer / final byte extent

**Status:** MANDATORY STATIC REVERSE OPEN

Close `ceil(totalBytes/0x800)` request versus exact logical EOF, physical/ZIP final-chunk clamp, short-read/progress/no-progress and success/failure composition around `0x140033390..0x1400335A0` plus the actual backend read path.

### B-L1-02 — Capacity / allocation / initialization

**Status:** MANDATORY STATIC REVERSE OPEN

Close `0x1401B7B90`: logical size vs rounded capacity, 64-byte alignment, integer width/overflow, allocation failure and initial backing-byte state.

### B-L1-03 — `.lst` planner/writer/failure equivalence

**Status:** MANDATORY STATIC REVERSE OPEN

Close `0x1401B79E0`, `0x1401B7FD0`, `0x1401B85C0`: packed/loose tests, planner/writer size authority, recursive propagation, child failure ordering, temporary cleanup, partial parent state and original padding-byte contents.

### B-L1-04 — Exact byte-producing ingress behind `0x1402EF4D0`

**Status:** MANDATORY STATIC REVERSE OPEN

Recover the actual byte-producing job/path and inherited materialization context without assigning the entire scheduler helper to L1. Queue/persistence/callback ownership remains L3 behavior unless direct evidence says otherwise.

### B-L1-05 — Partial-read / transform terminal composition

**Status:** MANDATORY STATIC REVERSE OPEN

Close STORE short-read, InflateRead partial production, early stream-end, retry/progress/no-progress and truncated-input behavior so partially written destinations cannot be mislabeled as successfully materialized.

### B-L1-06 — Packed-child extent scope boundary

**Status:** OPEN SCOPE LIMIT

Relative slot starts do not prove a universal original intrinsic child length. Layout-preserving patch scope and synthesized/reflowed scope must stay distinct unless new evidence closes this boundary.

### B-SEAM-01 — L1 terminal result -> L3 normal completion

**Status:** MANDATORY CROSS-LAYER RECONCILIATION

Known:

```text
[L1] terminal byte/result state
 -> UNKNOWN exact dependency / eligibility mechanism
 -> [L3] normal 0x1401B8DC0
 -> LoadedResource state1 -> state2
```

Normal `0x1401B8DC0` receives only one registry-relative u32 context and cannot inspect raw transfer status itself. After direct byte-terminal semantics are exact, reconcile:

- `0x1402EF4D0` byte job versus queued-job ownership;
- relevant `0x1402EF790` persistence/re-poll/retirement;
- fresh `0x1400333E0` / `0x140033390` semantics;
- `0x1400335A0` result binding;
- failed/incomplete completion suppression;
- relevant `0x1402EF460` pending-entry clear/rollback;
- `.lst` recursive failure ordering.

No generic fan-in counter is evidenced and FIFO alone is insufficient.

This seam does **not** reclassify LoadedResource `state1 -> state2` as L1; state publication remains L3.

### B-L1-07 — Direct-retail representative provenance

**Status:** REAL-RETAIL RECEIPT REQUIRED

Preserve actual L2 resolver winner, archive/member identity, hashes/central metadata and exact materialized SHA/size/transform provenance.

### B-L1-08 — Exact retail representation classification

**Status:** REAL EVIDENCE REQUIRED

Classify exact selected bytes before applying a writer.

### B-L1-09 — Real edit/rebuild/rematerialization

**Status:** REAL VALIDATION REQUIRED

Prove one supported real bounded edit, bottom-up rebuild, next-volume publication and exact rematerialization.

### B-L1-10 — Original DMC3 consumption + rollback

**Status:** FINAL EXTERNAL ACCEPTANCE REQUIRED / issue #209

Require an attributable original-game effect and rollback that preserves retail immutability. Crash-free launch is not enough.

### B-L1-11 — Final L1 audit

**Status:** OPEN / DEPENDS ON ALL MANDATORY DECLARED-SCOPE GATES

L1 can be `COMPLETE / 100%` only if static byte-exactness, required cross-layer seam evidence, real receipts, exact-head validation and canonical docs all agree without contradiction.

## Layer 2 blockers

**L2 = INCOMPLETE / NOT 100%.**

- **B-L2-01:** exact retail `0x0E` normalized-key collision census;
- **B-L2-02:** real protected-process multi-anchor mapping receipt;
- **B-L2-03:** trusted zero-loss original-process selected-provider identity;
- **B-L2-04:** direct-retail resolver identity promotion after collision state is known;
- **B-L2-05:** final contradiction-free L2 audit.

Selection and failure before a usable selected resource exist are L2. Exact selected-byte production is L1.

## Layer 3 blockers

**L3 = INCOMPLETE / NOT 100%.**

Open L3 work includes:

- scheduler/request/callback persistence details needed at B-SEAM-01;
- residual LoadedResource writer/value-flow census;
- family-complete ownership of `+0x08/+0x18/+0x20/+0x28` and stable fields;
- external typed/factory/dependency failure breadth;
- SCM `mesh +0x28` reconciliation;
- shared-owner coordination breadth;
- cross-build/profile differences;
- protected original-process V1–V7 lifecycle receipts;
- final contradiction-free L3 audit.

Normal LoadedResource `state1 -> state2` remains L3 lifecycle state publication. The L1 byte-terminal prerequisite must not be conflated with its owner.

## Strong slices not to reopen without contradiction

### L1/product

- artifact-bound acquisition implementation;
- atomic/no-replace publication;
- ZIP method-0/raw-DEFLATE core architecture;
- PAC/PNST sparse relative-slot structure and recursive traversal;
- bounded writer/product paths already promoted.

Strong structure does not erase open size/EOF/capacity/padding/error semantics.

### L2

- numbered-volume/bootstrap structure;
- basename candidates/provider order;
- archive normalization/index/search;
- bounded type-0 physical static chain.

### L3

- registry `363 x 0x48` and seven groups;
- normal state1 -> state2 callback ABI/publication;
- state2 typed-ready -> state3;
- cancellation `1|2 -> 4`;
- quiescence `{0,3}`;
- state4 cleanup and distinct release/reset paths;
- representative typed families and bounded loader-node claims;
- runtime/CRT/process teardown distinction.

## Evidence-gated freezes

- Binary AFS is not inferred from `.afs/` strings.
- Historical PACK parsing is not original DMC3 runtime authority.
- Capcom offline-writer equivalence is not required for bounded DMC Rengine product authoring.
- Stage Ops/ModViz do not count as L1/L2/L3 completion.

## Environment boundary

The connected environment does not expose every exact protected-install artifact/process required for real retail and original-process gates. Synthetic CI must not replace those receipts.