# GDSpaces L1 writer failure / integer-width reconciliation — 2026-08-28

**Status:** evidence-backed raw reverse checkpoint.  
**Layer:** L1 Resource Materialization, with an explicit L1/L3 dispatch seam.  
**Canonical executable:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.  
**Canonical implementation base for this pass:** `main@94692e8f9971cf8249b4b16ee88d309de8b49f11`.

This pass follows the transfer-extent correction merged in #255. It does not reopen the recovered `0x800` direct-child / `0x40` recursively-synthesized layout distinction. Its purpose is to reconcile the remaining **failure propagation and integer-width semantics** around the original `.lst` planner/writer and the queued whole-file ingress.

## Executive result

The original runtime does **not** provide one clean boolean meaning equivalent to “all exact bytes were successfully materialized”. Several functions return success after only planning or queue admission at a coarser level, and some downstream enqueue/recursive failures are ignored.

Therefore:

- an original return value of `true` from the loose writer is **not byte-completion proof**;
- a successful top-level materialization setup is **not proof that its completion callback was successfully enqueued**;
- exact byte completion must be reconciled with the queued consumer / whole-file status path separately;
- GDSpaces product code must remain fail-closed and must **not** copy the original unsafe failure-swallowing or 32-bit wrap behavior merely for literal emulation.

## 1. `0x1402EF4D0` is queue admission, not byte completion

Fresh raw disassembly confirms:

```text
0x1402EF4D0(queue, path, destination, lane)
  -> locate current 0x88-byte ring slot
  -> if slot.state > 0: return false
  -> slot.destination = destination
  -> slot.state = 2
  -> copy path
  -> normalize '/' to '\\'
  -> advance producer index modulo capacity
  -> mark queue active
  -> return true
```

The failure branch is direct at `0x1402EF4EB..0x1402EF4F2`: an occupied current slot (`state > 0`) returns `false` before any job is published.

This function therefore proves only **type-2 job enqueue acceptance**.

## 2. `0x1401B85C0` loose writer swallows child dispatch failures

The recovered loose writer first parses/plans the complete image, zero-initializes the planned destination and writes structural offsets. During child dispatch:

- direct child path calls `0x1402EF4D0`;
- recursively synthesized child path calls `0x1401B85C0` again.

Critically, the return values from those calls are not tested before the outer writer continues.

Observed call sites include:

```text
0x1401B8C11 -> call 0x1402EF4D0   ; direct child enqueue, return ignored
0x1401B8C2D -> call 0x1401B85C0   ; recursive writer, return ignored
0x1401B8C53 -> call 0x1402EF4D0   ; another direct branch, return ignored
```

The outer `0x1401B85C0` can therefore reach its success return even if a child enqueue was rejected because the ring slot was occupied or a nested writer returned failure.

### Consequence

`0x1401B85C0 == true` means that the outer loose-container writer passed its own preconditions / planning path sufficiently to dispatch work. It does **not** establish that every child job was admitted or that every child byte transfer completed.

This corrects any earlier wording that treated the loose writer return as an exact terminal materialization receipt.

## 3. `0x1401B8CA0` has branch-dependent return semantics

The materialization-result helper is not uniform across representation branches.

### Packed container branch

For the packed representation it tail-jumps to `0x1402EF4D0`, so the boolean directly reflects type-2 enqueue admission.

### Loose `.lst` branch

It tail-jumps to `0x1401B85C0`, inheriting the coarse loose-writer boolean described above. Child enqueue/recursive failures may already have been swallowed inside that writer.

### Alternate non-container/kind branch

A separate branch calls `0x1402EF4D0` and then explicitly sets `AL = 1`, so enqueue rejection is not propagated at all on that path.

### Consequence

A generic statement such as “`0x1401B8CA0 == true` means exact bytes are scheduled successfully” is false. The meaning is branch-dependent.

## 4. `0x1401B84E0` also ignores completion-job enqueue failure

The top-level setup path:

```text
0x1401B7B90 -> required backing size
 -> allocate backing storage
 -> 0x1401B8CA0 materialization dispatch
 -> set state +4 = 1
 -> 0x1402EF580 enqueue type-3 completion job
 -> return true
```

`0x1402EF580` itself returns `false` when its target ring slot is occupied (`state > 0`). `0x1401B84E0` does not test that return value before returning success.

This is an explicit L1/L3 seam: the L1 setup can be accepted while the later lifecycle/completion publication job was not admitted.

It must not be used to move normal LoadedResource lifecycle ownership into L1.

## 5. Queued consumer and whole-file terminal state remain separate

`0x1402EF790` is the consumer for the queued type-2 materialization job. For that path it enters:

```text
0x1400333F0(path)      open whole-file state / cached size
 -> 0x1400333C0        transfer chunk count
 -> 0x140033500        submit async whole-file transfer
 -> poll 0x1400333E0   global whole-file status
```

The previously recovered byte spine still applies:

- lower worker repeatedly reads until the request is filled or a backend returns EOF/no-progress/error;
- STORE NBZ clamps to logical EOF;
- callback `0x1400335A0` accumulates actual transferred bytes but does not independently require `loadedBytes == plannedBytes` before setting the success status;
- therefore a short source after the cached-size query can terminate as a short-success condition.

