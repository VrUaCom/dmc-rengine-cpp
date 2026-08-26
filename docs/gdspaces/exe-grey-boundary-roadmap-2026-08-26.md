# GDSpaces EXE Grey-Boundary Reverse Roadmap Addendum — 2026-08-26

**Parent ledger:** #225  
**Pass 1:** `exe-grey-boundary-audit-2026-08-26.md`  
**Pass 2:** `exe-grey-boundary-pass2-2026-08-26.md`  
**Canonical L3 raw authority:** `l3-boundary-audit-2026-08-26.md`, `l3-raw-exe-pass-2026-08-26.md`

This addendum does not replace the L1/L2/L3 master roadmap. It records orthogonal EXE boundary work that must remain visible without being counted as a speculative L4.

## Canonical architecture decision

```text
Top-level ownership layers:
  L1 Materialization
  L2 Resolution
  L3 Original Runtime / Lifecycle

Orthogonal / supporting architecture:
  RCP         Resource Control Plane
  TYPE/ID     Type, descriptor and identity mapping plane
  RT-IO       FileSlot/AsyncIO seam
  MEM/BACKING memory/backing substrate
  BOOTSTRAP   process/resource service startup substrate
  ERROR       per-owner failure/recovery matrix
  LV          V-owned live observation
  V           validation/equivalence + promotion authority
```

`RCP`, `TYPE/ID`, substrates, LV and V are **not additional numbered decompilation layers**.

## L3 accounting refinement

Use the following subdomain labels where useful:

```text
L3A — Typed Construction / Dependency
L3B — Ownership / Lifecycle
```

These are accounting labels only. They do not change L3 ownership.

## Critical non-linear edge

```text
L3A dependency discovery
 -> RCP dependency request emission
 -> L2 selection
 -> L1 bytes
 -> L3A child processing
```

A dependency-bearing resource cannot be validated as a broad equivalent implementation from the root resource alone.

## Readiness rule

Current raw EXE evidence proves central ordering:

```text
typed dispatcher
 -> optional ready callback
 -> state3
```

and central unknown/default typed dispatch is best-effort/no-op. Therefore:

```text
manager_ready_state3
!= universal family_semantic_ready
!= consumer_effect_observed
```

V must keep these claims distinct.

## Priority queue

1. **P2-R1 upstream request-origin census** — walk above all three direct `OpenGameResource` callers and classify root ingress.
2. **P2-R2 StageCfg dependency-preload reacquisition** — reconfirm/reject historical Wave-3 dependency graph on current canonical raw EXE.
3. **P2-R3 Type/Descriptor identity xref** — request identity ↔ descriptor/type ↔ LoadedResource `+0x18` ↔ loader/family consumer.
4. **P2-R4 factory/resource-set demand edges** — reconfirm gameplay identity -> resource-set -> logical resource demand where relevant.
5. **P2-R5 ownership hierarchy breadth** — LoadedResource backing vs loader claims vs family-local caches vs transition retention.
6. **P2-R6 readiness semantics** — classify manager-ready vs family-semantic-ready vs consumer effect/failure per representative family.
7. **P2-R7 dependency-aware LV/V integration** — only after static graph/identity edges are sufficiently bounded.

## Evidence discipline

- Current merged raw-EXE evidence = canonical within its recorded scope.
- Closed/unmerged historical recovered-source PRs can generate targets but cannot silently promote current authority.
- No whole function is assigned to a layer merely because one branch or caller belongs there.
- No new layer is created from importance alone; require an independently owned stable contract.

## Return condition to the main vertical path

This work may run in parallel with L1/L2/L3 acceptance only when it:

- closes an ambiguity that affects correct layer boundaries;
- supplies required V/LV identity/dependency semantics;
- or prevents false completion/equivalence claims.

It must not displace real-retail/original-process acceptance merely to broaden reverse coverage.
