# GDSpaces — Materialization Completion Boundary Pass — 2026-08-26

**Current-main reconciliation base:** `main@a90b017ab29171e00174f2a56c719c32241a63f1`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Primary layers:** L1 support + L3 lifecycle boundary  
**Primary ledgers:** #100, #88, #55, #217  
**Focused acquisition plan:** `../../data/reverse/dmc3-materialization-completion-boundary-plan.v1.json`  
**Merged scheduler/context authority:** #230  
**Pass-2 follow-up:** [`materialization-completion-dependency-pass2-2026-08-26.md`](materialization-completion-dependency-pass2-2026-08-26.md)

## 1. Why this pass exists

Current canonical raw-EXE authority proves the central acquisition/finalization ordering:

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

The useful reverse seam is therefore the exact bridge between lower materialization work and the higher scheduler callback that publishes state2.

## 2. Layer ownership

Canonical classification remains:

- FileSlot exact byte-read mechanics: **L1 support**;
- FileSlot/AsyncIO request ownership, scheduling and callback lifecycle: **L3**;
- `0x1401B8CA0`: explicit **L1/L3 seam** — representation/materialization mechanics are L1; boolean success gates L3 state1 publication;
- LoadedResource states 0/1/2/3/4: **L3**;
- typed post-load and ready visibility: **L3**.

The target is therefore:

> **materialization completion ordering / dependency bridge**

This is L1-supporting evidence and L3 lifecycle evidence simultaneously. It does not redefine the layer model.

## 3. Main correction — no generic fan-in counter is evidenced

Older shorthand used `fan-in/completion`. That must not be read as proof of an explicit original child counter.

Current evidence does not directly establish:

- a generic outstanding-child counter;
- a universal `N children -> one parent callback` field;
- one aggregate object shared by direct, packed and `.lst` paths;
- a counter whose zero transition schedules `0x1401B8DC0`.

Canonical open question:

> What ordering/dependency mechanism guarantees that state2 is published only after required materialization work is valid?

Possible mechanisms remain evidence-gated: scheduler lifecycle/order, status polling, lower callbacks, synchronous completion, an unobserved counter/object, or another mechanism.

## 4. Strong raw-EXE boundaries

### `0x1401B84E0`

Fresh direct authority proves:

```text
record +0x18 <- descriptor/type authority
 -> prepare +0x28 backing
 -> record +0x20 <- payload/materialized destination/handle
 -> call 0x1401B8CA0
 -> only on success: state1
 -> schedule completion callback 0x1401B8DC0
```

State1 is a post-materialization-dispatch-success lifecycle state, not an unconditional allocation marker.

### `0x1401B8CA0`

This is the L1/L3 seam. Its lower mechanics remain the place to trace actual materialization submission.

### `0x1401B8DC0`

Normal callback-context recovery and state2 publication are canonical. It is not a raw FileSlot I/O callback.

## 5. Merged #230 scheduler ABI

The canonical normal completion registration is:

```text
materialization success
 -> record.state = 1
 -> context = low32(record_ptr - 0x140C99D30)
 -> callback = 0x1401B8DC0
 -> argument_count = 1
 -> queue through 0x1402EF580
```

Valid normal contexts are `index * 0x48` for `index=0..362`, so their low bit is zero.

Scheduler ABI:

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

For normal `0x1401B8DC0`, the copied argument is only the registry-relative record context.

## 6. Hard narrowing from Pass 2

The normal `0x1401B8DC0` callback does **not** receive:

- transport status/error;
- bytes read;
- FileSlot/ReadRequest handle;
- child/outstanding-work metadata.

Therefore the final success/error dependency decision cannot happen inside normal `0x1401B8DC0` itself.

By dispatch time, the system must already have established that state2 publication is permissible, or the queued completion must have been removed/suppressed.

### FIFO-only hypothesis is insufficient

A model such as:

```text
materialization job queued first
 -> B8DC0 queued second
 -> FIFO guarantees correctness
```

