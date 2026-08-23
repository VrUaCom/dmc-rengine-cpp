# GDSpaces Layer 1 — Pass 90 — CEm000 resource materialization path

Date: 2026-08-23

Status: evidence checkpoint; Layer 1 remains **NOT COMPLETE**.

## Scope

This pass stays inside **Layer 1 — Resource Materialization**. It traces one concrete DMC3 HD resource path from resource registration/acquisition through VFS/AsyncIO into the runtime container buffer, then from the container slot into `CDraw`, `CPtxManager`, and the original PTX/TIM2 parser.

The concrete runtime descriptor used for the trace is the category-5/index-0 resource descriptor for:

```text
obj\em000.pac
```

No writer authority is promoted by this pass.

## Target and evidence caveat

Canonical target SHA-256 carried by the Phase 12/14/15/16 evidence set:

```text
e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
```

Fresh instruction re-dumps for this pass were taken from the preserved `dmc3_phase17_reng_probe.exe` derivative because a raw canonical `dmc3.exe` was not recoverable from the current Library search. The relevant addresses and control-flow anchors agree with the canonical Phase 12/14/15/16 maps, but a raw canonical-byte re-dump remains a promotion gate.

Accordingly:

- architecture/data-flow claims below are **confirmed/high** against the preserved evidence plus matching derivative instruction ranges;
- exact-byte canonical confirmation is still required before final promotion.

## 1. Resource node lookup is not the loader

`0x1401AE1D0` searches two linked-list domains in the resource manager:

- list head at manager `+0x628`;
- fallback/second list head at manager `+0x610`;
- node key A at `+0x24`;
- node key B at `+0x28`;
- node link at `+0x10`.

The function returns a matching node or null. It does not materialize file bytes.

### Spawn-side consumption

At `0x1401A4680` the caller:

1. performs the node lookup;
2. reads `node+0x18`;
3. passes that value as the resource argument into enemy factory dispatch.

For the CEm000 path the pointer then propagates without replacement through:

```text
CFactoryEnemy.r8
→ CEm000 factory.rdx
→ CEm000 full-object resource field
→ CEm000 runtime consumer
```

## 2. `0x1401AE310` is the node ensure/acquire path

On a cache miss, `0x1401AE310`:

1. finds a free internal node from a 0x30-byte-stride pool;
2. selects a static resource descriptor according to category/index;
3. calls `0x1401B8DF0` to acquire a resource record;
4. stores the returned resource record at `node+0x18`;
5. writes the two lookup keys at `node+0x24/+0x28`;
6. marks/inserts the node into the manager list.

This also corrects an older Phase-12 function-boundary artifact: the relevant function continues beyond the previously recorded `0x1401AE363` cut and reaches at least the descriptor acquisition/insertion region through `0x1401AE58F`.

## 3. CEm000 descriptor identity

For resource category 5, the selector uses the pointer table at:

```text
0x1405B1230
```

Index 0 resolves to descriptor:

```text
0x1405B10B0
```

Observed descriptor fields:

```text
+0x00  uint16 kind = 0
+0x08  const char* logicalPath -> "obj\em000.pac"
```

Therefore the traced CEm000 acquisition is tied to a concrete exact-resource logical path rather than only to a generic enemy-resource class.

## 4. Resource record construction

`0x1401B8DF0` acquires a record from a resource-record pool with 0x48-byte stride and calls `0x1401B84E0` for construction/materialization setup.

Recovered partial record contract:

```text
+0x04  state
+0x10  context/ownership field candidate
+0x18  ResourceTypeInfo / descriptor pointer
+0x20  materialized resource buffer
+0x28  allocator/storage field
```

`0x1401B84E0`:

1. stores the descriptor pointer at record `+0x18`;
2. computes required allocation size;
3. allocates storage;
4. stores the resulting buffer at record `+0x20`;
5. calls `0x1401B8CA0` to populate that buffer;
6. promotes record state on success.

