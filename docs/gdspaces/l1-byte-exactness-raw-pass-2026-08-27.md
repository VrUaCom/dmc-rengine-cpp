# GDSpaces L1 — Canonical Byte-Exactness Raw Pass — 2026-08-27

**Pass date:** 2026-08-27  
**Primary layer:** L1 — Resource Materialization  
**Canonical analysis executable:** `dmc3.exe`  
**Size:** `6,356,432` bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Parent gap pass:** `l1-byte-exactness-gap-pass-2026-08-27.md`  
**Primary ledger:** #100  
**Boundary rule:** L1 ends at the exact materializer byte/result state through `0x1401B8CA0`; normal LoadedResource `state1 -> state2` publication remains L3. The dependency between them is an explicit L1/L3 seam.

## 1. Verdict

Fresh raw disassembly of the exact canonical analysis executable materially closes most of the byte-exactness frontier identified by the parent gap pass.

Current bounded status:

| Gate | New status | Result |
|---|---|---|
| L1-R1 logical/materialized size authority | **BOUNDED-CLOSED** | FileSlot size source and zero-size behavior recovered |
| L1-R2 rounded transfer vs exact byte extent | **BOUNDED-CLOSED** | physical + ZIP STORE + raw-DEFLATE whole-file path recovered |
| L1-R3 destination capacity/allocation/initialization | **BOUNDED-CLOSED** | rounded capacity + full zero initialization recovered |
| L1-R4 `.lst` planner/writer equivalence | **STRUCTURAL BOUNDED-CLOSED / TERMINAL CHILD FAILURE SEAM OPEN** | size/offset planning matches writer; async child terminal failure is not locally aggregated |
| L1-R5 synthesized padding/alignment bytes | **CLOSED AT CANONICAL ALLOCATOR PATH** | full payload zeroed before materialization |
| L1-R6 exact byte-producing ingress behind `0x1402EF4D0` | **BOUNDED-CLOSED** | queue type-2 whole-file path-load job into caller-owned destination |
| L1-R7 partial-read/transform terminal composition | **BOUNDED-CLOSED FOR COMMON PHYSICAL/STORE/DEFLATE PATHS** | zero return is normal EOF/short completion; negative is failure |
| L1-R8 generic packed-child intrinsic-size boundary | **SCOPE LIMIT RETAINED** | runtime relative starts still do not prove universal intrinsic child EOF |
| L1/L3 terminal-result -> state2 completion seam | **OPEN / P0** | success polling is recovered; error rollback/suppression origin remains incomplete |

L1 remains **INCOMPLETE / NOT 100%**. The reason is now narrower and better evidenced: the common byte-production path is substantially recovered, while the cross-layer failed/incomplete completion suppression path and declared breadth/real-retail acceptance remain open.

## 2. Size authority — L1-R1 bounded-closed

### `0x14002F9F0` is a pure thunk

Fresh canonical bytes show:

```text
0x14002F9F0 -> jmp 0x140048E20
```

There is no independent size policy at `0x14002F9F0`.

### `0x140048E20` reads the FileSlot size field

The helper indexes the `0x20`-byte FileSlot record, reads the dword at record `+0x10`, writes it to the caller output and returns `0`.

`OpenGameResource 0x14002FCA0` establishes that field from the selected backend:

```text
selected provider wrapper
 -> 0x1403272A0
 -> archive: 0x1400629C0 -> [entry + 0x20]
 -> physical: 0x1403277C0 -> physical file-size API
 -> FileSlot +0x10
```

At this bounded path FileSlot `+0x10` is therefore the selected resource logical/materialized size authority.

### Zero-size boundary

`0x1400333F0` does not reject a successfully opened zero-size resource. It can construct a whole-file state whose `totalBytes == 0`.

However `0x1401B79E0` uses the rounded-size helper result as a positive-size representation test. A packed resource or `.lst` whose rounded result is `0` is treated as absent for this representation-selection path.

This distinguishes:

- valid open + zero size at the lower whole-file layer;
- zero-size representation treated as unavailable by the higher packed/`.lst` selector.

## 3. Rounded transfer and EOF — L1-R2 bounded-closed

### Whole-file request rounding

`0x1400333C0` computes:

```text
chunkCount = ceil(totalBytes / 0x800)
```

`0x140033500` submits:

```text
requestedBytes = chunkCount << 11
status = 2  // pending
```

The rounded request may exceed logical EOF.

### `0x14002F930` worker semantics

The ReadRequest worker repeatedly calls the FileSlot read dispatcher until one of three conditions:

```text
read < 0  -> error
read > 0  -> advance destination and continue
read == 0 -> normal terminal break
```

Its callback receives the **actual transferred byte count**, not the rounded requested count.

This means a backend EOF/short completion is normal success rather than automatic failure.

### Physical backend

The physical read path returns accumulated actual bytes. EOF/zero production terminates normally; failure returns negative.

### ZIP STORE

