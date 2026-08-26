# GDSpaces — Materialization Completion Boundary Pass — 2026-08-26

**Canonical repository base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Primary layers:** L1 support + L3 lifecycle boundary  
**Primary ledgers:** #100, #88, #55, #217  
**Focused acquisition plan:** `../../data/reverse/dmc3-materialization-completion-boundary-plan.v1.json`

## 1. Why this pass exists

The current main already has a fresh canonical raw-EXE L3 pass. It directly strengthens:

```text
0x1401B84E0
 -> 0x1401B8CA0 materialization dispatch
 -> only on success publish LoadedResource state1
 -> schedule 0x1401B8DC0

0x1401B8DC0
 -> normal completion writes state2

0x1401B92D0
 -> typed post-load
 -> optional callback
 -> state3
```

That new authority changes how the next materialization pass should be classified. The byte-read/materialization mechanics remain L1 or L1 support; scheduler ownership and LoadedResource states remain L3. The useful reverse seam is therefore cross-layer rather than a reason to redefine the canonical layer model.

## 2. Layer correction

Canonical classification retained from current main:

- FileSlot exact byte-read mechanics: **L1 support**;
- FileSlot/AsyncIO request ownership, scheduling and callback lifecycle: **L3**;
- `0x1401B8CA0`: explicit **L1/L3 seam** — representation/materialization mechanics are L1; its boolean result gates L3 state1 publication;
- LoadedResource states 0/1/2/3/4: **L3**;
- typed post-load and ready visibility: **L3**.

Therefore the next reverse target is not named “L1 state1→2 ownership”. It is:

> **materialization completion ordering / dependency bridge**

This bridge is supporting evidence for L1 byte correctness and direct L3 lifecycle evidence at the same time.

## 3. Main semantic correction — no generic fan-in counter is currently evidenced

Older shorthand used “fan-in/completion”. That is acceptable only as a loose description of multiple work items converging on one completion point. It must not be read as proof of an explicit original runtime child counter.

Current preserved evidence does not directly prove:

- a generic outstanding-child counter;
- a universal `N children -> one parent callback` field;
- one aggregate object shared by direct, packed and `.lst` paths;
- a counter whose zero transition schedules `0x1401B8DC0`.

Canonical open question:

> What exact ordering/dependency mechanism guarantees that resource completion is published only after required materialization work is valid?

Possible mechanisms remain open until exact bytes/dataflow prove one: scheduler ordering, nested callbacks, status polling, synchronous completion, an unobserved counter, or another dependency object.

## 4. Strong current boundaries from fresh raw-EXE authority

### `0x1401B84E0`

Fresh direct disassembly proves:

```text
record +0x18 <- descriptor/type authority
 -> prepare +0x28 backing
 -> record +0x20 <- payload/materialized destination/handle
 -> call 0x1401B8CA0
 -> only on success: state1
 -> schedule completion callback 0x1401B8DC0
```

This makes state1 publication a post-materialization-dispatch-success lifecycle event, not an unconditional allocation marker.

### `0x1401B8CA0`

Current main correctly classifies this as the L1/L3 seam. The mechanics inside it still require exact lower-call closure where needed; the return value is already proven to control whether L3 state1 is published.

### `0x1401B8DC0`

Fresh raw-EXE disassembly confirms normal callback-context recovery and state2 publication. The unusual tagged/sentinel branch remains deliberately unnamed.

`0x1401B8DC0` is not a raw FileSlot I/O callback.

## 5. Lower transport boundary still distinct

Preserved canonical whole-file evidence remains:

```text
0x140033500
 -> 0x14002EA40 ReadRequest/FileSlot submission
 -> backend transport
 -> 0x1400335A0(ticketId,userContext,errorFlag,bytesRead)
```

`0x1400335A0` updates lower transfer progress/status. The missing bridge is the exact relation between that lower status and the higher materialization/scheduler path which eventually allows state2 publication.

The focused packet therefore reacquires both sides rather than assuming that two completion-like callbacks are directly chained.

## 6. Scheduler targets

The next exact-byte pass must include the neighborhood around:

- `0x1402EF460` — pending scheduler-entry clear/rollback candidate from preserved direct evidence;
- `0x1402EF4D0` — materialization submission/scheduling wrapper;
- `0x1402EF580` — scheduler enqueue;
- `0x1402EF790` — scheduler worker.

Important evidence discipline:

- `0x1402EF460` must not be promoted as OS AsyncIO cancellation unless direct dataflow proves it;
- `0x1402EF4D0` must not be labeled exact-path resolver, final provider open, `ReadFile`, sync-only loader or async-only loader without direct body/callee proof;
- the inherited fourth materialization/load-context parameter remains mechanically propagated but semantically unnamed.

## 7. Address-authority correction

Current main blocked-window plan contains a staging-helper entry at:

```text
0x14002EF4D0
```

The accumulated canonical resource-runtime evidence identifies the materialization wrapper as:

```text
0x1402EF4D0
```

These are different VAs. The focused packet uses `0x1402EF4D0` and treats the extra-zero form as an address-drift defect until independently contradicted by exact byte evidence.

The general blocked plan should be corrected before it is reused for this target.

## 8. `.lst` boundary

The confirmed original loose-container materializer is anchored at `0x1401B85C0` in the preserved direct-disassembly authority. It builds the parent in place, submits ordinary children/packed siblings and recursively synthesizes nested loose lists.

The exact open question is not “find the grammar again”. It is:

> How do child submission return/failure and recursive population interact with the higher completion ordering?

A real `.lst` corpus remains a separate validation gate; this pass is static control/dataflow work.

## 9. Focused raw-byte questions

The next canonical-byte run should answer in this order:

1. What are the actual body/callee boundaries of `0x1402EF4D0`?
2. Does it call the whole-file loader family, enqueue scheduler work, or select multiple lower paths?
3. What object/context is handed from materialization submission to FileSlot/ReadRequest transport?
4. Where is the inherited load-context parameter first interpreted?
5. In what order are materialization work and `0x1401B8DC0` callback registration inserted/executed?
6. Is the dependency guarantee ring order, status polling, nested callback sequencing, synchronous completion, or an explicit counter/object?
7. How does `0x1400335A0` error status propagate upward?
8. Which branches suppress state1 or state2 publication after failure?
9. What exactly does `0x1402EF460` match/clear, and can it affect already executing scheduler work?
10. What happens to an already running FileSlot/ReadRequest during higher-level cancellation/rollback?
11. For `.lst`, how does one child submission/population failure prevent a false successful parent completion?

## 10. Completion consequence

This pass does not reopen the already promoted NBZ/PAC/PNST materialization core and does not change L1 or L3 completion criteria.

It narrows one supporting cross-layer seam:

```text
exact byte/materialization work [L1 support]
 -> success/failure bridge
 -> request/scheduler ownership [L3]
 -> state1
 -> materialization completion callback
 -> state2
```

Static closure improves the recovered model. Real-retail/original-process receipts remain mandatory for final layer completion claims.
