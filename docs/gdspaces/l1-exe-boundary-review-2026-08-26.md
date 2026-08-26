# GDSpaces L1 — Canonical EXE Boundary Review

**Review date:** 2026-08-26  
**Canonical repository base:** `main@c20544cfb7f3ddba69a128a88246550a35eb51c1`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Layer:** L1 — Resource Materialization  
**Primary ledgers:** #100, #55  
**Completion-ordering follow-up:** `l1-exe-materialization-completion-pass-2026-08-26.md`

This document reconciles the L1 EXE boundary reviews and the follow-up materialization-completion ordering pass. It replaces older shorthand that mixed resource selection, byte transport, materialization completion and typed-ready lifecycle into one undifferentiated runtime spine.

## 1. Canonical three-layer cut

For review purposes, L1 ends when the selected resource has become complete materialized bytes and the resource-level materialization handoff publishes that completion. On the recovered LoadedResource path this is the `state 1 -> 2` boundary.

```text
L2 selected logical/provider/member identity
 -> L1 materialization submission
 -> L1 whole-file/FileSlot transport
 -> L1 physical backend OR NBZ member backend
 -> L1 STORE direct read OR raw-DEFLATE InflateRead
 -> L1 caller-owned destination bytes
 -> L1 packed representation OR .lst in-place synthesis
 -> L1 resource-level completion ordering / dependency barrier
 -> L1 state 1 -> 2
 ===== END L1 =====
 -> L3 typed post-load
 -> L3 optional ready callback
 -> L3 state 2 -> 3 / consumer visibility
 -> L3 claims/cache/reset/release/shutdown
```

The cut is architectural. A function may cross it; functions are not assigned wholesale to one layer when their behavior spans allocation, scheduling and lifecycle.

No separate original-runtime child/outstanding-work **fan-in counter** is currently evidenced. If such a mechanism exists, it remains an exact-byte/dataflow target rather than a recovered fact.

## 2. Strong L1 EXE boundaries

### Generic acquisition and destination ownership

`0x1401B84E0` is a cross-layer acquisition constructor. Evidence-backed L1 behavior includes:

- store `ResourceTypeInfo*` at record `+0x18`;
- compute required materialization size through `0x1401B7B90`;
- obtain/allocate backing through manager-mode-dependent paths;
- store materialization destination at record `+0x20`;
- invoke `0x1401B8CA0` to start materialization.

The same function also publishes state/scheduler metadata, so it must not be labeled as wholly L1.

`0x1401B8CA0` is the materialization dispatcher. For direct/packed paths it submits work through `0x1402EF4D0`; for container TypeInfo it uses `0x1401B79E0` to select packed vs loose-list representation and invokes `0x1401B85C0` for `.lst` synthesis.

### `0x1402EF4D0` correction

Safe bounded label:

> **resource materialization submission/scheduling wrapper**

Evidence proves it accepts the full path, destination and inherited `modeFlag` and participates in successful materialization scheduling before state 1 publication. Evidence does **not** prove that it is:

- an exact-path resolver;
- the final provider/backend open;
- a synchronous file reader;
- an OS read wrapper.

Older text using those stronger labels is superseded.

### Whole-file transfer layer

The generic whole-file materialization family is distinct from the internal ZIP member stream:

- `0x1400333F0(path)` opens through `OpenGameResource`, receives a signed FileSlot ID, queries materialized size and constructs a `0x50`-byte load-state;
- `0x1400333C0(state)` computes `ceil(totalBytes / 0x800)`;
- `0x140033500(state, chunkCount, destination)` converts chunks with `chunkCount << 11`, chooses the appropriate logical offset and submits through `0x14002EA40` into a **caller-owned destination**;
- `0x1400335A0(ticketId,userContext,errorFlag,bytesRead)` is a transport/whole-file completion callback: success advances loaded bytes and publishes transport status 3, failure publishes status 4;
- caller families around `0x1402EF790` and a synchronous-style wrapper around `0x1402EF920` prove that the load-state is I/O metadata/state, not the resource byte buffer itself.

The synchronous-style wrapper around `0x1402EF920` must **not** be equated with the `.lst` temporary-text loader without a direct caller/callee edge.

### FileSlot / ReadRequest transport

Strong recovered transport architecture includes:

- 100 FileSlot records × `0x20`, base `0x140C18E60`;
- global FileSlot critical section `0x140C19AE0`;
- `ReadRequestV2` size `0x38`;
- completion ABI `callback(ticketId,userContext,errorFlag,bytesRead)`;
- provider/backend -> FileSlot -> sync/async request -> transport completion architecture.

Exact every-field/error/cancellation breadth remains bounded reverse work, not missing architecture.

### Resource scheduler layer

Preserved canonical direct-disassembly authority establishes a higher scheduler layer distinct from raw FileSlot transport:

