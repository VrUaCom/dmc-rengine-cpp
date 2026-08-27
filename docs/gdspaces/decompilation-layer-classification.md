# GDSpaces Decompilation-Layer Classification

**Canonical reconciliation:** 2026-08-27  
**Base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Boundary/status authority:** [`layer-boundary-status-reconciliation-2026-08-27.md`](layer-boundary-status-reconciliation-2026-08-27.md)  
**Focused L1 gap authority:** PR #244 / `l1-byte-exactness-gap-pass-2026-08-27.md`

Layer ownership is semantic and evidence-driven. A helper, queue, object or EXE address is not assigned wholesale to a layer. Split a helper by behavior when it participates in more than one authority.

## Canonical tags

### [L2] Resource Resolution

> Which logical resource/provider/source/volume/member is selected?

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume traversal
 -> duplicate/ambiguity/fallback/failure classification
 -> usable selected ResourceRef/provider/member identity
```

A provider hit that fails before producing a usable selected resource remains L2 failure semantics.

### [L1] Resource Materialization

> Given a selected identity, what exact byte representation is required and produced?

```text
selected provider/member identity
 -> logical/materialized size authority
 -> destination capacity/allocation
 -> selected byte/span acquisition semantics
 -> final-chunk / EOF / short-read / progress semantics
 -> transform/decompression
 -> exact caller-owned destination bytes
 -> packed OR .lst synthesized representation
 -> nested PAC/PNST/.lst byte construction
 -> terminal materializer success/error
 -> 0x1401B8CA0 materialization result
```

Product provenance, edit/rebuild/repack/publication/reopen-rematerialization also remain L1 product responsibilities.

### [L3] Original Runtime / Lifecycle

> How does the original runtime schedule, publish, ready, own, cancel and release resources?

```text
request/job ownership and scheduler lifetime
 -> completion eligibility
 -> LoadedResource state 1 -> 2 publication
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3
 -> consumer-ready visibility
 -> claims/cache/factory/dependency ownership
 -> cancellation/replacement
 -> state4 cleanup
 -> release/reset/shutdown
```

LoadedResource `state1 -> state2` remains L3 lifecycle state publication. L1 terminal byte correctness must already be satisfied before that publication can be valid.

### [SEAM] Cross-layer dependency

A seam is used when one layer's result gates another layer's action without transferring ownership of both behaviors.

The primary current seam is:

```text
[L1] terminal materializer byte/result state
 -> [SEAM] completion eligibility / suppression
 -> [L3] scheduler callback and state1 -> state2 publication
