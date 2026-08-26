# GDSpaces EXE Grey-Boundary Audit — Pass 1 — 2026-08-26

**Tracking:** #225  
**Canonical raw-EXE L3 authority:** #224 / `l3-boundary-audit-2026-08-26.md` / `l3-raw-exe-pass-2026-08-26.md`  
**Canonical analysis executable:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.

## Purpose

Audit the recovered DMC3 executable resource-runtime surface for behavior that does not fit cleanly into the canonical L1/L2/L3 ownership model, without forcing an entire function/address range into one layer and without inventing L4.

Canonical top-level model remains:

- **L1 — Resource Materialization**;
- **L2 — Resource Resolution**;
- **L3 — Original Runtime / Lifecycle**;
- **LV — V-owned live/original-process evidence acquisition, not L4**;
- **V — cross-cutting Validation / Equivalence authority, not a decompilation layer**.

## Executive decision

The EXE contains real grey domains, cross-layer substrates and outside-core boundaries. **Pass 1 does not justify a new top-level layer.**

Two areas require explicit architecture accounting:

1. **Consumer / Request Ingress before L2** — gameplay/system code creates the logical resource demand that L2 receives.
2. **Typed construction/dependency orchestration inside L3** — typed post-load/factory/dependency work is separable from ownership/release lifecycle, but remains an L3 subdomain at the current evidence level.

## Canonical full executable spine

Issue #55 already frames the original request-to-unload runtime as:

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

The important observation is that this contains more **phases** than there are top-level ownership layers. A phase can be a subdomain, control edge, shared substrate or outside-core consumer boundary without becoming L4.

---

## G1 — Consumer / Request Ingress

**Position:** before L2.

L2 begins with a logical request. It does not own the gameplay/system decision that resource X must be requested now.

Current evidence includes stage/scene-driven demand, loader-node resource kinds, selector/descriptor identities and transition-driven replacement/preservation.

### Verdict

**REAL GREY DOMAIN — OUTSIDE-CORE INGRESS.**

Do not name it L0 yet. First prove a generic stable request-origin ABI across independent callers/families.

```text
Consumer / Request Ingress
 -> logical request / TypeInfo / logical id
 -> [L2 starts]
```

Original request-generation functions belong to the Recovered Game Source Tree. Product Stage Ops may orchestrate its own consumer workflows but must not become original-runtime authority.

---

## G2 — Resource Type / Descriptor Authority

**Position:** shared L2 ↔ L3 authority plane.

Descriptor/type identity is used both for request/lookup identity and for LoadedResource typed processing. Therefore type authority cannot be safely duplicated as unrelated L2 and L3 tables.

### Verdict

**CROSS-LAYER AUTHORITY PLANE — NOT A LAYER.**

```text
TYPE / Descriptor Authority
 ├─ L2: request / lookup identity
 └─ L3: typed runtime / construction identity
```

Future xref work must bind the same descriptor/type identities across both uses.

---

## G3 — FileSlot / AsyncIO / Transport

**Position:** L1 ↔ L3 seam.

Raw-EXE reconciliation makes the semantic split explicit:

- exact byte transfer/materialization behavior supports L1;
- pool/request scheduling/callback/cancellation/service lifetime belongs to L3.

`0x1401B8CA0` is a semantic seam: representation/materialization behavior is L1; its success controls L3 state1 publication in `0x1401B84E0`.

### Verdict

**CONFIRMED GREY SUBSTRATE — NOT A LAYER.**

Recommended tag: **`RT-IO`**.

Classify operations/caller context, not a whole function solely by VA.

---

## G4 — Typed Construction / Factory / Dependency

**Position:** after materialized bytes and before stable ready/consumer state; currently inside L3.

The request-to-unload model separates:

```text
materialized bytes
 -> typed post-load
 -> factory/type dispatch
 -> dependency discovery
 -> allocation/object construction
 -> ownership registration
 -> consumer handoff
```

Current raw L3 authority closes the central best-effort typed dispatcher shape, but external factory/dependency behavior remains an explicit open gate.

### Verdict

**STRONGEST INTERNAL GREY DOMAIN — KEEP INSIDE L3 AS SUBDOMAINS.**

Recommended accounting split:

- **L3A — Typed Construction / Dependency**
  - state1/state2 completion boundary;
  - typed post-load;
  - factory/type dispatch;
  - dependency discovery/request emission;
  - allocation/object construction;
  - transition toward manager-ready state3.

