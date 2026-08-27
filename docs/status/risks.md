# Architecture and Project Risks

**Snapshot date:** 2026-08-27  
**Canonical base reviewed:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Boundary authority:** `../gdspaces/layer-boundary-status-reconciliation-2026-08-27.md`

## R-001 — Second resolver/materializer regression
**Severity:** critical

A tool or Stage subsystem may reopen paths, parse archives or rediscover resources independently.

**Mitigation:** GDSpaces remains the only product resource resolver/materializer. Stage Ops/ModViz/Binary Inspector consume shared identities and payloads.

## R-002 — False L1 completion from product maturity or synthetic composition
**Severity:** critical

Advanced product authoring, green A-to-Z synthetic tests, resolver success or structural parsing may be reported as Layer-1 completion.

**Mitigation:** L1 is canonically `INCOMPLETE / NOT 100%`. Completion requires both the mandatory materialization terminal-dependency gate and the real-retail/original-game receipts defined by the L1 roadmap.

## R-003 — Artifact snapshot split regression
**Severity:** high

Archive index metadata, selected member bytes and archive SHA may be observed from different physical file states.

**Mitigation:** preserve artifact-bound serialization/member observation; provenance-grade receipts bind all observations to one stable artifact identity.

## R-004 — No-clobber publication regression
**Severity:** high

New authoring/evidence commands may bypass shared atomic/no-replace publication.

**Mitigation:** staged validation + final no-replace publication remains mandatory; concurrency behavior stays a Windows+Ubuntu regression requirement.

## R-005 — Acquisition mutates measured retail tree
**Severity:** critical

An evidence command may publish output inside the retail tree whose identity/provider surface it measures.

**Mitigation:** fail closed for measured retail source trees; keep generated artifacts in explicit output/export locations.

## R-006 — Closed type-0 static boundary accidentally reopened
**Severity:** high

Older docs may still describe the bounded post-`0x0C` type-0 final-open path as the primary static reverse target.

**Mitigation:** #215/#204 remains canonical for that bounded L2 chain. Protected-process selected identity remains separate evidence work.

## R-007 — Materializer/repacker authority collapse
**Severity:** high

Read/materialization behavior may be treated as proof of original writer/offline-packer behavior.

**Mitigation:** separate original reader/materializer evidence, DMC Rengine writers, retail serialization preservation and Capcom offline-tool equivalence claims.

## R-008 — Retail representation laundering
**Severity:** critical

A transformed/historical corpus may be treated as pristine retail authority before direct-retail provenance exists.

**Mitigation:** acquire exact retail bytes first, then classify representation before writeback.

## R-009 — Pre-guessed archive member identity
**Severity:** high

Documentation/tooling may hard-code `GData*.afs/...` member paths instead of allowing the recovered resolver to select the actual winner.

**Mitigation:** acquisition starts from the game request and records the L2-selected source/member/volume.

## R-010 — Inferred parent extent treated as intrinsic child EOF
**Severity:** critical

Parent slot ranges may include padding/alignment and cannot automatically become intrinsic editable-child size authority.

**Mitigation:** exact-child authority requires independent intrinsic framing or validated complete-image writer receipts.

## R-011 — Recovered C++ false confidence
**Severity:** high

Readable/compiling recovered code may hide ABI, ownership or lifecycle errors.

**Mitigation:** exact artifact/range evidence, contradiction tracking and original-vs-reconstruction receipts.

## R-012 — Layer ownership bleed between L1/L2/L3
**Severity:** critical

A helper may be classified by subsystem name or address neighborhood rather than by semantics. The historical example is assigning selected-byte FileSlot/AsyncIO transport and normal state1->2 completion wholly to L3.

**Mitigation:** use the 2026-08-27 semantic rule:

```text
selection/usable identity -> L2
selected-byte acquisition/transport/transform/completion through state2 -> L1
state2 typed-ready/ownership/release/teardown -> L3
```

A shared scheduler/helper is classified per concrete action. L3 cancellation policy may suppress L1 completion without owning L1 transport.

## R-013 — AFS/PACK inference from names/history
**Severity:** high

