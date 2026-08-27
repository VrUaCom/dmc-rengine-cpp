# GDSpaces Decompilation-Layer Classification

**Canonical reconciliation:** 2026-08-27  
**Base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Boundary/status authority:** [`layer-boundary-status-reconciliation-2026-08-27.md`](layer-boundary-status-reconciliation-2026-08-27.md)  
**Materialization completion authority:** [`materialization-completion-boundary-pass-2026-08-26.md`](materialization-completion-boundary-pass-2026-08-26.md) + [`materialization-completion-dependency-pass2-2026-08-26.md`](materialization-completion-dependency-pass2-2026-08-26.md)

Layer ownership is semantic and evidence-driven. A helper, queue, object or EXE address is not assigned wholesale to a layer merely because it sits near lifecycle or resolver code. Classify each behavior by the question it answers.

## Canonical tags

### [L2] Resource Resolution

Question:

> Which logical resource/provider/source/member is selected?

```text
logical request
 -> candidate construction
 -> path normalization
 -> provider/source/volume traversal
 -> duplicate/ambiguity/fallback/failure classification
 -> successful selected ResourceRef/provider/member identity
```

L2 ends at successful usable selection. A lookup hit whose provider wrapper/open creation fails remains L2 selection/failure semantics because no usable selected resource exists yet.

### [L1] Resource Materialization

Question:

> How do selected resource bytes become exact materialized bytes, and how are those bytes reproduced/edited/rebuilt?

```text
selected provider/member identity
 -> selected backend/member range acquisition
 -> FileSlot/ReadRequest sync-or-async byte transport required by materialization
 -> transport completion/status needed for materialization success
 -> transform/decompression
 -> caller-owned destination population
 -> packed OR loose-list representation materialization
 -> nested PAC/PNST/.lst construction
 -> terminal materialization dependency/error suppression
 -> normal state 1 -> 2 publication
 -> exact materialized bytes + provenance
 -> WorkingCopy/edit
 -> rebuild/repack/publication
 -> reopen/rematerialization
```

L1 is not closed by lookup, enumeration, structural parsing, green synthetic CI or a product writer alone.

### [L3] Original Runtime / Lifecycle

Question:

> What happens after materialized bytes exist: typed normalization, ready visibility, ownership, reuse, cancellation policy, release and teardown?

```text
state 2 / materialized bytes complete
 -> typed post-load
 -> optional ready callback
 -> state 2 -> 3
 -> state-3 consumer visibility
 -> claims/cache/factory/dependency ownership
 -> cancellation/replacement policy
 -> state4 cleanup semantics
 -> owner release / group reset / full reset
 -> CRT/process-lifetime teardown
```

L3 can act on unfinished state1/state2 records during cancellation/replacement. That policy interaction does not move selected-byte transport or the materialization terminal dependency into L3.

### [V] Validation

Cross-cutting hashes, corpus receipts, CI, original-vs-reconstruction comparison and original-game execution. Validation is not a fourth decompilation layer.

### [DOMAIN] Stage Assembly / Stage Ops / ModViz

Stage/room semantic assembly, geometry/collision/camera/lighting/events/effects/audio relationships, Stage Ops and ModViz are downstream consumers of resource authority.

They are not L3 and must not create private resolver/materializer/lifecycle authority.

### [OUTSIDE]

Product/extraction metadata or tooling information not established as original DMC3 runtime behavior.

## Current classification matrix

| Area / behavior | Canonical layer | Boundary |
|---|---|---|
| NBZ local/central/EOCD structure and physical stored spans | L1 | byte acquisition/serialization |
| Archive normalized lookup / qsort/bsearch identity | L2 | selected member identity |
| Type-0 physical provider candidate/root/open selection | L2 | usable provider selection/failure |
| Selected archive/physical backend range read | L1 | byte acquisition starts after usable selection |
| STORE/raw-DEFLATE member materialization | L1 | exact bytes |
| FileSlot/ReadRequest selected-byte transport | L1 | sync/async transport required for materialization |
| `0x140033500` whole-file transfer submission | L1 | caller-owned destination transport |
| `0x1400335A0` whole-file transport completion/status | L1 | raw transport terminal information |
| `0x1402EF4D0` materialization submission/job creation | L1 boundary | exact job semantics still open |
| `0x1402EF790` materialization-job dispatch/persistence behavior | L1 boundary | exact persistence/re-poll/retirement still open |
| `0x1400333E0` terminal status/poll candidate | L1 OPEN | exact semantics require reacquisition |
| `0x140033390` terminal load-state release/close candidate | L1 OPEN | exact semantics require reacquisition |
| materialization completion ordering/dependency bridge | L1 MANDATORY OPEN | no generic fan-in counter evidenced |
| normal `0x1401B8DC0` `state1 -> state2` publication | L1 end boundary | callback receives registry-relative context only |
| PAC/PNST relative-slot parsing | L1 | exact materialized topology |
| recursive PAC/PNST expansion | L1 | nested bytes |
| bounded PAC/PNST reflow/reintegration | L1 | product authoring at evidenced scope |
| `.lst` packed-first-vs-loose representation choice | L1 | representation of same selected resource identity |
| `.lst` synthesized bytes / recursive children | L1 | materialization |
| `DMC3-N.nbz` bootstrap/first-gap/precedence | L2 | source/volume selection |
| request basename/six-prefix candidates | L2 | request policy |
| provider/backend failure before usable selected resource | L2 | selection failure, not byte materialization |
| typed MOD/EFM/SCM/SHW post-load | L3 | begins from state2 bytes |
| `0x1401B92D0` typed finalizer / optional callback / state3 | L3 | ready publication |
| loader-node claims/cache/factory ownership | L3 | shared ownership above LoadedResource |
| cancellation policy `state1|state2 -> state4` | L3 policy + L1 interaction | invalidates unfinished materialization but does not own its transport |
| `0x1402EF460` pending scheduler-entry clear/rollback | semantic seam | classify exact action by target: L1 completion suppression vs L3 cancellation policy |
| quiescence predicate | L3 | replacement/lifecycle coordination |
| state4 deferred cleanup | L3 | lifecycle cleanup |
| ordinary/group/full release/reset | L3 | ownership/lifecycle |
| runtime vs CRT vs process-lifetime teardown | L3 | lifecycle |
| archive/member provenance stability | L1 + V | materialization evidence |
| protected selected-provider trace | L2 + V | original selection evidence |
| dynamic typed-ready/use/release trace | L3 + V | original lifecycle evidence |
| StageBundle / StageAssemblyWorkspace | DOMAIN | downstream stage consumer |
| Stage Ops / Stage Editor / ModViz | DOMAIN | downstream tooling |
| `.index` manifests | OUTSIDE | metadata, not recovered original lookup authority |
| binary AFS backend | evidence-gated | not established by `.afs/` namespace strings |
| PACK original runtime use | evidence-gated | historical product parser is not original-runtime proof |