- `0x1402EF580` = scheduler-ring enqueue;
- `0x1402EF790` = scheduler worker / callback execution followed by slot clear;
- `0x1402EF460` = pending scheduled-entry clear/rollback.

The safe label for `0x1402EF460` is intentionally narrow. It is **not** evidence of OS-level AsyncIO cancellation or guaranteed cancellation of an already-running backend request.

The cancellation writer `0x1401B8430` uses this scheduler rollback before marking unfinished LoadedResource records state 4 and queueing deferred cleanup, so rollback belongs in the completion-ordering evidence packet.

### NBZ / ZIP backend

Strong original-runtime anchors:

- `0x140327CC0` ZIP archive index build;
- `0x140328160` normalized lookup;
- `0x1403291D0` normalized-name comparator;
- `0x140328C30` EOCD finder;
- `0x1403289F0` central-directory walker;
- `ZipEntryRead 0x140328F50` direct-vs-compressed member read;
- `InflateRead 0x140328820` raw-DEFLATE streaming materialization;
- lazy realization through `0x140328540`;
- compressed seek `0x140328FE0` semantically recovered as reset + reinflate/discard replay;
- raw/stored seek `0x1403290F0` with SET/CUR/END, logical clamp and physical `dataOffset + position` seek;
- teardown `0x140327DB0 / 0x140327D90` direct-EXE confirmed.

`0x140328540` and `0x140328FE0` still have exact-body/error-state breadth open, but their high-level architecture must not be reopened absent contradictory direct evidence.

## 3. Packed vs loose-container materialization

Container-backed resources have two original materialization representations:

```text
exact packed resource exists with positive size
 -> packed representation
else matching lowercase .lst exists with positive size
 -> synthesize loose representation
else
 -> acquisition failure
```

Recovered `.lst` mechanics include:

- scanner ceiling `0x1FC0` / 8128 bytes;
- child-token bound `0x100` / 256 bytes;
- CRLF-oriented grammar;
- `/` skip/comment state;
- `#XXXX` four-byte magic directive;
- default `PAC\0`;
- exact lowercase `dummy` sparse slot;
- declared physical slot order preservation;
- 64-byte synthesized header/child placement;
- nested `.lst` recursion;
- sibling packed `.pac` precedence;
- ordinary child and packed-sibling submissions through the generic materialization submission wrapper;
- recursive nested synthesis in-place inside the parent destination.

The `.lst` text itself is evidenced as being loaded synchronously into aligned temporary storage before bounded parsing. No direct stored edge proves that this loader is the synchronous-style `0x1402EF920` wrapper.

The exact mechanism that prevents parent completion from overtaking required child population is still open. Do not promote a child-count/fan-in object merely from recursive child submission.

## 4. Critical callback / completion-ordering correction

`0x1400335A0` and `0x1401B8DC0` belong to different completion layers.

### Transport completion

`0x1400335A0(ticketId,userContext,errorFlag,bytesRead)` observes transport I/O completion and updates the whole-file transfer state/status.

### Resource materialization completion

`0x1401B8DC0` is registered through scheduler enqueue helper `0x1402EF580`. For the normal record-context branch it reconstructs the LoadedResource record from the registry-relative context and publishes `state = 2`.

Therefore `0x1401B8DC0` must **not** be described as a raw I/O callback. There is a resource scheduler/materialization layer between transport completion and materialized-byte state publication.

The most valuable remaining L1 EXE seam is now named:

> **materialization completion ordering / dependency barrier**

Known cancellation-side ordering is stronger than the success-side dependency proof:

```text
0x1401B8430
 -> 0x1402EF460 pending scheduler clear/rollback
 -> state 1|2 -> 4
 -> enqueue 0x1401B8F00 via 0x1402EF580
 -> deferred cleanup -> state 0 + backing release
```

Open success-side questions include:

- what exact condition schedules or permits the state-2 completion handoff;
- what exact ordering exists between materialization work and `0x1401B8DC0` registration/execution;
- whether ring order alone is sufficient or another status/dependency object participates;
- whether an explicit outstanding-work/fan-in counter exists at all;
- how nested `.lst` child population participates in the completion decision;
- what happens when one child/submission fails;
- whether a partially populated parent destination remains live during failure/cancellation;
- how transport failure is mapped into resource-materialization scheduling failure;
- which failure paths prevent state 2 publication;
- what happens to already-running FileSlot/ReadRequest work when higher scheduler rollback begins.

These handoff questions are higher priority than re-reversing already strong ZIP seek/decompression architecture.

## 5. Current L1 EXE reverse matrix

