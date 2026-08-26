# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-26  
**Base:** `main@c147facb310d32ef084c56ba82d1e4b6b9b1b496`  
**Active L2 evidence slice:** PR #219

This is the execution roadmap for GDSpaces as one resource-runtime program. L1/L2/L3 are separate ownership layers, but execution follows dependencies rather than strict numeric order.

## Layers

### L1 — Resource Materialization

```text
physical/container bytes
 -> exact acquisition
 -> transform/decompression
 -> nested expansion
 -> editable child identity
 -> rebuild/repack
 -> reopen/rematerialize
```

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
selected/materialized bytes
 -> FileSlot/async completion
 -> LoadedResource states
 -> typed post-load
 -> ready visibility
 -> claims/cache
 -> reset/release/shutdown
```

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
 -> [L3] original consumer visibility
 -> observable effect attributable to authored bytes
 -> rollback / transition receipt
```

A crash-free launch is not sufficient.

## Track A — L1 final acceptance

Canonical pre-Level-E audit: `l1-final-audit-2026-08-25.md`.

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

## Track B — L2 closure

Canonical review baseline: `l2-review-2026-08-25.md` plus the 2026-08-26 protected-runtime mapping slice.

### Closed L2 internal slice

PR #215 / issue #204 closed the type-0 physical-provider post-`0x0C` boundary:

- exact static root-join/open/existence/miss semantics recovered from canonical analysis executable;
- native physical-path product capability separated from archive index semantics;
- product evidence classes remain distinct from recovered/original authority;
- controlled physical hit, complete miss and archive→physical fallback receipts integrated;
- Windows bounded `CreateFileA` parity scenario + Ubuntu/Windows exact-head validation passed;
- direct caller census and `0x400` overflow behavior are closed for the recovered direct-call surface.

Do not reopen this slice absent contradictory direct evidence.

### Current L2 work order

```text
L2-R2A real-retail 0x0E collision census
  -> direct-retail exact resolver identity receipt

L2-R2B protected-distribution runtime RVA mapping (#219)
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

### L2-R2B — protected runtime address mapping

PR #219 adds the acquisition seam, not the original-process proof itself:

- explicit PID;
- runtime read by `module_base + RVA`, never copied static VA;
- exact running image SHA + size gate;
- PE `SizeOfImage` and section containment checks;
- exact full-range `ReadProcessMemory` only;
- metadata-only child receipt by default;
- optional canonical artifact/window hash expectation;
- mismatch is negative evidence and non-zero;
- multi-anchor mapping validator requires one OpenGameResource anchor plus at least two independent type-0 physical anchors from one process/module session.

A child receipt proves only its listed range. A successful multi-anchor packet proves bounded mapping only for the listed anchors. It does **not** prove global build equivalence or original selected-provider identity.

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

For the first L1 vertical proof, L3 need only provide enough original-process evidence to attribute the consumer-visible result to the authored resource. Broader lifecycle closure remains a separate L3 program.

## Cross-layer dependency matrix

| Acceptance question | Primary | Required support |
|---|---|---|
| Which resource wins for a real game request? | L2 | protected retail corpus + mapped original-process observation |
| Are selected bytes exact? | L1 | L2 selected identity + artifact binding |
| Can the selected representation be edited safely? | L1 | direct retail representation evidence |
| Will the authored overlay win? | L2 | L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | L2 authored winner |
| Did original DMC3 consume those bytes? | L3 + validation | same L1/L2 identity chain |
| Was the test rolled back without retail mutation? | validation | exact artifact identity |

## Current priority queue

1. Keep the L1 real-retail/Level-E acceptance path ready; do not replace it with synthetic work.
2. Complete PR #219 acquisition/tooling review and exact-head CI.
3. When a protected original process is available, produce the L2 multi-anchor mapping packet.
4. Acquire a cryptographically bound DMC3 retail member-list/central-directory surface and run the `0x0E` census.
5. Capture original-process selected identity only after mapped L2 anchors are proven.
6. Reconcile final L2 evidence and run the final Layer-2 audit.

## Completion rule

No percentage alone can mark a layer complete. Completion requires mandatory gates, canonical code/docs, exact-head Windows+Ubuntu validation where applicable, representative real-corpus/original-process receipts and no unresolved contradiction changing the declared scope.

Percentages may be recalculated only as planning indicators after gate reconciliation.
