# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-26  
**Base:** `main@453373ff0977fc0aa1f6fab39273cdd9716da6af`  
**L2 R3 candidate tooling:** PR #221 merged; process-instance v2 hardening active in PR #236; real R2B/trusted original-process R3 remains open under #220/#229  
**Latest cross-layer reverse authority:** PR #228 — materialization-completion dependency boundary + canonical `0x1402EF4D0` plan correction  
**Latest L3 static authority:** `l3-r1-derived-alias-pass-2026-08-26.md` + merged #230 scheduler/context authority

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
Connected-access correction: `l1-connected-retail-access-reconciliation-2026-08-26.md`.

**Internal product implementation status:** CLOSED for the current representative DMC3-HD acceptance scope.

Promoted capabilities include artifact-bound retail acquisition, atomic no-replace publication, STORE/raw-DEFLATE materialization, PAC/PNST sparse/alias-preserving expansion, size-changing relative-slot reflow, nested root-to-leaf slot-path authoring, verified NBZ rebuild, next-volume overlay authoring and protected retail closure orchestration.

The protected retail install, protected `dmc3.exe`, and `data/dmc3/dmc3-0.nbz` are locatable in connected Drive. The observed NBZ size is 960,358,951 bytes. The connected transfer/materialization path still cannot deliver that archive and exposes no exact parsed central-directory/member surface, so artifact presence does not close L1-C.

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

Canonical review baseline: `l2-review-2026-08-25.md`, `l2-review-2026-08-26.md`, and `l2-exe-reconciliation-2026-08-26.md`.

### Closed/integrated L2 internal slices

PR #215 / issue #204 closed the type-0 physical-provider post-`0x0C` boundary:

- exact static root-join/open/existence/miss semantics recovered from canonical analysis executable;
- native physical-path product capability separated from archive index semantics;
- product evidence classes remain distinct from recovered/original authority;
- controlled physical hit, complete miss and archive→physical fallback receipts integrated;
- Windows bounded `CreateFileA` parity scenario + Ubuntu/Windows exact-head validation passed;
- direct caller census and `0x400` overflow behavior are closed for the recovered direct-call surface.

PR #219 integrated the protected-runtime mapping acquisition/tooling seam:

- explicit PID and actual module-base + RVA capture;
- exact protected executable SHA/size gate;
- bounded process-memory window receipts;
- canonical-window expectation binding;
- multi-anchor mapping validator requiring `OpenGameResource` plus at least two type-0 anchors;
- no global build-equivalence claim.

PR #221 integrated the selected-identity **content-candidate** contract, deterministic normalizer, artifact-backed binder and EXE reconciliation tooling. It did **not** promote trusted original-process selection evidence.

PR #236 is the active correction to the #219/#221 evidence seam. It versions the promotion path to v2 and adds one Windows process-instance identity — process creation `FILETIME` — from acquisition through R2B and R3 candidate/binder. Until #236 is merged, this remains branch truth rather than canonical merged capability.

Do not reopen closed slices absent contradictory direct evidence.

### Current L2 work order

