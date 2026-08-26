# GDSpaces Master Roadmap — L1 / L2 / L3 + V

**Snapshot:** 2026-08-26  
**Base:** `main@c20544cfb7f3ddba69a128a88246550a35eb51c1`  
**Validation architecture:** `validation-equivalence-architecture.md`  
**Validation roadmap:** `validation-roadmap.md`  
**Active original-process evidence slices:** PR #218 (L3 trace contract), PR #221 (L2 selected-identity contract)

This is the execution roadmap for GDSpaces as one resource-runtime program. L1/L2/L3 are separate ownership layers. **V is the single cross-cutting Validation / Equivalence authority. LV is V-owned live/original-process evidence acquisition, not a fourth decompilation layer.** Execution follows dependencies rather than strict numeric order.

## Architecture

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

### LV — Live Validation / Original-Process Observation

```text
exact protected process
 -> runtime mapping / trusted instrumentation
 -> ordered original-process observations
 -> sanitized/hash-bound LV receipt
 -> V evidence input
```

LV acquires original-process evidence for L1/L2/L3 validation. LV cannot issue equivalence or completion verdicts.

### V — Validation / Equivalence

```text
L1 evidence ----\
L2 evidence -----+-> evidence integrity
L3 evidence ----/    + original-process binding
LV receipts --------> + cross-layer identity binding
                       + contradiction checks
                       + scope-aware verdict
                       -> promotion / COMPLETE
```

V is not L4. V owns validation/promotion authority across all three layers.

## Core authority rule

L1/L2/L3 may report implementation/reverse states such as `implemented`, `bounded-closed`, `real-receipt-required` or `evidence-submitted`.

Only V may promote a declared scope to:

- `original-equivalent`;
- `game-ready-equivalent`;
- `COMPLETE`;
- `100%` when used as a completion claim.

Green CI, valid JSON, static disassembly, real-corpus parsing and crash-free execution are not interchangeable evidence classes.

## Execution rule

A task from another layer is allowed when it closes the current acceptance gap. Every task must record its primary layer, dependency and return condition to the vertical critical path.

Validation work must additionally record:

1. evidence class (`V-A` provenance, `V-B` product, `V-C` corpus, `V-D` original-process, `V-E` breadth/subsystem);
2. whether LV acquisition is required;
3. exact artifact/process/run identity;
4. what verdict the evidence is allowed to support.

Do not start broad L2/L3 work merely because it is interesting. Do not block required L2/L3 evidence because L1 is still open. Do not let layer-local tests bypass V.

## Current vertical acceptance target

The first cross-layer V proof must be one resource chain from one bounded validation run:

```text
ONE validation_run_id
 -> exact protected DMC3 executable
 -> [LV/V-A] trusted process/mapping authority
 -> [L2] exact original selected provider/volume/member
 -> [L1] artifact-bound exact materialized bytes
 -> [L1] supported top-level or nested edit/rebuild
 -> [L2] authored next-volume winner
 -> [L1] exact authored rematerialization
 -> [L3] original typed-ready/consumer visibility
 -> observable effect attributable to authored bytes
 -> rollback / transition receipt
 -> [V] parent cross-layer verdict
```

Three independent layer PASS results are not one vertical proof. V must bind the same run/resource/byte chain.

A crash-free launch is not sufficient.

## Track A — L1 implementation and evidence

Canonical pre-Level-E audit: `l1-final-audit-2026-08-25.md`.

**Internal product implementation status:** CLOSED for the current representative DMC3-HD acceptance scope.

Promoted capabilities include artifact-bound retail acquisition, atomic no-replace publication, STORE/raw-DEFLATE materialization, PAC/PNST sparse/alias-preserving expansion, size-changing relative-slot reflow, nested root-to-leaf slot-path authoring, verified NBZ rebuild, next-volume overlay authoring and protected retail closure orchestration.

Remaining mandatory L1 evidence sequence:

```text
direct-retail provenance receipt
 -> exact retail representation classification
 -> one supported real edit/rebuild/rematerialization receipt
 -> #209 original-game consumption + rollback
 -> V:L1 acceptance
 -> final L1 cross-stack audit
```

Issue #209 is now treated as **V:L1 / V-D** rather than a separate validation architecture owned by L1.

No new synthetic-only feature may displace this sequence unless real retail evidence reveals a concrete missing dependency.

## Track B — L2 implementation and evidence

Canonical review baseline: `l2-review-2026-08-25.md` plus the 2026-08-26 protected-runtime mapping promotion.

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

LV-L2-R2B protected-process mapping
  -> #219 tooling already merged
  -> real multi-anchor bounded mapping receipt from one protected process
  -> #220/#221 original-process selected-provider observation

[R2A + original selection]
  -> V:L2 original-vs-GDSpaces comparison
  -> final L2 audit
```

R2A and LV-L2 are independent evidence branches and may proceed in parallel when artifacts permit. Neither may be weakened to unblock the other.

### L2 authority split

Canonical instruction-reverse executable:

- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- size 6,356,432;
- preferred image base `0x140000000`.

Protected distribution/original execution candidate:

- SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`;
- size 6,567,320;
- not instruction-level reverse authority.

Canonical analysis VAs/RVAs must not be applied to the protected process without V-accepted independent runtime mapping evidence.

### L2-R2A — retail collision gate

The exact DMC3 retail `dmc3-0.nbz` is too large for the current connected Drive transfer channel and no exact cryptographically bound central-directory/member-list derivative is currently available.

Required closure:

1. obtain exact member-name/central-directory evidence bound to the retail archive;
2. run canonical `0x0E` normalized-key census;
3. preserve collision count and exact colliding identities if any;
4. only then promote a real-retail resolver selection receipt.

