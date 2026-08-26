# GDSpaces — Materialization Completion Boundary Pass — 2026-08-26

**Current canonical base:** `main@2ed43b438f1bf01638f3e56341e98f6085e5b0fd`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Primary layers:** L1 support + L3 lifecycle boundary  
**Primary ledgers:** #100, #88, #55, #217  
**Focused acquisition plan:** `../../data/reverse/dmc3-materialization-completion-boundary-plan.v1.json`  
**Merged scheduler/context authority:** PR #230; stale-base #227 remains superseded history

## 1. Why this pass exists

Current main contains fresh raw-EXE L3 authority proving the central acquisition/finalization ordering:

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

The next useful reverse seam is therefore not another broad LoadedResource pass. It is the exact bridge between lower materialization work and the higher scheduler callback that publishes state2.

## 2. Layer ownership retained from current main

Canonical classification remains:

- FileSlot exact byte-read mechanics: **L1 support**;
- FileSlot/AsyncIO request ownership, scheduling and callback lifecycle: **L3**;
- `0x1401B8CA0`: explicit **L1/L3 seam** — representation/materialization mechanics are L1; its boolean result gates L3 state1 publication;
- LoadedResource states 0/1/2/3/4: **L3**;
- typed post-load and ready visibility: **L3**.

So the target is named:

> **materialization completion ordering / dependency bridge**

This is L1-supporting evidence and L3 lifecycle evidence at the same time. It does not redefine the canonical layer cut.

## 3. Main semantic correction — no generic fan-in counter is currently evidenced

Older shorthand used “fan-in/completion”. That must not be read as proof of an explicit original runtime child counter.

Current evidence does not directly establish:

- a generic outstanding-child counter;
- a universal `N children -> one parent callback` field;
- one aggregate object shared by direct, packed and `.lst` paths;
- a counter whose zero transition schedules `0x1401B8DC0`.

Canonical open question:

> What exact ordering/dependency mechanism guarantees that state2 is published only after required materialization work is valid?

Possible mechanisms remain evidence-gated: queue ordering, status polling, nested callbacks, synchronous completion, an unobserved counter/object, or another dependency mechanism.

## 4. Strong current-main raw-EXE boundaries

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

State1 is therefore a post-materialization-dispatch-success lifecycle state, not an unconditional allocation marker.

### `0x1401B8CA0`

Current main correctly classifies this as the L1/L3 seam. Its lower mechanics remain the relevant place to trace the actual materialization submission path.

### `0x1401B8DC0`

Current main confirms normal callback-context recovery and state2 publication. The unusual tagged/sentinel branch remains deliberately unnamed in merged authority.

`0x1401B8DC0` is not a raw FileSlot I/O callback.

## 5. PR #230 scheduler callback ABI is now merged-main authority

PR #227 first recovered this slice but was closed unmerged after `main` advanced through #221. PR #230 cleanly replayed the same raw-EXE evidence on the newer main line, preserved the merged L2 selected-identity/binder work, passed exact-head CI and merged as `2ed43b438f1bf01638f3e56341e98f6085e5b0fd`.

The canonical normal completion registration path is now:

```text
materialization success
 -> record.state = 1
 -> context = low32(record_ptr - 0x140C99D30)
 -> callback = 0x1401B8DC0
 -> argument_count = 1
 -> queue through 0x1402EF580
```

For the canonical 363-record registry, valid normal contexts are exactly:

```text
context = index * 0x48
index = 0..362
range = 0..0x65D0
```

Every valid normal context therefore has low bit zero.

The scheduler ABI is also now canonical:

```text
0x1402EF580
  queue record size = 0x88
  +0x04 argument/dispatch metadata
  +0x08 callback pointer
  +0x10... copied u32 callback arguments

0x1402EF790 one-argument dispatch
  ECX = dword(queue_record + 0x10)
  call qword(queue_record + 0x08)
```

For `0x1401B8DC0`, the copied argument is the record-relative context above.

### Consequence

The following are no longer first-priority unknowns:

- whether `1B8DC0` receives the same context produced by `1B84E0`;
- whether `2EF580` stores callback + one copied u32 argument;
- whether `2EF790` dispatches the one-argument callback through ECX;
- whether normal record contexts can enter the odd low-bit branch.

The odd branch is outside the recovered canonical normal acquisition-registration domain. Its semantic intent remains unresolved.

## 6. The key remaining dependency question is narrower

Merged #230 proves the completion callback queueing ABI, but it does **not** prove what materialization work `0x1402EF4D0` places ahead of that callback or how completion failure is represented.

The highest-value unresolved relation is now:

```text
0x1401B8CA0
 -> 0x1402EF4D0 materialization submission
 -> ??? materialization work / lower loader / scheduler records
 -> return success
 -> 0x1401B84E0 publishes state1
 -> 0x1402EF580 queues 1B8DC0(context)
 -> 0x1402EF790 eventually calls 1B8DC0
 -> state2
```

If `0x1402EF4D0` queues one or more materialization jobs into the same ordered scheduler before `1B8DC0`, queue order may be the dependency barrier. That is a testable hypothesis, not a promoted fact.

## 7. Lower transport boundary remains distinct

Preserved whole-file evidence remains:

```text
0x140033500
 -> 0x14002EA40 ReadRequest/FileSlot submission
 -> backend transport
 -> 0x1400335A0(ticketId,userContext,errorFlag,bytesRead)
```

`0x1400335A0` updates lower transfer progress/status. The missing bridge is the exact relation between that lower status and the materialization job/scheduler path.

The focused packet therefore keeps both transport and scheduler neighborhoods, but scheduler enqueue/worker are regression/context anchors rather than the central unknown.

## 8. Current first-priority raw targets

### A. `0x1402EF4D0` — primary

Close:

- actual function body boundaries;
- direct callees;
- whether it queues materialization work through `0x1402EF580` or another mechanism;
- whether it calls the `0x1400333F0/333500` whole-file family;
- first concrete consumer of the inherited materialization/load-context parameter;
- synchronous vs queued mode differences, if any;
- submit-failure return contract.

Do not label it exact-path resolver, final provider open, `ReadFile`, sync-only or async-only loader without direct evidence.

### B. `0x1402EF460` — cancellation/control comparator

Preserved evidence identifies it as a pending scheduler-entry clear/rollback candidate. Reacquire exact:

- queue selection/matching;
- record states it clears;
- whether it can touch an already-dispatched callback;
- whether it interacts with lower FileSlot/ReadRequest state or only the higher scheduler.

Do not relabel it OS `CancelIo`/AsyncIO cancellation without proof.

### C. transport -> materialization error bridge

Trace:

```text
0x1400335A0 error/status
 -> whole-file load state
 -> 0x1402EF4D0 / scheduled materialization work
 -> completion suppression or failure handling
```

The exact question is which branch prevents false state2 publication after lower transport failure.

## 9. Address-authority correction

The pre-#228 general blocked-window plan contains an extra-zero staging-helper VA:

```text
0x14002EF4D0
```

The accumulated canonical resource-runtime evidence identifies the relevant materialization wrapper as:

```text
0x1402EF4D0
```

These are different addresses. This branch corrects the general plan and the focused packet uses `0x1402EF4D0`.

The extra-zero form is treated as plan/address drift unless direct canonical bytes prove an independent intended target there.

## 10. `.lst` boundary

The confirmed loose-container materializer is `0x1401B85C0` in preserved direct-disassembly authority. It builds the parent in place, submits ordinary children/packed siblings and recursively synthesizes nested loose lists.

The open question is not grammar recovery. It is:

> How do child submission return/failure and recursive population interact with the materialization job ordering and final state2 publication?

A real `.lst` corpus remains separate validation.

## 11. Updated focused raw-byte questions

The next canonical-byte run should answer, in order:

1. What are the exact body/callee boundaries of `0x1402EF4D0`?
2. Does `0x1402EF4D0` queue one or more scheduler records before returning success?
3. If it uses `0x1402EF580`, what callback(s), argument count and context are queued for materialization work?
4. Does it call the whole-file loader family `0x1400333F0/333500`, another FileSlot opener, or multiple lower paths?
5. Where is the inherited materialization/load-context parameter first interpreted?
6. Is the state2 dependency guarantee explained by scheduler FIFO/order once `1B8DC0` registration is placed after materialization work?
7. How does `0x1400335A0` transport failure propagate upward?
8. Which branches suppress completion/state2 publication after submit/read/materialization failure?
9. What exactly does `0x1402EF460` match/clear, and can it affect already executing scheduler work?
10. What happens to already-running FileSlot/ReadRequest work during higher-level rollback?
11. For `.lst`, how does one child submission/population failure prevent a false successful parent completion?

Questions already answered by merged #230 should not be re-reversed except as exact regression anchors.

## 12. Completion consequence

This pass does not reopen NBZ/PAC/PNST materialization architecture and does not change L1/L2/L3 completion criteria.

It narrows one cross-layer seam to:

```text
materialization mechanics [L1]
 -> 0x1402EF4D0 submission/dependency ordering
 -> success gate
 -> LoadedResource state1 [L3]
 -> queued 1B8DC0(context) [L3 scheduler]
 -> state2 [L3]
```

Static closure improves the recovered model. Real-retail and original-process validation remain separate mandatory evidence gates.
