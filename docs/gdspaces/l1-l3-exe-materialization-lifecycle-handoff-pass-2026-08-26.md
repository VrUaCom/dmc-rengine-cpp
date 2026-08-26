# GDSpaces L1/L3 — EXE Materialization-to-Lifecycle Handoff Pass

**Pass date:** 2026-08-26  
**Canonical repository base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Primary ledgers:** #100, #88, #55, #209  
**Canonical L3 raw authority:** `l3-boundary-audit-2026-08-26.md`  
**Focused acquisition plan:** `../../data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json`

## 1. Why this pass changed the boundary model

This pass began as another L1 review of:

```text
materialization submission
 -> transport completion
 -> scheduler completion
 -> LoadedResource state 1 -> 2
```

During the pass, `main` advanced with the canonical raw-EXE L3 boundary audit. That audit establishes a more precise semantic cut:

```text
L2 selected logical/provider identity
 -> L1 exact materialized bytes
 -> [L3 START]
    FileSlot / async request ownership and scheduling
    LoadedResource acquisition
    state 0 -> 1
    completion state 1 -> 2
    typed post-load
    state 2 -> 3
    lifecycle / release / reset / teardown
```

FileSlot is therefore a boundary subsystem:

- physical byte-read mechanics can support L1;
- request ownership, scheduling, completion, cancellation and close lifetime are L3.

This supersedes the earlier shorthand that placed the LoadedResource `state 1 -> 2` publication at the end of L1.

## 2. Correct L1/L3 handoff

The safe architecture after reconciliation is:

```text
L2 selected identity
 -> L1 backend/member byte acquisition
 -> L1 read/seek/decompression mechanics
 -> L1 exact caller-visible materialized byte buffer
 ===== L1 BYTE AUTHORITY =====
 -> L3 request/scheduler lifecycle ownership
 -> L3 acquisition/state 0 -> 1
 -> L3 completion/state 1 -> 2
 -> L3 typed post-load / optional ready callback
 -> L3 state 2 -> 3 / consumer-ready visibility
```

A function can still be a seam rather than wholly owned by one layer. `0x1401B8CA0` is the clearest example: its representation/materialization mechanics are L1-relevant, while its success result gates L3 state1 publication.

## 3. Critical correction — no evidenced generic fan-in counter

Earlier review shorthand used **materialization fan-in/completion**. That is too strong if interpreted as proof of an explicit child/outstanding-work counter.

Current preserved direct-disassembly authority establishes:

- lower byte-transfer/load-state mechanics;
- a higher scheduler ring;
- scheduler enqueue, worker and pending-entry clear/rollback;
- L3 state1 publication only after materialization-dispatch success;
- later scheduler-mediated normal completion to state2;
- cancellation rollback before unfinished records enter state4 cleanup.

Current evidence does **not** directly establish:

- a generic child-count field;
- a universal outstanding-request counter;
- a generic `N children complete -> parent complete` counter transition;
- one shared fan-in object across direct, packed and `.lst` materialization.

Safe wording is:

> **materialization-to-lifecycle completion ordering / dependency barrier**

If an explicit counter exists, it remains an exact-byte/dataflow target.

## 4. Lower byte-transfer mechanics

The bounded whole-file family remains useful to L1 byte reconstruction:

```text
0x1400333F0
 -> open selected resource / construct load-state
0x140033500
 -> submit caller-owned-destination byte transfer through 0x14002EA40
0x1400335A0(ticketId,userContext,errorFlag,bytesRead)
 -> update transfer progress/status
```

These mechanics explain how bytes reach caller-owned storage. They do not make FileSlot request ownership or callback lifetime L1.

The canonical L3 audit therefore controls classification:

- byte-read mechanics: L1 support;
- FileSlot request ownership, scheduling, completion, cancellation and close lifetime: L3.

## 5. Higher scheduler structure

### `0x1402EF580` — scheduler enqueue

Preserved direct evidence establishes a scheduler-ring enqueue helper that stores callback/context metadata, publishes the executable/pending slot state and advances the ring.

### `0x1402EF790` — scheduler worker

The worker executes the queued callback and clears the scheduler slot afterward.

### `0x1402EF460` — pending scheduled-entry clear/rollback

Safe bounded label:

> **pending scheduled-entry clear/rollback**

It is not evidence of:

- OS `CancelIo`;
- universal FileSlot cancellation;
- guaranteed synchronous cancellation of a backend read already in progress.

This is scheduler/lifecycle rollback evidence and therefore belongs to L3 ownership, while remaining relevant to the L1/L3 handoff audit.

