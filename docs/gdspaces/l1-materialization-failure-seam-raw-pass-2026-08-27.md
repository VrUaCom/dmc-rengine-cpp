# GDSpaces L1/L3 — Materialization Failure-Suppression Raw Pass — 2026-08-27

**Pass date:** 2026-08-27  
**Canonical analysis executable:** `dmc3.exe`, 6,356,432 bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Parent evidence:** `l1-byte-exactness-raw-pass-2026-08-27.md`  
**Primary concern:** prove that failed/incomplete L1 byte work cannot incorrectly reach normal L3 `state1 -> state2` publication.

## 1. Verdict

The previously open **false-success suppression** question is now **BOUNDED-CLOSED** for the canonical direct-call normal acquisition surface.

A transfer error does not retire the active type-2 queue record. The current consumer record remains at the head of its lane, so later queue records — including the normal `0x1401B8DC0` completion callback — cannot execute past it.

This is a fail-closed **head-of-line stall**, not a generic fan-in counter and not an OS-I/O cancellation mechanism.

The remaining error-path question is **recovery/cleanup after the stall**, which belongs primarily to L3/runtime lifecycle policy rather than to proof of L1 byte correctness.

## 2. Exact type-2 queue ordering

`0x1402EF4D0` inserts a type-2 path-load record into a lane ring:

```text
record +0x00 = type 2
record +0x08 = caller-owned destination
record +0x10 = normalized path
producer index advances modulo lane capacity
lane active = 1
```

`0x1402EF580` inserts callback records as type `3` into the same ring machinery and advances the producer index only after the record is committed.

`0x1402EF750 -> 0x1402EF790` processes the **current consumer record**. The consumer index is advanced only by the retirement path around `0x1402EF8DE`.

Therefore producer order and consumer retirement are distinct and directly observable.

## 3. Pending and success behavior

For a type-2 record, `0x1402EF790` phase 0 opens/submits the whole-file transfer and changes the lane phase to 1.

Phase 1 polls `0x1400333E0`:

```text
2 = pending
3 = success
4 = error
```

### Pending

`status == 2` returns immediately without:

- clearing the type-2 record;
- clearing the FileLoadState pointer;
- advancing the consumer index.

The same job is therefore polled again later.

### Success

`status == 3`:

```text
0x140033390(FileLoadState)
 -> FileLoadState pointer = null
 -> current job type = 0
 -> lane phase = 0
 -> consumer index advances
```

Only after this retirement can later records in that lane execute.

This directly proves completion-aware persistence and rejects the old FIFO-only hypothesis.

## 4. Error behavior is fail-closed by head-of-line stall

`status == 4` branches to the phase-reset path at `0x1402EF80B`.

Critically, that local error branch does **not**:

- call `0x140033390`;
- clear the FileLoadState pointer;
- clear the current type-2 record;
- advance the consumer index.

It only resets the lane phase to 0 and returns.

On the next scheduler tick, phase 0 encounters the still-non-null FileLoadState pointer and returns without reopening or advancing the record.

Therefore:

```text
transfer error
 -> status 4
 -> current type-2 record remains consumer head
 -> FileLoadState remains non-null
 -> consumer index does not advance
 -> all later same-lane records remain unreachable
```

This is sufficient to prove **no false later completion publication in the same lane**.

## 5. Normal LoadedResource completion is queued after materialization

`0x1401B84E0` performs:

```text
plan/allocate
 -> 0x1401B8CA0 materializer scheduling/acceptance
 -> publish LoadedResource state1
 -> enqueue 0x1401B8DC0 callback through 0x1402EF580
```

The normal `0x1401B8DC0` callback only writes state2; it does not inspect transport status.

That is safe on the recovered normal surface because the callback sits behind materialization work in queue order and cannot become the current consumer until preceding jobs retire successfully.

## 6. Direct caller census: canonical normal acquisition uses lane 0

A whole-image direct-call census found:

- **7** direct calls to `0x1401B84E0`;
- the direct callers explicitly use zero queue/control arguments except the wrapper call from `0x1401B8DF0`;
- **26** direct calls to `0x1401B8DF0`;
- all 26 directly set both `R8D = 0` and `R9D = 0` immediately for the recovered call surface.

This materially narrows the normal acquisition topology to the zero lane/control domain on the canonical image.

For the recovered direct-call surface, `.lst` queued children and the normal completion therefore share the same canonical queue domain rather than requiring an unevidenced cross-lane fan-in mechanism.

No claim is made about runtime-computed indirect callers that are not present in the direct-call census; a contradiction sweep remains the reopen condition.