`ZipEntryRead 0x140328F50` clamps the requested count against the entry remaining extent before reading. When remaining bytes are zero it returns zero. Therefore the rounded tail cannot write beyond the logical STORE member extent.

### raw-DEFLATE

`InflateRead 0x140328820` returns produced byte count, marks stream completion, and can subsequently return zero at terminal stream end. The outer worker treats that zero as normal short completion; negative remains error.

### Whole-file completion status

`0x1400335A0` directly establishes:

```text
error flag != 0 -> status = 4
error flag == 0 -> loadedBytes += actualTransferred; status = 3
```

`0x1400333E0` is the canonical status getter over the global status value.

Recovered domain:

- `2` = pending;
- `3` = success;
- `4` = error.

At the common physical / ZIP STORE / raw-DEFLATE whole-file path, the final `0x800` rounding is therefore compatible with exact logical destination bytes because the backends terminate/clamp at EOF and the worker reports actual production.

## 4. Rounded capacity and allocation — L1-R3 bounded-closed

### `0x1402EF620` is rounded-capacity authority, not intrinsic EOF

Fresh raw code shows this helper:

```text
open whole-file state
 -> chunkCount = 0x1400333C0(state)
 -> close state
 -> return chunkCount << 11
```

Therefore:

```text
0x1402EF620(path) = ceil(logicalSize / 0x800) * 0x800
```

Any documentation that used `0x1402EF620` as a generic intrinsic child-size helper is too strong. It is a rounded materialization-capacity helper at this path.

### `0x1401B7B90` required backing size

The planner consumes the rounded helper for direct/packed resources and uses 32-bit arithmetic for alignment/size propagation. No local wide-integer overflow guard is evidenced in this function.

The supported positive-size domain must therefore remain bounded; product hardening may be stricter than the original arithmetic.

## 5. Full zero initialization closes padding ambiguity — L1-R5 closed

`0x1401B84E0` obtains required backing size and allocates the materialization payload through the canonical allocator family.

`0x140337600` then explicitly calls imported `memset` with:

```text
value = 0
size = requested payload size
```

on the complete allocated payload before materialization begins.

Consequences for the recovered `.lst` synthesis path:

- the complete backing region starts as zero;
- header/table/children overwrite their declared ranges;
- untouched 64-byte alignment gaps remain zero;
- zero padding is original canonical behavior at this allocator/materialization path, not merely a DMC Rengine writer policy.

This closes the previous padding-content ambiguity at the evidenced allocator path.

## 6. `.lst` representation and planner/writer equivalence — L1-R4

### Representation choice

`0x1401B79E0` implements:

```text
packed path rounded size > 0 -> representation 1
else .lst path rounded size > 0 -> representation 2
else -> no representation
```

Packed-first remains confirmed, but the positive-size test now has an exact meaning: it is based on `0x1402EF620` rounded capacity.

### Planner `0x1401B7FD0`

The planner and the materializer use the same broad size rules:

- aligned header/table placement;
- 64-byte child alignment;
- child rounded size through `0x1402EF620` where packed bytes exist;
- recursive planner for synthesized `.lst` children.

### Writer `0x1401B85C0`

The materializer:

1. reads/parses the `.lst` description;
2. constructs the aligned offset table;
3. copies that table into the zeroed output;
4. walks children using the same rounded-size/recursive-size rules;
5. queues packed/general children through `0x1402EF4D0` or recursively synthesizes `.lst` children.

Structural planner/writer size and placement equivalence is therefore bounded-closed for the recovered branches.

### Important failure boundary

Child submission/recursive return values are not consistently aggregated into a single terminal parent result at the observed call sites. The `.lst` materializer can finish structural/scheduling work while child I/O remains queued.

Therefore:

```text
planner/writer structural equivalence = bounded-closed
all-child terminal success/failure aggregation = NOT locally closed in L1 materializer
```

The latter belongs to the explicit L1-terminal -> L3-completion seam and must not be disguised as a synchronous `.lst` success guarantee.

## 7. Exact ingress behind `0x1402EF4D0` — L1-R6 bounded-closed

Fresh raw code upgrades the previous safe label.

`0x1402EF4D0`:

- selects a queue lane;
- checks the current `0x88`-byte record is free;
- stores the caller-owned destination at record `+0x08`;
- writes job type `2` at record `+0x00`;
- copies and normalizes the path into record `+0x10`;
- advances the producer index modulo lane capacity;
- marks the lane active;
- returns success/failure for queue insertion.

Bounded role:

> **enqueue type-2 whole-file path-load into caller-owned destination**.

This closes the previous question of whether `0x1402EF4D0` was merely an unidentified materialization wrapper. It is still a cross-layer helper: byte destination/path authority supports L1, while queue lifetime/poll/retirement semantics are L3.

## 8. Persistent scheduler polling proves the completion barrier shape

`0x1402EF790` directly recovers the type-2 job state machine.