## 6. LoadedResource lifecycle ordering

The canonical state spine remains:

```text
0x1401B84E0
 -> acquisition construction
 -> 0x1401B8CA0 representation/materialization dispatch
 -> only on successful materialization start: state 0 -> 1
 -> register later completion through scheduler

0x1401B8DC0
 -> normal completion callback
 -> state 1 -> 2

0x1401B92D0
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3
```

The important correction is ownership:

- materialization mechanics consumed by `0x1401B8CA0` are L1-relevant;
- the LoadedResource state machine and scheduler completion are L3.

`0x1401B8DC0` remains **not a raw I/O callback**, but it is also no longer described as the terminal L1 handoff. It is an L3 lifecycle completion writer.

## 7. Cancellation ordering

The cancellation path supplies stronger negative/control-side ordering:

```text
0x1401B8430
 -> 0x1402EF460 pending scheduler clear/rollback
 -> unfinished state 1|2 -> 4
 -> enqueue 0x1401B8F00 through 0x1402EF580

0x1401B8F00
 -> deferred state4 cleanup
 -> state 0 / backing release path
```

This proves pending higher scheduler work is rolled back before unfinished lifecycle records proceed through state4 cleanup.

It does not prove an already-running lower transport request is synchronously cancelled by `0x1402EF460`.

## 8. `.lst` consequence

`0x1401B85C0` still provides the recovered loose-container mechanics:

- packed-sibling precedence;
- ordinary child materialization into the parent destination;
- nested recursive in-place synthesis;
- sparse `dummy` handling;
- already recovered grammar/layout rules.

The open question is no longer called **child fan-in**. It is:

> How do L1 child byte-population mechanics interact with L3 request/scheduler ordering so lifecycle completion cannot advance while required materialization is invalid or failed?

Possible mechanisms include synchronous ordering, scheduler ordering, nested callbacks, another status/dependency object or an explicit counter. None is promoted without direct evidence.

## 9. Focused exact-byte packet

The focused plan reacquires:

```text
0x1400333F0  whole-file load-state/open seam
0x140033500  caller-owned byte-transfer submit
0x1400335A0  transfer callback/status
0x14002EA40  FileSlot/ReadRequest submit seam
0x1402EF460  pending scheduler clear/rollback
0x1402EF4D0  materialization submission wrapper
0x1402EF580  scheduler enqueue
0x1402EF790  scheduler worker
0x1401B8430  cancellation writer
0x1401B84E0  acquisition constructor
0x1401B85C0  .lst materializer
0x1401B8CA0  explicit L1/L3 materialization seam
0x1401B8DC0  L3 state1->2 completion writer
0x1401B8F00  deferred state4 cleanup
```

All windows are probes; `0x400` is acquisition coverage only, not a function-size claim.

## 10. Exact questions for the next raw-byte pass

1. What exact lower object/state does `0x1402EF4D0` create or submit, and where does byte-materialization ownership end?
2. What is the first concrete consumer and numeric domain of the inherited materialization/load-context parameter?
3. Does `0x1402EF4D0` use the same scheduler ring directly, call the whole-file family, or select another path?
4. What exact ordering exists between materialization work and L3 state1/state2 scheduler actions?
5. Is lifecycle completion protected by ring order, a status/dependency object, nested callbacks or an explicit counter?
6. How does lower transfer failure reach the L3 acquisition/cancellation path?
7. For `.lst`, which failure prevents lifecycle advancement after one child population fails?
8. What exact entries does `0x1402EF460` match and clear?
9. Can scheduler rollback affect an already executing callback?
10. What happens to already-running FileSlot/ReadRequest transport when L3 cancellation begins?

## 11. Evidence/environment boundary

A fresh raw canonical `e454...` executable blob was not exposed through the connected file surface during the first part of this pass, so the new conclusions above are a reconciliation of preserved direct canonical-disassembly authority plus the newly merged raw-L3 audit.

The next fresh byte reacquisition must use the focused guarded plan rather than ad hoc ranges.

## 12. Completion consequence

This pass does not reopen the current representative L1 product implementation path and does not mark L3 complete.

The layer acceptance rules remain separate:

- L1 still requires representative real-retail byte identity/edit/rebuild/rematerialization evidence and the final game-backed vertical receipt;
- L3 still requires its remaining static census plus original-process lifecycle receipts;
- #209 still requires deterministic original-game consumer-visible attribution and rollback.

The value of this pass is architectural precision: **L1 owns exact materialized-byte authority; L3 owns original request/scheduler/LoadedResource lifecycle authority.**