```text
PR #236 exact-head validation / promotion
  -> process-window v2 + R2B v2 + R3 candidate/binder v2
  -> still NOT trusted origin

L2-R2A real-retail 0x0E collision census
  -> direct-retail exact resolver identity receipt

L2-R2B real protected-process multi-anchor mapping receipt
  -> after v2 hardening, one PID + creation FILETIME + module base session

L2-R3 selected-provider identity
  -> v2 content-candidate + artifact binder
  -> trusted runtime publisher independently rechecks active process instance
  -> zero-loss real protected-process selected-identity receipt

[R2A + real R2B + trusted R3]
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

The exact DMC3 retail `dmc3-0.nbz` is locatable, but its 960,358,951 bytes exceed the current connected transfer/materialization channel and no exact cryptographically bound central-directory/member-list derivative is currently available.

Required closure:

1. obtain exact member-name/central-directory evidence bound to the retail archive;
2. run canonical `0x0E` normalized-key census;
3. preserve collision count and exact colliding identities if any;
4. only then promote a real-retail resolver selection receipt.

Synthetic/DMCL corpus results do not close this DMC3 retail gate.

### L2-R2B — protected runtime address mapping

#219 tooling is integrated on `main`; PR #236 hardens its real-promotion evidence identity to v2. The real evidence gate remains open.

After #236 promotion, a valid real packet requires one exact protected process instance and:

- `OpenGameResource` canonical RVA `0x2FCA0`;
- at least two independent type-0 anchors among `0x326D20`, `0x327430`, `0x327800`;
- exact `0x40` live windows at actual `module_base + RVA`;
- canonical analysis-window hash expectations;
- exact protected executable SHA/size and image path;
- one non-zero OS-derived Windows process creation `FILETIME` captured from the same process handle as the byte reads;
- exact agreement on PID + creation FILETIME + module base across all child receipts.

A child receipt proves only its listed range. A successful multi-anchor packet proves bounded mapping only for the listed anchors. It does **not** prove global build equivalence or original selected-provider identity.

Legacy process-window / mapping v1 receipts remain historical evidence and are not eligible for the final real R2B/R3 promotion path after v2 lands.

### L2-R3 — selected-provider identity

Integrated PR #221 defines the content-candidate boundary, not trusted original-process proof. PR #236 extends that boundary with process-instance identity:

1. recovered request/candidate/provider/volume order is structurally validated;
2. self-authored serializer output must pass the strict deterministic v2 normalizer;
3. the R2B mapping packet is reconstructed from v2 process-window child receipts;
4. candidate PID + process creation FILETIME + module base must exactly match R2B;
5. exact observer artifact is hashed and bound;
6. every claimed numbered NBZ artifact is hashed and size-checked;
7. bound output remains `promotion_eligible=false` / `trusted_capture_bound=false` until a separate trusted publisher/origin mechanism exists.

Fresh canonical EXE review adds one mandatory failure distinction: archive normalized lookup may succeed while wrapper/open creation at `0x140328290` fails. `0x140327430` then exits through null/cleanup; it does **not** continue to a lower-volume lookup as a clean miss. Therefore the clean-path R3 v2 contract supports only `miss -> selected`; provider/backend failure is fail-closed.

The remaining #229 trusted-publisher requirement is stricter than carrying a creation timestamp through JSON: the publisher must independently query the active process instance immediately around trusted capture and reject PID reuse or creation-time mismatch rather than trusting candidate-supplied identity.

Only after real R2B mapping and trusted publisher binding may a real selected identity be promoted and compared to GDSpaces product resolution. Product comparison helpers are diagnostic/content-only before that point.

`preflight-dmc3-game-test` is build/archive-presence preflight, not a selected-identity receipt.

## Track C — L3 closure

Canonical static authority:

- baseline subsystem audit: `l3-audit-2026-08-25.md`;
- raw canonical-EXE reconciliation: `l3-boundary-audit-2026-08-26.md`;
- detailed second raw pass: `l3-raw-exe-pass-2026-08-26.md`;
- direct-base state-writer census: `l3-r1-direct-writer-census-2026-08-26.md`;
- leaf/no-unwind + completion-callback alias pass: `l3-r1-leaf-alias-pass-2026-08-26.md`;
- derived/indexed/stored-record release pass: `l3-r1-derived-alias-pass-2026-08-26.md`;
- merged cross-layer completion-boundary pass: `materialization-completion-boundary-pass-2026-08-26.md` / PR #228.

### L3 acquisition blocker — CLOSED

The canonical instruction-reverse `dmc3.exe` has been reacquired and independently re-identified as:

- size `6,356,432`;
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`;
- PE32+ x64, preferred image base `0x140000000`.

The old "raw canonical EXE unavailable" statement is superseded. Acquisition tooling remains useful for reproducibility/regression packets, not as the current L3 semantic blocker.

### Static L3 boundaries now strong

Do not restart without contradictory evidence:

- LoadedResource registry `363 x 0x48` and seven-group topology;
- canonical raw `.data` counts `[4,136,60,28,1,128,6]` and bases `[0,4,140,200,228,229,357,363]`;
- central lifecycle `0 -> 1 -> 2 -> typed post-load -> optional callback -> 3`;
- canonical cancellation source domain `1|2 -> 4`;
- quiescence predicate: every record must be state `0` or `3`;
- cancellation cleanup `4 -> 0` ordering;
- distinct ordinary/group/full release-reset policies;
- acquisition-failure rollback state0 writers for bounded base-228, group5 and base-357 paths;
- fixed-family indexed state3->release->state0 helpers for registry bases `0/4/140/200`;
- stored group5 record aliases through higher-level `object+0x10` are proven from `1B8DF0` acquisition through conditional backing release and state0 publication;
- representative MOD/EFM/SCM/SHW typed post-load and recursive PNST traversal;
- central typed dispatcher unknown/default behavior as best-effort no-op/return rather than a failure return that blocks state 3;
- higher-level loader-node claim/zero-claim release concept;
- runtime vs CRT vs process-lifetime teardown distinction;
- direct-base/unwind-bounded state-like false-positive class is separated from actual LoadedResource state authority;
- exact-immediate leaf/no-unwind `+0x04 <- 0..4` candidate class contains no new LoadedResource writer beyond canonical `0x1401B8DC0`;
- `0x1401B8DC0` normal completion callback is registered from `0x1401B84E0` with one `u32` context equal to `record_ptr - 0x140C99D30` and dispatched by `0x1402EF580/0x1402EF790`;
- valid normal callback contexts are `index*0x48` for `0..362`, so the odd/low-bit branch of `1B8DC0` is outside the recovered canonical normal acquisition-registration domain;
- nearby higher-level `1B9EE0` `+0x04` copy is rejected as LoadedResource state by caller/subobject provenance.

### L1/L3 seam now explicit

`0x1401B8CA0` is a mixed materialization/lifecycle boundary:

```text
L1 representation/materialization dispatch
 -> boolean success
 -> L3 acquisition publishes state 1 only after success
```

Merged #228 narrows the unresolved dependency bridge downstream/upstream of this seam and corrects the general reverse plan to canonical `0x1402EF4D0`. It does not prove a generic fan-in counter or close dynamic ordering.

Classify evidence by semantics, not by assigning the whole helper to one layer.

### Current L3 static work order

```text
finish residual R1 value-flow census
  -> non-state0/non-immediate writes carrying states 1/2/3/4
  -> record aliases outside the reviewed 0x1401B8xxx..0x1401B9xxx lifecycle cluster
  -> indirect callback/function-pointer registrations
  -> final whole-image contradiction sweep
 -> finish family-specific +0x08/+0x18/+0x20/+0x28 ownership census
 -> close external typed/factory/dependency and SCM edges
 -> finish shared-owner coordination breadth
 -> original-process Level-E receipts
```

Do not repeat the already-bounded direct-base, exact-immediate leaf, or reviewed state0 release/rollback scans unless contradictory evidence appears.

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
| Which resource wins for a real game request? | L2 | protected retail corpus + mapped trusted original-process observation |
| Are selected bytes exact? | L1 | L2 selected identity + artifact binding |
| Can the selected representation be edited safely? | L1 | direct retail representation evidence |
| Will the authored overlay win? | L2 | L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | L2 authored winner |
| Did original DMC3 consume those bytes? | L3 + validation | same L1/L2 identity chain |
| Was the test rolled back without retail mutation? | validation | exact artifact identity |

## Current priority queue

1. Finish exact-head/current-main Ubuntu + Windows validation of PR #236 and promote only the process-instance v2 tooling if green.
2. Keep the L1 real-retail/Level-E acceptance path ready; do not replace it with synthetic work.
3. Use merged #221 plus PR #236 candidate/normalizer/binder surfaces only as content/tooling until trusted publisher origin exists.
4. When a protected original process is available, produce the real L2 multi-anchor R2B v2 packet using one PID + creation FILETIME + module base session.
5. Implement/use the trusted R3 publisher path and independently re-query the active process instance; do not accept manually authored JSON as original-process authority.
6. Acquire a cryptographically bound DMC3 retail member-list/central-directory surface and run the `0x0E` census.
7. Capture and bind a zero-loss original-process selected identity only after mapped L2 anchors and trusted process identity are proven.
8. Continue from merged #228/#230 static authority instead of reopening recovered core/state0/scheduler ABI surfaces.
9. Capture L3 V1 and V5 first because they directly exercise ready-state and cancellation boundaries needed by the vertical proof.
10. Continue transition/reset/shutdown receipts and family/build breadth, then reconcile final L2/L3 evidence and run their final audits independently.

## Completion rule

No percentage alone can mark a layer complete. Completion requires mandatory gates, canonical code/docs, exact-head Windows+Ubuntu validation where applicable, representative real-corpus/original-process receipts and no unresolved contradiction changing the declared scope.

Percentages may be recalculated only as planning indicators after gate reconciliation.
