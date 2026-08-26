# GDSpaces L1 — Canonical EXE Boundary Review

**Review date:** 2026-08-26  
**Canonical repository base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Primary ledgers:** #100, #88, #55  
**L3 raw authority:** `l3-boundary-audit-2026-08-26.md`  
**Focused handoff follow-up:** `l1-l3-exe-materialization-lifecycle-handoff-pass-2026-08-26.md`

This document reconciles the L1 EXE byte-materialization boundary against the canonical raw-EXE L3 audit. It replaces older shorthand that mixed byte mechanics, FileSlot request ownership, scheduler completion and LoadedResource state publication.

## 1. Canonical layer cut

```text
L2 selected logical/provider identity
 -> L1 exact physical/member byte acquisition
 -> L1 read/seek/decompression mechanics
 -> L1 exact materialized byte buffer + provenance
 ===== L1 BYTE AUTHORITY =====
 -> L3 FileSlot / async request ownership and scheduling
 -> L3 LoadedResource acquisition / state 0 -> 1
 -> L3 completion / state 1 -> 2
 -> L3 typed post-load / optional ready callback
 -> L3 state 2 -> 3 / consumer-ready visibility
 -> L3 claim/reuse/cancellation/reset/release/teardown
```

FileSlot is a boundary subsystem. Its byte-read mechanics can support L1, while request ownership, scheduling, completion, cancellation and close lifetime belong to L3.

Functions are classified by behavior rather than by one contiguous VA range.

## 2. Strong L1 byte-materialization boundaries

### Whole-file byte mechanics

The bounded whole-file path establishes caller-owned destination semantics:

- `0x1400333F0` constructs the whole-file load-state after opening the selected resource;
- `0x1400333C0` computes the required chunk count;
- `0x140033500` submits transfer into a caller-owned destination through `0x14002EA40`;
- `0x1400335A0(ticketId,userContext,errorFlag,bytesRead)` updates transfer progress/status.

These functions are important to L1 byte reconstruction, but the request/callback lifecycle itself is classified by the canonical L3 audit.

### `0x1402EF4D0`

Safe bounded label:

> **resource materialization submission/scheduling wrapper**

Evidence proves it accepts the resource path, destination and inherited load-context/mode parameter and participates in materialization start.

It is not proven to be:

- an exact-path resolver;
- the final provider/backend open;
- a raw OS read wrapper;
- a synchronous reader;
- one universal FileSlot submit function.

Its complete body/callees and the first concrete consumer of the inherited parameter remain priority exact-byte targets.

### `0x1401B8CA0` — explicit L1/L3 seam

This function is best treated as a semantic seam:

- representation/materialization dispatch mechanics are L1-relevant;
- successful return gates L3 acquisition/state1 publication in `0x1401B84E0`.

Do not classify the whole function as exclusively L1 or L3.

## 3. Strong backend mechanics

Do not restart without contradictory evidence:

- numbered-volume bootstrap/precedence and resolver-selected archive identity;
- ZIP EOCD/central-directory/index/search architecture;
- `ZipEntryRead 0x140328F50` direct-vs-compressed split;
- `InflateRead 0x140328820` raw-DEFLATE streaming behavior;
- compressed seek reset+reinflate/discard architecture at `0x140328FE0`;
- raw/stored seek architecture at `0x1403290F0`;
- ZIP entry/stream teardown architecture;
- type-0 physical-provider post-`0x0C` final-open static chain promoted by #215.

Exact malformed/error/lifetime breadth remains bounded and claim-dependent.

## 4. `.lst` byte reconstruction

Strong mechanics remain:

- packed-first representation choice;
- `.lst` fallback;
- scanner ceiling `0x1FC0`;
- child-token bound `0x100`;
- CRLF-oriented grammar;
- `/` comment/skip state;
- `#XXXX` four-byte magic directive;
- default `PAC\0`;
- `dummy` sparse slot;
- 64-byte synthesized header/child placement;
- sibling packed `.pac` precedence;
- nested recursive in-place synthesis;
- ordinary/packed child materialization into the parent destination through the generic materialization submission layer.

