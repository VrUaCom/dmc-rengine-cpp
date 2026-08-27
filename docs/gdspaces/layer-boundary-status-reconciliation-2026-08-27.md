# GDSpaces — Layer Boundary and Status Reconciliation — 2026-08-27

**Authority:** canonical cross-layer status/ownership correction  
**Base:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Canonical analysis EXE:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, size 6,356,432  
**Supersedes for current status/ownership:** any older wording that treats the unresolved materialization terminal dependency as merely optional supporting breadth, or assigns the whole FileSlot/AsyncIO/completion path to L3.

## 1. Status correction

Layer 1 is **INCOMPLETE / NOT 100%**.

The product authoring/materialization implementation is advanced and may be implementation-ready for a representative protected-install acceptance run, but this is not the same as Layer-1 completion.

A mandatory original-materialization seam remains unresolved:

```text
materialization submission/job
 -> lower FileSlot/whole-file transport
 -> terminal success/error condition
 -> completion eligibility/suppression
 -> normal 0x1401B8DC0 dispatch
 -> state 1 -> 2
```

Merged materialization-completion passes prove that `0x1401B8DC0` receives only a registry-relative context and cannot itself decide raw transport success/error. The exact terminal dependency, persistence/polling/retirement rule and failed/incomplete suppression path remain open.

Therefore no canonical document may state or imply `L1 = COMPLETE`, `L1 = 100%`, or that only external receipts remain before completion. External Level-E receipts are still mandatory, but they are no longer the only remaining L1 gate.

## 2. Canonical semantic ownership rule

Layer ownership follows behavior, not address range, object name, queue name or historical issue ownership.

### L2 — Resource Resolution

L2 answers:

> Which logical resource/provider/source/member is selected?

Canonical scope:

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume traversal
 -> ambiguity/fallback/failure classification
 -> successful selected ResourceRef/provider identity
```

L2 ends when the selected provider/member identity is established. Provider-open failure that invalidates selection remains L2 selection/failure semantics. Once a selected source/backend is used to transfer or transform bytes, that work is L1.

### L1 — Resource Materialization

L1 answers:

> How do selected resource bytes become exact materialized bytes, and how are they reproduced/edited/rebuilt?

Canonical scope:

```text
selected provider/member identity
 -> backend/FileSlot byte acquisition
 -> synchronous/asynchronous byte transport required for materialization
 -> STORE/raw-DEFLATE or other evidenced transforms
 -> caller-owned destination population
 -> packed OR loose-list representation materialization
 -> nested PAC/PNST/.lst byte construction
 -> materialization terminal dependency/error suppression
 -> state 1 -> 2 materialized-byte publication boundary
 -> provenance / editable identity
 -> rebuild/repack/publication
 -> reopen/rematerialization
```

The following are therefore L1 or L1-owned boundary behavior, not generic L3:

- FileSlot/ReadRequest byte-transfer mechanics and transport completion needed to determine materialization success;
- `0x140033500/0x1400335A0` whole-file submission/transport completion;
- materialization scheduler/job behavior needed to hold, poll, retire or fail selected-byte work;
- `0x1402EF4D0` materialization submission/job creation at the recovered bounded scope;
- the terminal dependency that permits or suppresses normal `0x1401B8DC0` dispatch;
- normal `state 1 -> 2` publication as the end-of-materialization boundary;
- `.lst` packed-first-vs-loose representation choice, because it chooses how the same selected resource identity is materialized rather than which provider/resource identity wins.

This does not mean every field or owner of the FileSlot/AsyncIO subsystem is L1. Service/pool lifetime outside selected-byte correctness remains L3.

### L3 — Original Runtime / Lifecycle

L3 answers:

> What happens after materialized bytes exist: typed normalization, ready visibility, ownership, reuse, cancellation policy, release and teardown?

Canonical start:

```text
state 2 / materialized bytes complete
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3
 -> consumer-ready visibility
 -> claims/cache/factory/dependency ownership
 -> cancellation/replacement policy
 -> state 4 cleanup semantics
 -> owner release / group reset / full reset
 -> CRT/process-lifetime teardown
