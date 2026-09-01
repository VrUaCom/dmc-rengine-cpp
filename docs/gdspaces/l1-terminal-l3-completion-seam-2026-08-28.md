# L1 terminal byte/result -> L3 normal-completion seam — 2026-08-28

**Status:** canonical static normal-path reconciliation complete at the bounded addresses below; dynamic cancellation/current-slot concurrency remains L3 breadth.  
**Canonical executable:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Implementation base:** `main@94692e8f9971cf8249b4b16ee88d309de8b49f11`.

This checkpoint follows `l1-writer-failure-width-reconciliation-2026-08-28.md` and resolves the narrow static question:

> On the canonical normal acquisition path, what permits the queued `0x1401B8DC0` LoadedResource `state1 -> state2` callback to execute after L1 materialization work?

It deliberately does **not** claim broad L3 lifecycle completion.

## 1. Registration uses the same queue lane

`0x1401B84E0` derives a lane selector (`0` or `1`) and uses it for both materialization dispatch and normal completion registration.

Materialization:

```text
0x1401B84E0
 -> r8d = lane
 -> 0x1401B8CA0
 -> packed/direct branch copies lane into r9d
 -> 0x1402EF4D0(..., lane)
```

Normal completion registration:

```text
0x1401B84E0
 -> callback = 0x1401B8DC0
 -> one u32 context = record_ptr - 0x140C99D30
 -> r8d = same lane
 -> 0x1402EF580(..., lane, argCount=1)
```

Both enqueue helpers use the lane's producer index at `+0x18` and the same `0x88`-byte ring-slot geometry. Accepted jobs therefore enter one per-lane FIFO in producer order.

For a loose `.lst`, child type-2 jobs are emitted before `0x1401B84E0` enqueues the final type-3 normal-completion callback.

## 2. Consumer processes one current FIFO slot

`0x1402EF790` derives the current slot from the lane's consumer index and does not advance the consumer index while the current type-2 job remains pending/retrying.

For type-2 materialization jobs:

```text
phase 0
 -> open whole-file state (0x1400333F0)
 -> derive chunk count (0x1400333C0)
 -> submit (0x140033500)
 -> phase++

phase 1
 -> poll 0x1400333E0
```

The consumer index advances only through the retirement block at `0x1402EF8DE..0x1402EF8F8` after the current job has been cleared.

Thus a later type-3 callback in the same lane cannot execute before the current earlier type-2 job retires.

## 3. Whole-file status gating

`0x140033500` publishes global whole-file status `2` before submission.

- immediate submit failure -> status `4`;
- callback `0x1400335A0` with no error -> status `3` and adds actual produced bytes to the state's accumulated count;
- callback error -> status `4`.

`0x1402EF790` treats the states as follows:

### status `2` — pending

The consumer returns without clearing the job and without advancing the FIFO.

### status `4` — retry, not retirement

The consumer branches to `0x1402EF80B`, clears its local per-job phase byte and returns. It does **not** clear the type-2 ring slot or advance the consumer index.

On the next service pass the same current type-2 job re-enters phase 0 and retries open/submit.

Therefore a type-2 job that repeatedly reaches status `4` can keep the FIFO blocked; a later normal-completion callback cannot pass it on the static FIFO path.

### status `3` — retire current type-2 job

For the normal submitted path, status `3` is the success state produced by `0x1400335A0`. `0x1402EF790` then:

```text
close whole-file state (0x140033390)
 -> clear stored state pointer
 -> clear current ring job state
 -> clear local phase
 -> advance consumer index modulo capacity
```

Only after this retirement can the next FIFO job, including a type-3 callback, become current.

## 4. Static normal completion eligibility

For successfully admitted jobs on the canonical non-cancel path:

```text
accepted type-2 materialization job(s)
 -> each remains current while status=2
 -> each retries while status=4
 -> each retires after status=3
 -> FIFO reaches type-3 callback
 -> 0x1401B8DC0(context)
 -> LoadedResource state +4 = 2
```

`0x1401B8DC0` itself performs no fresh byte-count validation. The canonical registered context is `record_ptr - 0x140C99D30`; for the normal registry domain it is an even `index * 0x48` context, so the recovered normal path uses the base-plus-context branch.

This preserves layer ownership:

- status-3 byte-transfer terminal semantics are L1-relevant;
- FIFO/callback execution and `state1 -> state2` publication are L3 lifecycle semantics.

## 5. Critical short-success consequence

`0x1400335A0` does not compare accumulated actual bytes against the originally cached/planned total before publishing status `3`.

Therefore the static original path permits:

```text
cached/planned size = N
 -> backend produces M bytes where M < N
 -> lower operation reports no error
 -> callback adds M and publishes status 3
 -> type-2 job retires
 -> queued normal completion callback becomes eligible
 -> LoadedResource state 1 -> 2
```

So **normal L3 completion eligibility is gated by original backend success status, not by an independent exact-length equality check**.

This is an original-runtime behavior finding. GDSpaces product receipts that claim exact bytes remain stricter and must validate the relevant exactness contract.

## 6. Failure-swallowing before FIFO admission remains important

The FIFO ordering above applies only to jobs that were actually admitted.

The previous checkpoint proved:

- `0x1401B85C0` ignores direct child enqueue and recursive writer failure returns;
- `0x1401B8CA0` has branch-dependent boolean semantics;
- `0x1401B84E0` ignores the boolean result from the final type-3 `0x1402EF580` enqueue.

Consequences:

- outer materialization setup can return true while one or more expected child jobs were not admitted;
- setup can return true while the normal-completion type-3 job itself was not admitted;
- such an upstream `true` is not equivalent to FIFO completion eligibility.

The static FIFO guarantee is therefore conditional on **successful admission of the relevant jobs**.

## 7. Cancellation suppression is L3

`0x1401B8430` is the relevant cancellation/reset path for this seam.

It performs:

```text
0x1402EF460(queue)
 -> flush queued jobs by moving the producer index backward toward the consumer index
 -> queued-but-currently-unconsumed slots are cleared

then
 -> LoadedResource states 1 or 2 are changed to 4 across the registry
 -> enqueue cleanup callback 0x1401B8F00
```

`0x1401B8F00` later releases/reset records that remain in state `4` back toward state `0`.

This is an explicit normal-completion suppression mechanism: queued normal completion work can be removed during cancellation and affected LoadedResources are moved into cancellation state instead of normal state `2` publication.

`0x1402EF460` stops at the consumer index rather than proving that an already-current job/callback is synchronously absent. Exact concurrent current-slot races/order belong to L3 dynamic validation, not to the L1 static byte boundary.

## 8. Correct canonical seam

The corrected architecture is:

```text
[L1]
selected representation
 -> plan/allocate destination
 -> attempt queue admission of byte-producing jobs
 -> admitted type-2 job executes/retries
 -> whole-file status 3 retires job
 -> original byte-result path complete at its native success criterion
===== END L1 BYTE/RESULT AUTHORITY =====

[L3]
FIFO reaches admitted type-3 normal callback
 -> 0x1401B8DC0
 -> LoadedResource state 1 -> 2
 -> typed post-load / later ready-state lifecycle
```

Cancellation can suppress the queued normal path by flushing queued work and publishing state `4` instead.

## 9. What this closes

At static canonical-address scope this closes the old L1 frontier item:

> reconcile how terminal L1 materialization allows or suppresses normal completion without moving normal LoadedResource lifecycle ownership into L1.

Specifically closed:

- same-lane FIFO ordering between materialization and normal callback;
- status `2` pending behavior;
- status `4` type-2 retry/non-retirement behavior;
- status `3` type-2 retirement behavior;
- callback eligibility after preceding accepted type-2 retirement;
- short-success eligibility consequence;
- cancellation queued-work suppression boundary;
- explicit L1 vs L3 ownership cut.

## 10. Still open

Not closed by this static seam pass:

- dynamic concurrency/current-slot cancellation races;
- broader L3 transition/reset/shutdown receipts;
- exact recursive `.lst` cycle/depth and allocation/free lifetime semantics;
- remaining allocator/backend failure branches outside this normal whole-file path;
- representative real `.lst` corpus if a real loose-list equivalence claim is made;
- original-game Level-E authored-byte consumption and rollback;
- final L1 contradiction audit.

## Non-claims

This checkpoint does not claim:

- L3 complete;
- all queue concurrency behavior recovered;
- short-success is acceptable product behavior;
- L1 complete / 100%;
- original-game consumption proven.
