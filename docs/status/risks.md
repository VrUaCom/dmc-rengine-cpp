# Architecture and Project Risks

**Snapshot date:** 2026-08-15

## Critical risks

### R-001 — Second resource resolver regression

A consumer may bypass GDSpaces for direct path/archive lookup.

**Mitigation:** GDSpaces remains the only product resource resolver/materializer; Stage Ops receives resolved/materialized state; Semantic Graph and ModViz never perform their own archive discovery.

### R-002 — Tool ownership absorbs recovered vanilla code

Recovered resource, collision, stage, UI, cache, factory, or lifecycle functions may be placed under the tool that consumes their semantics.

**Mitigation:** original DMC3 runtime reconstruction belongs to the Recovered Game Source Tree. Tool relationship, game-subsystem membership, and temporary TaskClaim ownership remain distinct axes.

### R-003 — Duplicate reverse truth stores

EXE Editor, Binary Inspector, agents, or recovered-source tooling may create independent function/type/evidence identities.

**Mitigation:** one Reverse Core identity/reconstruction contract with adapters/bridges; no editor-local canonical function database.

### R-004 — Recovered C++ drifts from binary behavior

Readable/clean C++ can preserve neither ABI nor semantics.

**Mitigation:** exact artifact/range provenance, recorded ABI/lifetime assumptions, isolated builds, behavioral comparison, ValidationReceipt, correction/rejection history.

### R-005 — Canonical artifact identity corruption

The same SHA may be paired with conflicting sizes/metadata, causing real evidence to be rejected or false tuples to be accepted.

**Mitigation:** one canonical artifact record per immutable SHA, explicit correction/supersession records, repository invariant tests across machine-readable authorities, no silent metadata forks.

### R-006 — Integration-stack divergence

`main`, long-lived integration branches, recovered-runtime stacks, Stage Ops, and research can each be internally green while mutually stale.

**Mitigation:** one explicit integration spine, overlap review, whole-stack CI, promotion receipt, and status regenerated only against the composed commit.

### R-007 — Agent consensus masquerades as evidence

Parallel models may agree on the same wrong hypothesis.

**Mitigation:** agent agreement and TaskClaims are workflow metadata, never Evidence. Promotion requires artifact/runtime/test provenance.

### R-008 — TaskClaim liveness/deadlock

Claims may prevent duplicate mutation but become abandoned, circular, or permanently blocking.

**Mitigation:** scoped claims, ownership/conflict states, expiry/release/supersession rules, deterministic audit history, explicit blocked-by relationships.

### R-009 — ValidationReceipt invalidation is missing

A receipt may remain displayed as valid after its artifact, reconstruction, dependency, or behavior-test input changes.

**Mitigation:** receipts bind exact immutable input/reconstruction identities; dependency revision changes invalidate/recompute derived validation state.

## High risks

### R-010 — Materialized bytes are mislabeled game-ready

A `StageBundle`/payload may be treated as equivalent to original state-3 runtime state before post-load/factory/cache/lifetime reconstruction.

**Mitigation:** separate product-materialized and `game_ready_equivalent`; default the latter false until recovered-runtime evidence closes the boundary.

### R-011 — Stage identity axes collapse

Resource-set/catalog identity, numeric selector-facing Stage ID, and semantic gameplay identity may be equated by filename or convenience.

**Mitigation:** independent typed fields/identities, explicit mapping evidence, and tests that preserve aliases/shared resources. `st001` remains fixture-only.

### R-012 — Semantic Graph becomes a second scene model

Graph code may start resolving resources, owning mutable bytes, or manufacturing missing runtime semantics.

**Mitigation:** graph is deterministic/disposable Stage Ops projection only. Missing assembly information must be added to Stage Ops or reverse/parser authority first.

### R-013 — ModViz vertical-slice bypass

UI pressure may introduce direct archive/EXE writes or local scene discovery.

**Mitigation:** ModViz consumes Stage Ops + Semantic Graph and sends revision-guarded edits through shared WorkingCopy/patch contracts.

### R-014 — Mutable WorkingCopy is confused with immutable source span

