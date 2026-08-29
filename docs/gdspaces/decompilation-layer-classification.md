# GDSpaces Decompilation-Layer Classification

**Canonical reconciliation:** 2026-08-26  
**Current-main base:** `main@a90b017ab29171e00174f2a56c719c32241a63f1`  
**L3 raw authority:** [`l3-boundary-audit-2026-08-26.md`](l3-boundary-audit-2026-08-26.md)  
**Materialization completion authority:** [`materialization-completion-boundary-pass-2026-08-26.md`](materialization-completion-boundary-pass-2026-08-26.md) + [`materialization-completion-dependency-pass2-2026-08-26.md`](materialization-completion-dependency-pass2-2026-08-26.md)

This document keeps GDSpaces/resource-runtime work separated by ownership and acceptance layer. Layer ownership is semantic and evidence-driven; it is not inferred from tool ownership or one contiguous executable address range.

## Canonical tags

### [L1] Resource Materialization

```text
physical/container bytes
 -> bounded acquisition
 -> transform/decompression
 -> exact materialized bytes
 -> nested extraction
 -> exact editable child authority
 -> WorkingCopy/edit
 -> rebuild/repack/publication
 -> reopen/rematerialization
```

L1 is not closed by lookup, enumeration, structural parsing or synthetic composition alone.

### [L2] Resource Resolution

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume selection
 -> duplicate/ambiguity behavior
 -> exact ResourceRef identity
```

### [L3] Original Runtime / Lifecycle

```text
selected/materialized resource
 -> FileSlot / AsyncIO ownership and callbacks
 -> LoadedResource states
 -> typed post-load
 -> ready visibility
 -> claims/cache/factory handoff
 -> cancellation/reset/release/unload/shutdown
