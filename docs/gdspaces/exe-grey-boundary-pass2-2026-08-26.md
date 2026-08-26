# GDSpaces EXE Grey-Boundary Audit — Pass 2: Resource Control Plane — 2026-08-26

**Tracking:** #225  
**Extends:** `exe-grey-boundary-audit-2026-08-26.md`  
**Current canonical raw-EXE authority:** PR #224 / `l3-boundary-audit-2026-08-26.md` / `l3-raw-exe-pass-2026-08-26.md`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.

## Audit question

Pass 1 found multiple grey boundaries but no L4. Pass 2 asks a stronger architectural question:

> Do the request-origin, dependency, claim, pending/ready, transition and replacement behaviors collectively form an orthogonal **Resource Control Plane** that should be modeled explicitly even though it is not a new resource layer?

## Evidence classes used by this pass

This document deliberately separates authority strength.

### C — current canonical authority

Directly reconciled against the current canonical raw EXE or merged current-main reverse docs:

- `OpenGameResource` direct caller census and `flags=1` direct-call surface;
- LoadedResource state/lifetime spine;
- loader-node `(kind,id)` claim/release/sweep boundary;
- cancellation/quiescence/replacement linkage;
- central typed dispatcher best-effort/default no-op behavior;
- L1/L3 seam at `0x1401B8CA0`;
- state3 finalizer ordering.

### H — historical strong evidence requiring current reconfirmation

Direct-EXE Wave-3 research represented in closed/unmerged PR #84 and issue #55 history:

- StageCfg-driven dependency preload;
- owner-local 32-node `(group,index)` ready/pending cache with claim/refcount-like field at `+0x2C`;
- seven resource domains;
- player/enemy factory and external enemy resource-set mapping;
- root/nested scene-manager behavior;
- 4,039-entry resource-name catalog and numeric semantic resolver;
- central memory arena/registry anchors.

These are useful acquisition targets but are **not silently promoted to current-main canonical authority by this audit**.

---

# 1. Two orthogonal runtime planes

The best current architecture is not “four layers”. It is three ownership layers plus an orthogonal control plane.

## 1.1 Resource data path

```text
[L2] logical request -> selected provider/member identity
  -> [L1] exact stored/materialized bytes
  -> [L3A] typed processing / manager-ready transition
  -> [L3B] runtime ownership/lifetime
```

The data path answers:

- which resource wins;
- which bytes exist;
- how those bytes enter runtime ownership and become manager-ready;
- how the resulting live resource is retained/released.

## 1.2 Resource Control Plane (RCP)

```text
consumer/scene intent
 -> root request planning
 -> dependency planning
 -> request emission
 -> pending/ready coordination
 -> owner claims / retention
 -> transition cancellation / quiescence / replacement
 -> release/reset policy
```

The RCP answers:

- **why** a resource load exists;
- **when** a child/dependency load must be emitted;
- **which load set** must be complete before progress continues;
- **who** claims/retains an already loaded resource;
- **when** a transition cancels, waits, replaces or preserves resource ownership.

### Pass-2 verdict

**RESOURCE CONTROL PLANE IS A USEFUL AND EVIDENCE-SUPPORTED ORTHOGONAL MODEL. IT IS NOT L4.**

RCP behavior delegates actual selection to L2, bytes to L1 and live-resource state/ownership primitives to L3. It orchestrates those layers rather than replacing them.

---

# 2. Current canonical RCP evidence

## 2.1 Root request ingress remains outside L2

Current direct-call census for `OpenGameResource @ 0x14002FCA0` identifies three direct calls:

- `0x14003340A`;
- `0x1403380C7`;
- `0x1403381F7`.

All pass `EDX = 1`.

This proves the bounded direct-call resolver mode but does **not** yet classify the complete upstream reason/context of each caller.

### Consequence

The next reverse should walk **upstream from these three callers** and classify:

- generic engine/system ingress;
- media/resource catalog ingress;
- gameplay/stage/scene ingress;
- any wrapper that converts selector/resource-set identity into a logical path request.

Until that census is closed, keep `Consumer / Request Ingress` outside core L2 rather than inventing L0.

---

