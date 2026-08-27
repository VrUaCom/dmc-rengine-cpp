# GDSpaces — Materialization Completion Dependency Pass 2 — 2026-08-26

**Current-main reconciliation base:** `main@a90b017ab29171e00174f2a56c719c32241a63f1`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Primary target:** materialization completion ordering / dependency bridge  
**Authority used:** merged #228 + merged raw scheduler ABI from #230; historical Pass-90 direct-resource trace is reacquisition hypothesis only.  
**Status interpretation superseded:** 2026-08-27 by `layer-boundary-status-reconciliation-2026-08-27.md`.

> The technical findings in this pass remain valid. Only the old status sentence saying this gap did not change L1 completion criteria is superseded. The unresolved terminal dependency is now a mandatory L1 completion gate, and selected-byte FileSlot/transport/completion through normal state2 publication is canonically L1.

## 1. Scope

This pass does not claim a fresh raw re-disassembly of `0x1402EF4D0`. The connected file surface in that session did not expose a fresh canonical `e454...` raw executable blob/window for direct byte inspection.

It narrows the remaining question after merged #228/#230:

```text
0x1401B8CA0 materialization mechanics
 -> 0x1402EF4D0 submission/job creation
 -> lower whole-file/FileSlot work
 -> terminal dependency/completion condition
 -> queued 0x1401B8DC0(record-relative-context)
 -> state2 publication
```

No generic child/outstanding-work fan-in counter is claimed.

## 2. Hard narrowing from merged #230

Normal acquisition registers `0x1401B8DC0` with exactly one u32 argument:

```text
context = record_ptr - 0x140C99D30
```

The normal callback reconstructs the record and writes state2. It does not receive transport status, error flag, byte count, FileSlot handle, child count or outstanding-work metadata.

Therefore the normal callback cannot itself decide whether raw materialization succeeded. By the time `0x1401B8DC0` dispatches, success/failure eligibility must already have been resolved, or the queued completion must have been suppressed/removed before dispatch.

## 3. FIFO alone is insufficient

A simple model such as:

```text
materialization job queued first
 -> completion callback queued second
 -> FIFO guarantees correctness
```

is insufficient if the first job merely submits async I/O and retires immediately. In that case the later completion record could dispatch while FileSlot transport is still pending, and `0x1401B8DC0` has no transport-status input to reject early state2.

If scheduler order is the dependency barrier, at least one stronger property must hold:

1. the materialization scheduler record persists/re-polls until lower transfer reaches terminal state;
2. a lower callback marks shared state terminal before the scheduler record retires;
3. another status/dependency gate blocks later dispatch;
4. failure removes/invalidates the queued completion record before it can execute.

## 4. Historical Pass-90 trace — reacquisition hypothesis only

Historical derivative evidence recorded a candidate direct-resource chain:

```text
0x1402EF4D0 enqueue direct-read job
 -> 0x1402EF790 process/poll job
 -> 0x1400333F0 VFS open
 -> 0x1400333C0 chunk count
 -> 0x140033500 submit caller-owned destination read
 -> status/poll around 0x1400333E0
 -> 0x140033390 release load-state
```

Because that pass did not use a fresh canonical raw-byte window, these helper roles are not promoted solely from the historical trace. They become focused exact-byte reacquisition targets.

## 5. Falsifiable dependency models

### H1 — persistent polling scheduler job

The materialization scheduler record stays live/re-dispatchable while whole-file status is pending and retires only on terminal success/error.

### H2 — callback-driven terminal state

`0x1400335A0` writes a terminal shared status; the scheduler observes it and retires the materialization job only after that transition.

### H3 — separate scheduler gate/status dependency

The materialization job may retire after submission, but a separate scheduler condition prevents later completion records from dispatching until terminal state changes.

### H4 — synchronous completion before `0x1402EF4D0` success

`0x1402EF4D0` may not return successful scheduling until bytes are already complete. Existing whole-file async evidence makes this less attractive, but it is not rejected without the canonical body.

## 6. Error-path consequence

`0x1400335A0` exposes lower transport success/error information; normal `0x1401B8DC0` does not.

Therefore transport failure must be handled before normal state2 dispatch by a mechanism equivalent to:

```text
transport error
 -> materialization job terminal-failed / not retired as success
 -> completion callback cannot execute normally
```

or:

```text
transport error
 -> queued completion entry is cleared/rolled back before dispatch
```

`0x1402EF460` remains the key cancellation/control comparator, with the bounded label **pending scheduler-entry clear/rollback**. It is not promoted as OS AsyncIO cancellation.

## 7. Focused acquisition anchors

The focused plan needs the terminal cluster explicitly:

- `0x140033390` — historical load-state release/close anchor;
- `0x1400333E0` — historical whole-file status/poll anchor;
- `0x140033500` — transfer submit;
- `0x1400335A0` — transport completion/status write;
- `0x1402EF4D0` — materialization submission/job creation;
- `0x1402EF790` — materialization-job dispatch/persistence/retirement case;
- `0x1402EF460` — pending scheduler clear/rollback;
- `0x1401B8DC0` — regression anchor for normal state2 publication.

## 8. Revised raw-pass order

1. close `0x1402EF4D0` queued job identity/type and inherited load-context consumer;
2. identify the corresponding `0x1402EF790` dispatch case and whether it persists/re-polls;
3. recover `0x1400333E0` pending/success/error domain;
4. recover `0x140033390` terminal cleanup/release point;
5. bind `0x1400335A0` transport writes into that state;
6. determine what prevents later `0x1401B8DC0` dispatch on incomplete/failed transfer;
7. recover `0x1402EF460` suppression/rollback of queued higher work;
8. only then apply the confirmed direct-resource mechanism to `.lst` child/recursive failure ordering.

## 9. Promotion boundary

Stronger after this pass:

- normal `0x1401B8DC0` cannot be the place where raw transport success/error is decided;
- dependency correctness must be established before normal `0x1401B8DC0` dispatch;
- FIFO insertion order alone is insufficient unless the earlier materialization work has completion-aware persistence/retirement semantics;
- terminal polling/status and load-state release are first-class evidence targets;
- historical Pass-90 semantics are explicitly downgraded to reacquisition hypotheses where fresh canonical bytes are absent.

Still not claimed:

- exact canonical `0x1402EF4D0` job body;
- scheduler FIFO as the proven barrier;
- exact `0x1400333E0` status values;
- exact transport-error suppression path;
- already-running FileSlot cancellation behavior;
- `.lst` child failure aggregation;
- original-process timing equivalence.

## 10. Completion consequence — reconciled 2026-08-27

This unresolved dependency is now a **mandatory Layer-1 completion gate**.

The product authoring/materialization implementation may remain advanced and representative-path-ready, but L1 cannot be reported as `COMPLETE`, `100%`, or as having only external receipts remaining while this terminal condition is unresolved.

Canonical ownership after reconciliation:

```text
selected-byte FileSlot/ReadRequest transport -> L1
transport completion/status needed for materialization -> L1
materialization job terminal dependency -> L1
normal state1 -> state2 publication -> L1 end boundary
state2 typed post-load -> L3 start
```

The next reverse question remains:

> **What terminal condition keeps or releases the materialization scheduler job, and how does that condition prevent normal `0x1401B8DC0` from dispatching on failed or incomplete transport?**