```

Original code here belongs to the Recovered Game Source Tree.

### [V] Validation

Cross-cutting hashes, corpus receipts, CI, original-vs-reconstruction comparison and original-game execution. Validation is not a fourth decompilation layer.

### [DOMAIN] Stage Assembly / Stage Ops

Stage/room semantic assembly, geometry/collision/camera/lighting/events/effects/audio relationships and Stage Ops UI are downstream consumers of resource authority.

They are **not Layer 3** in the canonical L1/L2/L3 resource-runtime model and must not create their own materializer/resolver/lifecycle authority.

### [OUTSIDE]

Product/extraction metadata or tooling information not established as original DMC3 runtime behavior.

## Current classification matrix

| Area | Layer | Current boundary |
|---|---|---|
| NBZ local/central/EOCD and stored spans | L1 | canonical bounded read/serialization evidence |
| STORE/raw-DEFLATE member materialization | L1 | strong canonical |
| archive/member provenance stability | L1 + V | evidence execution remains representation-specific |
| PAC/PNST relative-slot parsing | L1 | strong canonical |
| recursive PAC/PNST byte expansion | L1 | strong canonical |
| bounded PAC/PNST reflow/reintegration | L1 | canonical at evidenced writer scope |
| `.lst` synthesized bytes | L1 | structurally recovered; dynamic completion/error edges separate |
| `.lst` packed-first representation choice | L2 | recovered selection behavior |
| `DMC3-N.nbz` bootstrap/first-gap/precedence | L2 | strong canonical |
| request basename candidates | L2 | strong recovered boundary |
| archive normalization/index/qsort/bsearch | L2 | strong recovered boundary |
| type-0 physical final filename/open behavior | L2 | bounded static chain recovered; protected-process selected identity separate |
| FileSlot byte-read mechanics | L1 support | byte acquisition behavior only |
| FileSlot pool/AsyncIO request ownership | L3 | substantial recovered static spine |
| `0x1401B8CA0` representation/materialization dispatch | L1/L3 seam | mechanics are L1; success controls L3 state1 publication |
| `0x1402EF4D0` materialization submission/job creation | L1 support + L3 scheduling seam | exact queued job/terminal dependency behavior still open |
| lower whole-file transport `0x140033500/0x1400335A0` | L1 support + L3 request lifecycle | caller-owned byte transport/status; not LoadedResource state2 callback |
| materialization completion ordering / dependency bridge | cross-layer | no generic fan-in counter evidenced; terminal gating before normal `1B8DC0` dispatch remains open |
| LoadedResource 0/1/2/3/4 lifecycle | L3 | strong central static spine |
| cancellation `1|2 -> 4` | L3 | source-state domain closed for canonical writer |
| quiescence predicate | L3 | all 363 records must be in `{0,3}` |
| ordinary/cancel/group/full release policies | L3 | distinct ordering/result semantics recovered |
| typed MOD/EFM/SCM/SHW post-load | L3 | bounded family authority |
| central typed unknown/default behavior | L3 | best-effort/no-op; not a state2-blocking failure return |
| loader-node claims/reset/release | L3 | substantial recovered authority |
| group-5 exhaustion | L3 original behavior | hard capacity invariant; not safe product policy |
| runtime vs CRT vs process-lifetime teardown | L3 | distinct lifetime domains recovered |
| dynamic transition/reload/shutdown receipts | L3 + V | OPEN representative Level-E coverage |
| StageBundle / StageAssemblyWorkspace | DOMAIN | downstream consumer, not L3 |
| Stage Ops / Stage Editor | DOMAIN | downstream tooling, not L3 |
| `.index` manifests | OUTSIDE | metadata, not original lookup authority on recovered path |
| binary AFS backend | evidence-gated | not established by `.afs/` namespace strings |
| PACK original runtime use | evidence-gated | Web DMC Rengine v6 product parser is not original-runtime proof |

## GDS-relevant EXE function-boundary matrix

### Strong / do not restart without contradiction

#### L1/L2

- resource bootstrap / numbered-volume registration family around `0x14002E930`;
- mounted-source resolver family around `0x140327430`;
- basename-oriented `OpenGameResource` request path around `0x14002FCA0`;
- normalization family including archive `0x0E` and physical `0x0C` behavior;
- archive central index/sort/search family;
- `ZipEntryRead 0x140328F50` direct-vs-inflated branch;
- `InflateRead 0x140328820` raw-DEFLATE streaming behavior at the recovered scope;
- major `.lst` packed-first/synthesis structure;
- caller-owned whole-file transfer submission/callback boundary around `0x140033500/0x1400335A0`.

#### L3 / cross-layer scheduler

- LoadedResource registry `0x140C99D30`, 363 records × `0x48`, seven groups;
- acquisition `0x1401B84E0` with state1 only after materialization success;
- normal completion `0x1401B8DC0` state2 publication;
- merged #230 one-u32 normal completion context ABI through `0x1402EF580/0x1402EF790`;
- finalizer `0x1401B92D0`: typed post-load -> optional callback -> state3;
- cancellation `0x1401B8430`: only states1/2 -> state4;
- quiescence `0x1401B84B0`: every record must be state0 or state3;
- cancel cleanup `0x1401B8F00` vs ordinary release `0x1401B9530` vs group/full reset `0x1401B9560/0x1401B95E0`;
- typed dispatcher `0x1401B9FA0`, representative families and recursive PNST behavior;
- loader claim/release anchors `0x1401AE220`, `0x1401AF6A0`, `0x1401AF6F0`;
- runtime backing release `0x140337710` distinguished from CRT backing destructor `0x140337440`.

These boundaries may still have open family/error/profile edges; that does not justify restarting the already recovered core behavior.

## Materialization completion dependency correction

Merged #228 supersedes broad `fan-in/completion` wording as evidence of a generic child/outstanding-work counter. No such universal counter is currently established.

Merged #230 proves the normal `0x1401B8DC0` callback receives only one u32 registry-relative context. It does **not** receive transport status, error flag, byte count, FileSlot handle or child/outstanding-work metadata.

Therefore the success/error dependency must be resolved before normal `0x1401B8DC0` dispatch, or the queued completion must be removed/suppressed before it executes.

FIFO insertion order alone is insufficient if a prior materialization scheduler job can submit asynchronous transport and retire immediately. The next raw pass must determine the actual terminal condition: persistent polling job, callback-driven terminal state, separate scheduler gate, synchronous completion, or another directly evidenced mechanism.

Historical helper roles around `0x1400333E0` (status/poll) and `0x140033390` (terminal release/close) remain reacquisition hypotheses until fresh canonical bytes confirm them.

## Bounded open reverse targets

### Cross-layer materialization completion bridge — current first priority

1. `0x1402EF4D0`: exact queued materialization job callback/state/type and inherited load-context consumer;
2. `0x1402EF790`: identify that job's dispatch case and persistence/re-poll/retirement behavior;
3. `0x1400333E0`: reacquire pending/success/error status domain;
4. `0x140033390`: reacquire terminal cleanup/release ordering;
5. `0x1400335A0`: bind lower transport writes into the terminal state;
6. determine what prevents normal `0x1401B8DC0` dispatch on failed/incomplete transport;
7. `0x1402EF460`: recover higher scheduler suppression/rollback without relabeling it OS AsyncIO cancellation;
8. only then apply the confirmed direct-resource mechanism to `.lst` child/recursive failure ordering.

### L1/L2

1. exact protected-process selected-provider identity after independent runtime mapping;
2. real-retail normalized-key collision evidence where required by final L2 acceptance;
3. remaining ZIP/loose-container error/lifetime edges only where they affect a claimed compatibility boundary.

### L3

1. alias-aware residual whole-image census of possible `LoadedResource +0x04` value-flow writers/callers;
2. family-complete ownership of `+0x08/+0x18/+0x20/+0x28` and stable adjacent fields;
3. external typed/factory/dependency failure paths outside the central best-effort dispatcher;
4. SCM `mesh +0x28` reconciliation;
5. shared-owner coordination breadth outside already bounded loader-node families;
6. protected original-process V1–V7 lifecycle receipts;
7. cross-build/profile differences.

## Cross-boundary rules

- Physical provider selection is L2; materializing the selected bytes is L1.
- `.lst` representation choice is L2; synthesized container bytes are L1.
- `0x1401B8CA0` is an explicit L1/L3 seam: materialization mechanics are L1, its success result gates L3 state1 publication.
- FileSlot can support L1 byte-read reconstruction while original pool/scheduler/callback ownership remains L3.
- State1/state2 remain L3 lifecycle states even when their ordering is supporting evidence for L1 byte/materialization correctness.
- `0x1400335A0` lower transport completion and `0x1401B8DC0` state2 completion are distinct layers.
- A lower generic allocator/backing helper is not automatically L3; classify by caller/state ownership context.
- Stage Ops consumes L1/L2/L3 authority and is not itself one of the three resource-runtime layers.
- Product hardening never becomes original-game acceptance behavior automatically.
- Writer compatibility with read/runtime contracts does not prove Capcom offline-writer equivalence.
- Evidence-grade archive/member receipts require artifact stability across index/member/hash observation.

## Current priority accounting

The three layers have separate remaining gates:

```text
L1 -> real-retail / edit-rebuild-rematerialization evidence
L2 -> protected-process mapped selected identity + retail collision gate
L3 -> narrow residual static census + materialization-completion scheduler bridge + V1–V7 original-process lifecycle
```

Progress in one layer must not be reported as completion of another, even when a vertical acceptance receipt spans all three.