## 2.2 Loader-node claims are control-plane ownership above LoadedResource

Current raw authority reconfirms:

- `0x1401AE220` — lookup by `(kind,id)` + claim increment;
- `0x1401AF6A0` — matching lookup + claim decrement;
- `0x1401AF6F0` — zero-claim sweep into underlying release.

This is not the 363-record LoadedResource state machine itself and is not a universal resource refcount.

### Architectural meaning

There are at least two current L3B ownership levels:

```text
L3B.1 — live resource record/backing lifetime
L3B.2 — higher-level owner/loader claim coordination
```

This is a subdomain hierarchy, not a new top-level layer.

---

## 2.3 Transition control is not just “release code”

Current raw pass directly links higher-level transition code with:

```text
mark transition/change state
 -> global unfinished-resource cancellation 0x1401B8430
 -> wait for quiescence 0x1401B84B0
 -> cleanup/replacement progression
```

Quiescence requires every LoadedResource record to be in `{0,3}`. Cancellation changes only `1|2 -> 4` on the canonical global path.

### Architectural meaning

The transition layer controls **when the data/lifecycle path is allowed to progress**, so it belongs to RCP orchestration while the state writers themselves remain L3B primitives.

---

# 3. Readiness is not universal semantic-success authority

This is the most important Pass-2 correction.

Current raw EXE proves:

```text
state2
 -> central typed dispatcher 0x1401B9FA0
 -> optional ready callback
 -> state3
```

But `0x1401B9FA0` is best-effort/void at the central boundary:

- null input returns;
- known magics dispatch to known handlers;
- unknown/unrecognized magic falls through/no-ops;
- no dispatcher failure result is consumed by the state2 finalizer before state3 publication.

## Consequence

`state3` proves a **manager/lifecycle-ready boundary**, not a universal statement:

> “every family-specific semantic object/factory/dependency was successfully and completely constructed.”

Family-internal helper failure and external factory/dependency failure semantics remain open.

### New terminology rule

Prefer:

- `manager_ready_state3` for the generic LoadedResource boundary;
- `family_semantic_ready` only when family-specific evidence proves the stronger condition;
- `consumer_effect_observed` only when LV/V observes the actual downstream consumer/effect.

Do not collapse these three states.

### V implication

A V-D/V-E receipt must not promote semantic equivalence solely because the record reached state3 when the tested family has additional construction/dependency semantics.

---

# 4. Historical dependency evidence strongly supports RCP, but requires reconfirmation

Wave-3 historical direct-EXE research recorded the following StageCfg-driven flow:

```text
stage cfg
 -> READY
 -> scan cfg records
 -> extract enemy IDs
 -> map enemy IDs to resource-set selectors
 -> deduplicate
 -> preload enemy object PAC + enemy sound PAC
 -> schedule stage script + stage effect
 -> wait pending dependencies
```

This is exactly the behavior expected of a resource control plane: a ready resource becomes an **input to future request planning**, not merely a terminal consumer payload.

However PR #84 was closed without merge. Therefore Pass 2 classifies this as:

**H — strong historical direct-EXE evidence / current raw reconfirmation required.**

### Acquisition/reverse target

Reacquire the StageCfg dependency-preload caller chain from the canonical raw EXE and record:

1. exact caller/function boundaries;
2. exact CFG record fields used to derive enemy IDs;
3. exact enemy-ID -> resource-set mapping authority;
4. deduplication key and capacity;
5. emitted object/sound request identities;
6. pending/ready wait condition;
7. script/effect scheduling order relative to dependency completion;
8. release/transition ownership for the dependency set.

Only then promote it from H to C.

---

# 5. Identity is a cross-layer chain, not one ResourceId

Pass 2 finds that several different identity domains participate in one load:

```text
consumer/scene intent
 -> selector / resource-set identity
 -> type/descriptor authority
 -> logical request
 -> provider candidate
 -> selected archive/physical identity
 -> selected member/path identity
 -> exact L1 byte identity
 -> LoadedResource record identity
 -> loader-node (kind,id) identity
 -> family object / consumer identity
```

These identities must not be collapsed merely because some maps are deterministic.

## Current strong pieces

