# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-25  
**Base:** `main@8e67235fd26cf7af94146f4dc660eb49e3c1d133`

This is the execution roadmap for GDSpaces as one resource-runtime program. It supersedes the idea that L1, L2 and L3 must be completed in strict numeric order.

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

The critical path is dependency-driven, not layer-number-driven.

A task from another layer is allowed when it closes the current acceptance gap. Every task must record:

1. primary layer;
2. acceptance gap;
3. dependency on other layers;
4. why now;
5. return condition to the vertical critical path.

Do not start broad L2/L3 feature work merely because it is interesting. Do not block required L2/L3 reverse merely because L1 is still open.

## Current vertical acceptance target

The first subsystem-wide proof target is one representative retail resource observed end to end:

```text
real protected DMC3 installation
 -> [L2] game request and exact selected provider/volume/member
 -> [L1] artifact-stable exact materialized bytes
 -> [L1] bounded edit/rebuild/repack
 -> [L2] overlay/new-volume selection of authored identity
 -> [L1] rematerialized authored bytes
 -> [L3] original typed post-load / state-3 consumer visibility
 -> observable original-game effect
 -> release/transition receipt
```

A crash-free launch is not sufficient. The receipt must bind EXE identity, selected ResourceRef, archive/member identity, materialized hash, authored hash and consumer/lifecycle observation.

## Track A — L1 closure

Current strong areas include NBZ STORE/raw-DEFLATE materialization, artifact-bound acquisition, PAC/PNST relative-slot parsing/expansion, bounded reflow/rebuild, atomic no-replace publication, NBZ copy rebuild, protected-retail authoring orchestration, nested composition and texture runtime-relocation compatibility. PR #212 is active receipt-bundle hardening; PR #210 is active recursive slot-path hardening. Both remain branch truth until merged.

Remaining L1 closure sequence:

```text
closure/acquisition receipt binding (#212)
 -> recursive slot-path product hardening (#210)
 -> direct-retail provenance
 -> retail representation classification
 -> representative real edit/rebuild
 -> verified overlay/new-volume artifact
 -> canonical rematerialization
 -> original-game consumption
 -> final L1 audit
```

L1 can request L2 work whenever exact selected identity is needed and L3 work whenever consumer/runtime acceptance is required.

## Track B — L2 closure

Canonical audit: `l2-audit-2026-08-24.md`.

Current work order:

```text
L2-R1 exact type-0 physical provider after 0x0C (#204)
 -> physical-provider model/receipt
 -> real-retail 0x0E collision census
 -> direct-retail resolver receipt
 -> OpenGameResource caller/fallback census
 -> 0x400 oversized-candidate behavior where relevant
 -> controlled physical/missing/fallback receipts
 -> original-process selected-identity receipt
 -> final L2 audit
```

The type-0 final-open reverse is evidence-blocked until the canonical raw EXE/callee chain is reacquired. Non-blocked validation/census tooling may proceed in parallel.

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

L3 implementation in product code is not the goal by itself. The main target is executable-backed lifecycle truth and validation of consumer readiness/ownership.

## Cross-layer dependency matrix

| Acceptance question | Primary | Required support |
|---|---|---|
| Which resource wins for `obj\\em000.pac`? | L2 | real retail corpus / V |
| Are the selected bytes exact? | L1 | L2 selected identity + V hash binding |
| Can the resource be edited/rebuilt losslessly? | L1 | representation evidence |
| Will the authored overlay win? | L2 | L1 built artifact |
| Did the game actually consume authored bytes? | L3 + V | L2 winner + L1 byte identity |
| Was it retained/released correctly? | L3 | same L1/L2 identity chain |

## Current priority queue

1. Keep #212 and #210 whole-head Ubuntu/Windows CI-green and promote only after review.
2. Obtain the first direct-retail resolver + artifact-bound materialization receipt.
3. Classify that exact representation and execute the representative edit/rebuild/overlay/rematerialization chain.
4. Complete #209 with deterministic original-process consumer and rollback evidence.
5. Continue #204 evidence reacquisition and real-retail L2 collision census where they support the vertical receipt.
6. Continue the L3 state-writer/field-writer census needed to instrument consumer readiness.
7. Expand format/resource breadth only after the first vertical receipt is deterministic.

## Completion rule

No percentage can mark a layer or GDSpaces complete. Completion requires mandatory gates, canonical code/docs, exact-head Windows+Ubuntu validation where applicable, representative real-corpus/original-process receipts, and no unresolved contradiction that changes the claimed architecture.

After L1/L2/L3 audits are accepted, percentage estimates may be recalculated strictly as planning indicators.
