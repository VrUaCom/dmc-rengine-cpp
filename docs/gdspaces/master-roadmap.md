# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-26  
**Base:** `main@c20544cfb7f3ddba69a128a88246550a35eb51c1`  
**Latest L2 tooling promotion:** PR #219 — protected-runtime RVA mapping acquisition seam  
**Canonical L1 EXE review:** `l1-exe-boundary-review-2026-08-26.md`  
**Focused L1 completion pass:** `l1-exe-materialization-completion-pass-2026-08-26.md`

This is the execution roadmap for GDSpaces as one resource-runtime program. L1/L2/L3 are separate ownership/review layers, but execution follows dependencies rather than strict numeric order.

## Layers

### L1 — Resource Materialization

```text
selected physical/container identity
 -> whole-file/FileSlot byte transport
 -> exact member/span acquisition
 -> STORE/raw-DEFLATE transform
 -> caller-owned materialized bytes
 -> packed representation OR .lst synthesis
 -> nested expansion
 -> editable child identity
 -> rebuild/repack
 -> reopen/rematerialize
 -> resource-level completion ordering / dependency barrier
 -> state 1 -> 2 handoff
```

`FileSlot` and AsyncIO participate in L1 where they transport the selected bytes. Their wider pool ownership, cancellation/release/reset and runtime-lifecycle policy are L3.

The completion-ordering review explicitly does **not** promote a generic child/outstanding-work fan-in counter. Scheduler-mediated ordering is evidenced; the exact success-side dependency mechanism is still open.

### L2 — Resource Resolution

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume selection
 -> ambiguity/fallback
 -> exact ResourceRef identity
```

### L3 — Original Runtime / Lifecycle

```text
materialized-byte state2 input
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3 / ready visibility
 -> claims/cache/family ownership
 -> cancellation/reset/release/shutdown
```

Cross-layer constructors/schedulers are classified by behavior, not wholesale by function address. `0x1401B84E0` is the canonical example: allocation/materialization start is L1, while state/scheduler/lifecycle behavior reaches the L1/L3 boundary.

Validation is cross-cutting and is not a fourth layer.

## Execution rule

A task from another layer is allowed when it closes the current acceptance gap. Every task must record its primary layer, dependency and return condition to the vertical critical path.

Do not start broad L2/L3 work merely because it is interesting. Do not block required L2/L3 evidence because L1 is still open.

## Current vertical acceptance target

```text
real protected DMC3 installation
 -> [L2] exact selected provider/volume/member
 -> [L1] artifact-bound exact materialized bytes
 -> [L1] supported top-level or nested edit/rebuild
 -> [L2] authored next-volume winner
 -> [L1] exact authored rematerialization
 -> [L3] original typed-ready/consumer visibility
 -> observable effect attributable to authored bytes
 -> rollback / transition receipt
```

A crash-free launch is not sufficient.

## Track A — L1 final acceptance

Canonical pre-Level-E audit: `l1-final-audit-2026-08-25.md`.  
Canonical EXE boundary review: `l1-exe-boundary-review-2026-08-26.md`.  
Focused completion-ordering pass: `l1-exe-materialization-completion-pass-2026-08-26.md`.

**Internal product implementation status:** CLOSED for the current representative DMC3-HD acceptance scope.

Promoted capabilities include artifact-bound retail acquisition, atomic no-replace publication, STORE/raw-DEFLATE materialization, PAC/PNST sparse/alias-preserving expansion, size-changing relative-slot reflow, nested root-to-leaf slot-path authoring, verified NBZ rebuild, next-volume overlay authoring and protected retail closure orchestration.

Remaining mandatory L1 sequence is evidence execution:

```text
direct-retail provenance receipt
 -> exact retail representation classification
 -> one supported real edit/rebuild/rematerialization receipt
 -> #209 original-game consumption + rollback
 -> final L1 cross-stack audit
 -> L1 COMPLETE / 100%
```

No new synthetic-only feature may displace this sequence unless real retail evidence reveals a concrete missing dependency.

### Supporting L1 EXE reverse while Level-E is externally blocked

Do not restart strong ZIP/FileSlot architecture. Current priority is the weak completion-ordering seam:

```text
materialization completion ordering / dependency barrier
 -> scheduler pending-entry rollback semantics
 -> transport error -> resource scheduler/materialization error mapping
 -> .lst child completion/failure + temporary-buffer cleanup
 -> acceptance-activated FileSlot partial-read/cancellation breadth
 -> acceptance-activated ZIP exact-body/error breadth
```

Correct labels:

- `0x1402EF4D0` = resource materialization submission/scheduling wrapper;
- `0x1400335A0` = transport/whole-file completion callback;
- `0x1401B8DC0` = resource-level scheduler/materialization completion handoff to state2, not raw I/O callback;
- `0x1402EF580` = scheduler-ring enqueue;
- `0x1402EF790` = scheduler worker/callback execution;
- `0x1402EF460` = pending scheduled-entry clear/rollback, not OS AsyncIO cancellation.

Preferred next static packet: `data/reverse/dmc3-l1-materialization-completion-plan.v1.json`.

## Track B — L2 closure

Canonical review baseline: `l2-review-2026-08-25.md` plus the protected-runtime mapping tooling promoted by #219.

### Closed L2 internal slices

PR #215 / issue #204 closed the type-0 physical-provider post-`0x0C` boundary:

- exact static root-join/open/existence/miss semantics recovered from canonical analysis executable;
- native physical-path product capability separated from archive index semantics;
- product evidence classes remain distinct from recovered/original authority;
- controlled physical hit, complete miss and archive→physical fallback receipts integrated;
- Windows bounded `CreateFileA` parity scenario + Ubuntu/Windows exact-head validation passed;
- direct caller census and `0x400` overflow behavior are closed for the recovered direct-call surface.

PR #219 promoted the protected-runtime RVA mapping **acquisition/tooling seam**:

- explicit PID and module-base + RVA reads;
- protected image SHA/size gating;
- PE image/section/range validation;
- exact full-range `ReadProcessMemory` behavior;
- metadata-only receipts by default;
- multi-anchor mapping receipt validation.

Neither promotion is an original-process selected-provider receipt. Real process evidence remains mandatory.

### Current L2 work order

```text
L2-R2A real-retail 0x0E collision census
  -> direct-retail exact resolver identity receipt

