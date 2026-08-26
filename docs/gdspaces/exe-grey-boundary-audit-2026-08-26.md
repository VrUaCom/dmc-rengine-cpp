# GDSpaces EXE Grey-Boundary Audit — 2026-08-26

## Purpose

Audit the recovered DMC3 executable resource-runtime surface for behavior that does not fit cleanly into the canonical L1/L2/L3 ownership model, and determine whether any such behavior justifies a new top-level layer.

This audit uses the current canonical model:

- L1 — Resource Materialization;
- L2 — Resource Resolution;
- L3 — Original Runtime / Resource Lifecycle;
- LV — V-owned live/original-process evidence acquisition, not L4;
- V — cross-cutting Validation / Equivalence authority, not a decompilation layer.

The goal is to find **grey boundaries** rather than force every executable function into one layer by address.

## Executive conclusion

The executable does contain several real grey domains. The audit does **not** currently justify adding an L4 or renumbering L1/L2/L3.

Two domains deserve explicit formalization because they are architecturally meaningful:

1. **Request/consumer ingress before L2** — gameplay/system code creates the logical resource demand that L2 receives. This is outside core GDSpaces resolution and should be tracked as a consumer/ingress domain rather than hidden inside L2.
2. **Typed construction/dependency orchestration inside the current L3 envelope** — post-load normalization, factory/dependency construction and ownership/lifetime are separable runtime phases. This warrants L3 sub-domain accounting, not a new top-level layer.

Several additional grey areas are confirmed cross-boundary substrates or outside-core consumers, not new layers.

## Canonical end-to-end executable spine

The current request-to-unload reverse authority already exposes distinct phases:

```text
game/system request
 -> source/archive discovery
 -> logical lookup / tables / indexes
 -> archive/volume selection
 -> byte acquisition
 -> decompression/transformation
 -> nested container expansion
 -> typed post-load normalization / factory dispatch
 -> dependency discovery
 -> allocation/object construction
 -> ownership/lifetime registration
 -> consumer handoff
 -> reload/transition behavior
 -> release/unload/shutdown
```

The important audit observation is that this sequence contains more phase boundaries than the three top-level ownership layers. Some phases are sub-domains, cross-layer seams or outside-GDSpaces consumer behavior.

---

## G1 — Request Origin / Consumer Ingress

**Location relative to layers:** before L2.

**Question:** who decides that logical resource X must be requested now, with which gameplay/system identity?

Current L2 begins at the logical request and owns candidate construction/provider selection. It does not own the gameplay reason that generated that request.

Examples already present in recovered authority include:

- stage/scene-driven resource demand;
- loader-node kinds such as StageMain, StageCfg, StageEffect, demo/cutscene, enemy object and enemy sound packages;
- selector/descriptor-driven stage/resource-set identities;
- scene transitions that trigger replacement or preservation of resource groups.

### Verdict

**REAL GREY DOMAIN — STRONG OUTSIDE-CORE BOUNDARY.**

Do not call this L0 yet. Formalize it as **Consumer/Request Ingress** or equivalent outside-core domain until a wider executable census proves a stable generic ABI independent of Stage/gameplay systems.

Suggested ownership rule:

```text
Consumer/Request Ingress
 -> emits logical request / TypeInfo / logical id
 -> L2 starts here
```

Stage Ops may model product-side consumer orchestration, but original EXE request-generation functions remain Recovered Game Source Tree authority.

---

## G2 — Resource Type / Descriptor Authority

**Location relative to layers:** shared L2 <-> L3 plane.

The same type/descriptor authority participates in more than one phase:

- logical request identity and path/provider planning on the L2 side;
- LoadedResource descriptor/type fields and typed post-load dispatch on the L3 side.

This means `ResourceTypeInfo`-style metadata is not naturally owned by one layer even though individual uses are.

### Verdict

**REAL CROSS-LAYER AUTHORITY PLANE — NOT A NEW LAYER.**

Create/retain a shared recovered type-authority model with explicit uses:

```text
Type Authority
 ├─ L2: request/lookup identity
 └─ L3: typed construction/post-load identity
```

Do not duplicate type tables independently in L2 and L3 product/recovered models.

---

## G3 — FileSlot / AsyncIO / Transport Scheduler

**Location relative to layers:** L1 <-> L3 seam.

Current evidence already proves FileSlot is a boundary subsystem:

