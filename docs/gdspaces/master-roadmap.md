# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-27  
**Base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Canonical boundary/status authority:** `layer-boundary-status-reconciliation-2026-08-27.md`  
**Overall:** **L1 INCOMPLETE / L2 INCOMPLETE / L3 INCOMPLETE**

This roadmap treats L1/L2/L3 as separate semantic ownership layers. A function, object or queue may participate in more than one layer, but each behavior is assigned to the layer whose question it answers. Address proximity and historical issue labels do not decide ownership.

## 1. Canonical layers

### L2 — Resource Resolution

Question:

> Which logical resource/provider/source/member is selected?

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume traversal
 -> ambiguity/fallback/failure classification
 -> successful selected ResourceRef/provider identity
```

L2 ends when a usable selected identity exists. A provider hit that fails to become a usable selected resource remains L2 failure/selection semantics. Once the selected source/backend is used to transfer/transform bytes, execution is L1.

### L1 — Resource Materialization

Question:

> How do the selected resource bytes become exact materialized bytes, and how are they reproduced/edited/rebuilt?

```text
selected provider/member identity
 -> backend/FileSlot byte acquisition
 -> sync/async byte transport required by materialization
 -> transform/decompression
 -> caller-owned destination bytes
 -> packed OR loose-list representation materialization
 -> nested PAC/PNST/.lst construction
 -> materialization terminal success/error dependency
 -> normal state 1 -> 2 publication
 -> exact materialized bytes + provenance
 -> edit / rebuild / repack / publication
 -> reopen / rematerialization
```

Canonical L1 boundary corrections:

- FileSlot/ReadRequest selected-byte transport is L1;
- raw transport completion/status needed for materialization correctness is L1;
- `0x1402EF4D0` materialization submission/job behavior is L1 boundary work;
- the unresolved job terminal/poll/retire/fail condition is an L1 mandatory gate;
- normal `0x1401B8DC0` `state1 -> state2` publication is the end-of-materialization boundary;
- `.lst` packed-first-vs-loose choice is L1 representation materialization.

### L3 — Original Runtime / Lifecycle

Question:

> What happens after materialized bytes exist: typed normalization, ready visibility, ownership, reuse, cancellation policy, release and teardown?

```text
state 2 / materialized bytes complete
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3
 -> consumer-ready visibility
 -> claims/cache/factory/dependency ownership
 -> cancellation/replacement policy
 -> state4 cleanup semantics
 -> owner release / group reset / full reset
 -> CRT/process-lifetime teardown
```

L3 may mark unfinished state1/state2 records for cancellation/replacement. That policy is L3; it does not reclassify the underlying selected-byte transport or completion dependency as L3.

### DOMAIN — Stage Assembly / Stage Ops / ModViz

Stage assembly, semantic graph, scene/editor/runtime visualization and ModViz are downstream consumers. They are not L3 and must not create a private resource resolver/materializer/lifecycle truth.

### V — Validation

Hashes, corpus receipts, CI, protected-process evidence and original-vs-product comparison are cross-cutting validation, not a fourth decompilation layer.

## 2. Vertical acceptance target

```text
real protected DMC3 installation
 -> [L2] exact selected provider/volume/member
 -> [L1] exact selected-byte materialization through state2
 -> [L1] supported edit/rebuild/repack/rematerialization
 -> [L2] authored next-volume winner
 -> [L3] typed-ready/consumer visibility
 -> observable effect attributable to authored bytes
 -> rollback / transition receipt
```

A crash-free launch is insufficient.

## 3. Track A — L1 completion

**Status: INCOMPLETE / NOT 100%**

The product authoring/materialization implementation is advanced. The layer is still open for both static/original-materialization equivalence and real acceptance.

### A1 — mandatory static terminal-dependency closure

```text
0x1401B8CA0 materialization dispatch
 -> 0x1402EF4D0 job/submission
 -> lower whole-file/FileSlot work
 -> UNKNOWN exact terminal dependency
 -> normal 0x1401B8DC0 eligibility/suppression
 -> state 1 -> 2
```

Required next pass:

1. `0x1402EF4D0` exact job identity/type + inherited context consumer;
2. matching `0x1402EF790` dispatch and persistence/re-poll/retirement behavior;
3. `0x1400333E0` pending/success/error semantics;
4. `0x140033390` terminal cleanup/release ordering;
5. bind `0x1400335A0` transport completion writes into that state;
6. prove failed/incomplete suppression before normal `0x1401B8DC0`;
7. recover relevant `0x1402EF460` pending-entry clear/rollback behavior;
8. apply the confirmed terminal model to `.lst` child/recursive failure ordering.

No generic fan-in counter is assumed.

### A2 — real-retail acceptance

After/alongside A1 where artifacts permit:

```text
direct-retail provenance
 -> representation classification
 -> one supported real edit/rebuild
 -> next-volume publication
 -> canonical reopen/rematerialization
 -> original-game consumption + rollback
 -> final L1 audit