Size-changing edits can invalidate tools that assume `ResourceId::size == current bytes.size()`.

**Mitigation:** ResourceId retains immutable/source-span identity; active Binary Document/parser views carry their own byte size and revision lineage. Regression-test insert/delete edits.

### R-015 — Branch-local green CI is overclaimed

A feature branch may pass tests but still omit newer sibling-stack behavior or evidence corrections.

**Mitigation:** always record branch/PR/head scope; require composed-stack CI before project-wide promotion.

### R-016 — HITS compatibility overclaim

A deterministic DMC Rengine SAT writer may be described as equivalent to Capcom's unknown offline builder/runtime.

**Mitigation:** retain research-required status until real corpus and controlled runtime receipts support equivalence.

### R-017 — Collision source2 or arbitration is invented

Unknown source2 backing/lifetime or unresolved arbitration rules may be filled by plausible product logic.

**Mitigation:** keep explicit unknowns; prioritize raw evidence around `0x14005E7A0`, `0x14005B460`, `0x14005FEC0`, `0x1400601E0`.

### R-018 — SCM post-load conflict is silently promoted

Conflicting interpretation around `mesh+0x28` may contaminate typed post-load/game-ready behavior.

**Mitigation:** gate SCM post-load promotion until contradiction is resolved by evidence.

### R-019 — Resource materialization budgets are unbounded

Recursive container expansion or broad stage assembly can create excessive memory/time usage on malformed or huge inputs.

**Mitigation:** explicit depth/count/byte budgets, bounded readers, diagnostics, partial-result preservation, and fail-closed writer behavior.

### R-020 — Parser/input vulnerabilities

Malformed binary/JSON can trigger overflow, bounds, recursion, traversal, or allocation failures.

**Mitigation:** checked arithmetic, containment guards, parser limits, deterministic diagnostics, malformed corpora, fuzzing where appropriate.

### R-021 — Public repository contamination

Proprietary game binaries/assets/saves or leaked source may be committed.

**Mitigation:** clean-room policy, synthetic fixtures, local user-supplied resources, scanning/ignore policy, and public CI free of proprietary bytes.

### R-022 — Recovered-source bulk import

Large historical source snapshots may bypass per-unit evidence/ABI review.

**Mitigation:** narrow promotion through immutable source package -> evidence -> reviewed C++ -> compile/tests -> behavioral receipt.

## Medium risks

### R-023 — UI drives domain architecture

Editor widgets may redefine resource identity, ownership, semantic state, or write policy.

**Mitigation:** domain/API contracts precede UI breadth.

### R-024 — Architecture monolith

Triangle Forge, Reverse Core, GDSpaces, Stage Ops, or ProjectWorkspace may absorb unrelated responsibilities.

**Mitigation:** enforce explicit ownership boundaries and dependency direction.

### R-025 — Documentation/research drift

GitHub docs/issues, Drive research, code, and receipts may diverge.

**Mitigation:** date/scope every status surface, append corrections, sync after evidence or integration changes, and never infer current authority from filename alone.

### R-026 — Premature production writes

Archive or executable writes may expand faster than read/validation/rollback evidence.

**Mitigation:** immutable source, WorkingCopy, expected-byte/hash guards, copied output, deterministic manifests, rollback, evidence-backed writer specs.

### R-027 — Release boundary overclaim

Custom build identity or copied-output patching may be called a reproducible/public release system.

**Mitigation:** no release claim without deterministic build/output, validation, signing/attestation, and runtime regression receipts.

## Current architecture invariant

```text
GDSpaces = product resource authority
Recovered Game Source Tree = reconstructed vanilla runtime
Reverse Core = generic evidence/reconstruction authority
Stage Ops = scene assembly + operational state
Semantic Graph = derived semantic/evidence representation
ModViz = editor over Stage Ops state
Binary Inspector = byte/structure/evidence inspection
EXE Editor = reconstruction editor over shared identities/source
```

Any change that makes two of these layers independently own the same canonical state is an architecture regression unless an explicit superseding ADR says otherwise.