### Phase 0

For type 2 it opens the path through `0x1400333F0`, obtains rounded chunk count, submits `0x140033500`, then advances the lane phase.

### Phase 1

It polls `0x1400333E0`:

```text
status 2 -> return without retiring the job
status 3 -> close/free whole-file state, clear job record, advance lane
status 4 -> failure/reset branch
```

This directly rejects a FIFO-only dependency model.

The materialization job **persists while lower transfer is pending**. Normal later work cannot simply pass it by in the same lane on successful common-path execution.

No generic fan-in counter is needed for this bounded direct-resource case.

## 9. Remaining P0 seam: error rollback/suppression origin

The success path is now substantially recovered, but the error path is not fully closed.

On status `4`, `0x1402EF790` takes its failure/reset branch. At the observed local branch it does not perform the same success cleanup that clears the state/job and advances the lane.

Separately, `0x1402EF460` is directly recovered as a **pending queue rollback/truncation** helper:

- walks queued `0x88`-byte records backward;
- clears pending record type fields;
- updates ring producer/active state;
- does not itself prove cancellation of already-running lower FileSlot I/O.

Its direct recovered use from `0x1401B8430` places it under LoadedResource cancellation/replacement policy before state `1|2 -> 4` marking and deferred cleanup scheduling.

The remaining high-priority question is therefore narrower:

> **What exact owner/path detects a terminal status-4 materialization lane and guarantees that the normal later `0x1401B8DC0` state2 callback cannot publish success for the failed resource?**

This must be recovered without moving normal `state1 -> state2` publication into L1.

Canonical seam:

```text
[L1] byte transfer terminal success/error
 -> [SEAM] queue eligibility / rollback / suppression
 -> [L3] normal 0x1401B8DC0 state1 -> state2 publication OR cancellation/state4 path
```

## 10. `0x1401B8CA0` return semantics are path-dependent

Fresh raw review shows a subtle but important difference:

- nonzero descriptor-type direct path queues `0x1402EF4D0`, then returns true without propagating that queue-call result;
- packed representation path tail-jumps to `0x1402EF4D0`, so its insertion result propagates;
- `.lst` representation path tail-jumps to `0x1401B85C0`, whose return describes local structural/scheduling acceptance rather than terminal completion of every child;
- no representation returns false.

Therefore `0x1401B8CA0 == true` must **not** be documented as universal proof that all selected bytes are already terminally resident.

It is the L1 materializer acceptance/result seam whose exact meaning depends on representation path; later queue completion still matters.

## 11. `0x1402EF920` synchronous helper caveat

`0x1402EF920` opens a whole-file state, submits rounded transfer, polls until status `3` or `4`, closes the state and returns rounded capacity.

At the observed path it does not encode status `4` into a distinct return value; it still returns the rounded size after terminal error.

This helper is used in `.lst` description loading and therefore remains a bounded error-path compatibility concern. Common successful-path structure is strong; exhaustive malformed/I/O-failure equivalence remains secondary breadth and must not be conflated with the now-closed normal byte path.

## 12. L1-R8 remains a deliberate scope limit

Nothing in this pass creates a universal original intrinsic child-length field for generic relative-slot PAC/PNST children.

Relative starts and parent spans remain different authorities from intrinsic serialized child EOF.

Continue to separate:

1. layout-preserving patch against original packed bytes;
2. reflow/synthesis only where exact child extent is independently evidenced.

## 13. Revised immediate work order

```text
1. trace status-4 owner/caller path around 0x1402EF790
2. prove exact failed-job rollback/suppression before normal 0x1401B8DC0
3. bind 0x1402EF460 to the precise cancellation/suppression cases without calling it OS CancelIo
4. reconcile .lst queued-child failure through the same confirmed seam
5. run a whole-image contradiction sweep for alternate type-2 queue/error owners
6. update canonical L1 roadmap/status after review/promotion of this raw pass
7. continue real-retail provenance -> representation -> edit/rebuild/rematerialization -> #209 Level-E
```

Do not restart already-bounded ZIP/PAC/PNST grammar or the common final-chunk path absent contradictory bytes.

## 14. Completion consequence

This raw pass materially narrows L1 but does not make it complete.

Allowed current statements:

- `L1 common physical/ZIP STORE/raw-DEFLATE whole-file byte path = bounded-closed at canonical analysis image`;
- `L1 .lst structural planner/writer placement = bounded-closed`;
- `L1 original padding zero initialization = closed at canonical allocator path`;
- `L1/L3 materialization failure-suppression seam = open`;
- `L1 overall = INCOMPLETE / NOT 100%`.

Disallowed:

- `L1 COMPLETE`;
- `state1 -> state2 is L1`;
- `0x1401B8CA0 true means all async bytes completed`;
- `FIFO alone proves terminal ordering`;
- `0x1402EF460 is OS AsyncIO cancellation`;
- `relative slot span is universal intrinsic child EOF`.