- exact selected byte-range transfer needed to explain materialization supports L1;
- global slot pool, tickets, worker scheduling, callbacks, cancellation and service lifetime are L3.

The helper around `0x1401B8CA0` is also explicitly semantic rather than address-owned:

```text
representation/materialization mechanics [L1]
 -> boolean success
 -> state1 publication / lifecycle ownership [L3]
```

### Verdict

**CONFIRMED GREY SUBSTRATE — NOT A NEW LAYER.**

Recommended tag: `RT-IO` / `Runtime I/O Substrate`.

Functions must be classified by responsibility/range/operation, not by whole function address when one helper crosses the seam.

---

## G4 — Typed Construction / Factory / Dependency Orchestration

**Location relative to layers:** after materialized bytes, before stable consumer-ready object state; currently inside L3.

The executable spine separates:

```text
materialized bytes
 -> typed post-load normalization
 -> factory dispatch
 -> dependency discovery
 -> allocation/object construction
 -> ownership/lifetime registration
 -> consumer handoff
```

Issue #88 confirms the central typed dispatcher boundary but still leaves external factory/dependency construction failures and family-specific behavior open.

This is also where a linear layer diagram becomes insufficient: dependency discovery may request additional resources, creating a feedback path back into L2.

```text
L3 typed/dependency processing
        |
        +---- dependency request ----> L2
                                      -> L1
                                      -> L3
```

### Verdict

**STRONGEST INTERNAL GREY DOMAIN.**

Do not create L4. Split L3 conceptually into sub-domains:

- `L3A — Load/Typed Construction`
  - state1/state2 completion boundary;
  - typed post-load;
  - factory/type dispatch;
  - dependency discovery/request emission;
  - allocation/object construction;
  - transition to ready state3.

- `L3B — Ownership/Lifecycle`
  - claims/reuse;
  - retention/shared ownership;
  - cancellation;
  - release/reset;
  - scene transition lifecycle;
  - shutdown/process-lifetime distinction.

This is an accounting split only unless future evidence proves independently reusable runtime subsystems with their own stable public ABI.

---

## G5 — Dependency Feedback Loop

**Location relative to layers:** L3 -> L2 recursive edge.

This is not merely part of factory code. It changes the topology of the architecture.

A resource can become materialized, enter typed processing, discover a dependency, and cause another logical request. Therefore the runtime should not be documented only as:

```text
L2 -> L1 -> L3
```

The more faithful model is:

```text
Ingress
  -> L2 -> L1 -> L3A
                  |
                  +---- dependency ----> L2 -> L1 -> L3A
                  |
                  -> L3B -> consumer
```

### Verdict

**CONFIRMED ARCHITECTURAL LOOP / GREY EDGE — NOT A LAYER.**

This edge must be included in future EXE call-graph and LV/V receipts when validating dependency-bearing resource families.

A single root-resource receipt must not silently claim dependency closure unless child requests/identities are also accounted for.

---

## G6 — Memory / Backing / Allocation Plane

**Location relative to layers:** L1 <-> L3 plus process/CRT lifetime.

Memory behavior participates in several contracts:

- L1 when allocation/buffer shape affects exact byte materialization;
- L3 when backing ownership, object lifetime, release ordering or cancellation is involved;
- process/CRT teardown where lifetime is outside ordinary runtime release.

Current reverse already distinguishes runtime backing release from CRT backing destruction and multiple state-zero policies.

### Verdict

**REAL CROSS-CUTTING RUNTIME SUBSTRATE — NOT A NEW LAYER.**

Recommended tag: `MEM/BACKING`.

Do not create one universal allocator/refcount semantic. Ownership remains family/path specific where evidence says so.

---

## G7 — Error / Recovery Semantics

**Location relative to layers:** cross-cutting L1/L2/L3.

Examples:

- L2 missing-resource/default/selector fallback;
- L1 malformed ZIP/partial-read/transform behavior;
- L3 factory/dependency/allocation failure, pool exhaustion, cancellation/reset;
- profile/build differences.

### Verdict

**CROSS-CUTTING BEHAVIOR MATRIX — NOT A LAYER AND NOT V.**

V validates error behavior but must not own the behavior itself.

Each error branch stays with the layer/sub-domain that performs it, while V records whether the reconstructed behavior matches the original.

---

## G8 — Post-L3 Consumer / Scene Assembly