```

### [V] Validation

Hashes, corpus receipts, CI, original-vs-product comparisons and original-process observations. V is cross-cutting, not a numbered layer.

### [DOMAIN] Stage Assembly / Stage Ops / ModViz

Stage/room semantic assembly, geometry/collision/camera/lighting/events/effects/audio relations, Stage Ops and ModViz are downstream consumers. They are not L3.

### [OUTSIDE]

Product/extraction metadata or tooling information not established as original DMC3 runtime behavior.

## Current classification matrix

| Area / behavior | Canonical owner | Boundary |
|---|---|---|
| request basename/candidate construction | L2 | logical request policy |
| numbered-volume bootstrap/precedence | L2 | provider/source selection |
| archive normalization/qsort/bsearch | L2 | selected archive/member identity |
| type-0 physical provider selection/open failure | L2 | usable-resource selection/failure |
| materialized/logical size authority | L1 OPEN | `0x14002F9F0 -> 0x140048E20` exact zero/error semantics open |
| selected backend/member byte extent | L1 | selected-byte acquisition |
| final 0x800 chunk clamp / EOF / short read | L1 OPEN | byte-exactness gap |
| STORE/raw-DEFLATE byte production | L1 | exact bytes |
| destination capacity / 64-byte rounding | L1 OPEN | `0x1401B7B90` breadth open |
| destination initialization / `.lst` padding contents | L1 OPEN | original byte state not proven |
| PAC/PNST relative-slot topology | L1 | strong bounded byte structure |
| recursive PAC/PNST expansion | L1 | nested exact bytes |
| `.lst` packed-vs-loose representation decision | L1 | representation of same selected identity |
| `.lst` planner `0x1401B7FD0` | L1 OPEN | planner/writer equivalence open |
| `.lst` materializer `0x1401B85C0` | L1 | grammar/layout strong; error/padding breadth open |
| generic packed child extent | L1 scope limit | relative starts do not prove universal intrinsic child size |
| FileSlot/ReadRequest byte count/result semantics | L1 support | only where needed for exact selected bytes |
| FileSlot/ReadRequest object/queue/callback ownership | L3 | runtime I/O lifecycle |
| `0x140033500/0x1400335A0` byte/progress/result semantics | L1 support | final-chunk/terminal composition open |
| `0x140033500/0x1400335A0` request/callback lifetime | L3 support | scheduler/request ownership |
| `0x1402EF4D0` exact byte-producing ingress/context | L1 OPEN | safe label remains submission/scheduling wrapper |
| `0x1402EF4D0` queued-job ownership/persistence | L3 OPEN | do not infer from byte role |
| relevant `0x1402EF790` dispatch/poll/retire behavior | L3 | scheduler behavior supporting seam |
| materialization terminal -> completion eligibility | SEAM OPEN | no generic fan-in counter evidenced |
| normal `0x1401B8DC0` state1 -> state2 | L3 | callback ABI strong; state publication lifecycle |
| typed MOD/EFM/SCM/SHW post-load | L3 | consumes state2/materialized bytes |
| `0x1401B92D0` state2 -> typed-ready -> state3 | L3 | ready publication |
| cancellation `1|2 -> 4` | L3 | lifecycle policy; can suppress pending completion |
| `0x1402EF460` pending-entry clear/rollback | SEAM | classify exact action by target after reverse |
| state4 cleanup / ordinary/group/full release | L3 | lifecycle ownership |
| loader-node claims/cache/factory ownership | L3 | post-materialization shared ownership |
| runtime/CRT/process teardown | L3 | lifetime domains |
| archive/member provenance stability | L1 + V | materialization evidence |
| protected selected-provider trace | L2 + V | original selection evidence |
| typed-ready/use/release trace | L3 + V | original lifecycle evidence |
| StageBundle / StageAssemblyWorkspace / Stage Ops / ModViz | DOMAIN | downstream consumer/tooling |
| `.index` manifests | OUTSIDE | metadata, not recovered lookup authority |
| binary AFS / PACK original runtime use | evidence-gated | no promotion without direct evidence |

## L1 mandatory reverse frontier

The current L1 reverse is **not exhaustive**. Mandatory focused classes are:

1. rounded transfer vs exact logical extent;
2. size/zero/error semantics at `0x14002F9F0 -> 0x140048E20`;
3. capacity/alignment/overflow at `0x1401B7B90`;
4. `.lst` planner/writer equivalence (`0x1401B7FD0` vs `0x1401B85C0`);
5. synthesized padding byte initialization;
6. exact byte-producing ingress/context behind `0x1402EF4D0`;
7. partial read / InflateRead terminal composition;
8. explicit non-proof of universal packed-child intrinsic size.

Focused reacquisition order is defined in `l1-roadmap.md` and PR #244.

## Cross-layer completion dependency

`0x1401B8DC0` receives only one u32 registry-relative context and cannot validate raw I/O status. Therefore the exact L1 terminal result must be converted into an allowed/suppressed L3 completion before normal state2 publication.

Do not solve this by relabeling all scheduler code L1. Equally, do not hide L1 byte-terminal gaps by labeling all FileSlot/AsyncIO work L3.

## Strong boundaries not to restart generically

### L2

- numbered-volume bootstrap/first-gap structure;
- basename candidate construction;
- archive/physical provider order;
- archive normalization/index/search;
- bounded type-0 static final-open chain.

### L1

- ZIP method-0 vs raw-DEFLATE core architecture;
- PAC/PNST sparse relative-slot structure;
- recursive PAC/PNST traversal;
- `.lst` grammar and 64-byte placement structure.

These strong structures do not close the newly explicit size/capacity/error/padding gaps.

### L3

- LoadedResource registry `363 x 0x48` and seven groups;
- normal `state1 -> state2` callback ABI/state publication;
- state2 typed-finalizer -> optional callback -> state3;
- cancellation `1|2 -> 4`;
- quiescence `{0,3}`;
- state4 cleanup;
- ordinary/group/full release/reset distinction;
- representative typed families;
- bounded loader claim/release model;
- runtime vs CRT vs process-lifetime distinction.

## Cross-boundary rules

- Identity selection is L2; exact selected-byte representation is L1.
- `.lst` representation choice is L1, not L2.
- Byte-count/EOF/result semantics may be L1 even when the owning request object is L3.
- Queue/poll/callback/state-publication semantics remain L3 even when they must wait on an L1 terminal result.
- `state1 -> state2` is L3; `0x1401B8CA0` materializer result is the L1 original-byte cut.
- A shared helper may contain behaviors from multiple layers.
- Stage Ops consumes L1/L2/L3 authority and is not one of the resource-runtime layers.
- Product hardening is not original-game equivalence.
- Writer compatibility is not Capcom offline-writer equivalence.

## Current priority accounting

```text
L1 -> byte-exactness G1..G8 + terminal-result/seam reconciliation + real retail/edit/rebuild/Level-E
L2 -> retail collision evidence + protected mapping + trusted selected identity + final audit
L3 -> residual static lifecycle ownership + completion-scheduler seam support + V1..V7 receipts
```

**L1, L2 and L3 are all INCOMPLETE / NOT 100%.** Progress in one layer must never be reported as completion of another.