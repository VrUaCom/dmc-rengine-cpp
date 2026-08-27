# GDSpaces Layer 3 — raw-EXE boundary audit — 2026-08-26

**Canonical executable:** `dmc3.exe`  
**Size:** `6,356,432` bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**ImageBase:** `0x140000000`  
**EntryPoint:** `0x14034615C`  
**Ownership reconciliation:** 2026-08-27 — see `layer-boundary-status-reconciliation-2026-08-27.md`

> **Supersession note:** the raw EXE findings in this audit remain valid unless separately contradicted, but the earlier layer interpretation that placed FileSlot/AsyncIO transport and the normal `state 1 -> 2` materialization completion wholly inside L3 is superseded. Selected-byte transport/completion through state2 is now canonically L1. L3 begins from completed state2/materialized bytes for typed-ready and lifecycle semantics.

## Canonical Layer-3 definition

Layer 3 is **Original Runtime / Resource Lifecycle after materialization**.

It begins when exact materialized bytes have reached the recovered state2 boundary and continues through typed normalization, ready visibility, shared ownership, cancellation/replacement policy, release/reset and teardown.

```text
[L2] selected logical/provider/member identity
 -> [L1] selected-byte acquisition / FileSlot transport / transform
 -> [L1] terminal materialization dependency
 -> [L1] normal state 1 -> 2 publication
 -> [L3 START]
    state2 materialized bytes
    typed post-load
    optional ready callback
    state 2 -> 3
    state-3 consumer visibility
    loader-node claims/reuse
    cancellation/replacement policy
    state 1|2 -> 4 policy action on unfinished records
    deferred state 4 -> 0 cleanup
    owner release / group reset / full reset
    runtime vs CRT vs process-lifetime teardown
 -> [L3 END]
```

The fact that L3 cancellation policy can target unfinished state1/state2 records does not make the underlying byte transport or completion dependency L3. Policy ownership and byte-mechanism ownership are classified separately.

Stage Assembly / Stage Ops / ModViz remains downstream DOMAIN work and is not L3.

## Layer boundaries

### Not L3 — Layer 1

The following are now canonically L1:

- selected archive/physical backend byte acquisition;
- FileSlot/ReadRequest byte transport required for materialization;
- whole-file transfer submission `0x140033500`;
- whole-file transport completion/status `0x1400335A0`;
- STORE/raw-DEFLATE transforms;
- caller-owned destination population;
- PAC/PNST nested byte expansion;
- `.lst` packed-vs-loose representation materialization and recursive synthesis;
- materialization scheduler/job behavior needed to determine terminal success/error;
- failed/incomplete completion suppression;
- normal `0x1401B8DC0` `state1 -> state2` publication as the end-of-materialization boundary;
- byte provenance and rebuild/repack/rematerialization.

FileSlot/AsyncIO **service lifetime**, pool lifetime, shutdown ownership and broader runtime infrastructure may still be L3. The selected-byte transfer itself is L1.

### Not L3 — Layer 2

L2 owns logical selection:

- candidate construction;
- provider/source precedence;
- numbered-volume choice;
- archive-vs-physical fallback;
- normalization/ambiguity;
- selected provider/member identity;
- provider/open failure before a usable selected resource exists.

Any provider-selection finding discovered while tracing lifecycle remains L2.

### Not L3 — Stage/domain tooling

- StageBundle / StageAssemblyWorkspace;
- SCM geometry assembly;
- collision, triggers/events, lighting, camera, effects/audio composition;
- Stage Ops / Stage Editor / ModViz.

These are downstream consumers and must not create independent resolver/materializer/lifecycle authority.

## Raw-EXE lifecycle anchors

### Registry topology

- global LoadedResource registry base: `0x140C99D30`;
- `363` records;
- record stride `0x48`;
- observed seven-group partition `[4,136,60,28,1,128,6]`.

Known fields remain:

- `+0x00` group;
- `+0x04` state;
- `+0x08` family-specific selector/index metadata;
- `+0x10` optional ready callback/context path where applicable;
- `+0x18` descriptor/type authority;
- `+0x20` materialized payload;
- `+0x28` owned/backing subobject region requiring family-complete ownership census.

### State spine — corrected ownership split

The raw state transitions remain evidenced; only their layer ownership is corrected.

#### L1 materialization boundary

- `0x1401B84E0` — cross-layer acquisition constructor; backing/destination setup and materialization start are L1, while the record participates in lifecycle state;
- successful state `0 -> 1` publication is acquisition bookkeeping tied to materialization start and is treated as the L1/L3 boundary state;
- `0x1401B8DC0` — normal completion callback and `state 1 -> 2`; the callback ABI/context is strong, but the exact terminal dependency that permits its dispatch remains an **L1 mandatory open gate**.

#### L3 ready/lifecycle path

- `0x1401B92D0` — scans state2 records, performs typed post-load, invokes optional ready callback and writes state3;
- `0x1401B8430` — cancellation/replacement policy marks unfinished state1/state2 records state4;
- `0x1401B8F00` — deferred state4 cleanup to state0 and backing release path;
- `0x1401B9530` — ordinary owner-driven release to state0 only when runtime backing release succeeds;
- `0x1401B9560` — group reset;
- `0x1401B95E0` — full 363-record reset.