The `.lst` text is evidenced as being acquired synchronously into aligned temporary storage before bounded parsing. No direct evidence equates that temporary loader with `0x1402EF920`.

Open L1 byte-side questions are temporary-buffer allocation/free/failure cleanup and exact child-population failure propagation.

## 5. Scheduler/completion correction

`0x1400335A0` and `0x1401B8DC0` are different completion layers:

- `0x1400335A0` = lower transfer progress/status callback;
- `0x1401B8DC0` = L3 LoadedResource normal completion writer, `state 1 -> 2`.

The higher scheduler is bounded by:

- `0x1402EF580` = scheduler-ring enqueue;
- `0x1402EF790` = scheduler worker/callback execution and slot clear;
- `0x1402EF460` = pending scheduled-entry clear/rollback.

`0x1402EF460` is not OS AsyncIO cancellation authority.

## 6. Fan-in correction

No generic original-runtime child/outstanding-work **fan-in counter** is currently evidenced.

The safe open concept is:

> **materialization-to-lifecycle completion ordering / dependency barrier**

The exact mechanism may be synchronous ordering, scheduler ordering, nested callbacks, another status/dependency object or an explicit counter. Do not promote one without direct dataflow.

This is especially important for `.lst`: recursive/ordinary child population does not by itself prove a generic aggregate counter.

## 7. Current reverse matrix

| Boundary | Status | Consequence |
|---|---|---|
| whole-file caller-owned byte destination | STRONG | L1 byte mechanics established |
| FileSlot byte-read mechanics | STRONG / boundary | may support L1 |
| FileSlot request ownership/scheduling/completion | L3 | canonical raw-L3 authority |
| `0x1402EF4D0` submission wrapper | STRONG bounded label | exact body/callees/load-context semantics open |
| `0x1401B8CA0` materialization dispatch | STRONG seam | L1 mechanics; success gates L3 state1 |
| `0x1402EF580/790` scheduler ring | STRONG | L3 scheduler ownership |
| `0x1402EF460` scheduler rollback | STRONG bounded label | not OS cancellation |
| `0x1401B8DC0` state1->2 | STRONG | L3 lifecycle completion writer |
| generic fan-in counter | NOT EVIDENCED | do not encode as recovered behavior |
| `.lst` grammar/layout/recursion | STRONG | byte-side failure/temp lifetime still open |
| ZIP core read/inflate/seek architecture | STRONG | exact malformed/error breadth bounded |

## 8. Next exact-byte priority

Use `data/reverse/dmc3-materialization-lifecycle-handoff-plan.v1.json`.

Priority:

1. exact `0x1402EF4D0` body/callees and load-context consumer;
2. exact handoff from byte-materialization work into L3 request/scheduler ownership;
3. success-side completion ordering/dependency mechanics without assuming a fan-in counter;
4. `0x1402EF460` scheduler rollback match/clear semantics and interaction with already-running transport;
5. transport failure -> L3 acquisition/cancellation mapping;
6. `.lst` child-population failure + temporary-buffer cleanup;
7. acceptance-activated FileSlot partial/error/cancellation breadth;
8. ZIP exact-body/error tails only when a claimed compatibility boundary requires them.

## 9. Superseded shorthand

Do not reintroduce:

- `L1 ends at LoadedResource state1->2`;
- `FileSlot/AsyncIO is wholly L1`;
- `FileSlot/AsyncIO is wholly L3` without distinguishing byte mechanics from request ownership;
- `0x1402EF4D0 == exact-path resolver/final provider open/raw reader`;
- `0x1401B8DC0 == raw I/O callback`;
- `.lst synchronous temporary load == 0x1402EF920`;
- `materialization fan-in == evidenced generic counter`;
- `0x1402EF460 == OS AsyncIO cancellation`;
- type-0 physical final-open semantics still open after #215.

## 10. Acceptance consequence

This reconciliation does not reopen the current representative L1 product implementation path and does not change the external L1 acceptance gates.

L1 still requires exact real-retail byte/provenance/classification, one supported real edit/rebuild/rematerialization path, and the game-backed vertical receipt. L3 lifecycle completion remains separately gated by its canonical static and original-process evidence requirements.
