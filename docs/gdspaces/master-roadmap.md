# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-27  
**Base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Canonical boundary/status authority:** `layer-boundary-status-reconciliation-2026-08-27.md`  
**L1 byte-exactness authority:** PR #244 / `l1-byte-exactness-gap-pass-2026-08-27.md`  
**Overall:** **L1 INCOMPLETE / L2 INCOMPLETE / L3 INCOMPLETE**

L1/L2/L3 are semantic ownership layers. A function or subsystem may participate in several layers; ownership is assigned per behavior, not per address or class name.

## 1. Canonical layers

### L2 — Resource Resolution

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume traversal
 -> ambiguity/fallback/failure classification
 -> usable selected ResourceRef/provider/member identity
```

L2 ends at usable selection.

### L1 — Resource Materialization

```text
selected provider/member identity
 -> materialized-size authority
 -> capacity/allocation requirements
 -> selected byte/span acquisition semantics
 -> EOF/final-chunk/short-read/progress semantics
 -> transform/decompression
 -> exact caller-owned destination bytes
 -> packed OR .lst synthesized representation
 -> nested PAC/PNST/.lst byte construction
 -> terminal materializer success/error
 -> 0x1401B8CA0 result
```

Product provenance/edit/rebuild/repack/publication/reopen-rematerialization remain L1 product responsibilities.

### L3 — Original Runtime / Lifecycle

```text
scheduler/request ownership
 -> completion eligibility
 -> LoadedResource state 1 -> 2
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3
 -> ready visibility
 -> claims/cache/factory ownership
 -> cancellation/replacement
 -> state4 cleanup
 -> release/reset/shutdown
```

The L1 terminal result must gate L3 completion, but this dependency does not transfer LoadedResource state publication into L1.

### DOMAIN / V

Stage Assembly, Stage Ops and ModViz are downstream DOMAIN consumers. Validation is cross-cutting V authority.

## 2. Current vertical acceptance target

```text
real protected DMC3 installation
 -> [L2] exact selected provider/volume/member
 -> [L1] exact selected-resource byte representation
 -> [L1] supported edit/rebuild/repack/rematerialization
 -> [L2] authored overlay wins
 -> [L3] original runtime publishes/typed-readies/uses the resource
 -> [V] attributable consumer effect + rollback
```

A crash-free launch is insufficient.

## 3. Track A — L1 completion

**Status: INCOMPLETE / NOT 100%**

Three status dimensions must remain separate:

- **product implementation:** advanced / representative-path implementation-ready at bounded scope;
- **original EXE materialization reverse:** NOT EXHAUSTIVE;
- **real acceptance:** OPEN.

### A1 — byte-exactness reverse closure

Mandatory frontier:

1. size/zero/error semantics: `0x14002F9F0 -> 0x140048E20`;
2. rounded 0x800 transfer vs exact logical extent and final-chunk clamp;
3. physical/ZIP EOF and short-read/progress behavior;
4. required capacity/alignment/overflow: `0x1401B7B90`;
5. allocation initialization and synthesized padding contents;
6. `.lst` representation tests/planner: `0x1401B79E0`, `0x1401B7FD0`;
7. `.lst` writer equivalence/failure propagation: `0x1401B85C0`;
8. exact byte-producing ingress/context behind `0x1402EF4D0`;
9. partial-read/InflateRead terminal composition;
10. preserve the boundary that relative slot starts do not prove universal intrinsic child size.

### A2 — L1/L3 completion seam

After direct byte-terminal semantics are exact:

```text
[L1] terminal materializer success/error
 -> [SEAM] allowed/suppressed completion
 -> [L3] scheduler/callback -> state1 -> state2
```

Need to reconcile `0x1402EF4D0`, relevant `0x1402EF790`, fresh `0x1400333E0/0x140033390`, `0x1400335A0`, suppression before `0x1401B8DC0`, and relevant `0x1402EF460` behavior.

No generic fan-in counter is assumed.

### A3 — real-retail / Level-E acceptance

```text
direct-retail selected-member provenance
 -> exact representation classification
 -> one supported real edit/rebuild
 -> next-volume publication
 -> exact reopen/rematerialization
 -> original DMC3 consumption
 -> rollback / retail immutability
 -> final L1 audit
