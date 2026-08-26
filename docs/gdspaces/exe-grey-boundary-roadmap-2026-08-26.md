# GDSpaces EXE Grey-Boundary Reverse Roadmap Addendum — 2026-08-26

**Parent ledger:** #225  
**Pass 1:** `exe-grey-boundary-audit-2026-08-26.md`  
**Pass 2:** `exe-grey-boundary-pass2-2026-08-26.md`  
**Canonical L3 raw authority:** `l3-boundary-audit-2026-08-26.md`, `l3-raw-exe-pass-2026-08-26.md`  
**Current reconciliation:** 2026-08-27 / canonical main through merged #242

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

## Post-#242 materialization dependency correction

Merged #230/#242 adds a higher-priority bounded seam at the L1/L3 boundary that this grey-boundary work must respect.

Normal completion `0x1401B8DC0` receives only one u32 registry-relative LoadedResource context. It receives no raw transport status/error, bytes-read value, FileSlot/ReadRequest handle or child/outstanding-work metadata.

Therefore:

```text
lower materialization terminal success/failure
 -> must be established before normal 0x1401B8DC0 state2 publication
    OR queued completion must be suppressed/removed
```

FIFO insertion order alone is not a proven dependency barrier, and no generic fan-in/outstanding-child counter is evidenced.

This correction does **not** create new RCP ownership. The exact FileSlot/AsyncIO persistence/cancellation/completion mechanism remains L3/RT-IO evidence; `0x1401B8CA0` remains the explicit L1/L3 success seam.

### Immediate exact-byte dependency queue

Before broadening RCP work where the result is not needed by an active acceptance gate, prioritize:

1. `0x1402EF4D0` queued materialization job identity/type, callees and inherited context consumer;
2. relevant `0x1402EF790` materialization case, including persistence/re-poll/terminal retirement;
3. reacquire historical `0x1400333E0` status/poll hypothesis;
4. reacquire historical `0x140033390` terminal cleanup/release hypothesis;
5. `0x1400335A0` lower transport terminal success/error writes;
6. identify how incomplete/failed transport prevents or suppresses normal `0x1401B8DC0`;
7. `0x1402EF460` higher-scheduler clear/rollback and queued-completion suppression;
8. only then propagate the proven mechanism to `.lst` recursive child/failure ordering.

Return to broader RCP work after this seam is either closed or shown not to be required by the active representative L1 acceptance path.

## L2 topology correction relevant to RCP/bootstrap

Merged #235 establishes that numbered filename discovery/registration attempts and actual successful mounts are separate authority surfaces:

```text
VolumeBootstrapPlan = discovery / attempt order
RuntimeMountTopology = actual successful mount results
```

The first missing filename bounds discovery only. An existing archive may fail registration while later numbered filenames are still discovered, so the successful set may be sparse. Successful archive mounts prepend and preserve higher-successful-volume -> lower-successful-volume -> physical precedence.

Issue #237 tracks the product correction; pending PR #241 implements the split. RCP/BOOTSTRAP documentation must not convert filename presence into mount-success truth.

## Pending L3 R1 boundary

Pending PR #240 proposes the exact canonical `LoadedResource +0x04` writer census as:

`STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED`.

Until merge it is branch truth. If promoted, broad R1 writer discovery should stop absent concrete contradictory record provenance and the next static work should move to R2 field/backing ownership. This does not make L3 complete.

## Priority queue

### P0 — only grey-boundary work required by active L1/L2/L3 acceptance

1. **Materialization dependency exact-byte closure** — the post-#242 queue above.
2. **Successful-mount topology semantics** — preserve #235 and reconcile pending #241 where RCP/bootstrap reasoning touches mounted-source state.
3. **R1 contradiction handling** — review pending #240 at its bounded writer-census scope; do not reopen broad R1 without direct contradiction.

### P1 — bounded RCP/grey-boundary reverse

1. **P2-R1 upstream request-origin census** — walk above all three direct `OpenGameResource` callers and classify root ingress.
2. **P2-R2 StageCfg dependency-preload reacquisition** — reconfirm/reject historical Wave-3 dependency graph on current canonical raw EXE.
3. **P2-R3 Type/Descriptor identity xref** — request identity ↔ descriptor/type ↔ LoadedResource `+0x18` ↔ loader/family consumer.
4. **P2-R4 factory/resource-set demand edges** — reconfirm gameplay identity -> resource-set -> logical resource demand where relevant.
5. **P2-R5 ownership hierarchy breadth** — LoadedResource backing vs loader claims vs family-local caches vs transition retention.
6. **P2-R6 readiness semantics** — classify manager-ready vs family-semantic-ready vs consumer effect/failure per representative family.
7. **P2-R7 dependency-aware LV/V integration** — only after static graph/identity edges are sufficiently bounded.

P1 items must yield to the real L1 acceptance chain when they are not discharging a concrete dependency.

## Evidence discipline

- Current merged raw-EXE evidence = canonical within its recorded scope.
- Open #226/#238/#240/#241 remain branch truth until merged.
- Closed/unmerged historical recovered-source PRs can generate targets but cannot silently promote current authority.
- No whole function is assigned to a layer merely because one branch or caller belongs there.
- No new layer is created from importance alone; require an independently owned stable contract.
- No FIFO/fan-in/materialization-completion mechanism is promoted without exact evidence.
- No discovered archive filename is promoted into successful-mounted topology without mount-outcome evidence.

## Return condition to the main vertical path

This work may run in parallel with L1/L2/L3 acceptance only when it:

- closes an ambiguity that affects correct layer boundaries;
- supplies required V/LV identity/dependency semantics;
- closes the #242 materialization dependency mechanism;
- reconciles actual selected/mounted source identity needed by final L1/L2 evidence;
- or prevents false completion/equivalence claims.

It must not displace the current real L1 sequence:

```text
exact selected/member lineage
 -> exact materialization receipt
 -> representation classification
 -> supported real edit/rebuild
 -> next-volume reopen/rematerialization
 -> original DMC3 consumer-visible effect
 -> rollback
 -> final cross-stack/V audit
```

Documentation synchronization or a bounded static RCP/L3 result does not itself produce an L1/L2/L3 COMPLETE verdict.