## 7. `.lst` child failure no longer needs a local parent error accumulator

`0x1401B85C0` does not consistently test every child `0x1402EF4D0`/recursive return as a synchronous all-child completion result.

The raw scheduler model explains why local synchronous aggregation is not required for false-success prevention on the bounded normal surface:

```text
.lst structural construction
 -> child type-2 jobs queued
 -> normal state2 callback queued later
 -> scheduler processes current record in order
 -> any child status4 remains at queue head
 -> later callback cannot execute
```

Thus:

- `.lst` local return = structural/scheduling acceptance;
- terminal child success = scheduler retirement responsibility;
- terminal child failure = head-of-line fail-closed suppression;
- normal state2 cannot overtake failed queued child work on the recovered lane-0 surface.

This closes the prior P0 concern that absence of a local `.lst` child counter necessarily permits false state2 publication.

## 8. `0x1402EF460` role is recovery/cancellation, not primary failure gating

Fresh direct code already identifies `0x1402EF460` as pending queue rollback/truncation:

- walks producer records backward toward the consumer boundary;
- clears pending record type fields;
- updates ring producer/active state;
- does not directly establish OS `CancelIo` semantics.

The only direct call recovered is from `0x1401B8430`, which applies LoadedResource cancellation/replacement policy before marking states `1|2 -> 4` and scheduling deferred cleanup.

Therefore the canonical split is:

```text
[L1/SEAM] status4 current-head stall prevents false success immediately
[L3 policy] later cancellation/replacement may truncate pending queue work via 0x1402EF460
[L3] state4 cleanup/release handles lifecycle recovery
```

The L1 correctness proof does not depend on pretending `0x1402EF460` directly cancels the active OS read.

## 9. Separate synchronous helper `0x1402EF920`

`0x1402EF920` is a different path from the persistent queued type-2 scheduler.

It:

1. opens a whole-file state;
2. submits `0x140033500`;
3. polls until status is `3` or `4`;
4. closes the state;
5. returns `chunkCount << 11` in both terminal cases.

It does **not** encode status4 as a distinct return failure.

The `.lst` planner/materializer uses this helper to read `.lst` description bytes and then proceeds into parsing. Therefore original I/O-error behavior for this synchronous descriptor-load path is permissive/unsafe: terminal transport error is not represented by the helper return value.

This is now an exact recovered behavior, not an unknown. DMC Rengine product code may deliberately be stricter and fail closed.

## 10. Status change

The parent raw-pass status should now be interpreted as:

```text
L1-R1 size authority                         = bounded-closed
L1-R2 final chunk / EOF / short read        = bounded-closed common path
L1-R3 capacity / allocation / initialization= bounded-closed
L1-R4 .lst planner/writer structure         = bounded-closed
L1-R5 .lst padding zero state                = closed at canonical allocator path
L1-R6 type-2 byte-producing ingress          = bounded-closed
L1-R7 partial-read terminal composition      = bounded-closed common path
L1/L3 false state2 suppression               = bounded-closed direct normal surface
L1-R8 intrinsic child EOF                    = retained scope limit, not disproven
```

### Still open after this pass

- broader malformed/rare error equivalence where intentionally claimed;
- active failed FileLoadState cleanup/recovery timing outside false-success prevention;
- indirect/runtime-computed caller contradictions;
- unsupported/evidence-gated formats/backends;
- real-retail selected-member provenance and representation classification;
- real edit/rebuild/rematerialization and original-game Level-E acceptance.

## 11. Next reverse order

The next L1 static work should no longer spend priority on the now-bounded completion barrier.

Proceed to breadth that can still change actual byte/materialization equivalence:

```text
1. complete 0x140328540 ZIP lazy stream initializer ownership/error paths
2. complete 0x140328FE0 compressed seek/reset/reinflate behavior
3. close remaining .lst parse/malformed/temp-buffer error exits
4. audit 32-bit size/alignment overflow boundaries against actual supported resource domain
5. whole-image contradiction sweep for alternate materialization ingress/backends
6. keep binary AFS/PACK frozen unless direct evidence activates them
7. then reconcile canonical roadmap/status and continue real-retail Level-E
```

## 12. Completion boundary

This pass does **not** mark L1 complete.

It does establish that the previously feared async false-success path is not an open common-path blocker on the canonical direct-call surface.

Current safe statement:

> **Canonical normal materialization uses completion-aware queue persistence; failed type-2 work is fail-closed by consumer head-of-line stall, and normal LoadedResource state2 publication cannot overtake it on the recovered lane-0 direct-call surface.**
