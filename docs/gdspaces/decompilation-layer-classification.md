# GDSpaces Decompilation-Layer Classification

**Canonical reconciliation:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`  
**L3 R1 authority:** [`l3-r1-current-main-reconciliation-2026-09-02.md`](l3-r1-current-main-reconciliation-2026-09-02.md)  
**Layer roadmaps:** [`l1-roadmap.md`](l1-roadmap.md) / [`l2-roadmap.md`](l2-roadmap.md) / [`l3-roadmap.md`](l3-roadmap.md)

Layer ownership is semantic and evidence-driven. It is not inferred from product-module ownership, one executable address range, or a filename/format label.

## Canonical tags

### [L2] Resource Resolution

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume/member traversal
 -> ambiguity / provider-failure behavior
 -> exact ResourceRef / ResourceId
```

L2 answers **which resource identity wins**.

### [L1] Resource Materialization

```text
L2 selected identity
 -> representation / extent / allocation
 -> byte acquisition
 -> EOF / short-read / progress semantics
 -> decompression / transforms
 -> exact destination bytes
 -> nested PAC/PNST/.lst construction
 -> ByteProvenance
 -> edit/rebuild/repack/publication
 -> reopen/rematerialization
```

L1 answers **which exact bytes/result are produced and reproduced**.

### [L3] Original Runtime / Resource Lifecycle

```text
L1 byte/result identity
 -> request/scheduler/callback lifetime
 -> LoadedResource state publication
 -> typed post-load / ready visibility
 -> claims / reuse
 -> cancellation / replacement
 -> release / reset / teardown
```

L3 answers **how the original runtime owns and advances the materialized resource through its lifetime**.

### [V] Validation / live observation

Cross-cutting evidence, hashes, CI, original-process observation, same-run/same-resource binding and promotion. V is not a fourth decompilation layer.

### [DOMAIN] Stage Assembly / Stage Ops / ModViz

Downstream semantic/gameplay/editor consumers. They may consume L1/L2/L3 authority but must not invent a second resolver/materializer/lifecycle authority.

### [OUTSIDE]

Historical extraction metadata, presentation names or product-only behavior not established as original runtime authority.

## Behavior matrix

| Behavior | Classification | Current boundary |
|---|---|---|
| logical request/basename candidates | L2 | strong bounded direct-call surface |
| archive `0x0E` normalization/index lookup | L2 | strong bounded static authority |
| physical `0x0C` normalization/final Win32 open | L2 | strong bounded static authority |
| filename discovery / registration attempts | L2 bootstrap evidence | not proof of successful linked mount |
| successful runtime mount topology | L2 | stronger #246 evidence; current-main product correction still open |
| exact selected `ResourceId` | L2 output | L1 input identity seam |
| NBZ/ZIP member span + STORE/raw-DEFLATE bytes | L1 | strong product/read authority; original edge breadth bounded-open |
| PAC/PNST physical slot topology | L1 | strong |
| `physical_slot_index` | L1 identity | physical topology authority |
| `extracted_ordinal` | L1 extraction/naming evidence | not physical slot identity |
| external `.index` | OUTSIDE/L1 naming evidence | historical extraction metadata, not runtime manifest |
| embedded aliases | L1 naming evidence | not lookup/write authority |
| semantic type evidence | classification evidence | does not retarget write identity |
| `.lst` representation/materialized bytes | L1 | representation/byte semantics; remaining dynamic error/lifetime breadth open |
| FileSlot/ReadRequest byte/result mechanics | L1 support | byte/result behavior only |
| FileSlot/ReadRequest object/queue/callback lifetime | L3 support | scheduler/lifetime behavior |
| `0x1401B8CA0` | L1/L3 seam | materializer result influences later lifecycle, not universal exact-all-bytes receipt |
| `0x1402EF4D0` | L1-support + L3 scheduling seam | byte-job admission evidence + scheduler ownership; classify by behavior |
| `0x140033500/0x1400335A0` | L1-support + L3 request-lifetime seam | lower transport/result state, distinct from LoadedResource state2 |
| `0x1401B8DC0 state 1 -> 2` | L3 | normal completion publication |
| `0x1401B92D0 typed post-load -> state3` | L3 | consumer-ready manager lifecycle |
| `1|2 -> 4 -> 0` cancellation path | L3 | static spine strong; dynamic timing breadth open |
| ordinary/group/full release/reset | L3 | distinct policies, not one generic release |
| runtime type registry probe `0x1402DB1F0` | type evidence | three-byte site-scoped registry identity |
| container dispatcher `0x1401B9FA0` | L3 typed post-load evidence | MOD/EFM/SCM/SHW handlers + EFW/EFE sentinels + PNST recursion |
| family-mask classifier `0x1402FD650` | higher runtime type evidence | four-byte MOD/EFM/SCM/MRP/MCV/SHW identity |
| StageBundle/StageAssemblyWorkspace | DOMAIN | downstream consumer |
| Stage Ops/Stage Editor/ModViz | DOMAIN | downstream tooling |