L2-R2B real protected-process runtime mapping receipt using merged #219 tooling
  -> multi-anchor bounded mapping receipt from one protected process
  -> L2-R3 original-process selected-provider identity receipt

[R2A + R3]
  -> docs/issues/evidence reconciliation
  -> exact-head Windows + Ubuntu validation
  -> final L2 audit / promotion
```

R2A and R2B are independent evidence branches and may proceed in parallel when artifacts permit. Neither may be weakened to unblock the other.

### L2 authority split

Canonical instruction-reverse executable:

- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size 6,356,432;
- preferred image base `0x140000000`.

Protected distribution/original execution candidate:

- SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`;
- size 6,567,320;
- not instruction-level reverse authority.

Canonical analysis VAs/RVAs must not be applied to the protected process without independent runtime mapping.

### L2-R2A — retail collision gate

The exact DMC3 retail `dmc3-0.nbz` is too large for the current connected Drive transfer channel and no exact cryptographically bound central-directory/member-list derivative is currently available.

Required closure:

1. obtain exact member-name/central-directory evidence bound to the retail archive;
2. run canonical `0x0E` normalized-key census;
3. preserve collision count and exact colliding identities if any;
4. only then promote a real-retail resolver selection receipt.

Synthetic/DMCL corpus results do not close this DMC3 retail gate.

### L2-R2B — protected runtime address mapping receipt

Merged #219 tooling is the acquisition seam, not the original-process proof itself.

A valid real packet must preserve one protected process/module session and satisfy the approved multi-anchor mapping contract. A child receipt proves only its listed range. A successful multi-anchor packet proves bounded mapping only for the listed anchors. It does **not** prove global build equivalence or original selected-provider identity.

### L2-R3 — original-process selected identity

Only after a valid protected-runtime mapping packet exists:

1. instrument mapped resolver entry/selection points;
2. capture exact logical request and ordered candidate/provider traversal;
3. capture final provider/source/archive/member identity selected by original DMC3;
4. bind the receipt to the exact protected executable and real retail corpus;
5. compare to GDSpaces product resolver without relabeling product evidence as original-process evidence.

`preflight-dmc3-game-test` is build/archive-presence preflight, not a selected-identity receipt.

## Track C — L3 closure

Canonical audit: `l3-audit-2026-08-25.md`.

Current work order:

```text
state-writer/caller census
 -> known-field write ownership/order
 -> typed-dispatch breadth/failure semantics
 -> shared-owner coordination by family
 -> initial-load Level-E receipt
 -> transition/restart/cancellation receipts
 -> menu/full-reset receipt
 -> shutdown receipt
 -> family/build breadth
 -> final L3 audit
```

For the first L1 vertical proof, L3 need only provide enough original-process evidence to attribute the consumer-visible result to the authored resource after L1 state2/materialized-byte completion. Broader lifecycle closure remains a separate L3 program.

The scheduler rollback mechanics around unfinished L1 materialization are valid cross-boundary evidence; wider cancellation/reset/release policy remains L3.

## Cross-layer dependency matrix

| Acceptance question | Primary | Required support |
|---|---|---|
| Which resource wins for a real game request? | L2 | protected retail corpus + mapped original-process observation |
| Are selected bytes exact? | L1 | L2 selected identity + artifact binding |
| Can the selected representation be edited safely? | L1 | direct retail representation evidence |
| Will the authored overlay win? | L2 | L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | L2 authored winner |
| Did byte transport/materialization complete? | L1 | FileSlot transport + scheduler-mediated completion-ordering receipt |
| Did original DMC3 consume those bytes? | L3 + validation | same L1/L2 identity chain |
| Was the test rolled back without retail mutation? | validation | exact artifact identity |

## Current priority queue

1. Keep the L1 real-retail/Level-E acceptance path ready; do not replace it with synthetic work.
2. While external acceptance is unavailable, run/reverse the focused L1 completion-ordering packet without assuming a fan-in counter.
3. Use merged #219 tooling to produce a real protected-process multi-anchor mapping packet when the process is available.
4. Acquire a cryptographically bound DMC3 retail member-list/central-directory surface and run the `0x0E` census.
5. Capture original-process selected identity only after mapped L2 anchors are proven.
6. Reconcile final L2 evidence and run the final Layer-2 audit.

## Completion rule

No percentage alone can mark a layer complete. Completion requires mandatory gates, canonical code/docs, exact-head Windows+Ubuntu validation where applicable, representative real-corpus/original-process receipts and no unresolved contradiction changing the declared scope.

Percentages may be recalculated only as planning indicators after gate reconciliation.
