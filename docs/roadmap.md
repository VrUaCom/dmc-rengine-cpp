# DMC Rengine Roadmap

**Snapshot:** 2026-08-27  
**Canonical authority reconciled through:** merged PR #242  
**Primary execution program:** **GDSpaces Layer 1 — final real same-lineage acceptance**

The project roadmap is dependency-driven rather than a linear feature checklist. Detailed GDSpaces execution authority lives in:

- [GDSpaces L1 Roadmap](gdspaces/l1-roadmap.md)
- [GDSpaces Master Roadmap — L1/L2/L3](gdspaces/master-roadmap.md)

## Current critical path — GDSpaces L1

```text
exact real selected/member lineage
 -> exact member materialization + ByteProvenance
 -> representation classification
 -> supported bounded real edit
 -> PAC/PNST bottom-up rebuild where applicable
 -> next-volume NBZ publication
 -> canonical resolver/reopen/rematerialization
 -> original DMC3 deterministic consumer-visible effect
 -> rollback / retail immutability
 -> final cross-stack/V audit
```

Current immediate work order:

1. obtain an exact real member/materialization receipt using the protected-install path or a Pocket real-device sub-receipt;
2. bind that exact archive/member identity to the accepted actual selected/successful source lineage — never by filename alone;
3. classify the exact retail representation;
4. use only an evidenced writer domain for one bounded real edit/rebuild;
5. publish as the next contiguous `DMC3-N.nbz`, reopen through canonical GDSpaces and verify exact authored bytes;
6. execute issue #209 original-game consumption + rollback;
7. perform final cross-stack/V review before any `L1 COMPLETE` claim.

No new synthetic-only feature may displace this sequence unless the real run exposes a concrete missing implementation dependency.

## Maintained foundations

The following are maintained infrastructure rather than future phases:

- C++20/CMake and Windows + Ubuntu CI;
- evidence/artifact identity and hash-gated EXE analysis;
- GDSpaces ResourceId/ResourceRef/ByteProvenance/SourceRegistry;
- canonical NBZ/PAC/PNST materialization paths;
- WorkingCopy and bounded authoring contracts;
- atomic/no-replace publication;
- Binary Inspector byte/structure authority;
- Reverse Core and Recovered Game Source Tree boundaries;
- guarded patch/export and validation infrastructure.

## Connected evidence boundary

Merged #233 establishes that protected `dmc3.exe` and executable-relative `data/dmc3/dmc3-0.nbz` are locatable. The archive is observed at `960,358,951` bytes while the connected raw materialization ceiling is `268,435,456` bytes.

This is a transport/access limitation, not archive absence. Pocket GDS may provide an exact local member-materialization receipt when the archive is already on-device, but that is not protected original-process selected-provider or consumer evidence.

## GDSpaces L2 — Resource Resolution

L2 is structurally advanced. Merged #235 promotes process-instance-bound R2B v2 tooling and corrects a key bootstrap assumption:

```text
numbered filename discovery / registration attempts
!= actual successful mounted sources
```

The first missing filename bounds discovery only. An existing archive may fail registration while discovery continues; successful mounts can therefore be sparse. Successful archive registrations prepend, preserving higher successful volume -> lower successful volume -> physical precedence.

Issue #237 tracks the product correction and pending PR #241 implements the discovery/successful-mount separation.

Remaining L2 work:

```text
exact retail 0x0E collision census
 -> review/merge/reconcile #241
 -> real protected-process R2B v2 seven-anchor packet
 -> trusted zero-loss R3 selected-provider identity
 -> exact successful-mounted archive/member binding
 -> final L2 audit
```

L2 work may support L1 but cannot replace L1's final same-lineage evidence.

## Original runtime / lifecycle L3

The static DMC3 resource-runtime spine is advanced: FileSlot/AsyncIO, ZIP read/inflate, LoadedResource state progression, typed post-load, claims and reset/release behavior.

Merged #230/#242 narrow the active materialization-completion seam. Normal `0x1401B8DC0` receives only one u32 registry-relative context and cannot inspect raw transport success/error, byte count, FileSlot/ReadRequest or child metadata. Lower materialization success/failure must therefore already be terminal before normal state2 dispatch, or completion must be suppressed/removed.

FIFO alone is not a proven dependency barrier; no generic fan-in/outstanding-child counter is claimed.

Current exact-byte reverse order:

```text
0x1402EF4D0
 -> relevant 0x1402EF790 persistence/re-poll/retirement
 -> reacquire historical 0x1400333E0 / 0x140033390 hypotheses
 -> 0x1400335A0 terminal transport writes
 -> identify incomplete/failure completion suppression
 -> 0x1402EF460 clear/rollback
 -> only then .lst recursive child failure ordering
```

Pending #240 proposes exact canonical `LoadedResource +0x04` R1 writer census as `STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED`. Even if merged, R2-R5 and V1-V7 remain open and L3 is not complete.

## RCP / V / LV

Draft #226 models an orthogonal Resource Control Plane for root/dependency planning, pending/ready coordination, claims and transition control. V/LV remains cross-cutting validation/live observation.

Neither RCP nor V/LV is L4. Neither may absorb L1 materialization, L2 selection or L3 lifecycle ownership. Broad RCP reverse runs only when it resolves an active dependency/identity/ownership ambiguity.

## Stage Ops and Stage Semantic Graph

After L1 has a real edit/rebuild/original-consumption receipt, Stage Ops becomes the primary product-side integration frontier:

```text
GDSpaces resolved/materialized resources
 -> Stage Ops assembly/orchestration
 -> Stage Semantic Graph
 -> ModViz
```

Stage Ops must never create a second resource resolver/materializer. Stage descriptor identity, numeric Stage identity and semantic gameplay identity remain distinct.

## EXE Editor / recovered source

EXE Editor remains the frontend over the Recovered Game Source Tree, exact binary mappings and evidence identities. Progressive reconstruction goals are:

- exact function/data identities;
- source-equivalent bounded C++ units;
- ABI/ownership/lifetime reconciliation;
- isolated compilation;
- controlled original-vs-reconstruction behavioral receipts;
- progressive replacement/recompilation milestones.

Readable pseudocode or compile success alone is not completion.

## ModViz / editor verticals

ModViz remains downstream of Stage Ops and shared resource authority. High-value editor verticals should be built only over canonical GDSpaces/Stage Ops state so resource parsing, scene assembly and edit ownership do not fork.

## Long-term milestones

1. **GDSpaces L1 accepted** with same-lineage real provenance, representation, edit/rebuild/reopen, original-game consumption and rollback.
2. **L2 accepted** with retail collision evidence, real R2B v2, trusted selected identity and successful-mount topology reconciliation.
3. **Representative L3 lifecycle validation** beyond bounded R1 static closure.
4. **Stage Ops game-backed assembly** over representative catalog selections.
5. **Stable Stage Semantic Graph and ModViz editing verticals.**
6. **First bounded recovered subsystem behavioral equivalence receipt.**
7. **Progressive recompilation** with controlled replacement modules.
8. **Working rebuilt executable milestones** without weakening evidence gates.

No milestone is promoted because of synthetic tests or documentation alone. See [current status](status/current.md), [blockers](status/blockers.md), [L1 roadmap](gdspaces/l1-roadmap.md) and [master roadmap](gdspaces/master-roadmap.md).
