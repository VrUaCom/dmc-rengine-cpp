# GDSpaces — Layer Boundary and Status Reconciliation — 2026-08-27

**Authority:** canonical cross-layer status/ownership correction  
**Base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Canonical analysis EXE:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Companion L1 gap authority:** `l1-byte-exactness-gap-pass-2026-08-27.md` / PR #244

## 1. Status correction

Layer 1 is **INCOMPLETE / NOT 100%**.

The current product authoring/materialization path is advanced and can remain representative-path implementation-ready at its evidenced scope. That is not a Layer-1 reverse-completion claim.

Two independent classes remain open:

1. **original L1 byte-exactness/materialization reverse gaps**;
2. **real-retail / original-game acceptance receipts**.

No canonical status may state or imply `L1 COMPLETE`, `L1 100%`, or that only external receipts remain.

## 2. Canonical ownership rule

Layer ownership follows the semantic question being answered. A helper, queue, FileSlot object or EXE address is never assigned wholesale to one layer merely because it participates in that layer.

### L2 — Resource Resolution

L2 answers:

> Which logical resource/provider/source/volume/member is selected?

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume traversal
 -> ambiguity/fallback/failure classification
 -> usable selected ResourceRef/provider/member identity
```

Provider/open failure before a usable selected identity exists remains L2 failure semantics.

### L1 — Resource Materialization

L1 answers:

> Given a selected resource identity, what exact byte image is required and how is it materialized/reproduced?

```text
[L2] usable selected provider/member identity
 -> [L1] logical/materialized size authority
 -> destination capacity/allocation requirements
 -> selected byte/span acquisition semantics
 -> final-chunk clamp / EOF / short-read / progress semantics
 -> STORE/raw-DEFLATE or other evidenced transforms
 -> exact caller-owned destination bytes
 -> packed OR .lst synthesized representation
 -> nested PAC/PNST/.lst byte construction
 -> terminal materializer success/error result
 -> materialization return through 0x1401B8CA0
 ===== END L1 ORIGINAL BYTE-MATERIALIZATION CUT =====
```

Product-side provenance, edit, rebuild/repack, publication and reopen/rematerialization remain L1 product responsibilities.

### L3 — Original Runtime / Lifecycle

L3 answers:

> How does the original runtime schedule, publish, own, ready, cancel, release and tear down materialized resources?

```text
materialization submitted / lifecycle record active
 -> scheduler/request ownership
 -> completion callback eligibility
 -> LoadedResource state 1 -> 2 publication
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3
 -> consumer-ready visibility
 -> claims/cache/factory/dependency ownership
 -> cancellation/replacement
 -> state4 cleanup
 -> release/reset/shutdown
```

**LoadedResource `state1 -> state2` remains L3 lifecycle state publication.** It is not used as the L1 endpoint.

## 3. Cross-layer materialization-completion seam

The unresolved dependency between lower byte production and normal `0x1401B8DC0` publication is real, but it must be split by semantics instead of assigned wholesale to L1 or L3.

```text
[L1] exact byte-producing work reaches terminal success/error
        |
        v
[SEAM] terminal dependency / completion eligibility
        |
        v
[L3] scheduler dispatches or suppresses normal 0x1401B8DC0
 -> LoadedResource state 1 -> 2