is insufficient if the materialization job merely submits async I/O and retires immediately. The later completion could then run before FileSlot transport is terminal, and `B8DC0` has no lower-status input to reject that early transition.

If scheduler order is the actual barrier, the materialization job must have completion-aware persistence/retirement semantics, or another gate must exist.

## 7. Lower transport boundary

Preserved whole-file evidence remains:

```text
0x140033500
 -> 0x14002EA40 ReadRequest/FileSlot submission
 -> backend transport
 -> 0x1400335A0(ticketId,userContext,errorFlag,bytesRead)
```

`0x1400335A0` updates lower transfer progress/status. The missing bridge is the relation between that lower status and the materialization scheduler job.

Historical derivative Pass-90 evidence also identified candidate helpers:

- `0x1400333E0` — status/poll;
- `0x140033390` — terminal load-state release/close.

These two roles are now **canonical reacquisition hypotheses**, not promoted semantics until fresh `e454...` bytes confirm them.

## 8. Falsifiable dependency models

The next raw pass should distinguish:

### H1 — persistent polling scheduler job

The materialization scheduler record remains live/re-dispatchable until whole-file status is terminal.

### H2 — callback-driven terminal state

`0x1400335A0` writes shared status; the higher scheduler job observes it and retires only after terminal success/error.

### H3 — separate scheduler gate

The materialization job may retire after submission, but another status/dependency condition prevents later completion dispatch.

### H4 — synchronous completion before `0x1402EF4D0` success

`0x1402EF4D0` may not report success until bytes are terminal. This remains possible until the canonical body is closed.

No generic counter is privileged over these alternatives.

## 9. Cancellation/control comparator

`0x1402EF460` retains the bounded label:

> **pending scheduler-entry clear/rollback**

It is useful for determining whether a queued `0x1401B8DC0` completion can be suppressed after higher-level cancellation/failure.

Do not relabel it OS `CancelIo`/AsyncIO cancellation without direct lower-I/O interaction.

## 10. `.lst` boundary

The confirmed loose-container materializer is `0x1401B85C0`. It builds the parent in place, submits ordinary children/packed siblings and recursively synthesizes nested loose lists.

Do not reopen grammar/layout. The remaining question is child submission/failure/recursive ordering relative to the direct-resource terminal dependency mechanism.

Apply the direct-resource model first; only then generalize to `.lst` child failure.

## 11. Address authority

The extra-zero address:

```text
0x14002EF4D0
```

is superseded for the materialization-wrapper target. Canonical accumulated evidence identifies:

```text
0x1402EF4D0
```

Both general and focused acquisition plans use `0x1402EF4D0`.

## 12. Revised exact-byte priority

```text
1. 0x1402EF4D0 — exact queued job identity/type, callees, inherited load-context consumer
2. 0x1402EF790 — materialization-job dispatch case, persistence/re-poll/retirement
3. 0x1400333E0 — pending/success/error domain
4. 0x140033390 — terminal cleanup/release point
5. 0x1400335A0 — lower transport writes into that state
6. identify what prevents normal 0x1401B8DC0 dispatch on failed/incomplete transport
7. 0x1402EF460 — queued higher-work suppression/rollback
8. .lst child/recursive failure ordering
```

Questions already answered by merged #230 are regression anchors, not first-priority unknowns.

## 13. Non-claims

This pass does not claim:

- a generic fan-in counter;
- scheduler FIFO as the proven dependency barrier;
- exact `0x1400333E0` status values;
- exact `0x140033390` role before reacquisition;
- `0x1402EF460` is OS AsyncIO cancellation;
- exact already-running FileSlot cancellation behavior;
- `.lst` failure aggregation is closed;
- original-process timing equivalence;
- any L1/L2/L3 completion status change.

## 14. Completion consequence

The remaining reverse frontier is now a small state-machine question:

> **What terminal condition keeps or releases the materialization scheduler job, and how does that condition prevent normal `0x1401B8DC0` from dispatching on failed or incomplete transport?**

Static closure strengthens the recovered model. Real-retail and original-process validation remain separate mandatory evidence gates.
