# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-09-02  
**Reviewed base:** `main@9483663959e5452f9a224c1535445bb5a3b33520`  
**Canonical layer status:** L1 INCOMPLETE / L2 INCOMPLETE / L3 R1 CLOSED, R2 ACTIVE / no layer is 100%

This roadmap is the canonical dependency map for the three GDSpaces resource-runtime layers. Layer ownership is semantic and evidence-driven, not assigned by one contiguous executable address range or by whichever product module currently contains a helper.

## Layer model

### L2 — Resource Resolution

```text
logical request
 -> candidates
 -> normalization
 -> provider/source/volume/member selection
 -> ambiguity/provider-failure semantics
 -> exact ResourceRef / ResourceId
```

Canonical roadmap: [L2 roadmap](l2-roadmap.md).

### L1 — Resource Materialization

```text
L2 selected identity
 -> representation / byte extent
 -> allocation / byte acquisition
 -> transforms / decompression
 -> exact materialized bytes + provenance
 -> nested materialization
 -> edit/rebuild/repack/publication
 -> reopen/rematerialization
```

Canonical roadmap: [L1 roadmap](l1-roadmap.md).

### L3 — Original Runtime / Resource Lifecycle

```text
L1 exact byte/result identity
 -> request/scheduler/callback lifetime
 -> LoadedResource state publication
 -> typed post-load / ready visibility
 -> claims/reuse
 -> cancellation/release/reset/teardown
```

Canonical roadmap: [L3 roadmap](l3-roadmap.md).

### DOMAIN — downstream consumers

Stage Assembly, Stage Ops, ModViz and editors consume L1/L2/L3 authority. They do not create a second resolver/materializer/lifecycle truth.

### Validation / live observation

Validation is cross-cutting and is not L4. Trusted original-process observations must preserve same-run/same-resource identity across the layers they claim to connect.

## Canonical boundary rules

1. **L2 selects; L1 materializes; L3 owns runtime lifecycle.**
2. A helper may contribute evidence to multiple layers. Classify concrete behavior, not the whole function/subsystem.
3. FileSlot/ReadRequest byte/result mechanics can support L1 while request/queue/callback lifetime remains L3.
4. `LoadedResource state 1 -> 2` is L3 lifecycle publication even when terminal L1 byte/result ordering is required before it.
5. `.index`, display names, embedded aliases and semantic suffixes are not runtime lookup or write authority unless independently proven for that exact role.
6. Product hardening may be stricter than original behavior and must not be weakened to imitate unsafe wrap, short-success or failure-swallowing paths.
7. A stronger open branch is evidence input, not current-main truth until semantically reconciled and reviewed.

## Current L1 status

**L1 = INCOMPLETE / NOT 100%.**

Current main has advanced product materialization/authoring capability: NBZ/ZIP materialization, PAC/PNST expansion/reflow, nested authoring, provenance, no-replace publication, next-volume overlay generation, reopen/rematerialization and the current naming/identity architecture.

However newer raw evidence rejects the old broad statement that only external receipts remain. Original materialization/failure/width semantics are still bounded-open, and one upstream boolean cannot be treated as proof that every required byte job was admitted and completed exactly.

Primary remaining chain:

```text
original-L1 reverse closure required by claimed scope
 -> direct-retail selected-member provenance
 -> exact representation classification
 -> supported real edit/rebuild
 -> exact next-volume reopen/rematerialization
 -> original-game consumption + rollback (#209)
 -> final L1 audit
```

See [L1 roadmap](l1-roadmap.md).

## Current L2 status

**L2 = ADVANCED / INCOMPLETE.**

