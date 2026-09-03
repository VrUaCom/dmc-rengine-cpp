# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-28  
**Base:** `main@94692e8f9971cf8249b4b16ee88d309de8b49f11`  
**L1 status:** **INCOMPLETE / NOT 100% — active original-materialization reverse + real acceptance open**  
**Latest L1 layout promotion:** #255  
**Latest L1 raw checkpoints:** `l1-writer-failure-width-reconciliation-2026-08-28.md`, `l1-terminal-l3-completion-seam-2026-08-28.md`  
**L2:** incomplete; advanced static/tooling, real selected-identity evidence open  
**L3:** incomplete; strong static spine, dynamic lifecycle/original-process evidence open

This is the execution roadmap for GDSpaces as one resource-runtime program. L1/L2/L3 are separate ownership layers, but execution follows dependencies rather than strict numeric order.

The previous master-roadmap wording that L1's internal path was closed is superseded by fresh canonical-EXE reverse. Product implementation is advanced; original L1 materialization semantics and real acceptance are both still open.

## Layers

### L1 — Resource Materialization

```text
[L2 already selected identity]
 -> cached/logical/materialized size
 -> capacity/allocation
 -> exact acquisition / transfer / decompression
 -> exact destination bytes
 -> packed/.lst/nested representation construction
 -> bounded authoring + rebuild/repack
 -> reopen/rematerialize
 -> native terminal byte/result authority
```

### L2 — Resource Resolution

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume/member selection
 -> ambiguity/fallback
 -> exact selected ResourceRef/provider identity
```

### L3 — Original Runtime / Lifecycle

```text
L1 native terminal byte/result state
 -> FIFO callback eligibility / request-callback lifetime
 -> normal LoadedResource publication
 -> typed post-load
 -> ready visibility
 -> claims/cache
 -> cancellation/reset/release/shutdown
```

**Stage Assembly / Stage Ops is not L3.** It is a downstream domain/tooling consumer of the three-layer resource authority.

Validation is cross-cutting and is not a fourth layer.

## Execution rule

A task from another layer is allowed only when it closes the current acceptance dependency or its own explicitly scheduled gate. Every pass must record its primary layer and avoid authority drift.

Do not start broad work merely because it is interesting. Do not block required cross-layer evidence because another layer is still open. Do not move lifecycle ownership into L1 merely because an L1 byte path uses a shared queue/helper.

## Current vertical acceptance target

```text
real protected DMC3 installation
 -> [L2] exact selected provider/volume/member
 -> [L1] artifact-bound exact materialized bytes
 -> [L1] supported representation + bounded edit/rebuild
 -> [L2] authored next-volume winner
 -> [L1] exact authored rematerialization
 -> [bounded L1/L3 seam] native terminal result permits admitted normal callback to become eligible
 -> [L3] original lifecycle reaches consumer-ready visibility
 -> observable effect attributable to authored bytes
 -> rollback / transition receipt
