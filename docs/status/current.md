# Current Project Status

**Snapshot:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`  
**Overall:** GDSpaces is advanced but **L1, L2 and L3 are all incomplete**. L3-R1 is bounded-closed and R2 is active.

## Authority model

- GitHub `main` is merged implementation truth.
- This reconciliation branch may propose current-main semantic ports after explicit review; it does not make unrelated open-branch code canonical.
- Reverse claims remain artifact/range/scope bound.
- Synthetic CI validates product/tool behavior, not original-game equivalence.
- Original-process promotion requires trusted origin and same-resource identity binding.
- Historical pass documents remain chronology; current status lives in the roadmaps/status surfaces.

## Layer model

- **L2 — Resource Resolution:** request, candidates, normalization, successful provider/source/volume/member selection, exact ResourceRef/ResourceId.
- **L1 — Resource Materialization:** representation, byte acquisition/result semantics, transforms, exact bytes/provenance, edit/rebuild/repack/reopen/rematerialization.
- **L3 — Original Runtime/Lifecycle:** request/scheduler/callback lifetime, LoadedResource states, typed-ready, claims/reuse, cancellation/release/reset/teardown.
- Stage Ops/ModViz/editors are downstream DOMAIN consumers.
- Validation/live observation is cross-cutting, not L4.

See [master roadmap](../gdspaces/master-roadmap.md).

## L1 current state

**Status: INCOMPLETE / NOT 100%.**

Product capabilities are advanced: NBZ/ZIP materialization, STORE/raw-DEFLATE, ByteProvenance, PAC/PNST expansion/reflow, nested authoring, no-replace publication, next-volume overlays, reopen/rematerialization, and the current naming/identity system are implemented on main.

However the old statement `INTERNAL PRODUCT PATH CLOSED; only external receipts remain` is too broad as a Layer-1 status. Newer raw-EXE evidence shows original byte/result/failure/width semantics remain bounded-open. A coarse upstream success boolean cannot be treated as proof that all required child/queue/materialization work completed byte-exactly.

Current L1 chain:

```text
finish original-L1 reverse gaps required by claimed scope
 -> direct-retail selected-member provenance
 -> exact representation classification
 -> supported real edit/rebuild
 -> exact next-volume reopen/rematerialization
 -> original-game consumption + rollback (#209)
 -> final L1 audit
```

Naming authority is now explicitly separated from physical/write authority: physical slot, extracted ordinal, external `.index`, embedded alias, semantic type and display name are distinct evidence domains.

See [L1 roadmap](../gdspaces/l1-roadmap.md).

## L2 current state

**Status: ADVANCED / INCOMPLETE.**

Merged current-main authority includes:

- bounded direct `OpenGameResource` request/candidate behavior;
- archive `0x0E` vs physical `0x0C` normalization split;
- archive lookup/index semantics;
- bounded type-0 physical final-open behavior;
- protected-process mapping tooling from #219;
- selected-identity content-candidate/normalizer/artifact binder from merged #221;
- exact ResourceId-based `RuntimeNamingBridge` into L1.

Current product-model gap: `VolumeBootstrapPlan` still represents discovered pre-gap archives as registered. Stronger #246 raw evidence proves discovery/registration attempts are not equivalent to successful linked mounts. Sparse successful mount topology is possible and should be semantically ported into current-main code.

Remaining L2 evidence gates:

```text
successful-mount topology product correction
 -> real-retail 0x0E collision census
 -> real protected-process mapping packet
 -> trusted selected-provider publisher/origin binding
 -> zero-loss original selected identity
 -> final L2 audit
```

See [L2 roadmap](../gdspaces/l2-roadmap.md).

## L3 current state

**Status: INCOMPLETE. R1 CLOSED / R2 ACTIVE.**

The historical #240 final R1 writer review has been semantically ported and checked against newer merged type/family/payload research. No contradictory exact `LoadedResource +0x04` writer was found.

Current proposed authority:

> **R1 = STATIC BOUNDED-CLOSED / CONTRADICTION-GATED** for the canonical analysis executable.

Broad state-writer discovery stops unless exact contradictory record provenance appears.

Active static work is now **R2**: family-complete ownership of `+0x08/+0x18/+0x20/+0x28` and stable adjacent fields, including initialization/finalization/release ordering and the SCM `mesh +0x28` reconciliation.

R3 is partial and materially improved by the merged runtime type split and MOD/EFM/SCM/SHW family research. R4 is partial. Dynamic R5 remains open.

The lifecycle trace validator from #218 is not in current main. It should be respawned semantically from current main, followed by a trusted process-bound publisher/binder. Dynamic acceptance order is V1 initial load, then V5 in-flight cancellation, then V2/V3/V4/V6/V7.

See [L3 roadmap](../gdspaces/l3-roadmap.md) and [R1 current-main reconciliation](../gdspaces/l3-r1-current-main-reconciliation-2026-09-02.md).

## Critical cross-layer seams

### L2 -> L1

Exact identity only:

```text
L2 resolved ResourceId == L1 parent/materialization ResourceId
```

No filename/display/semantic fallback.

### L1 -> L3

```text
[L1] exact byte/result semantics -> terminal materializer result
[L3] scheduler/callback lifetime -> normal state 1 -> 2 publication
```

Stronger raw evidence says admitted type-2 work remains current while pending/retrying and retires on status 3 before later FIFO work can become current. Dynamic cancellation/concurrency is still an L3 evidence gap.

## Current project priority

1. review/land this all-layer documentation and boundary reconciliation;
2. run L3-R2 field/backing ownership research;
3. port the L2 discovery-vs-successful-mount topology correction into current-main product code;
4. keep L1 product exactness stricter than unsafe original wrap/short-success/failure-swallowing behavior;
5. obtain real retail/protected-process artifacts and build one same-lineage L2→L1→L3 vertical receipt;
6. respawn L3 validation tooling and implement trusted publisher/binder infrastructure;
7. run independent final audits before any `COMPLETE / 100%` claim.

## Navigation

- [L1 roadmap](../gdspaces/l1-roadmap.md)
- [L2 roadmap](../gdspaces/l2-roadmap.md)
- [L3 roadmap](../gdspaces/l3-roadmap.md)
- [Master roadmap](../gdspaces/master-roadmap.md)
- [Layer classification](../gdspaces/decompilation-layer-classification.md)
- [2026-09-02 layer/L3 research review](../gdspaces/layer-boundaries-l3-research-review-2026-09-02.md)
- [Blockers](blockers.md)
- [Phase map](phase-map.md)
- [Risks](risks.md)
- [Machine-readable status](canonical-status.json)

No percentage, preview, parser, writer, resolver or crash-free launch overrides the mandatory evidence gates.