## Strong frozen boundaries

Do not restart absent exact contradictory evidence:

### L2

- bounded direct `OpenGameResource` request/candidate surface;
- archive-before-physical attempt ordering for the recovered direct-call mode;
- archive `0x0E` vs physical `0x0C` normalization distinction;
- normalized archive lookup/index model;
- bounded type-0 physical final-open semantics.

### L1

- PAC/PNST physical relative-slot topology and current product reflow invariants;
- exact physical identity vs naming/presentation separation;
- no-replace publication and artifact-stable provenance principles;
- `.index` is not runtime lookup authority on the recovered path.

### L3

- LoadedResource registry `363 x 0x48` and seven groups;
- R1 `record+0x04` state-writer map, now bounded-closed/contradiction-gated;
- central state spine `0 -> 1 -> 2 -> typed/callback -> 3`;
- cancellation source state `1|2 -> 4` and deferred cleanup;
- quiescence `{0,3}` requirement;
- ordinary/group/full release distinction;
- normal completion one-u32 registry-relative context ABI;
- runtime vs CRT vs process-lifetime teardown distinction.

## Current active reverse targets

### L1

1. original byte/result/failure/width semantics activated by the claimed compatibility scope;
2. recursive `.lst` cycle/depth/allocation/free/error semantics where applicable;
3. final original-L1 contradiction sweep;
4. direct-retail representation/rebuild/rematerialization/game-consumption chain.

### L2

1. port discovery-vs-successful-mount topology into current-main product code;
2. real-retail `0x0E` collision census;
3. real protected-process mapping receipt;
4. trusted selected-provider publisher/origin binding;
5. final L2 audit.

### L3

R1 is no longer active broad discovery.

Active static package is **R2**:

1. family-complete `+0x08/+0x18/+0x20/+0x28` writer/owner census;
2. stable adjacent-field ownership;
3. initialization/finalization/release ordering by family/group;
4. SCM `mesh +0x28` reconciliation;
5. use R3 factory/dependency evidence only where needed to resolve ownership.

Then continue:

- R3 external typed/factory/dependency breadth;
- R4 shared-owner breadth;
- R5 dynamic lifecycle timing;
- current-main lifecycle validator + trusted publisher;
- V1/V5 then remaining V2/V3/V4/V6/V7;
- final L3 audit.

## Cross-boundary rules

- Discovery is not successful mount topology.
- Selection is not materialization.
- Materialization success is not consumer-ready lifecycle.
- State3 is not universal family-semantic success.
- Runtime type identity is not one universal detector/handler/schema.
- A name or semantic suffix is not physical identity or write authority.
- Product safety may be stricter than original unsafe behavior.
- Branch evidence must be semantically ported/reviewed before becoming canonical status.