This resolves the earlier apparent ambiguity around `+0x18/+0x20`: descriptor metadata and materialized bytes are distinct fields.

## 5. Exact-resource versus `.lst` fallback

For `kind == 0`, `0x1401B79E0` performs a two-stage source test:

1. test the exact descriptor path with `0x1402EF620`;
2. if absent, replace the extension with `.lst` and test the manifest path.

Recovered result classes:

```text
1 -> exact resource exists
2 -> exact resource absent, .lst fallback exists
0 -> neither source exists
```

`0x1401B7B90` uses that result to compute allocation size, and `0x1401B8CA0` uses the same distinction for population.

### Exact path

When the exact resource exists:

```text
record+0x20 destination
+ descriptor+0x08 logical path
→ 0x1402EF4D0 queued read job
```

No PAC/PTX/DDS/TM2 semantic converter is invoked on this branch.

### `.lst` path

When the exact resource is absent but the `.lst` exists, `0x1401B85C0` is used instead.

Recovered `.lst` behavior is a **manifest-driven container assembly fallback**, not a texture-format conversion:

- the `.lst` is read through the same VFS read layer;
- line-oriented entries are parsed;
- `dummy` entries are treated specially/omitted;
- a PAC-like header identity is synthesized when required;
- child regions are size-accounted/aligned on a 0x40 boundary;
- an existing child `.pac` can be queued directly;
- nested `.lst` entries can recurse back through `0x1401B85C0`.

The full `.lst` grammar and every generated header field remain unresolved, so this pass does not promote a complete `.lst` writer/assembler model.

## 6. Central VFS/AsyncIO bridge

`0x1402EF620` is the synchronous size/probe helper used by the materializer. It:

1. normalizes the logical path;
2. opens it through `0x1400333F0`;
3. obtains sector count through `0x1400333C0`;
4. closes through `0x140033390`;
5. returns sector-padded allocation bytes.

`0x1400333C0` computes:

```text
ceil(logicalSize / 0x800)
```

For actual population, `0x1402EF790` processes the queued exact-path job:

```text
0x1400333F0   open logical resource
→ 0x1400333C0 sector count
→ 0x140033500 submit read into caller destination
→ poll completion
→ 0x140033390 close
```

At `0x140033500`:

```text
requestedBytes = sectorCount << 11
```

and the destination pointer received from the higher materializer is forwarded unchanged into the lower AsyncIO request (`0x14002EA40`). The completion callback updates byte-count/state; it does not perform PAC/PTX/DDS/TM2 conversion.

### Confirmed materialization consequence

For logical bytes through the actual file EOF:

```text
VFS/archive provider bytes
→ AsyncIO read
→ record+0x20
```

There is no intermediate resource-format transformation on the exact-resource branch.

The state of sector padding beyond logical EOF is not promoted as meaningful data by this pass.

## 7. CEm000 container slot to original PTX parser

CEm000 runtime consumption at `0x140097B40` dereferences the materialized record:

- descriptor at record `+0x18`;
- materialized container base at record `+0x20`.

For `kind == 0`, the consumer resolves container offsets directly from the materialized buffer:

```text
slot 0 offset field -> materializedBase + offset -> PTX pointer
slot 1 offset field -> materializedBase + offset -> second resource pointer
```

The slot-0 pointer is passed as `r8` into the real CDraw subobject.

CDraw virtual slot `+0x50` resolves to `0x140089960`. That function forwards the slot-0 pointer unchanged into:

```text
CPtxManager::loadA @ 0x140314E00
```

`0x140314E00` uses the same pointer as cache key and, on miss, passes the same pointer directly to:

```text
PTX parse @ 0x140336BB0
```

No conversion buffer is created between CEm000's slot resolution and the original PTX parser.

## 8. End-to-end exact-resource chain

For the traced CEm000 path the recovered chain is:

```text
Resource descriptor
  kind=0
  path="obj\em000.pac"
        |
        v
0x1401AE310 node ensure
        |
        v
0x1401B8DF0 resource-record acquire
        |
        v
0x1401B84E0 allocate record+0x20
        |
        v
0x1401B79E0 exact-source test
        |
        v
0x1402EF4D0 queue direct read
        |
        v
0x1402EF790
        |
        v
0x1400333F0 VFS open
        |
        v
0x140033500 / 0x14002EA40
AsyncIO directly into record+0x20
        |
        v
CEm000 PAC/PNST slot lookup
        |
        v
slot 0 pointer
        |
        v
CDraw 0x140089960
        |
        v
CPtxManager 0x140314E00
        |
        v
PTX/TIM2 parser 0x140336BB0
```

For the **exact-resource branch**, this closes the previously missing runtime materialization bridge from logical resource bytes to the PTX parser input.

## 9. Consequence for preserved DDS-bearing `em000.pac`

The preserved Phase-16 corpus sample named `em000.pac` has:

```text
container magic: PNST
slot count: 44
whole-file DDS signatures: 19
whole-file TM2 signatures: 0
```

Its texture-like physical slot 0 has:

```text
size: 149504
textureCount candidate: 4
blockCount[]: 22, 22, 22, 6
first entry: +0x800
exact outer-size match: yes
DDS signatures in slot 0: 4
TM2 signatures in slot 0: 0
DDS begins at entry +0x70
```

The runtime chain above proves that, when the exact `obj\em000.pac` source is selected, the slot-0 bytes reach the original PTX parser without a hidden texture conversion between VFS read and `CPtxManager`.

Therefore:

> The preserved DDS-bearing `em000.pac` sample cannot be promoted as pristine parser-ready evidence for the exact canonical `obj\em000.pac` resource consumed by this executable path.

This is a **provenance correction**, not proof of which external tool or pipeline produced the transformed sample.

The strongest supported model is now:

- outer 0x800-sector PTX-like envelope geometry can survive into the transformed corpus;
- per-entry representation in the preserved corpus was transformed from the original parser-ready TIM2 representation or came from a different post-processing/repack pipeline;
- the exact producer, direction, and timing of that transform are still unresolved.

## 10. Promotion boundaries

### Promoted / confirmed-high

- resource-node lookup versus acquisition responsibilities;
- category-5/index-0 descriptor path `obj\em000.pac`;
- resource record `+0x18` descriptor / `+0x20` materialized-buffer split;
- exact source versus `.lst` fallback distinction;
- exact logical resource read reaches `record+0x20` without format conversion;
- CEm000 slot 0 reaches CDraw, CPtxManager, and PTX parser without an intervening conversion;
- `.lst` is a container-assembly fallback domain, not evidence of a TM2↔DDS converter;
- preserved DDS-bearing `em000.pac` must not be treated as pristine exact-resource parser input.

### Still blocked / unresolved

1. Raw canonical `dmc3.exe` byte re-dump at the Pass-90 anchors.
2. Pristine retail extraction of exact logical `obj\em000.pac` directly from the canonical `DMC3-0.nbz`, with source/container hash lineage.
3. Identification of the producer of the preserved DDS-bearing PAC corpus.
4. Full `.lst` grammar, header flags, sparse-slot rules, and exact assembly writer behavior.
5. Reconciliation of PR #172 versus #174 `blockCount == 0` semantics.
6. Original PTX/TIM2 writer and DDS↔TIM2 conversion remain blocked.
7. End-to-end edit → rebuild → repack → game-consumption validation remains blocked.

## Product rule

GDSpaces / Binary Inspector must carry separate representation identities and provenance:

```text
pristine retail bytes
runtime direct materialization
runtime .lst-assembled container
transformed DDS-bearing corpus
```

These must never be silently collapsed into one generic `PTX` or `PAC texture` identity.

## Layer-1 status

**NOT COMPLETE.**

Pass 90 closes the exact-resource runtime read bridge for one concrete CEm000 resource path, but it does not yet provide the pristine retail `obj\em000.pac` receipt, complete `.lst` assembly semantics, or writer/round-trip authority.