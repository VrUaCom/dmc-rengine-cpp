# GDSpaces layer boundaries + Layer 3 research/review — 2026-09-02

**Repository base reviewed:** `main@9483663959e5452f9a224c1535445bb5a3b33520`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Size:** `6,356,432` bytes  
**ImageBase:** `0x140000000`  
**Execution result:** R1 SEMANTIC PORT COMPLETE / ALL-LAYER DOC RECONCILIATION COMPLETE / L3-R2 ACTIVE

## Purpose and execution order

This checkpoint followed the required order:

1. research current L1/L2/L3 boundaries;
2. research L3 against merged documentation plus stronger open reverse evidence;
3. review actual current-main reverse/implementation state;
4. semantically port the L3-R1 closure from stale #240 onto current main;
5. activate L3-R2 as the next static work package;
6. reconcile canonical documentation for **all three layers**, not only L3.

Authority classes remain explicit:

- **MERGED CANONICAL** — present on `main`;
- **CURRENT RECONCILIATION** — semantically ported/reviewed on this branch, proposed for promotion by PR #277;
- **STRONGER UNMERGED EVIDENCE** — useful open-PR reverse evidence, not current-main code truth;
- **IMPLEMENTATION CANDIDATE** — code exists only on an open/stale branch;
- **PROPOSAL** — architecture/planning, not runtime truth.

Historical pass documents remain chronology and are not rewritten to look current.

## 1. Updated canonical layer model

### L2 — Resource Resolution

L2 answers **which resource identity wins**.

```text
logical request
 -> candidates
 -> normalization
 -> provider/source/volume/member traversal
 -> ambiguity/provider-failure semantics
 -> exact ResourceRef / ResourceId
```

### L1 — Resource Materialization

L1 answers **which exact bytes/result are produced and reproduced**.

```text
L2 selected identity
 -> representation / extent / allocation
 -> byte acquisition
 -> EOF / short-read / progress semantics
 -> decompression / transforms
 -> exact destination bytes + ByteProvenance
 -> nested materialization
 -> edit/rebuild/repack/publication
 -> reopen/rematerialization
```

### L3 — Original Runtime / Resource Lifecycle

L3 answers **how the original runtime owns the selected/materialized resource through its lifetime**.

```text
L1 exact byte/result identity
 -> request/scheduler/callback lifetime
 -> LoadedResource state publication
 -> typed post-load / ready visibility
 -> claims/reuse
 -> cancellation/release/reset/teardown
```

Stage Ops/ModViz/editors remain downstream DOMAIN consumers. Validation/live observation is cross-cutting, not L4.

## 2. Behavior-scoped seams

Functions/subsystems are not assigned wholesale to one layer.

| Behavior | Owner |
|---|---|
| logical candidate/provider/volume/member selection | L2 |
| exact selected ResourceId | L2 output / L1 input seam |
| byte extent/EOF/short-read/decompression/exact result | L1 |
| FileSlot/ReadRequest byte/result mechanics | L1-support |
| FileSlot/ReadRequest request/queue/callback lifetime | L3-support |
| scheduler admission/persistence/retirement | L3 |
| normal LoadedResource `state 1 -> 2` publication | L3 |
| typed post-load / callback / `2 -> 3` | L3 |
| cancellation/reset/release/shutdown | L3 |

The same helper can therefore contain evidence relevant to more than one layer.

## 3. L1/L3 terminal-completion seam

Current semantic cut:

```text
[L1]
representation / byte execution
 -> native byte/result semantics
 -> terminal materializer result

[L3]
request/scheduler/callback lifetime
 -> normal 0x1401B8DC0 state 1 -> 2
 -> typed/ready lifecycle
```

Stronger raw evidence from #258/#269 reports for admitted type-2 work:

```text
status 2 -> pending / remains current
status 4 -> retry / remains current
status 3 -> retire current byte job / advance FIFO
```

This narrows the normal static dependency without moving state2 into L1. Dynamic current-slot cancellation/concurrency remains L3 breadth. Product exactness remains stricter than unsafe original short-success/wrap/failure-swallowing behavior.

## 4. L3-R1 research result and semantic port

Current main already had a strong writer census. Historical/open #240 added the final contradiction challenges:

- startup whole-manager zero vs runtime per-record state0 initialization;
- bulk-zero/memset hidden writer paths;
- generic interprocedural `arg+0x04` writers;
- exact record producer -> stored alias -> callee mutation;
- partial-width/non-enum `+0x04` writes;
- atomic/lock mutation forms;
- completion/cancellation callback-address surfaces;
- malformed odd completion context;
- possible LoadedResource state values outside `0..4`.

