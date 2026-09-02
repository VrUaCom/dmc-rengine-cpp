# GDSpaces — Materialization Completion Dependency Pass 2 — 2026-08-26

**Current-main reconciliation base:** `main@a90b017ab29171e00174f2a56c719c32241a63f1`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Primary target:** materialization completion ordering / dependency bridge  
**Authority used:** merged #228 + merged raw scheduler ABI from #230; historical Pass-90 trace is reacquisition hypothesis only.  
**Ownership/status interpretation superseded:** 2026-08-27 by `layer-boundary-status-reconciliation-2026-08-27.md` and PR #244.

> The technical dependency findings in this pass remain valid. The later reconciliation changes only the layer/completion interpretation: exact byte-terminal semantics are L1; queue/request/callback ownership and normal LoadedResource `state1 -> state2` publication remain L3; the dependency that binds the L1 terminal result to an allowed/suppressed L3 completion is a cross-layer seam. PR #244 also establishes additional L1 byte-exactness gaps that must be closed before this scheduler seam can be treated as the only remaining internal question.

## 1. Scope

This pass does not claim a fresh raw re-disassembly of `0x1402EF4D0`. It narrows the scheduler/completion dependency after merged #228/#230:

```text
0x1401B8CA0 materializer interaction
 -> 0x1402EF4D0 submission/job creation
 -> lower whole-file/FileSlot work
 -> terminal dependency/completion condition
 -> queued 0x1401B8DC0(record-relative-context)
 -> LoadedResource state2 publication
```

No generic child/outstanding-work fan-in counter is claimed.

## 2. Hard narrowing from merged #230

Normal acquisition registers `0x1401B8DC0` with one u32 context:

```text
context = record_ptr - 0x140C99D30
```

The callback reconstructs the LoadedResource record and writes state2. It does not receive transport status, error flag, byte count, FileSlot handle, child count or outstanding-work metadata.

Therefore raw materialization success/failure cannot be decided inside `0x1401B8DC0`. By dispatch time, the L1 byte/materializer result must already be terminal, or the L3 queued completion must have been suppressed/removed.

## 3. FIFO alone is insufficient

A model of “materialization job queued first, completion queued second, therefore FIFO proves correctness” is insufficient if the first job can submit asynchronous I/O and retire immediately.

A valid dependency requires direct evidence for at least one stronger property:

1. the earlier scheduler record persists/re-polls until lower transfer is terminal;
2. a lower callback publishes terminal state before that record retires;
3. another dependency gate blocks later completion dispatch;
4. failure removes/invalidates the queued normal completion;
5. or the path is proven synchronous at the relevant scope.

## 4. Historical Pass-90 trace — reacquisition hypothesis only

Historical derivative evidence recorded a candidate chain:

```text
0x1402EF4D0 enqueue direct-read job
 -> 0x1402EF790 process/poll
 -> 0x1400333F0 VFS open
 -> 0x1400333C0 chunk count
 -> 0x140033500 submit caller-owned destination read
 -> status/poll around 0x1400333E0
 -> 0x140033390 release load-state
```

Those helper roles remain focused reacquisition hypotheses until fresh canonical bytes confirm them.

## 5. Falsifiable dependency models

- **H1 persistent scheduler job:** the L3 scheduler record remains alive/re-dispatchable while L1 byte work is pending.
- **H2 callback-driven terminal state:** lower completion writes terminal status consumed before the scheduler retires.
- **H3 separate dependency gate:** the job may retire, but another condition blocks normal completion.
- **H4 synchronous completion:** materialization is already terminal before successful scheduling returns.

No hypothesis is promoted without direct evidence.

## 6. Error-path consequence

`0x1400335A0` carries lower transfer result information; normal `0x1401B8DC0` does not.

Therefore failure must be resolved before normal state2 publication, conceptually as:

```text
[L1] transfer/materializer terminal failure
 -> [SEAM] completion not eligible / queued completion cleared
 -> [L3] normal B8DC0 does not dispatch
```

`0x1402EF460` remains safely labeled **pending scheduler-entry clear/rollback**. It is not automatically OS AsyncIO cancellation.

## 7. Focused anchors

- `0x140033390` — historical terminal load-state cleanup anchor;
- `0x1400333E0` — historical status/poll anchor;
- `0x140033500` — transfer submit;
- `0x1400335A0` — lower transfer completion/result;
- `0x1402EF4D0` — materialization submission/scheduling wrapper;
- `0x1402EF790` — scheduler dispatch/persistence/retirement;
- `0x1402EF460` — pending-entry clear/rollback;
- `0x1401B8DC0` — L3 normal state2 publication regression anchor.

## 8. Revised use order after PR #244

This pass is no longer the first L1 raw step. First close the direct byte-exactness questions defined by PR #244:

```text
size/zero/error
 -> final-chunk/EOF/short-read
 -> capacity/allocation/initialization
 -> .lst planner/writer/padding/failure
 -> exact byte-producing ingress behind 0x1402EF4D0
 -> then this L1-terminal / L3-completion dependency seam
```

Within this seam, investigate:

1. the queued-job half of `0x1402EF4D0` and inherited context;
2. matching `0x1402EF790` persistence/re-poll/retirement;
3. fresh `0x1400333E0` pending/success/error semantics;
4. fresh `0x140033390` terminal cleanup ordering;
5. `0x1400335A0` result binding;
6. what prevents normal `0x1401B8DC0` on incomplete/failure;
7. relevant `0x1402EF460` suppression/rollback;
8. `.lst` recursive failure ordering using the confirmed mechanism.

## 9. Promotion boundary

Stronger after this pass:

- `0x1401B8DC0` cannot decide raw transport success/error itself;
- dependency correctness must be established before normal state2 dispatch;
- FIFO alone is insufficient absent completion-aware persistence/retirement;
- historical Pass-90 roles stay hypotheses until reacquired.

Still not claimed:

- exact canonical `0x1402EF4D0` job body;
- exact `0x1400333E0` values;
- exact failure suppression path;
- already-running lower-I/O cancellation behavior;
- `.lst` child failure aggregation;
- original-process timing equivalence.

## 10. Corrected completion consequence

This dependency is mandatory to reconcile the **L1 terminal result with L3 normal completion**, but it is not authority for moving all completion machinery or `state1 -> state2` into L1.

Canonical split:

```text
L1: exact byte size/extent/capacity/transform/result
SEAM: terminal result -> completion eligibility/suppression
L3: queue/request/callback ownership + normal state1 -> state2 publication
```

L1 remains **INCOMPLETE / NOT 100%** because PR #244 exposes unresolved original byte-exactness gaps and because real-retail/original-game acceptance is still open. L3 remains **INCOMPLETE / NOT 100%** for its own scheduler/lifecycle breadth and original-process receipts.