The queue/writer booleans are consequently insufficient terminal byte evidence.

## 6. Original planner arithmetic is 32-bit and wrap-prone

The original planner/writer performs key size arithmetic in 32-bit registers. The product implementation intentionally uses checked host/32-bit bounds instead of reproducing unsafe wrap.

### `0x1400333C0` chunk-count arithmetic

Exact instruction shape:

```text
mov eax, [state+8]     ; cached u32 size
add eax, 0x7ff         ; 32-bit wrap possible
cdq
and edx, 0x7ff
add eax, edx
sar eax, 11
```

This is equivalent to signed truncation-toward-zero of the **wrapped signed 32-bit** value `(size + 0x7ff)` divided by `0x800`; it is only the ordinary mathematical `ceil(size/0x800)` in the safe positive domain.

Concrete boundaries from the exact instruction semantics:

| cached size | chunk count | `0x1402EF620` extent after `<< 11` |
| --- | ---: | ---: |
| `0x00000000` | `0` | `0x00000000` |
| `0x00000001` | `1` | `0x00000800` |
| `0x7FFFF7FF` | `0x000FFFFF` | `0x7FFFF800` |
| `0x7FFFF800` | `0x000FFFFF` | `0x7FFFF800` |
| `0x7FFFF801` | `-0x00100000` | `0x80000000` |
| `0xFFFFF001` | `-1` | `0xFFFFF800` |
| `0xFFFFF002`..`0xFFFFFFFF` | `0` | `0x00000000` |

So the original helper has real signed/wrap discontinuities. A large physical/NBZ cached size can become a negative extent or even zero after the original arithmetic.

### Planner / writer accumulation

`0x1401B7FD0` and `0x1401B85C0` similarly use 32-bit additions/alignment for header and child placement. No clean checked-overflow contract is recovered in this path.

### Product rule

GDSpaces must continue to reject output/offset overflow explicitly. Original wrap is **evidence about the game**, not a product requirement to create unsafe buffers or forge successful receipts.

## 7. Scanner ceilings are bounds, not clean original error enums

Fresh branch-by-branch review also sharpens the malformed-list boundary.

### `0x1401B7C70`

The scanner is bounded by `0x1FC0`, but reaching the ceiling does not return a dedicated scan-limit error code; the helper is void.

### `0x1401B7D10`

When NUL / the `0x1FC0` ceiling is reached, the helper returns the count accumulated so far rather than an explicit “scan limit exceeded” status.

### `0x1401B7E60`

If the requested child is not found before NUL / the ceiling, it returns false.

### `0x100` token boundary

At the exact token-copy bound, the original code branches out of the normal copy/terminator path and continues toward formatting logic rather than returning a dedicated token-limit error. The product `token_limit_exceeded` status is therefore **fail-closed hardening**, not an original error enum reconstruction.

No claim is made here about deterministic contents beyond that malformed boundary; the important recovered fact is the absence of a clean original error contract.

## 8. Corrected L1 completion interpretation

For original runtime reverse, keep these states separate:

```text
representation/planner accepted
 -> outer writer dispatched
 -> child queue admission(s)
 -> queued consumer started whole-file state(s)
 -> actual byte transfer terminal status
 ===== L1 exact byte/result boundary =====
 -> completion/lifecycle publication
 ===== L3 lifecycle =====
```

Because the original outer writer can swallow enqueue/recursive failure, the exact L1 terminal state cannot be represented by one upstream boolean alone.

## 9. Product implications

No product code change is required to imitate the unsafe original behavior.

Required product stance:

- keep explicit overflow rejection;
- keep `scan_limit_exceeded` / `token_limit_exceeded` fail-closed safety where exposed;
- require exact child capabilities and valid receipts in size-changing authoring;
- do not weaken successful authoring/materialization receipts to match original queue failure swallowing;
- distinguish recovered original behavior from product validation policy in documentation/tests.

## 10. Updated reverse frontier

Now substantially closed by fresh raw EXE review:

- cached materialized-size source and zero behavior;
- direct 0x800 transfer extent vs exact logical bytes;
- lower final-read clamp / EOF / short-produced composition;
- `.lst` planner vs writer layout arithmetic;
- original zero initialization/padding;
- enqueue vs byte-producing ingress behind `0x1402EF4D0`;
- writer child-failure propagation behavior;
- original 32-bit chunk/extent wrap semantics;
- scanner/token bounds vs product fail-closed policy.

Still open before an exhaustive L1 original-runtime claim:

1. exact recursive cycle/depth behavior and lifetime/free semantics;
2. remaining allocator/backend failure branches not already classified;
3. final L1-terminal -> L3 normal-completion suppression/eligibility reconciliation, including `0x1402EF460` / `0x1401B8DC0` context;
4. representative real `.lst` artifact/corpus receipt for a real loose-list claim;
5. controlled original-game consumption receipt for authored bytes;
6. final contradiction-free cross-stack audit.

## Non-claims

This checkpoint does not claim:

- L1 COMPLETE / 100%;
- original malformed-input safety;
- Capcom offline writer equivalence;
- that unsafe original queue/wrap behavior should be reproduced in GDSpaces;
- controlled original-game consumption.
