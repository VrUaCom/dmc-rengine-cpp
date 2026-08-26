# GDSpaces Layer 3 — raw-EXE boundary audit — 2026-08-26

**Canonical executable:** `dmc3.exe`  
**Size:** `6,356,432` bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**ImageBase:** `0x140000000`  
**EntryPoint:** `0x14034615C`

## Canonical Layer-3 definition

Layer 3 is **Original Runtime / Resource Lifecycle**.

It begins when the original runtime takes lifecycle ownership of an already selected/materialized resource and ends only after the applicable owner/reuse/release/reset/destruction/process-lifetime policy is accounted for.

```text
L2 selected logical/provider identity
 -> L1 exact materialized bytes
 -> [L3 START]
    FileSlot / async ownership and scheduling
    LoadedResource acquisition
    state 0 -> 1
    completion state 1 -> 2
    typed post-load
    optional ready callback
    state 2 -> 3
    state-3 consumer visibility
    loader-node claims/reuse
    cancellation/replacement
    state 1|2 -> 4
    deferred state 4 -> 0 cleanup
    owner release / group reset / full reset
    runtime vs CRT vs process-lifetime teardown
 -> [L3 END]
```

Stage Assembly / Stage Ops is a downstream consumer/domain layer and must not be called Layer 3 in the canonical three-layer GDSpaces model.

## Layer boundaries

### Not L3 — Layer 1

Physical byte acquisition and reproduction:

- NBZ/ZIP/PAC/PNST parsing;
- exact member span selection;
- read/seek/decompression/CRC;
- materialized byte buffers and byte provenance;
- rebuild/repack/round-trip.

FileSlot is a boundary subsystem: its byte-read mechanics can support L1, while request ownership, scheduling, completion, cancellation and close lifetime are L3.

### Not L3 — Layer 2

Logical selection:

- candidate construction;
- provider/source precedence;
- numbered-volume choice;
- archive-vs-physical fallback;
- selected provider identity;
- ambiguity/fallback policy.

Any provider-selection finding discovered while tracing lifecycle remains tagged L2.

### Not L3 — Stage/domain tooling

- StageBundle / StageAssemblyWorkspace;
- SCM geometry assembly;
- collision, triggers/events, lighting, camera, effects/audio domain composition;
- Stage Ops / Stage Editor presentation and UX.

Those consumers receive already selected/materialized/ready resource authority. They must not create a second resolver/materializer/lifecycle authority.

## Raw-EXE lifecycle anchors

### Registry topology

- global LoadedResource registry base: `0x140C99D30`;
- `363` records;
- record stride `0x48`;
- observed group partition `[4,136,60,28,1,128,6]`.

Known fields remain:

- `+0x00` group;
- `+0x04` state;
- `+0x08` family-specific selector/index metadata;
- `+0x10` optional ready callback/context path where applicable;
- `+0x18` descriptor/type authority;
- `+0x20` loaded payload;
- `+0x28` owned/backing subobject region requiring family-complete ownership census.

### Canonical state spine

- `0x1401B84E0` — acquisition construction and successful state `0 -> 1` publication;
- `0x1401B8DC0` — normal completion callback and state `1 -> 2`;
- `0x1401B92D0` — scan/finalize state-2 records, typed post-load, optional ready callback, then state `2 -> 3`;
- `0x1401B8430` — unfinished-resource cancellation/invalidation state `1|2 -> 4`;
- `0x1401B8F00` — deferred state `4 -> 0` cleanup before/around backing release;
- `0x1401B9530` — ordinary owner-driven release to state 0 only when runtime backing release succeeds;
- `0x1401B9560` — group reset;
- `0x1401B95E0` — full 363-record runtime reset.

State 3 is therefore a **consumer-ready lifecycle state after typed post-load and optional callback**, not merely "bytes loaded".

### Typed post-load

Central typed-dispatch/finalization path includes:

- dispatcher `0x1401B9FA0`;
- MOD helper `0x1402FE3B0`;
- EFM helper `0x1402F7A90`;
- SCM helper `0x1403051B0`;
- SHW helper `0x1403204C0`;
- recursive PAC/PNST typed traversal where applicable.

Known typed branches are direct EXE evidence. Remaining work is branch breadth, unknown/default/failure behavior, dependency/factory behavior and the SCM `mesh +0x28` contradiction.

### Shared ownership above LoadedResource

The original runtime does not support one universal `LoadedResource.refCount` model.

Evidence-backed loader-node ownership:

- `0x1401AE220` — claim increment;
- `0x1401AF6A0` — claim decrement;
- `0x1401AF6F0` — zero-claim sweep and underlying release;
- `(kind,id)` is the bounded node identity in the recovered gameplay path.

Remaining work is family/breadth coordination outside already bounded callers and cross-build/profile differences.

## Release and shutdown boundary

L3 contains multiple lifetime layers rather than one symmetric `ResourceRuntime::Shutdown()`.

### Runtime/owner lifetime

- ordinary LoadedResource release;
- cancellation and deferred cleanup;
- group/full resets;
- FileSlot/backend/ZipEntryStream close;
- loader-node zero-claim release.

### CRT/static destruction

The LoadedResource manager is statically constructed and atexit-registered. The registered destruction path destructs manager backing and all 363 record backing subobjects after application return.

This is not equivalent to runtime full reset.

### Process-lifetime infrastructure

Current canonical DMC3 evidence does not recover a normal explicit teardown for all infrastructure:

- NBZ/physical mount linked list;
- FileSlot critical section;
- successfully started lazy AsyncIO manager/thread object.

These are bounded original-lifetime findings, not recommendations for reconstructed-product ownership. DMC Rengine/GDSpaces should use explicit safe lifetime while preserving the original behavior in evidence metadata.

## Important call-graph consequence

Layer 3 is a **semantic/lifetime boundary, not one contiguous EXE address range**.

The central state finalizer has a narrow caller set, while ordinary release/backing primitives have broad caller fan-out across many game owners. Therefore ownership must be classified by caller/state contract, not by assigning an arbitrary `[VA start, VA end]` range to L3.

## Current completion state

### Strong/bounded

- registry topology and seven groups;
- generic 0/1/2/3/4 state spine;
- representative typed post-load families;
- state-3 consumer-ready meaning;
- cancellation `1|2 -> 4 -> 0`;
- ordinary/group/full release/reset distinction;
- loader-node claim model for bounded gameplay families;
- runtime vs CRT vs process-lifetime teardown distinction.

### Still mandatory

1. whole-image alias-aware census of every `LoadedResource +0x04` writer and caller context;
2. family-complete writer/owner census for `+0x08/+0x18/+0x20/+0x28` and remaining stable fields;
3. exhaustive typed dispatcher branch/key/default/failure census;
4. SCM `mesh +0x28` reconciliation;
5. shared-owner breadth outside currently bounded loader-node families;
6. allocation/error/cancellation edge paths;
7. cross-build/profile differences;
8. original-process V1–V7 lifecycle receipts;
9. final contradiction-free L3 audit.

## Acceptance rule

A static EXE pass can close static boundaries but cannot by itself mark Layer 3 complete.

Layer 3 is complete only when static reverse and original-process receipts jointly prove, at required breadth:

```text
L2 selected provider identity
 + L1 exact materialized byte identity
 + L3 ordered lifecycle ownership/ready/use/release evidence
```

No Stage Ops UI success, crash-free launch, synthetic trace or manually edited JSON is sufficient promotion evidence.