```

Merged evidence proves normal `0x1401B8DC0` receives only one u32 registry-relative context and cannot itself inspect raw transport status/error/byte count. Therefore some earlier mechanism must make the L1 result terminal before L3 normal completion is permitted, or suppress/remove that completion.

No generic fan-in counter is assumed and FIFO alone is not proven sufficient.

### Behavior-level split

| Behavior | Canonical owner |
|---|---|
| request/candidate/provider/volume/member selection | L2 |
| selected logical size / uncompressed size authority | L1 |
| final byte extent, EOF, short-read and clamp semantics | L1 |
| STORE/raw-DEFLATE byte production | L1 |
| destination capacity / initialization / padding relevant to exact bytes | L1 |
| PAC/PNST/.lst representation byte construction | L1 |
| FileSlot/ReadRequest byte-count/result semantics needed for exact bytes | L1 |
| FileSlot/ReadRequest object ownership, queue lifetime, callback lifetime | L3/runtime-I/O lifecycle |
| `0x140033500/0x1400335A0` byte/result semantics | L1 support |
| `0x140033500/0x1400335A0` request/callback ownership/lifetime | L3 support |
| `0x1402EF4D0` byte-producing ingress/context | L1 OPEN |
| `0x1402EF4D0` queued-job ownership/persistence | L3 OPEN |
| `0x1402EF790` scheduler dispatch/persistence/retirement | L3 |
| dependency that binds terminal L1 result to allowed/suppressed L3 completion | L1/L3 seam |
| normal `0x1401B8DC0` state1 -> state2 publication | L3 |
| typed post-load / state2 -> state3 / ready | L3 |
| cancellation/replacement/release/reset/shutdown | L3 |
| Stage Assembly / Stage Ops / ModViz | DOMAIN downstream |
| hashes / CI / original-process receipts | V cross-cutting |

`.lst` packed-first-versus-loose is classified as **L1 representation materialization**, because it determines how the same selected resource identity becomes bytes; it is not a provider-selection question.

## 4. Mandatory L1 reverse frontier

PR #244 exposes a broader L1 frontier than the completion bridge alone. These are mandatory before claiming exhaustive original L1 materialization reverse:

1. **G1 — rounded transfer vs exact logical extent:** `ceil(totalBytes/0x800)` request, final-chunk clamp, EOF/short-read/progress semantics;
2. **G2 — materialized-size authority:** `0x14002F9F0 -> 0x140048E20`, zero/error/sentinel and compressed-vs-logical size semantics;
3. **G3 — capacity/allocation:** `0x1401B7B90`, 64-byte rounding, integer width/overflow, allocation failure and initial byte state;
4. **G4 — `.lst` planner/materializer equivalence:** `0x1401B7FD0` vs `0x1401B85C0`, child-size source, failure propagation and recursive planning;
5. **G5 — `.lst` padding contents:** prove zero/unspecified/explicitly initialized alignment gaps from original allocation/write flow;
6. **G6 — exact byte-producing ingress behind `0x1402EF4D0`:** identify the materialization job path and inherited context consumer without conflating scheduler ownership;
7. **G7 — partial-read / transform terminal semantics:** short STORE reads, InflateRead partial production, no-progress and truncated-stream composition;
8. **G8 — packed child extent boundary:** runtime relative-slot starts do not prove a universal intrinsic child-size field; keep layout-preserving patch and synthesized-image scopes distinct.

The materialization-completion scheduler seam remains a required cross-layer reconciliation after direct byte-terminal semantics are exact.

## 5. Correct raw-pass order

```text
1. 0x14002F9F0 + 0x140048E20 — logical/materialized size + zero/error domain
2. 0x140033390..0x1400335A0 — final chunk, short read, progress, terminal result
3. lower backend clamp — physical/ZIP exact EOF behavior
4. 0x1401B7B90 — capacity, alignment, integer width, allocation failure
5. allocation/backing helpers — initialization and padding state
6. 0x1401B79E0 + 0x1401B7FD0 — representation tests and .lst planning
7. 0x1401B85C0 — planner/writer equivalence, child failures, padding
8. 0x1402EF4D0 — exact byte-producing ingress and inherited context
9. reconcile L1 terminal result with L3 scheduler/completion suppression before 0x1401B8DC0
10. real-retail provenance/edit/rebuild/rematerialization + original-game acceptance
```

This order prevents broad L3 scheduler work from hiding unresolved L1 byte-exactness questions.

## 6. L2 and L3 boundary consequences

### L2

Keep selection identity and provider failure semantics in L2. Do not move selected-byte transfer or transforms into L2 merely because they originate from a selected provider.

### L3

Keep LoadedResource states, scheduler/request ownership, callback lifetime, cancellation/replacement, typed-ready, claims and release/reset/shutdown in L3. Do not move those lifecycle semantics into L1 merely because they gate or observe materialization completion.

A scheduler helper may carry both an L1-relevant byte result and L3 ownership behavior. Classify the concrete action, not the function wholesale.

## 7. Real acceptance remains mandatory

After static reverse closure at the declared scope, L1 still requires:

- direct-retail selected-member provenance;
- exact retail representation classification;
- one supported real edit/rebuild/rematerialization receipt;
- attributable original DMC3 consumption;
- rollback / retail immutability;
- final contradiction-free cross-stack audit.

## 8. Canonical completion labels

Allowed current labels:

- `L1 = INCOMPLETE / NOT 100%`;
- `L1 product implementation = advanced / representative-path implementation-ready at bounded scope`;
- `L1 original EXE materialization reverse = NOT EXHAUSTIVE`;
- `L2 = INCOMPLETE / NOT 100%`;
- `L3 = INCOMPLETE / NOT 100%`.

Disallowed current labels:

- `L1 COMPLETE`;
- `L1 100%`;
- `only external receipts remain for L1`;
- `FileSlot/AsyncIO is wholly L1`;
- `FileSlot/AsyncIO is wholly L3`;
- `state1 -> state2 is L1`;
- `.lst packed-first representation choice is L2`.

Historical observations remain evidence history. Where old ownership wording conflicts with this reconciliation, keep the observation and supersede only the ownership/completion interpretation.