- **L3B — Ownership / Lifecycle**
  - claims/reuse;
  - owner coordination;
  - cancellation/quiescence/replacement;
  - release/reset;
  - runtime/CRT/process-lifetime distinction.

This is an accounting split, not new layer numbering.

---

## G5 — Dependency feedback edge

**Position:** L3A → L2 recursive edge.

A dependency-bearing resource can enter typed/domain processing and cause additional logical resource requests. Therefore the architecture must not be documented as only a one-way line.

```text
Ingress
 -> L2 -> L1 -> L3A
                 |
                 +---- dependency request ----> L2 -> L1 -> L3A
                 |
                 -> L3B -> consumer
```

### Verdict

**ARCHITECTURAL LOOP / GREY EDGE — NOT A LAYER.**

Future EXE call graphs and LV/V receipts for dependency-bearing families must account for child requests rather than validating only the root resource.

---

## G6 — Memory / Backing / Allocation

**Position:** L1 ↔ L3 plus CRT/process lifetime.

Memory behavior affects L1 when buffer/allocation shape changes exact materialization, and L3 when backing ownership/release/order changes runtime lifetime. Raw L3 authority also distinguishes runtime backing release from CRT backing destruction.

### Verdict

**CROSS-CUTTING RUNTIME SUBSTRATE — NOT A LAYER.**

Recommended tag: **`MEM/BACKING`**.

Do not invent one universal allocator/refcount semantic.

---

## G7 — Error / Recovery matrix

Failure behavior exists at every layer:

- L2 missing/default/selector/provider fallback;
- L1 malformed archive/read/transform failures;
- L3 factory/dependency/allocation/cancellation/reset failures.

### Verdict

**CROSS-CUTTING BEHAVIOR MATRIX — NOT A LAYER AND NOT V.**

The behavior stays with the component that performs it. V only validates equivalence.

Recommended tag: **`ERROR`**.

---

## G8 — Post-L3 consumer / Stage / scene semantics

After manager-ready visibility and runtime ownership, gameplay/domain code can interpret resources into stage/room/scene semantics.

### Verdict

**OUTSIDE CORE RESOURCE LAYERS — NOT L4.**

Stage Assembly / Stage Ops remains downstream. Scene-transition effects that operate on resource ownership remain L3B; semantic scene assembly after handoff is outside core GDSpaces resource layers.

---

## G9 — Bootstrap / process services

Numbered source registration is L2 behavior; lazy AsyncIO service ownership is L3; CRT/process-lifetime infrastructure is a platform/runtime substrate.

### Verdict

**GREY BOOTSTRAP PLANE — NOT A LAYER.**

Recommended tag: **`BOOTSTRAP`**, with each operation retaining its actual L2/L3 ownership.

---

## G10 — LV instrumentation

Protected-runtime mapping, hooks/watchpoints/trace publishers and observation receipts exist to observe the original executable.

### Verdict

**NOT AN ORIGINAL RESOURCE LAYER.**

Keep under LV/V. Instrumentation behavior must never count as L1/L2/L3 reverse progress.

---

## Pass-1 classification

No L4 is justified.

Canonical core remains:

```text
L1 Materialization
L2 Resolution
L3 Original Runtime / Lifecycle
```

Formal supporting classifications:

- outside-core `Consumer / Request Ingress`;
- `L3A` Typed Construction / Dependency — accounting subdomain;
- `L3B` Ownership / Lifecycle — accounting subdomain;
- `TYPE` shared type/descriptor authority;
- `RT-IO` FileSlot/AsyncIO boundary substrate;
- `MEM/BACKING` allocation/backing substrate;
- `BOOTSTRAP` startup/service substrate;
- `ERROR` per-owner failure/recovery matrix;
- explicit `L3A -> L2` dependency feedback edge.

## V consequence

For dependency-bearing validation, a root resource cannot imply dependency closure. V must eventually be able to bind:

```text
root request
 -> root L2 selected identity
 -> root L1 bytes
 -> root L3A processing
 -> dependency request set
      -> child L2 identities
      -> child L1 bytes
      -> child L3A readiness
 -> root/child ownership relationship
 -> root consumer handoff
 -> L3B release/reset
```

A bounded non-dependent resource receipt can remain simpler.

## Next pass

Pass 2 must test whether the grey edges collectively form a stable **Resource Control Plane** orthogonal to the data path, and must distinguish current canonical raw-EXE authority from historical/unmerged reverse evidence.