State3 remains a **consumer-ready lifecycle state after typed post-load and optional callback**, not merely bytes loaded.

## Typed post-load

Central typed-dispatch/finalization path includes:

- dispatcher `0x1401B9FA0`;
- MOD helper `0x1402FE3B0`;
- EFM helper `0x1402F7A90`;
- SCM helper `0x1403051B0`;
- SHW helper `0x1403204C0`;
- recursive PNST traversal where applicable.

These are L3 because they consume completed materialized bytes and prepare consumer-ready runtime state.

Known branches are direct EXE evidence. Remaining work includes branch breadth, external factory/dependency failure behavior, SCM `mesh +0x28` reconciliation and profile differences.

## Shared ownership above LoadedResource

The original runtime does not support one universal `LoadedResource.refCount` model.

Evidence-backed loader-node ownership remains:

- `0x1401AE220` — claim increment;
- `0x1401AF6A0` — claim decrement;
- `0x1401AF6F0` — zero-claim sweep and underlying release;
- `(kind,id)` is the bounded node identity in the recovered gameplay path.

This is L3 shared ownership after resources are ready/retained. Family breadth remains open.

## Cancellation interaction with L1

`0x1401B8430` and state4 policy are L3. They may suppress/remove unfinished work that would otherwise lead to L1 completion.

Therefore cancellation must be described as an interaction:

```text
[L3] replacement/cancel policy decides unfinished resource is invalid
 -> [seam] pending scheduler entry may be cleared/rolled back
 -> [L1] failed/incomplete materialization must not publish normal state2
 -> [L3] state4 cleanup/release returns lifecycle state toward 0
```

`0x1402EF460` remains safely labeled **pending scheduler-entry clear/rollback** until exact target semantics are closed. If it removes normal materialization completion eligibility, that action is L1 completion suppression driven by L3 policy. It is not automatically OS AsyncIO cancellation.

## Release and shutdown boundary

L3 contains multiple lifetime layers rather than one symmetric `ResourceRuntime::Shutdown()`.

### Runtime/owner lifetime

- ordinary LoadedResource release;
- cancellation and deferred cleanup;
- group/full resets;
- loader-node zero-claim release;
- ready-resource owner/family lifecycle.

### Transport/backend close nuance

Closing a FileSlot/backend as part of a specific selected-byte materialization operation can be L1 terminal cleanup. Global service/pool/resource-runtime lifetime for FileSlot/AsyncIO remains L3. Classify the concrete action, not the helper name.

### CRT/static destruction

The LoadedResource manager is statically constructed and atexit-registered. The registered path destructs manager backing and all 363 record backing subobjects after application return. This is not equivalent to runtime full reset.

### Process-lifetime infrastructure

Current canonical DMC3 evidence does not recover normal explicit teardown for all infrastructure, including:

- NBZ/physical mount linked list;
- FileSlot critical section;
- successfully started lazy AsyncIO manager/thread object.

These are bounded original-lifetime findings, not recommendations for reconstructed-product ownership.

## Important call-graph consequence

L3 is a **semantic/lifetime boundary, not one contiguous EXE address range**.

The same scheduler/allocator/helper family may contain L1 materialization actions and L3 lifecycle actions. Each writer/caller must be classified by what it does to selected bytes versus post-materialization ownership.

## Current L3 completion state

### Strong/bounded

- registry topology and seven groups;
- state2 -> typed post-load -> optional callback -> state3 ready path;
- representative typed post-load families;
- state3 consumer-ready meaning;
- cancellation policy `1|2 -> 4`;
- quiescence predicate `{0,3}`;
- state4 deferred cleanup;
- ordinary/group/full release/reset distinction;
- loader-node claim model for bounded gameplay families;
- runtime vs CRT vs process-lifetime teardown distinction.

### Still mandatory

1. residual alias-aware state writer/value-flow census outside already bounded paths;
2. family-complete writer/owner census for `+0x08/+0x18/+0x20/+0x28` and remaining stable fields;
3. external typed/factory/dependency failure breadth;
4. SCM `mesh +0x28` reconciliation;
5. shared-owner breadth outside bounded loader-node families;
6. cancellation/replacement interactions after the L1 terminal mechanism is closed;
7. cross-build/profile differences;
8. original-process V1–V7 lifecycle receipts;
9. final contradiction-free L3 audit.

The unresolved materialization terminal dependency itself is **L1**, not an L3 completion item.

## Acceptance rule

A static EXE pass cannot by itself mark L3 complete.

L3 is complete only when static reverse and original-process receipts jointly prove, at required breadth:

```text
L2 selected provider/member identity
 + L1 completed state2 materialized-byte identity
 + L3 ordered typed-ready/ownership/use/release evidence
```

No Stage Ops success, crash-free launch, synthetic trace or manually edited JSON is sufficient promotion evidence.

**Current status: L3 INCOMPLETE / NOT 100%.**
