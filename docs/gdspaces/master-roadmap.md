# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-26  
**Base:** `main@eb701b9c523a3ec87f3c73bb8764038f1f2ef8dc`  
**L2 runtime-mapping seam:** PR #219 merged; R2A/R2B/R3 evidence execution remains open  
**Latest L3 static authority:** `l3-r1-leaf-alias-pass-2026-08-26.md`

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
 -> FileSlot/async lifecycle ownership
 -> LoadedResource states
 -> typed post-load
 -> ready visibility
 -> claims/cache
 -> cancellation/reset/release/shutdown
```

**Stage Assembly / Stage Ops is not L3.** It is a downstream domain/tooling consumer of the three-layer resource authority.

Validation is cross-cutting and is not a fourth layer.

## Execution rule

A task from another layer is allowed when it closes the current acceptance gap. Every task must record its primary layer, dependency and return condition to the vertical critical path.

Do not start broad work merely because it is interesting. Do not block required cross-layer evidence because another layer is still open.

## Current vertical acceptance target

```text
real protected DMC3 installation
 -> [L2] exact selected provider/volume/member
 -> [L1] artifact-bound exact materialized bytes
 -> [L1] supported top-level or nested edit/rebuild
 -> [L2] authored next-volume winner
 -> [L1] exact authored rematerialization
 -> [L3] original lifecycle reaches consumer-ready visibility
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

L2-R2B protected-distribution runtime RVA mapping (seam merged in #219)
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

Merged PR #219 provides the acquisition seam, not the original-process proof itself:

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

Canonical static authority:

- baseline subsystem audit: `l3-audit-2026-08-25.md`;
- raw canonical-EXE reconciliation: `l3-boundary-audit-2026-08-26.md`;
- detailed second raw pass: `l3-raw-exe-pass-2026-08-26.md`;
- direct-base state-writer census: `l3-r1-direct-writer-census-2026-08-26.md`;
- leaf/no-unwind + completion-callback alias pass: `l3-r1-leaf-alias-pass-2026-08-26.md`.

### L3 acquisition blocker — CLOSED

The canonical instruction-reverse `dmc3.exe` has been reacquired and independently re-identified as:

- size `6,356,432`;
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- PE32+ x64, preferred image base `0x140000000`.

The old "raw canonical EXE unavailable" statement is superseded. Acquisition tooling remains useful for reproducibility/regression packets, not as the current L3 semantic blocker.

### Static L3 boundaries now strong

Do not restart without contradictory evidence:

- LoadedResource registry `363 x 0x48` and seven-group topology;
- central lifecycle `0 -> 1 -> 2 -> typed post-load -> optional callback -> 3`;
- canonical cancellation source domain `1|2 -> 4`;
- quiescence predicate: every record must be state `0` or `3`;
- cancellation cleanup `4 -> 0` ordering;
- distinct ordinary/group/full release-reset policies;
- representative MOD/EFM/SCM/SHW typed post-load and recursive PNST traversal;
- central typed dispatcher unknown/default behavior as best-effort no-op/return rather than a failure return that blocks state 3;
- higher-level loader-node claim/zero-claim release concept;
- runtime vs CRT vs process-lifetime teardown distinction;
- direct-base/unwind-bounded state-like false-positive class is separated from actual LoadedResource state authority;
- exact-immediate leaf/no-unwind `+0x04 <- 0..4` candidate class contains no new LoadedResource writer beyond canonical `0x1401B8DC0`;
- `0x1401B8DC0` normal completion callback is registered from `0x1401B84E0` with one `u32` context equal to `record_ptr - 0x140C99D30` and dispatched by `0x1402EF580/0x1402EF790`;
- valid normal callback contexts are `index*0x48` for `0..362`, so the odd/low-bit branch of `1B8DC0` is outside the recovered canonical normal acquisition-registration domain.

### L1/L3 seam now explicit

`0x1401B8CA0` is a mixed materialization/lifecycle boundary:

```text
L1 representation/materialization dispatch
 -> boolean success
 -> L3 acquisition publishes state 1 only after success
```

Classify evidence by semantics, not by assigning the whole helper to one layer.

### Current L3 static work order

```text
finish residual R1 data-flow census
  -> non-immediate state writes
  -> caller-propagated / indexed / derived record aliases
  -> indirect callback registrations
 -> finish family-specific +0x08/+0x18/+0x20/+0x28 ownership census
 -> close external typed/factory/dependency and SCM edges
 -> finish shared-owner coordination breadth
 -> original-process Level-E receipts
```

Do not repeat the already-bounded direct-base or exact-immediate leaf census unless contradictory evidence appears.

The central old question "does unknown/default `0x1401B9FA0` post-load leave state 2?" is no longer open for that path: the dispatcher is best-effort/void and the state-2 finalizer proceeds to callback then state3.

The old low-bit question on `0x1401B8DC0` is also narrowed: the odd branch is not reachable from the recovered canonical normal acquisition registration, whose context is always `index*0x48`. Its semantic intent outside that domain remains deliberately unnamed.

### Current L3 dynamic work order — issue #217

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
2. Complete L2 protected-runtime mapping/selected-identity evidence when a protected process is available.
3. Finish the now-narrow L3 static census instead of reopening the recovered core spine.
4. Capture V1 and V5 first because they directly exercise ready-state and cancellation boundaries needed by the vertical proof.
5. Continue transition/reset/shutdown receipts and family/build breadth.
6. Reconcile final L2/L3 evidence and run their final audits independently.

## Completion rule

No percentage alone can mark a layer complete. Completion requires mandatory gates, canonical code/docs, exact-head Windows+Ubuntu validation where applicable, representative real-corpus/original-process receipts and no unresolved contradiction changing the declared scope.

Percentages may be recalculated only as planning indicators after gate reconciliation.