```

L3 may observe or control unfinished state1/state2 records during cancellation/replacement, but that does not transfer the underlying byte-transport/materialization mechanism into L3. The policy action is L3; the selected-byte terminal condition remains L1.

## 3. Corrected boundary table

| Behavior | Canonical owner |
|---|---|
| Request/candidate/provider/volume selection | L2 |
| Archive normalized lookup / physical provider selection | L2 |
| Provider hit that fails to become a usable selected resource | L2 failure semantics |
| Selected backend/member range read | L1 |
| FileSlot/ReadRequest byte transport | L1 |
| Raw transport callback/status required for byte completion | L1 |
| ZIP STORE/raw-DEFLATE materialization | L1 |
| PAC/PNST parse/nested byte expansion | L1 |
| `.lst` packed-vs-loose representation decision | L1 |
| `.lst` synthesis and recursive child materialization | L1 |
| `0x1402EF4D0` materialization submission/job behavior | L1 boundary |
| Terminal job persistence/poll/retire/fail condition | L1 mandatory open gate |
| Normal `0x1401B8DC0` `state1 -> state2` publication | L1 end boundary |
| Typed post-load and `state2 -> state3` | L3 |
| Loader-node claims/cache/factory ownership | L3 |
| Cancellation/replacement policy `1|2 -> 4` | L3 policy with L1 interaction |
| State4 deferred cleanup / owner release / resets | L3 |
| Runtime/CRT/process teardown | L3 |
| Stage assembly / Stage Ops / ModViz | DOMAIN downstream |
| Corpus hashes / CI / original-process receipts | V cross-cutting |

## 4. L1 completion gates after correction

L1 cannot become `100% / COMPLETE` until both classes below are closed.

### A. Static/materialization-equivalence closure

Mandatory current open gate:

1. close exact `0x1402EF4D0` queued materialization job identity and inherited context consumer;
2. identify its `0x1402EF790` dispatch/persistence/re-poll/retirement behavior;
3. reacquire `0x1400333E0` status/poll semantics;
4. reacquire `0x140033390` terminal cleanup/release ordering;
5. bind `0x1400335A0` transport writes into that terminal state;
6. prove what prevents normal `0x1401B8DC0` dispatch on failed or incomplete transport;
7. recover relevant `0x1402EF460` queued-work suppression/rollback behavior without relabeling it OS `CancelIo`;
8. apply the confirmed terminal mechanism to `.lst` child/recursive failure ordering where required.

No generic fan-in counter is assumed.

### B. Real acceptance closure

Still mandatory:

- direct-retail selected-member provenance;
- exact retail representation classification;
- one real bounded edit/rebuild/rematerialization receipt;
- original protected DMC3 consumption with attributable effect;
- rollback/retail immutability;
- final contradiction-free cross-stack audit.

## 5. Roadmap consequence

Immediate priority is now:

```text
L1 terminal materialization dependency reverse
 -> L1 direct-resource error/completion suppression proof
 -> L1 .lst child/recursive failure propagation using the confirmed mechanism
 -> real-retail L1 provenance/edit/rebuild/rematerialization
 -> Level-E original-game consumption/rollback
 -> final L1 audit
```

L2 real selected-identity evidence and L3 lifecycle receipts continue in parallel only where they provide dependencies for the vertical proof. They do not absorb L1 work.

## 6. Historical-document rule

Historical passes remain evidence history. They are not deleted solely because ownership/status changed.

When historical text conflicts with this reconciliation:

- keep the historical observation;
- mark the old layer/completion interpretation superseded;
- use this document, `decompilation-layer-classification.md`, `master-roadmap.md`, `l1-roadmap.md`, and machine-readable current status as canonical planning authority.

## 7. Completion labels

Allowed current labels:

- `L1 = INCOMPLETE / NOT 100%`;
- `L1 product authoring implementation = advanced / representative path implementation-ready at bounded scope`;
- `L2 = NOT COMPLETE`;
- `L3 = NOT COMPLETE`.

Disallowed current labels:

- `L1 COMPLETE`;
- `L1 100%`;
- `only external receipts remain for L1 completion`;
- `FileSlot/AsyncIO/completion is wholly L3`;
- `.lst packed-first representation selection is L2`;
- `state1->2 completion is wholly L3`.