```

L1 becomes COMPLETE/100% only after A1 + required A2 seam evidence + A3 all close at the declared scope.

## 4. Track B — L2 closure

**Status: ADVANCED / NOT COMPLETE**

Strong/integrated slices:

- request/candidate/provider precedence and numbered-volume structure;
- archive normalization/index/search;
- bounded type-0 physical-provider static chain;
- protected-runtime mapping tooling;
- selected-identity candidate/normalizer/artifact binder tooling.

Open gates:

```text
L2-R2A exact retail 0x0E normalized-key collision census
L2-R2B real protected-process multi-anchor mapping receipt
L2-R3 trusted zero-loss original-process selected-provider identity
L2-R4 final contradiction-free L2 audit
```

Boundary rule: once a usable provider/member identity is selected, exact selected-byte size/read/transform semantics belong to L1.

## 5. Track C — L3 closure

**Status: ADVANCED STATIC SPINE / NOT COMPLETE**

L3 retains scheduler/request ownership and LoadedResource lifecycle. Strong/bounded authority includes:

- LoadedResource registry `363 x 0x48` and seven groups;
- normal state1 -> state2 callback ABI/state publication;
- state2 typed post-load -> optional callback -> state3;
- state3 consumer-ready meaning;
- cancellation `1|2 -> 4`;
- quiescence `{0,3}`;
- state4 cleanup and distinct release/reset paths;
- representative typed families;
- loader-node claim/release model;
- runtime vs CRT vs process-lifetime teardown distinction.

Open static work:

```text
residual alias/value-flow census
 -> family-specific +0x08/+0x18/+0x20/+0x28 ownership
 -> external typed/factory/dependency and SCM edges
 -> shared-owner breadth
 -> completion-scheduler behavior needed at L1/L3 seam
 -> final contradiction sweep
```

Open dynamic work:

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

L3 work must not absorb unresolved L1 size/extent/capacity/padding/byte-result questions.

## 6. Cross-layer dependency matrix

| Acceptance question | Primary owner | Support |
|---|---|---|
| Which provider/member wins? | L2 | retail corpus + trusted original-process evidence |
| What is the exact logical/materialized size? | L1 | selected L2 identity |
| Are final bytes exact, including EOF/short-read behavior? | L1 | backend/FileSlot evidence |
| Is packed or `.lst` synthesized representation used? | L1 | selected L2 identity |
| Does terminal L1 success permit normal state2 publication? | L1/L3 seam | L1 terminal result + L3 scheduler evidence |
| Who publishes LoadedResource state1 -> state2? | L3 | terminal L1 prerequisite |
| Can bytes be edited/rebuilt safely? | L1 | real representation evidence |
| Will authored overlay win? | L2 | L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | L2 authored winner |
| Did original runtime typed-ready/use them? | L3 + V | same L1/L2 identity chain |
| Was rollback clean? | V | exact artifact identity |

## 7. Current priority queue

1. **L1:** close materialized-size/zero/error semantics.
2. **L1:** close rounded-transfer/final-chunk/EOF/short-read semantics.
3. **L1:** close capacity/allocation/initialization/padding semantics.
4. **L1:** close `.lst` planner/writer/failure equivalence.
5. **L1:** bind exact byte-producing ingress/context behind `0x1402EF4D0`.
6. **L1/L3 seam:** prove terminal-result gating and failed/incomplete completion suppression before state2.
7. **L2:** produce real protected mapping/trusted selected identity and retail collision evidence when artifacts permit.
8. **L1:** obtain real selected-member provenance, classify representation, edit/rebuild/rematerialize.
9. **L3/V:** capture same-resource original typed-ready/use evidence.
10. **L1/V:** execute original-game consumption + rollback and final L1 audit.
11. Continue independent L2 and L3 closure; one layer never inherits another layer's completion.

## 8. Completion rule

No percentage alone marks a layer complete.

Current labels:

- **L1: INCOMPLETE / NOT 100%**;
- **L2: INCOMPLETE / NOT 100%**;
- **L3: INCOMPLETE / NOT 100%**.

A layer becomes complete only after its declared static reverse/evidence gates, required real receipts, exact-head validation and contradiction-free canonical documentation agree.