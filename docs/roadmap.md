# DMC Rengine Roadmap

**Snapshot:** 2026-08-24  
**Canonical base:** `main@c4920c8602dd7492b6c89e9fc8ecf8a6d8397ee0`

The project roadmap is dependency-driven rather than a linear feature checklist. The current primary execution program is **GDSpaces Layer 1 — Resource Materialization**. Its detailed acceptance gates live in [GDSpaces L1 Roadmap](gdspaces/l1-roadmap.md).

## Current critical path — GDSpaces L1

```text
publication integrity
 -> artifact-stable retail member acquisition
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

1. unify atomic/no-replace publication across NBZ repack, overlay and acquisition outputs;
2. bind NBZ index/member bytes/archive SHA to one stable artifact observation;
3. correct and promote the retail-member acquisition seam (#191);
4. acquire an exact retail receipt using game request `obj\\em000.pac` and record the actual resolver-selected member;
5. classify the direct-retail texture/container representation;
6. use only an evidenced writer domain for the real edit/rebuild;
7. publish as the next contiguous `DMC3-N.nbz`, reopen through canonical GDSpaces and verify exact edited bytes;
8. obtain an original-game consumption receipt on the protected distribution execution authority;
9. perform final cross-stack review before any L1 completion claim.

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