- L2 explicitly separates logical request, provider candidate and selected provider/member identity.
- L3 record field `+0x18` carries descriptor/type authority on acquisition.
- loader-node identity is bounded as `(kind,id)` for the recovered gameplay path.
- V already requires same-run/same-resource binding at the parent receipt level.

## Recommended formal model

Create an **Identity Chain / Identity Plane** in recovered evidence and V metadata.

It is not a layer. It is a mapping graph across layers.

For each edge record:

- source identity kind/value;
- destination identity kind/value;
- function/VA or runtime event establishing the mapping;
- evidence class/confidence;
- run/artifact binding when dynamic.

### High-value EXE work

Build a TypeInfo/descriptor xref census that connects:

```text
request construction / L2 use
 <-> descriptor/type table
 <-> LoadedResource +0x18 / L3 use
 <-> loader/family consumer
```

This is more useful than trying to assign the type table itself to one layer.

---

# 6. Factory/resource-set mapping is another RCP ingress class

Historical Wave-3 evidence records:

- player factory boundary;
- 46-slot enemy factory with known null/shared construction cases;
- separate 64-entry external enemy mapping that feeds both class selection and resource-set selection;
- examples pairing enemy resource-set selectors with object PAC + sound PAC requests.

These mappings show that **semantic/gameplay identity can be converted into resource-set demand before L2 path selection**.

Status remains H until current raw reconciliation.

### Pass-2 classification

Factory object construction itself is a gameplay/runtime subsystem. The **resource-demand mapping edge** belongs to RCP/Ingress accounting because it emits resource requirements into L2.

Do not move player/enemy gameplay factories into GDSpaces.

---

# 7. RCP is graph-shaped, not stack-shaped

The combined model is:

```text
                    ┌─────────────────────────────┐
                    │  Resource Control Plane     │
                    │                             │
Consumer intent ───>│ root plan / request emit   │
                    │ dependency planning        │
                    │ pending/ready coordination │
                    │ claims / retention         │
                    │ transition / replacement   │
                    └───────┬───────────▲─────────┘
                            │           │
                            v           │ feedback/status
                           L2           │
                            │           │
                            v           │
                           L1           │
                            │           │
                            v           │
                           L3A ─────────┘
                            │
                            v
                           L3B
                            │
                            v
                         consumer
```

The data path remains authoritative for the actual selection/bytes/runtime lifecycle. RCP owns orchestration relationships.

---

# 8. Memory/backing is not the missing layer

Current raw pass shows generic backing release `0x140337710` has very broad fan-out and cannot be assigned wholesale to L3 by address. Runtime release and CRT destructor are different.

Historical evidence also records a central memory arena/registry.

### Verdict

Memory remains a **substrate** (`MEM/BACKING`), not RCP and not L4.

The key question is ownership context, not allocator address.

---

# 9. V/LV must become dependency-graph aware

The current parent V architecture already rejects unrelated L1/L2/L3 receipts. Pass 2 adds one requirement for breadth.

For a non-dependent bounded resource, one vertical chain is enough:

```text
request -> L2 -> L1 -> L3 -> consumer/effect
```

For a dependency-bearing resource, V must support a DAG/graph:

```text
root validation_run_id
  root request
    -> root L2 identity
    -> root L1 bytes
    -> root L3A event
    -> dependency edges[]
         child request
           -> child L2 identity
           -> child L1 bytes
           -> child L3A manager-ready/family-ready state
    -> dependency barrier satisfied
    -> root consumer/effect
    -> L3B ownership/release graph
```

Every child must remain bound to:

- the same original-process run/session or an explicitly allowed correlated session contract;
- exact executable authority;
- exact root dependency edge;
- exact selected provider/member and byte identity.

## Recommended LV event vocabulary

Future trusted LV instrumentation should distinguish at minimum:

- `root_request_emitted`;
- `dependency_discovered`;
- `dependency_request_emitted`;
- `provider_selected`;
- `bytes_materialized`;
- `loaded_resource_state_write`;
- `typed_dispatch_enter/exit`;
- `manager_ready_state3`;
- `family_semantic_ready` when directly observable/proven;
- `loader_claim_add/remove`;
- `dependency_barrier_satisfied`;
- `consumer_effect_observed`;
- `transition_cancel`;
- `quiescence_observed`;
- `release/reset`.