Synthetic/DMCL corpus results do not close this DMC3 retail gate.

### LV-L2-R2B — protected runtime address mapping

PR #219 merged the acquisition seam, not the original-process proof itself:

- explicit PID;
- runtime read by `module_base + RVA`, never copied static VA;
- exact running image SHA + size gate;
- PE `SizeOfImage` and section containment checks;
- exact full-range `ReadProcessMemory` only;
- metadata-only child receipt by default;
- optional canonical artifact/window hash expectation;
- mismatch is negative evidence and non-zero;
- multi-anchor mapping validator requires one OpenGameResource anchor plus at least two independent type-0 physical anchors from one process/module session.

A child receipt proves only its listed range. A successful multi-anchor packet proves bounded mapping only for the listed anchors. It does not prove global build equivalence or original selected-provider identity.

### LV/V:L2 original-process selected identity

PR #221 defines the receipt contract/tooling boundary. Real promotion still requires:

1. a valid protected-runtime mapping packet;
2. trusted original-process resolver observation;
3. exact logical request and ordered candidate/provider traversal;
4. final provider/source/archive/member identity selected by original DMC3;
5. binding to the exact protected executable and retail corpus;
6. V comparison to GDSpaces product resolver without relabeling product evidence as original-process evidence.

`preflight-dmc3-game-test` is build/archive-presence preflight, not a selected-identity receipt.

## Track C — L3 implementation and evidence

Canonical audit: `l3-audit-2026-08-25.md`.

Current work order:

```text
state-writer/caller census
 -> known-field write ownership/order
 -> typed-dispatch breadth/failure semantics
 -> shared-owner coordination by family
 -> trusted LV-L3 observer/publisher
 -> initial-load original-process receipt
 -> transition/restart/cancellation receipts
 -> menu/full-reset receipt
 -> shutdown receipt
 -> family/build breadth
 -> V:L3 acceptance
 -> final L3 audit
```

Issue #217 and PR #218 are now treated as V-owned L3 validation contracts, with LV acquiring the original-process event stream.

For the first L1 vertical proof, L3 need only provide enough original-process evidence to attribute the consumer-visible result to the authored resource. Broader lifecycle closure remains a separate L3/V-E program.

## Track V — unified Validation / Equivalence

Canonical architecture: `validation-equivalence-architecture.md`.

Canonical execution roadmap: `validation-roadmap.md`.

### V evidence classes

- **V-A:** artifact/provenance/receipt/trusted-origin integrity;
- **V-B:** product CI/tests/controlled contracts;
- **V-C:** exact real-corpus validation;
- **V-D:** trusted original-process equivalence;
- **V-E:** breadth and subsystem acceptance.

### Immediate V work order

```text
V-1 architecture/roadmap integration
 -> V-2 machine/current status ownership update
 -> V-3 parent V execution ledger
 -> V-4 validation_run_id + cross-receipt binding
 -> V-5 reconcile #209/#220/#217 under V
 -> V-6 trusted LV publisher/binder contract
 -> V-7 first accepted LV-L2 observation
 -> V-8 first accepted V:L1 consumption receipt
 -> V-9 bind same identity into V:L3
 -> V-10 first parent cross-layer vertical receipt
 -> V-11 breadth matrix
 -> V-12 final V audit / subsystem promotion
```

V-1 through V-6 are architecture/infrastructure and cannot be counted as original-process equivalence.

## Cross-layer dependency matrix

| Acceptance question | Implementation/reverse owner | Validation owner / required support |
|---|---|---|
| Which resource wins for a real game request? | L2 | V:L2 + LV protected-process observation |
| Are selected bytes exact? | L1 | V:L1 with L2 selected identity + artifact binding |
| Can the selected representation be edited safely? | L1 | V-B/V-C real receipt |
| Will the authored overlay win? | L2 | V:L2 + L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | V:L1 + L2 authored winner |
| Did original DMC3 consume those bytes? | L3 | V-D + LV-L3 using the same L1/L2 identity chain |
| Was the test rolled back without retail mutation? | product/operator path | V-A/V-D exact artifact identity |
| May the layer/subsystem be called COMPLETE? | none of L1/L2/L3 alone | V-E only |

## Anti-laundering invariants

```text
Windows + Ubuntu green != original equivalence
real corpus parses != original selection/consumption equivalence
static EXE model != dynamic timing/order proof
GDSpaces selected X != original selected X
schema-valid JSON != trusted original observation
one mapped RVA window != global build equivalence
L1 run A + L2 run B + L3 run C != one vertical proof
crash-free launch != consumer acceptance
```

## Current priority queue

1. Keep the L1 real-retail acceptance path ready; do not replace it with synthetic work.
2. Finish review/promotion of the V/LV architecture integration slice.
3. Produce a real protected-process multi-anchor mapping receipt with merged #219 tooling.
4. Acquire a cryptographically bound DMC3 retail member-list/central-directory surface and run the `0x0E` census.
5. Use #221 contract only after trusted LV-L2 original-process observation exists.
6. Define the parent V `validation_run_id` and cross-receipt binder before combining layer-local PASS results.
7. Reconcile #209/#220/#217 statuses under V ownership.
8. Capture the first same-resource L2 -> L1 -> L3 vertical receipt.

## Completion rule

No percentage alone can mark a layer or GDSpaces complete.

Completion requires:

- mandatory implementation/reverse gates for the declared scope;
- canonical code/docs;
- exact-head Windows+Ubuntu product validation where applicable;
- representative real-corpus/original-process evidence;
- no unresolved contradiction changing the declared scope;
- **a V-owned promotion verdict**.

Layer-local completion indicators remain planning/status data until V accepts the required evidence.