Merged authority includes the bounded direct-call resolver policy, `0x0C`/`0x0E` normalization split, type-0 physical-provider final-open semantics, protected-runtime mapping tooling (#219), selected-identity candidate/normalizer/artifact binder (#221), and exact ResourceId-based L2→L1 naming bridge.

Key current product-model gap: filename discovery/registration attempts must not be equated with successful linked runtime mounts. Stronger #246 evidence proves sparse successful mount topology is possible; this correction remains to be semantically ported into current-main product code.

Primary remaining chain:

```text
discovery-vs-successful-mount topology correction
 -> retail 0x0E collision evidence
 -> real protected-process mapping receipt
 -> trusted selected-provider publisher/origin binding
 -> zero-loss original selected identity
 -> final L2 audit
```

See [L2 roadmap](l2-roadmap.md).

## Current L3 status

**L3 = INCOMPLETE. R1 is current-main bounded-closed; R2 is ACTIVE.**

R1 state-writer closure was semantically ported from historical #240 and reviewed against newer merged runtime type/family/payload evidence. Broad `LoadedResource +0x04` writer discovery stops unless contradicted by exact record provenance.

Active static work is now R2 field/backing ownership for `+0x08/+0x18/+0x20/+0x28` and stable adjacent fields. R3 typed/factory/dependency work is partial and materially strengthened by the split runtime type evidence and MOD/EFM/SCM/SHW family research. R4 shared-owner breadth remains partial. R5 dynamic transition/cancellation/reset/shutdown timing remains open.

The lifecycle validator from #218 is not integrated in current main and should be respawned semantically from current main. Trusted original-process lifecycle publishing/binding is still missing.

Primary L3 chain:

```text
R2 field/backing ownership
 -> R3/R4 breadth as needed
 -> current-main lifecycle validator
 -> trusted process-bound L3 publisher/binder
 -> V1 initial load
 -> V5 in-flight cancellation
 -> V2/V3/V4/V6/V7
 -> final L3 audit
```

See [L3 roadmap](l3-roadmap.md) and [R1 current-main reconciliation](l3-r1-current-main-reconciliation-2026-09-02.md).

## Cross-layer seams

### L2 → L1

The join is exact physical/resource identity:

```text
L2 resolved ResourceId == L1 materialization/naming parent ResourceId
```

No basename/display/semantic fallback is valid.

### L1 → L3

Semantic cut:

```text
[L1]
byte execution / exact byte-result semantics
 -> terminal materializer result

[L3]
scheduler/request lifetime
 -> normal completion callback
 -> LoadedResource state 1 -> 2
```

Stronger raw evidence reports admitted type-2 jobs remain current while pending/retrying and retire on status 3 before later FIFO work can become current. Dynamic cancellation/concurrency remains L3 breadth.

## Vertical acceptance target

The strongest useful proof is one same-lineage vertical chain:

```text
real original request
 -> [L2] exact selected provider/member identity
 -> [L1] exact materialized bytes + provenance
 -> [L1] supported authored replacement/rebuild
 -> [L2] authored overlay winner
 -> [L1] exact authored rematerialization
 -> [L3] original lifecycle reaches consumer-ready/use path
 -> deterministic consumer-visible effect
 -> rollback / retail immutability
```

This vertical proof can close a declared acceptance scope without pretending every layer's full breadth is complete.

## Current project priority

1. keep the L1 real-retail acceptance path ready while finishing the original-L1 reverse gaps activated by the claimed scope;
2. port the L2 discovery-vs-successful-mount topology correction into current-main code;
3. run L3 R2 field/backing ownership research now that R1 is closed;
4. obtain retail/protected-process artifacts needed for L2/L1 evidence;
5. respawn the L3 lifecycle validator on current main;
6. implement trusted original-process publisher/binder infrastructure without self-authored promotion fields;
7. capture the first same-lineage L2→L1→L3 vertical receipt;
8. continue layer-specific breadth and final audits independently.

## Completion rule

No layer is complete because a percentage, parser, writer, resolver, preview or synthetic test says so. Each completion claim requires the mandatory static/product/original-process gates for its declared scope, exact-head Windows+Ubuntu validation where applicable, and a contradiction-free final audit.