No contradictory exact `LoadedResource +0x04` writer survived provenance review.

The conclusion was checked against newer merged 31.08–01.09 type/family/payload research. Those findings change runtime type/family semantics but do not introduce a new LoadedResource state writer.

The result has now been semantically ported into:

[`l3-r1-current-main-reconciliation-2026-09-02.md`](l3-r1-current-main-reconciliation-2026-09-02.md)

Current proposed authority:

> **L3-R1 = STATIC BOUNDED-CLOSED / CONTRADICTION-GATED.**

Broad state-writer discovery stops. Reopen only on exact contradictory record provenance.

## 5. L3-R2 is now active

The active static L3 package is family/group ownership of:

```text
LoadedResource +0x08
               +0x10 where applicable
               +0x18
               +0x20
               +0x28
               + stable adjacent fields
```

R2 must recover producer/writer/owner/borrow/release semantics, initialization/finalization ordering, fixed-family vs group-5 distinctions and the SCM `mesh +0x28` contradiction.

R3/R4 may be used where they resolve R2 ownership ambiguity but do not replace the R2 census.

## 6. Runtime type evidence remains split

Merged current-main evidence proves there is no universal DMC3 runtime type detector:

```text
0x1402DB1F0  registry three-byte content probe
0x1401B9FA0  container post-load dispatcher
0x1402FD650  four-byte family-mask classifier
```

Consequences:

- registry identity != post-load handler existence;
- post-load handler != parser/writer maturity;
- family mask != universal format enum;
- geometry semantics != naming authority.

Merged family research now supports MOD/EFM/SCM as related mesh-bearing model-document families, SHW as a distinct self-contained shadow-hull mesh, MRP runtime identity without a proven normal central handler, MCV four-byte family recognition, and EFW/EFE dispatcher sentinels without proven normal handlers.

## 7. Current all-layer review

### L1

**INCOMPLETE / NOT 100%.**

Product materialization/authoring capability is advanced, but original byte/result/failure/width reverse remains bounded-open and real-retail/game-backed acceptance remains open. The old broad `INTERNAL PRODUCT PATH CLOSED` wording is superseded as a Layer-1 status.

### L2

**ADVANCED / INCOMPLETE.**

Merged resolver/mapping/selected-identity candidate tooling is strong. Current product gap: discovery/registration attempts are still conflated with registered archives in `VolumeBootstrapPlan`; stronger #246 evidence proves successful mount topology must be explicit and may be sparse.

### L3

**INCOMPLETE. R1 CLOSED / R2 ACTIVE.**

R3 and R4 remain partial; dynamic R5 and trusted original-process lifecycle validation remain open. The old #218 validator is useful design evidence but is absent from current main and should be respawned semantically.

## 8. Documentation synchronized in this pass

Current/canonical surfaces updated or added:

- root `README.md`;
- `docs/README.md`;
- `docs/roadmap.md`;
- `docs/gdspaces/master-roadmap.md`;
- `docs/gdspaces/l1-roadmap.md`;
- new `docs/gdspaces/l2-roadmap.md`;
- new `docs/gdspaces/l3-roadmap.md`;
- `docs/gdspaces/decompilation-layer-classification.md`;
- new `docs/gdspaces/l3-r1-current-main-reconciliation-2026-09-02.md`;
- `docs/status/README.md`;
- `docs/status/current.md`;
- `docs/status/blockers.md`;
- `docs/status/phase-map.md`;
- `docs/status/risks.md`;
- `docs/status/canonical-status.json` schema v5.

Historical audits remain unchanged as evidence chronology.

## 9. Next execution order

```text
1. review/promote this reconciliation PR
2. begin L3-R2 field/backing ownership research
3. port L2 discovery-vs-successful-mount topology into current-main code
4. keep L1 product exactness stricter than unsafe original behavior
5. respawn L3 lifecycle validator on current main
6. implement trusted original-process publisher/origin binding
7. acquire retail/protected-process evidence
8. build first same-lineage L2 -> L1 -> L3 vertical receipt
9. continue layer-specific breadth and final audits
```

## 10. Completion boundary

No layer is complete on this checkpoint.

```text
L1 COMPLETE = NO
L2 COMPLETE = NO
L3 COMPLETE = NO
```

The result of this pass is a corrected authority map, an R1 closure promoted onto current-main review context, R2 activation, and synchronized all-layer documentation — not a completion percentage claim.