## Function-boundary guidance

### L2 strong boundaries

Do not reopen absent contradiction:

- bootstrap / numbered-volume registration around `0x14002E930`;
- `OpenGameResource 0x14002FCA0` basename/six-prefix/provider traversal;
- `ResourceMountResolve 0x140327430` provider selection;
- normalization: archive `0x0E`, physical `0x0C`;
- archive normalized index/sort/search;
- bounded type-0 static final-open chain at the recovered direct-call scope.

### L1 strong boundaries

Do not reopen absent contradiction:

- whole-file caller-owned destination transport around `0x140033500/0x1400335A0`;
- FileSlot/ReadRequest transport architecture at bounded recovered scope;
- `ZipEntryRead 0x140328F50` STORE-vs-compressed split;
- `InflateRead 0x140328820` raw-DEFLATE streaming behavior;
- PAC/PNST relative-slot structure and recursive expansion;
- `.lst` grammar/layout/synthesis structure;
- normal `0x1401B8DC0` callback ABI/context and state2 publication.

### L1 mandatory open boundary

The exact terminal dependency between submitted materialization work and normal state2 publication remains open:

```text
0x1402EF4D0 job/submission
 -> lower transport
 -> terminal success/error state
 -> job persistence/re-poll/retirement OR another gate
 -> failed/incomplete completion suppression
 -> normal 0x1401B8DC0
```

Merged evidence rejects a generic fan-in-counter assumption. FIFO order alone is not sufficient unless the earlier materialization job is completion-aware.

Focused targets:

1. `0x1402EF4D0` exact queued job identity/type and inherited load-context consumer;
2. matching `0x1402EF790` dispatch/persistence/re-poll/retirement behavior;
3. `0x1400333E0` pending/success/error domain;
4. `0x140033390` terminal cleanup/release ordering;
5. `0x1400335A0` transport writes into that domain;
6. failed/incomplete suppression before normal `0x1401B8DC0`;
7. relevant `0x1402EF460` pending-entry clear/rollback behavior;
8. `.lst` child/recursive failure ordering after the direct-resource terminal model is closed.

### L3 strong boundaries

Do not restart absent contradiction:

- LoadedResource registry `363 x 0x48`, seven groups;
- state2 typed finalizer `0x1401B92D0` and state3 ready publication;
- cancellation policy `0x1401B8430`: states1/2 -> state4;
- quiescence `0x1401B84B0`: all records in `{0,3}`;
- state4 cleanup `0x1401B8F00`;
- ordinary release `0x1401B9530`;
- group/full reset `0x1401B9560/0x1401B95E0`;
- typed dispatcher `0x1401B9FA0` and representative typed families;
- loader claim/release `0x1401AE220`, `0x1401AF6A0`, `0x1401AF6F0`;
- runtime backing release vs CRT destructor distinction.

The existence of state fields 0/1/2 does not make their entire acquisition/materialization path L3. State2 is the handoff boundary: L1 proves materialized-byte completion; L3 begins typed-ready lifecycle from that completed state.

## Cross-boundary rules

- Selection/identity is L2; transfer/transform of selected bytes is L1.
- Provider failure before a usable selected resource exists is L2 failure semantics.
- `.lst` representation choice is L1, because it selects how the same resource identity becomes bytes.
- FileSlot/ReadRequest selected-byte transfer is L1; service/pool/shutdown ownership outside byte correctness is L3.
- Normal state1->2 completion is the L1 end boundary.
- Typed post-load/state2->3 is L3.
- L3 cancellation policy may suppress L1 completion; classify policy and byte mechanism separately.
- A shared scheduler helper is not automatically L3; classify the specific queued action.
- Stage Ops consumes L1/L2/L3 authority and is not one of these resource-runtime layers.
- Product hardening does not become original-game behavior automatically.
- Writer compatibility does not prove Capcom offline-writer equivalence.

## Current priority accounting

```text
L1 -> mandatory materialization terminal-dependency reverse + real-retail/edit/rebuild/rematerialization/Level-E receipts
L2 -> retail collision evidence + real protected mapping + trusted selected identity + final audit
L3 -> residual static ownership breadth + original-process typed-ready/transition/cancellation/release/shutdown receipts
```

All three layers are currently **INCOMPLETE / NOT 100%**. Progress in one layer must never be reported as completion of another.
