# DMC Rengine Roadmap

**Snapshot:** 2026-08-25  
**Canonical base:** `main@8e67235fd26cf7af94146f4dc660eb49e3c1d133`

The project roadmap is dependency-driven rather than a linear feature checklist. The current primary execution program is **GDSpaces Layer 1 — Resource Materialization**. Its detailed acceptance gates live in [GDSpaces L1 Roadmap](gdspaces/l1-roadmap.md).

## Current critical path — GDSpaces L1

```text
composite closure-receipt integrity
 -> recursive PAC/PNST slot-path hardening
 -> direct-retail provenance receipt
 -> retail representation classification
 -> bounded real edit
 -> PAC/PNST bottom-up rebuild
 -> next-volume NBZ publication
 -> canonical resolver/reopen/rematerialization
 -> original DMC3 consumption receipt
 -> final L1 acceptance audit
```

Current immediate work order:

1. validate and promote #212 so the closure receipt cryptographically binds its artifact-bound acquisition sidecar;
2. validate and promote #210 so recursive PAC/PNST slot-path receipts bind every declared slot to the actual changed span;
3. run the exact retail request `obj\\em000.pac` and record the resolver-selected member/archive receipt;
4. classify the observed retail representation without laundering the transformed texture corpus;
5. execute a bounded real edit, bottom-up rebuild, next-volume publication and canonical rematerialization;
6. complete issue #209 with deterministic original-game consumption and rollback evidence;
7. perform final L1 cross-stack synchronization/audit before any `L1 COMPLETE` claim.

## Maintained foundations

The following are established infrastructure and remain continuously maintained rather than treated as future phases:

- C++20/CMake and Windows + Ubuntu CI;
- evidence/artifact identity and hash-gated EXE analysis;
- GDSpaces ResourceId/ResourceRef/ByteProvenance/SourceRegistry;
- canonical PAC/PNST/NBZ read/materialization paths;
- WorkingCopy and bounded authoring contracts;
- Binary Inspector byte/structure authority;
- Reverse Core and Recovered Game Source Tree boundaries;
- guarded patch/export and validation infrastructure.

## GDSpaces L2 — Resource Resolution

L2 is structurally advanced. Candidate construction, numbered-volume bootstrap/precedence, archive normalization/index lookup and resolver ownership are strong. Remaining work is narrow evidence closure, especially exact type-0 physical-provider final-open semantics and representative real receipts.

L2 work may proceed when it supports L1, but it must not displace L1 closure.

## Original runtime / lifecycle

Recovered DMC3 resource runtime contains a substantial static spine: FileSlot/AsyncIO, ZIP read/inflate, LoadedResource state progression, typed post-load, loader claims and reset/release behavior. Remaining work includes exact open/error boundaries, selected ZIP helper bodies, broader dynamic lifecycle traces and Level-E validation.

Original runtime code remains in the Recovered Game Source Tree; GDSpaces consumes only confirmed contracts.

## Stage Ops and Stage Semantic Graph

After L1 has a real retail edit/rebuild/game-consumption receipt, Stage Ops becomes the primary product-side integration frontier:

```text
GDSpaces resolved/materialized resources
 -> Stage Ops assembly/orchestration
 -> Stage Semantic Graph
 -> ModViz
```

Stage Ops must never create a second resource resolver/materializer. Stage descriptor identity, numeric Stage identity and semantic gameplay identity remain distinct.

## EXE Editor / recovered source

The EXE Editor continues to become the front end over the Recovered Game Source Tree, exact binary mappings and evidence identities. Progressive reconstruction goals are:

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

1. **GDSpaces L1 accepted** with real retail provenance, rebuild/reopen and original-game consumption receipt.
2. **Narrow L2 closure** including exact physical-provider behavior where required.
3. **Representative L3 lifecycle validation** across load/reload/transition/release.
4. **Stage Ops game-backed assembly** over representative catalog selections.
5. **Stable Stage Semantic Graph and ModViz editing verticals.**
6. **First bounded recovered subsystem behavioral equivalence receipt.**
7. **Progressive recompilation** with controlled replacement modules.
8. **Working rebuilt executable milestones** without weakening evidence gates.

No milestone is promoted because of synthetic tests alone. See [completion policy](status/current.md), [blockers](status/blockers.md), and the [canonical L1 roadmap](gdspaces/l1-roadmap.md).