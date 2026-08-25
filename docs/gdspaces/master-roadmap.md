# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-25  
**Base:** `main@fd80f2b63c0a9920230d3e74b1debafc07e240b1`

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

The exact type-0 final-open reverse remains evidence-gated. It does not block the representative L1 path unless that path actually depends on the unresolved physical provider.

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
| Which resource wins for a real game request? | L2 | protected retail corpus / validation |
| Are selected bytes exact? | L1 | L2 selected identity + artifact binding |
| Can the selected representation be edited safely? | L1 | direct retail representation evidence |
| Will the authored overlay win? | L2 | L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | L2 authored winner |
| Did original DMC3 consume those bytes? | L3 + validation | same L1/L2 identity chain |
| Was the test rolled back without retail mutation? | validation | exact artifact identity |

## Current priority queue

1. Obtain the first real direct-retail acquisition/provenance receipt.
2. Classify the exact observed representation.
3. Use existing L1 writers for one supported real edit/rebuild/rematerialization receipt.
4. Execute #209 original-game consumption + rollback.
5. Run the final L1 audit and only then mark L1 complete.
6. Continue L2/L3 breadth after or when directly required by the vertical receipt.

## Completion rule

No percentage alone can mark a layer complete. Completion requires mandatory gates, canonical code/docs, exact-head Windows+Ubuntu validation where applicable, representative real-corpus/original-process receipts and no unresolved contradiction changing the declared scope.

Percentages may be recalculated only as planning indicators after gate reconciliation.