| Boundary | Status | Review consequence |
|---|---|---|
| `0x1401B84E0` acquisition constructor | STRONG / cross-layer | L1 allocation/start + scheduler/state boundary; do not classify wholly L1 |
| `0x1401B8CA0` materialization dispatcher | STRONG | direct/packed/loose dispatch before state1 publication |
| `0x1402EF4D0` submission wrapper | STRONG bounded label | body/callees and load-context consumer still open |
| `0x1402EF580` scheduler enqueue | STRONG | higher scheduler callback registration is distinct from FileSlot transport |
| `0x1402EF790` scheduler worker | STRONG | executes queued callback then clears scheduler slot |
| `0x1402EF460` pending scheduler clear/rollback | STRONG bounded label | queue rollback, not OS AsyncIO cancellation authority |
| `0x1400333F0/3C0/500/5A0` whole-file transfer | STRONG | caller-owned destination and raw transport callback recovered |
| FileSlot / ReadRequestV2 architecture | STRONG | exact error/cancellation breadth remains bounded |
| ZIP index/EOCD/central walk | STRONG | do not restart core architecture |
| `0x140328F50` ZipEntryRead | EXACT-DISASSEMBLY BACKED | STORE vs compressed split |
| `0x140328820` InflateRead | EXACT-DISASSEMBLY BACKED | raw-DEFLATE streaming path |
| `0x140328540` lazy realization | HIGH | exact body/error-state breadth open |
| `0x140328FE0` compressed seek | HIGH | reset+replay semantics known; exact branches open |
| `0x1403290F0` raw seek | HIGH | SET/CUR/END + logical/physical mapping recovered |
| `0x140327DB0/7D90` teardown | DIRECT-EXE CONFIRMED | close/free ownership architecture known |
| `.lst` grammar/layout/recursion | STRONG | temp allocation/free/failure/completion-ordering breadth open |
| `0x1401B8DC0` state1→2 handoff | STRONG boundary | scheduler/resource completion, not raw I/O callback |
| explicit generic fan-in counter | NOT EVIDENCED | do not model as recovered behavior without direct dataflow |
| typed post-load / state2→3 | OUT OF L1 | L3 authority |

## 6. Updated reverse priority

When canonical EXE bytes are available, use the focused plan `data/reverse/dmc3-l1-materialization-completion-plan.v1.json` and this order unless a concrete acceptance receipt activates another dependency:

1. **materialization completion ordering / dependency barrier** around `0x1402EF4D0`, `0x1402EF580`, `0x1402EF790` and `0x1401B8DC0`;
2. **scheduler rollback semantics** around `0x1402EF460` and `0x1401B8430`;
3. **transport error -> resource scheduler/materialization error mapping** through `0x1400335A0` and the higher scheduler;
4. **`.lst` child completion/failure ordering** plus temporary allocation/free identity and failure cleanup;
5. FileSlot/ReadRequest partial-read/error/cancellation breadth where needed by a claimed compatibility boundary;
6. exact `0x140328540` / `0x140328FE0` body breadth only when an acceptance claim requires those details.

The broad blocked-window packet remains useful for multi-layer reacquisition, but the focused completion plan is the preferred next L1 byte packet.

## 7. L2/L3 exclusions and dependencies

Do not count as L1 reverse gaps:

- `OpenGameResource 0x14002FCA0` candidate/provider selection — L2 input;
- type-0 physical-provider selection/open policy — L2; the post-`0x0C` static boundary is already recovered/promoted by #215 and must not be reopened absent contradictory evidence;
- typed PAC/PNST MOD/EFM/SCM/SHW post-load — L3;
- optional ready callback and `state 2 -> 3` — L3;
- loader-node claims, cache-family policy, reset/release/shutdown — L3;
- wider cancellation lifecycle is L3, while the scheduler rollback mechanics immediately guarding unfinished L1 materialization are valid boundary evidence for this L1 completion-ordering review;
- Stage Ops / ModViz semantics — downstream.

L2/L3 work remains allowed when a concrete L1 acceptance path depends on it.

## 8. Acceptance consequence

These remaining exact-body/completion-ordering/error questions are bounded reverse breadth. They do not retroactively invalidate the current representative packed-NBZ/PAC/PNST product implementation path.

`L1 COMPLETE / 100%` still requires the real-retail and original-game receipts defined by the L1 roadmap and issue #209. Static EXE review cannot substitute for Level-E consumption.

## 9. Superseded shorthand

The following wording is now explicitly superseded:

- `0x1402EF4D0 == packed-file reader`;
- `0x1402EF4D0 == exact-path resolver/final provider open`;
- `0x1401B8DC0 == raw I/O callback`;
- `.lst synchronous temporary load == 0x1402EF920`;
- `FileSlot/AsyncIO as wholly L3` when discussing the L1 byte-transport path;
- `materialization fan-in` when intended to assert an evidenced child/outstanding-work counter;
- `0x1402EF460 == OS AsyncIO cancellation`;
- `0x140328540/0x140328FE0 architecture unknown`;
- `type-0 physical final-open semantics still open` after #215.

Historical pass comments remain evidence history; future status/roadmap/review work must use this reconciled boundary and the completion-ordering follow-up pass.