```

A crash-free launch is not sufficient.

## Track A — L1 reverse + final acceptance

Canonical L1 roadmap: `l1-roadmap.md`.  
Current raw checkpoints: `l1-writer-failure-width-reconciliation-2026-08-28.md` and `l1-terminal-l3-completion-seam-2026-08-28.md`.

**Status: INCOMPLETE / active.**

### Product capability state

Promoted capabilities include:

- artifact-bound NBZ/member acquisition;
- STORE/raw-DEFLATE materialization;
- ByteProvenance after successful member validation (#250);
- atomic no-replace publication;
- PAC/PNST sparse/alias-preserving expansion;
- same-size, size-changing and nested relative-slot authoring;
- verified NBZ rebuild and next-volume overlay authoring;
- protected build preflight / product closure tooling;
- runtime-synth `.lst` layout corrected to original direct `0x800` transfer extents vs recursively synthesized `0x40` complete-image structural extents;
- original zero-filled runtime-synth padding (#255);
- Ubuntu + Windows CI on promoted paths.

These are product capabilities, not an L1 completion certificate.

### Fresh L1 reverse result

The canonical EXE pass now confirms/corrects:

- FileSlot cached size source for physical and NBZ entries;
- rounded whole-file transfer request vs exact terminal produced bytes;
- lower EOF/short-read behavior;
- `.lst` planner/writer direct-vs-recursive extent semantics;
- original synthesized-image zero initialization;
- `0x1402EF4D0` as queue admission and `0x1402EF790` as consumer;
- `0x1401B85C0` swallowing child enqueue/recursive writer failures;
- `0x1401B8CA0` branch-dependent boolean semantics;
- `0x1401B84E0` ignoring type-3 completion enqueue failure;
- 32-bit wrap-prone chunk/planner arithmetic;
- scanner/token ceilings as bounds rather than clean original error enums;
- admitted type-2 materialization jobs and the admitted normal type-3 callback share one per-lane FIFO;
- whole-file status `2` leaves current materialization pending, status `4` retries the same current type-2 job without retirement, and status `3` retires it and advances the FIFO;
- original status `3` can be published after fewer actual bytes than originally planned, so short-success can make normal completion eligible;
- cancellation `0x1401B8430 -> 0x1402EF460` flushes queued work and moves LoadedResource `1/2 -> 4`, establishing a queued-work suppression path owned by L3.

This means the old model “materializer boolean == exact byte success” is not valid globally, while the static normal FIFO gate is now bounded well enough to separate the native L1 terminal result from L3 publication.

### Bounded static L1/L3 seam — CLOSED

For **successfully admitted** canonical normal-path jobs:

```text
current type-2 materialization
 -> status 2: pending, no FIFO advance
 -> status 4: phase reset/retry same current job, no FIFO advance
 -> status 3: close/clear/retire type-2, advance consumer index
 ===== native L1 byte/result terminal =====
 -> admitted later type-3 callback can become current
 -> 0x1401B8DC0 publishes LoadedResource state 1 -> 2
 ===== L3 lifecycle =====
```

Important qualifications:

- this FIFO guarantee does not retroactively prove that every expected child/completion job was admitted; upstream original code can swallow enqueue failure;
- status `3` is the original success criterion and does not independently prove actual-bytes == planned-bytes;
- cancellation can flush queued-but-not-current work and publish state `4` instead;
- exact concurrent/current-slot cancellation races remain L3 dynamic scope.

Static seam evidence: `l1-terminal-l3-completion-seam-2026-08-28.md`.

### L1 reverse work order

```text
A-R1 finish residual recursive .lst cycle/depth/allocation/free lifetime + allocator/backend failure branches
 -> A-R2 final original-L1 contradiction sweep
```

Dynamic current-slot cancellation/concurrency, transitions, reset and shutdown remain L3. They are not an L1 reverse blocker unless a concrete L1 acceptance receipt activates them.

### L1 real acceptance sequence

After the reverse contradiction boundary is clean:

```text
direct-retail provenance receipt
 -> exact retail representation classification
 -> one supported real edit/rebuild/rematerialization receipt
 -> #209 original-game consumption + rollback
 -> final L1 cross-stack audit
 -> L1 COMPLETE / 100%
```

No new synthetic-only feature may displace this sequence unless direct evidence reveals a concrete missing dependency.

### Product safety rule

Unsafe original behavior is evidence, not a product requirement:

| Original | Product |
| --- | --- |
| 32-bit overflow/wrap | checked overflow / fail closed |
| child enqueue failure may be swallowed | successful receipt must not launder rejected work |
| completion enqueue failure may be ignored | completion authority stays explicit |
| malformed scanner/token boundary lacks clean error enum | explicit fail-closed diagnostics |
| short status-3 transfer can permit normal completion | exactness validation when product receipt claims exact bytes |

## Track B — L2 closure

**Status: INCOMPLETE.**

Canonical review baseline remains the L2 review/reconciliation documents. Existing static/provider and runtime-mapping tooling work should not be reopened without contradictory evidence.

### Strong/integrated L2 slices

- type-0 physical-provider static reverse and native physical product path;
- numbered-volume bootstrap/precedence model;
- bounded protected-runtime RVA/window acquisition and multi-anchor mapping tooling;
- selected-identity content-candidate normalization/artifact-binding tooling.

### Current L2 work order

```text
L2-R2A real-retail normalized-key/member collision census

L2-R2B real protected-process multi-anchor mapping receipt

L2-R3 trusted original-process selected-provider identity
  -> mapped anchors
  -> trusted publisher/origin binding
  -> zero-loss selected identity receipt

[R2A + real R2B + trusted R3]
  -> contradiction/docs/evidence reconciliation
  -> exact-head validation
  -> final L2 audit