```

L1 becomes COMPLETE/100% only when A1, real acceptance and final audit are all closed.

## 4. Track B — L2 closure

**Status: ADVANCED / NOT COMPLETE**

Closed/integrated static/tooling slices include:

- type-0 physical provider post-`0x0C` static chain and product model (#215/#204);
- protected-runtime explicit-PID RVA acquisition and bounded mapping tooling (#219);
- selected-identity candidate/normalizer/artifact binder tooling (#221), which is not trusted original-process evidence by itself.

Open L2 gates:

```text
L2-R2A real-retail 0x0E normalized-key collision census
L2-R2B real protected-process multi-anchor mapping receipt
L2-R3 trusted zero-loss original-process selected-provider identity
L2-R4 final contradiction-free L2 audit
```

Authority split remains:

- canonical instruction reverse: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432;
- protected distribution/original execution candidate: `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`, size 6,567,320;
- canonical analysis VA/RVA mappings must not be applied to the protected process without independent runtime mapping evidence.

Important L2/L1 boundary:

- candidate/provider/member identity and selection failure are L2;
- selected backend range read, decompression and materialized-byte production are L1.

## 5. Track C — L3 closure

**Status: ADVANCED STATIC SPINE / NOT COMPLETE**

Canonical L3 starts from completed materialized state2, not from FileSlot transport.

Strong/bounded static authority includes:

- LoadedResource registry `363 x 0x48` and seven-group topology;
- typed post-load/finalizer path from state2 to state3;
- state3 consumer-ready meaning;
- cancellation policy `1|2 -> 4`;
- quiescence predicate `{0,3}`;
- state4 cleanup and distinct ordinary/group/full release/reset behavior;
- representative MOD/EFM/SCM/SHW typed post-load and recursive PNST traversal;
- higher-level loader-node claim/zero-claim release model;
- runtime vs CRT vs process-lifetime teardown distinction.

Current L3 static work:

```text
residual alias/value-flow writer census
 -> family-specific +0x08/+0x18/+0x20/+0x28 ownership breadth
 -> external typed/factory/dependency and SCM edges
 -> shared-owner breadth
 -> final static contradiction sweep
```

Current L3 dynamic work:

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

Cross-layer cancellation rule: L3 owns the policy that invalidates unfinished resources; L1 owns the byte-terminal condition and normal materialization-completion eligibility being suppressed.

## 6. Cross-layer dependency matrix

| Acceptance question | Primary owner | Required support |
|---|---|---|
| Which resource/provider/member wins? | L2 | retail corpus + mapped trusted original-process evidence |
| Did selected bytes transfer/materialize correctly? | L1 | selected L2 identity |
| Did transport reach terminal success/error before state2? | L1 | scheduler/FileSlot evidence; L3 cancellation policy may interact |
| Is `.lst` packed or loose synthesis used? | L1 | selected resource identity from L2 |
| Can exact bytes be edited/rebuilt safely? | L1 | real representation evidence |
| Will authored overlay win? | L2 | L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | L2 authored winner |
| Did original DMC3 reach consumer-ready state/use? | L3 + V | same L1/L2 identity chain |
| Was rollback clean? | V | exact artifact identity |

## 7. Current priority queue

1. **L1:** close the materialization terminal dependency and failed/incomplete completion suppression from exact EXE evidence.
2. **L1:** bind the confirmed direct-resource terminal mechanism to `.lst` child/recursive failure ordering.
3. **L2:** produce real R2B protected-process mapping and trusted R3 selected-identity receipts when process access exists.
4. **L2:** obtain exact retail member-list/central-directory evidence and run the `0x0E` collision census.
5. **L1:** obtain real selected-member provenance, classify representation, run supported edit/rebuild/rematerialization.
6. **L3/V:** capture consumer-ready/use evidence for the same authored resource.
7. **L1/V:** execute original-game consumption + rollback and run final L1 audit.
8. **L3:** continue transition/reset/shutdown/family breadth independently after the vertical proof.
9. Run final L2 and L3 audits independently; one layer's progress never marks another complete.

## 8. Completion rule

No percentage alone marks a layer complete.

Current canonical labels are:

- **L1: INCOMPLETE / NOT 100%**;
- **L2: INCOMPLETE / NOT 100%**;
- **L3: INCOMPLETE / NOT 100%**.

A layer becomes complete only after its mandatory static/evidence gates, real receipts where required, exact-head validation and contradiction-free canonical documentation all agree.