Logical `.afs/` namespaces or historical PACK parser code may be promoted as original DMC3 binary backend authority.

**Mitigation:** freeze both absent direct runtime/raw evidence on the supported path.

## R-014 — Stage identity/scene truth collapse
**Severity:** high

Descriptor identity, Stage identity and semantic gameplay identity may collapse, or ModViz may create a second scene truth.

**Mitigation:** preserve distinct identity axes; Stage Ops owns assembly/orchestration; ModViz consumes Stage Ops state.

## R-015 — Historical/active branch truth reported as main truth
**Severity:** high

Stacked/stale PR findings may be described as canonical implementation/evidence.

**Mitigation:** branch truth remains branch truth until merged; status names exact main authority; stale branches are explicitly superseded.

## R-016 — Historical checklist/status drift
**Severity:** high

Old issues/docs may preserve superseded completion labels or layer ownership.

**Mitigation:** canonical current docs point to `layer-boundary-status-reconciliation-2026-08-27.md`, corrected roadmaps/classification and machine-readable status. Historical evidence remains history with supersession notes.

## R-017 — Public repository contamination
**Severity:** high

Proprietary game bytes or leaked source may be committed.

**Mitigation:** synthetic/public-safe fixtures, sanitized receipts and lawful local evidence only.

## R-018 — Premature original-file modification
**Severity:** high

Safe authoring work may evolve into implicit retail mutation.

**Mitigation:** WorkingCopy + explicit generated output + atomic publication + reopen/validation; retail files remain immutable by default.

## R-019 — Transport callback and higher completion conflation
**Severity:** critical

A reverse pass may treat `0x1400335A0` lower transport completion as equivalent to `0x1401B8DC0` state2 publication.

**Mitigation:** both are L1 but are different completion levels. `0x1400335A0` exposes raw transfer status; normal `0x1401B8DC0` receives only registry-relative context. The terminal dependency between them remains mandatory open reverse work.

## R-020 — Generic fan-in counter overclaim
**Severity:** critical

`fan-in/completion` may be read as proof of an original generic outstanding-child counter.

**Mitigation:** merged completion passes explicitly narrow the claim to **materialization completion ordering / dependency**. No universal counter is promoted.

## R-021 — FIFO-only completion proof
**Severity:** critical

Queue insertion order may be treated as sufficient proof that materialization is terminal before `0x1401B8DC0` executes.

**Mitigation:** require direct evidence of completion-aware persistence/retirement, callback-driven terminal state, another gate, synchronous completion or another proven mechanism. FIFO alone is insufficient after async submission.

## R-022 — Historical Pass-90 helper-role laundering
**Severity:** high

Historical derivative roles for `0x1400333E0` and `0x140033390` may be promoted as fresh canonical semantics.

**Mitigation:** keep them reacquisition anchors until exact canonical bytes validate status/poll and terminal release roles.

## R-023 — `0x1402EF460` mislabeled as OS cancellation
**Severity:** high

Higher scheduler pending-entry clear/rollback may be described as `CancelIo`/AsyncIO cancellation.

**Mitigation:** retain the bounded label **pending scheduler-entry clear/rollback**. Classify the concrete action: L1 completion suppression if it removes pending materialization completion, L3 policy if it originates from lifecycle replacement/cancellation. Already-running lower I/O cancellation is a separate question.

## R-024 — `.lst` completion semantics inferred before direct-resource closure
**Severity:** high

Loose-container child/recursive failure may be generalized before the direct-resource terminal dependency is known.

**Mitigation:** close `2EF4D0 -> 2EF790 -> 333E0/333390/3335A0 -> B8DC0` first; then apply the confirmed mechanism to `.lst` child failure/recursive ordering without reopening grammar/layout.

## R-025 — `internal product path closed` used as a completion proxy
**Severity:** critical

A bounded product-implementation statement may be interpreted as `L1 COMPLETE` or `only external receipts remain`.

**Mitigation:** canonical status explicitly separates product maturity from layer completion. Current allowed label is `L1 INCOMPLETE / NOT 100%`; the materialization terminal dependency is a mandatory internal gate.