**Location relative to layers:** after resource-ready handoff.

Once state3 visibility and resource ownership are established, gameplay systems can assemble stage/room/scene semantics, bind geometry/collision/camera/effects/audio and perform domain-specific work.

Current canonical architecture already places Stage Assembly / Stage Ops downstream rather than inside L3.

### Verdict

**REAL OUTSIDE-CORE BOUNDARY — NOT L4.**

The resource runtime should stop at the explicit consumer handoff/ownership contract. Domain interpretation after that belongs to the relevant game subsystem / Stage Ops counterpart.

Resource-lifecycle effects caused by scene transition remain L3; scene semantic assembly itself is outside L3.

---

## G9 — Bootstrap / Process Service Lifetime

**Location relative to layers:** mixed L2/L3 startup substrate.

Examples:

- numbered-volume/source registration and mount precedence — L2;
- lazy AsyncIO startup/service lifetime — L3;
- process/CRT teardown — lifecycle/platform substrate.

### Verdict

**GREY BOOTSTRAP PLANE — NOT A NEW LAYER.**

Recommended explicit documentation tag: `BOOTSTRAP` with per-operation L2/L3 ownership.

---

## G10 — LV instrumentation / protected-runtime mapping

**Location relative to layers:** outside original behavior; V-owned acquisition.

Runtime RVA mapping, trace publishers, watchpoints/hooks and sanitized observation receipts exist to observe the executable. They are not recovered DMC3 resource-runtime semantics.

### Verdict

**NOT AN EXE RESOURCE LAYER.**

Keep under LV/V. Never count instrumentation behavior as L1/L2/L3 reverse progress.

---

## Layer-addition decision

### No new top-level layer is justified today

The current core remains:

```text
L1 Materialization
L2 Resolution
L3 Original Runtime / Lifecycle
```

with V/LV outside the decompilation-layer count.

### Formal additions recommended

1. Add explicit outside-core `Consumer/Request Ingress` boundary before L2.
2. Split L3 accounting/documentation into:
   - `L3A Typed Construction / Dependency`;
   - `L3B Ownership / Lifecycle`.
3. Record non-layer runtime planes:
   - `TYPE` — shared resource type/descriptor authority;
   - `RT-IO` — FileSlot/AsyncIO transport substrate;
   - `MEM/BACKING` — allocation/backing lifetime substrate;
   - `BOOTSTRAP` — startup/service plane;
   - `ERROR` — per-layer failure/recovery matrix.
4. Add explicit dependency feedback edge `L3A -> L2` to diagrams and validation plans.
5. Keep post-state3 semantic Stage/scene assembly outside core GDSpaces resource layers.

## Why this matters to V

The V parent receipt must not assume one strictly linear root resource when the selected family can issue dependencies.

For dependency-bearing validation, future V vertical bindings should be able to represent:

```text
root logical request
 -> root L2 selected identity
 -> root L1 bytes
 -> root L3A typed processing
 -> dependency request set
      -> child L2 identities
      -> child L1 bytes
      -> child L3A readiness
 -> root/child ownership relationship
 -> root state3/consumer handoff
 -> L3B release/reset
```

This is not required for a bounded non-dependent resource receipt, but it is required before broad V-E equivalence claims across dependency-bearing families.

## Reverse priorities created by this audit

1. Build an EXE caller census **upstream of L2** to identify stable request-origin ABIs and distinguish generic resource ingress from Stage/game-specific callers.
2. Expand #88 R3 around external factory/dependency construction and record every dependency-triggered request edge back into L2.
3. Separate L3A versus L3B functions in recovered-source/evidence maps without moving code ownership.
4. Build TypeInfo/descriptor xref map spanning request construction (L2 use) and typed construction (L3 use).
5. Add dependency-aware LV event vocabulary and V receipt binding before claiming V-E breadth for dependent families.
6. Preserve RT-IO/MEM/BOOTSTRAP as tagged cross-boundary substrates instead of inventing layers around implementation details.

## Promotion boundary

This audit changes classification/accounting only. It does not promote a new original-runtime semantic claim by itself.

A new top-level layer may be introduced later only if executable evidence proves a stable, independently owned contract that cannot be represented as:

- an L1/L2/L3 sub-domain;
- a cross-layer substrate;
- an outside-core consumer/ingress domain;
- or a V/LV validation concern.