Do not synthesize family-semantic events merely from state3.

---

# 10. Pass-2 decision matrix

| Candidate | Evidence | Decision |
|---|---|---|
| L4 | No independent stable contract demonstrated | **REJECT / NOT JUSTIFIED** |
| L0 before L2 | Request origin is real, generic ABI not yet proven | **DEFER; call Consumer/Request Ingress** |
| L3A Typed/Dependency | Strong phase separation | **ADOPT as accounting subdomain** |
| L3B Ownership/Lifecycle | Strong current raw evidence | **ADOPT as accounting subdomain** |
| Resource Control Plane | Multiple current control edges + historical dependency evidence | **ADOPT as orthogonal architecture plane** |
| Identity Plane | Multiple distinct mapped identities across L2/L1/L3/consumer | **ADOPT as cross-layer evidence model** |
| RT-IO | Mixed L1/L3 behavior | **TAGGED SUBSTRATE** |
| MEM/BACKING | Mixed L1/L3/CRT behavior | **TAGGED SUBSTRATE** |
| BOOTSTRAP | Mixed L2/L3/process service | **TAGGED SUBSTRATE** |
| ERROR | Per-owner failure behavior | **CROSS-CUTTING MATRIX** |
| Stage/scene semantic assembly | After core resource handoff | **OUTSIDE CORE RESOURCE LAYERS** |

---

# 11. New reverse work order created by Pass 2

## P2-R1 — upstream request-origin census

Start at the three canonical direct calls to `OpenGameResource` and walk callers upward until stable ingress classes are established.

Deliverable: caller/call-chain map with root semantic/system context and no invented L0.

## P2-R2 — current raw StageCfg dependency-preload reacquisition

Reconfirm or reject the historical Wave-3 StageCfg dependency graph on the current canonical raw EXE.

Deliverable: exact dependency-emission and barrier graph.

## P2-R3 — Type/Descriptor identity xref

Map descriptor/type authority from request planning through LoadedResource `+0x18` and family/loader consumers.

Deliverable: cross-layer identity edge table.

## P2-R4 — factory/resource-set demand edges

Reconfirm historical enemy/player/resource-set mappings only where they emit or select resource demand.

Deliverable: gameplay identity -> resource-set -> logical request mapping, without pulling gameplay factories into GDSpaces.

## P2-R5 — ownership hierarchy breadth

Separate:

- LoadedResource backing ownership;
- loader-node `(kind,id)` claims;
- family-local caches/reuse policies;
- transition preservation/replacement.

Deliverable: family-qualified ownership graph; no universal refcount.

## P2-R6 — readiness semantics

For each representative family, classify:

- manager state3 only;
- family semantic readiness signal if any;
- actual consumer/effect boundary;
- failure path.

Deliverable: V-ready readiness taxonomy.

## P2-R7 — dependency-aware LV/V schema integration

Extend #222 architecture only after the static identity/dependency edges above are bounded enough to avoid inventing a graph schema disconnected from original behavior.

---

# Final Pass-2 conclusion

The second audit **strengthens the decision not to add L4**.

The missing concept was not another sequential byte/runtime layer. It was an **orthogonal Resource Control Plane** plus a **cross-layer Identity Plane**.

Recommended canonical architecture:

```text
OUTSIDE-CORE Consumer / Request Ingress
                    |
                    v
          +--------------------+
          | Resource Control   |
          | Plane (RCP)        |
          +----+----------^----+
               |          |
               v          | dependency/status/claim/transition feedback
              L2          |
               |          |
               v          |
              L1          |
               |          |
               v          |
             L3A ----------+
               |
               v
             L3B
               |
               v
        Consumer handoff

TYPE / Identity Plane spans mappings across the graph.
RT-IO / MEM/BACKING / BOOTSTRAP are tagged substrates.
LV observes; V validates and owns promotion.
```

This model explains the current raw-EXE evidence without weakening L1/L2/L3 ownership and gives the next reverse pass precise targets instead of creating a speculative L4.