```

R2A and R2B are independent evidence branches and may proceed in parallel when artifacts permit.

### L2 authority split

Canonical instruction-reverse executable:

- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size 6,356,432;
- preferred image base `0x140000000`.

A protected distribution/original execution build is a separate authority and must not inherit canonical VAs/RVAs without independent runtime mapping.

Provider/backend failure must remain distinct from a clean lookup miss.

## Track C — L3 closure

**Status: INCOMPLETE.**

The static LoadedResource / typed-ready / release/reset spine is strong. Dynamic lifecycle breadth, cancellation/transition/reset/shutdown receipts and original-process Level-E evidence remain open.

### Strong static L3 boundaries

Current evidence strongly supports:

- LoadedResource registry topology and bounded family organization;
- central lifecycle state progression and cancellation/reset classes;
- typed post-load dispatch for representative formats;
- release/reset paths and bounded alias ownership;
- canonical normal-completion callback `0x1401B8DC0` registered through the queue/lifecycle path;
- separation between direct-base false positives and actual LoadedResource state authority.

Do not restart already bounded static areas absent contradictory evidence.

### L1/L3 seam — corrected and bounded

Older shorthand:

```text
0x1401B8CA0 true -> L3 normal completion
```

is false as a general statement.

Fresh reverse proves:

- packed `0x1401B8CA0` branch can directly propagate type-2 enqueue admission;
- loose branch inherits `0x1401B85C0`, which can swallow child enqueue/recursive failure;
- another branch can force success after a failed enqueue;
- `0x1401B84E0` can return success despite failed type-3 completion enqueue;
- admitted materialization/completion jobs use the same lane/FIFO;
- status `2` blocks later FIFO work as pending;
- status `4` retries the current type-2 job and still blocks later FIFO work;
- status `3` retires the current type-2 job, after which a later admitted normal callback can become current;
- status `3` itself does not prove exact planned length;
- cancellation can remove queued normal work and publish state `4` instead.

Correct seam:

```text
[L1]
representation/planner accepted
 -> queue-admission attempts
 -> admitted type-2 consumer execution
 -> native status-3 byte/result terminal
===== END L1 BYTE/RESULT AUTHORITY =====

[L3]
FIFO reaches admitted type-3 normal callback
 -> 0x1401B8DC0 state 1 -> 2
 -> typed post-load / ready lifecycle
```

The bounded static seam is **closed** for this scope. Dynamic current-slot cancellation/concurrency remains L3 and must not be pulled back into L1.

### L3 dynamic work order

```text
V1 initial load
 -> V2 room/stage transition
 -> V3 restart/reload
 -> V5 in-flight cancellation
 -> V4 return-to-menu/full reset
 -> V6 shutdown
 -> V7 family/build breadth
 -> final L3 audit
```

For the first L1 vertical proof, L3 only needs enough original-process evidence to attribute the consumer-visible result to the exact authored resource. Broader L3 completion remains independent.

## Cross-layer dependency matrix

| Acceptance question | Primary | Required support |
|---|---|---|
| Which resource wins for a real request? | L2 | real retail/protected evidence |
| Are selected bytes exact? | L1 | L2 selected identity + artifact binding |
| Is original materialization result semantics understood? | L1 | bounded static queue/completion seam evidence now recorded |
| Can selected representation be edited safely? | L1 | representation-specific authority |
| Will authored overlay win? | L2 | L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | L2 authored winner |
| Did original DMC3 consume those bytes? | L3 + validation | same L1/L2 identity chain |
| Was rollback clean? | validation | exact artifact identities |

## Current priority queue

1. Finish residual recursive `.lst` cycle/depth/allocation/free lifetime and allocator/backend failure branches.
2. Run final original-L1 contradiction sweep.
3. Execute representative real-retail L1 acquisition/classification/edit/rebuild/rematerialization.
4. Execute #209 original-game consumption + rollback.
5. Run final L1 audit.
6. Continue L2 real collision/mapping/trusted-selection evidence as dependencies/artifacts permit.
7. Continue L3 dynamic receipts without reopening already strong static boundaries or the bounded static L1/L3 seam.
8. Reconcile final L2/L3 audits independently.

## Completion rule

No percentage alone can mark a layer complete. Completion requires mandatory gates, canonical code/docs, exact-head Windows+Ubuntu validation where applicable, representative real-corpus/original-process receipts and no unresolved contradiction changing the declared scope.

**Current canonical statuses:**

- **L1: INCOMPLETE / NOT 100%**
- **L2: INCOMPLETE**
- **L3: INCOMPLETE**

Percentages may be used only as planning indicators after gate reconciliation.
