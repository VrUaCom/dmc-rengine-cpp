# GDSpaces — Materialization Completion Dependency Pass 2 — 2026-08-26

**Stack base:** `docs/gdspaces-materialization-completion-boundary-2026-08-26@0fcddd83ec6c6795fc52ffbff3ff1da42021f01f`  
**Canonical main below stack:** `main@2ed43b438f1bf01638f3e56341e98f6085e5b0fd`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Primary target:** materialization completion ordering / dependency bridge  
**Evidence classes used:** merged canonical raw-EXE authority (#230) + historical derivative Pass-90 evidence as a reacquisition hypothesis only

## 1. Scope

This pass does not claim a new raw byte re-disassembly of `0x1402EF4D0`. The exact canonical executable blob/window is not exposed through the current connected file surface for this session.

Instead it takes the merged raw-EXE callback ABI from #230 and reconciles it against the older Pass-90 direct-resource materialization trace. The goal is to remove another class of ambiguous completion models before the next exact-byte run.

The open chain remains:

```text
0x1401B8CA0 materialization mechanics
 -> 0x1402EF4D0 submission/job creation
 -> lower whole-file/FileSlot work
 -> dependency/completion condition
 -> state1 publication already succeeded
 -> queued 0x1401B8DC0(context)
 -> state2
```

## 2. New hard narrowing from merged #230

Merged #230 proves that normal acquisition registers `0x1401B8DC0` with exactly one u32 argument:

```text
context = record_ptr - 0x140C99D30
```

The scheduler copies that one dword and invokes the callback through ECX. Valid record contexts are `index*0x48`, so they are all even and use the normal callback branch.

The normal `0x1401B8DC0` branch mechanically reconstructs the record and writes state2. It does not receive:

- a transport-status pointer;
- an error flag;
- a byte count;
- a child-count/outstanding-work value;
- a direct FileSlot/ReadRequest handle.

### Consequence

For the canonical normal path, **the final success/error dependency decision cannot be performed inside `0x1401B8DC0` itself**.

By the time the scheduler dispatches the normal completion callback, the system must already have established that state2 publication is permissible, or else have removed/suppressed that callback before dispatch.

This is stronger than the previous generic statement that transport completion and state2 completion are separate layers.

## 3. FIFO by itself is not sufficient

A tempting hypothesis is:

```text
materialization job queued first
 -> completion callback queued second
 -> FIFO ordering guarantees materialization before state2
```

That statement is incomplete.

If the first scheduler job merely submits asynchronous I/O and is then immediately considered complete/cleared, FIFO permits the next scheduler record to run while the lower FileSlot/ReadRequest is still pending. Because normal `0x1401B8DC0` does not validate transport status, that would allow a false early state2 publication.

Therefore, if scheduler order is the real dependency barrier, at least one stronger property must also hold:

1. the materialization scheduler record remains live and is re-polled until the lower transfer reaches a terminal result; or
2. the scheduler record does not retire until nested/lower completion callback(s) mark it terminal; or
3. another scheduler/status gate blocks dispatch of the later completion record; or
4. failure removes/invalidates the queued completion record before it can run.

Simple insertion order alone is not an adequate recovered model.

## 4. Historical Pass-90 evidence — reacquisition target, not promotion authority

Historical PR #177 / Pass 90 used a derivative executable probe rather than a fresh canonical `e454...` byte window. It recorded this exact-resource candidate chain:

```text
0x1402EF4D0 enqueue direct-read job
 -> 0x1402EF790 process/poll job
 -> 0x1400333F0 VFS open
 -> 0x1400333C0 sector count
 -> 0x140033500 submit caller-owned destination read
 -> poll completion
 -> 0x140033390 release load-state
```

That older pass also identifies a status/poll helper around `0x1400333E0`.

Because the direct instruction source was derivative, none of the following is promoted solely from Pass 90:

- exact body identity of the materialization job callback;
- exact scheduler record state/type;
- exact `0x1400333E0` return domain;
- exact retry/poll loop;
- exact relationship between transport terminal/error state and queue retirement.

But this is now a high-value **canonical reacquisition hypothesis** because it directly tests the stronger condition required by section 3.

## 5. Falsifiable dependency models

The next raw pass should discriminate these models instead of searching generically for a fan-in object.

### H1 — persistent polling scheduler job

```text
2EF4D0 queues materialization job
 -> worker invokes it
 -> job starts/open/submits transfer
 -> while whole-file status is pending, scheduler record remains live/re-dispatchable
 -> on terminal success, job closes load-state and retires
 -> only then later B8DC0 record can run
```

If confirmed, queue lifecycle + terminal polling is the dependency barrier. No child counter is required for the direct-resource path.

### H2 — callback-driven job terminal state

```text
materialization job submits transfer
 -> lower 3335A0 callback changes shared job/load state
 -> scheduler sees terminal state and retires materialization job
 -> B8DC0 can then dispatch
```

This can coexist with H1 if the worker polls status written by the lower callback.

### H3 — separate scheduler gate/status dependency

The materialization job may retire after submission, but a separate scheduler condition prevents later records from dispatching until a shared terminal state changes.

This remains possible but requires a directly evidenced gate because `B8DC0` itself does not inspect that state.

### H4 — synchronous completion before `2EF4D0` success

`2EF4D0` may not return success until bytes are complete. This would also make later completion registration safe.

Historical whole-file AsyncIO evidence makes this less attractive, but it is not rejected until canonical `2EF4D0` control flow is directly reacquired.

## 6. Error-path consequence

`0x1400335A0` exposes lower transport success/error information, but normal `0x1401B8DC0` does not.

Therefore a transport failure must be resolved upstream of normal state2 dispatch by one of these classes:

```text
transport error
 -> materialization job becomes terminal-failed and does not permit later completion dispatch
```

or

```text
transport error
 -> queued completion record is cleared/rolled back before dispatch
```

or another directly evidenced equivalent.

This makes `0x1402EF460` particularly important as the cancellation/control comparator, but current evidence still supports only the safe label **pending scheduler-entry clear/rollback**. It is not OS AsyncIO cancellation unless direct lower-I/O interaction is recovered.

## 7. Why `0x1400333E0` and `0x140033390` become first-class acquisition anchors

The focused packet previously included transfer submit/callback and the higher scheduler, but omitted the historical polling and load-state release anchors.

That omission makes H1/H2 hard to decide instruction-by-instruction.

The next packet therefore adds:

- `0x1400333E0` — reacquire exact whole-file load-state/status polling semantics and terminal return domain;
- `0x140033390` — reacquire exact load-state close/release ordering relative to terminal status and scheduler job retirement.

These addresses are **historical reacquisition targets** until fresh canonical bytes validate their current role.

## 8. Revised raw-pass order

The exact-byte run should now proceed in this order:

1. `0x1402EF4D0`: identify exact queued job callback/state/type and inherited load-context consumer;
2. `0x1402EF790`: identify the materialization-job dispatch case separately from the already-closed one-u32 callback ABI;
3. `0x1400333E0`: recover pending/success/error status domain and poll semantics;
4. `0x140033390`: recover terminal cleanup/release point;
5. `0x1400335A0`: bind lower transport success/error writes into that status object;
6. determine whether the scheduler record remains live until terminal status, or another gate exists;
7. `0x1402EF460`: recover which pending higher scheduler records are removed on rollback and whether a queued `B8DC0` completion can be suppressed;
8. only after the direct-resource mechanism is closed, apply the same model to `.lst` child/recursive failure ordering.

## 9. Promotion boundary after Pass 2

### Stronger now

- normal `B8DC0` cannot be the place where raw transport success/error is decided;
- dependency correctness must be established before normal `B8DC0` dispatch;
- FIFO insertion order alone is insufficient unless the earlier materialization job has completion-aware persistence/retirement semantics;
- polling/status and load-state release are now primary evidence targets, not incidental helper details;
- historical Pass-90 direct-read/poll chain is explicitly downgraded to a canonical reacquisition hypothesis rather than silently reused as current authority.

### Still not claimed

- exact canonical `2EF4D0` job body;
- scheduler FIFO as the proven barrier;
- exact `333E0` status values;
- exact transport-error suppression path;
- already-running FileSlot cancellation behavior;
- `.lst` child failure aggregation;
- original-process timing equivalence.

## 10. Completion consequence

This is a static model-narrowing pass. It does not reopen the current internal product path and does not change L1/L2/L3 completion criteria.

The reverse frontier is now a small state-machine question rather than a broad “fan-in” search:

> **What exact terminal condition keeps or releases the materialization scheduler job, and how does that condition prevent normal `0x1401B8DC0` from dispatching on failed/incomplete transport?**
