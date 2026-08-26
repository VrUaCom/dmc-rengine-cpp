# Architecture and Project Risks

**Snapshot date:** 2026-08-26  
**Canonical base:** `main@a90b017ab29171e00174f2a56c719c32241a63f1`

## R-001 — Second resolver/materializer regression
**Severity:** critical

A tool or Stage subsystem may reopen paths, parse archives or rediscover resources independently.

**Mitigation:** GDSpaces remains the only product resource resolver/materializer. Parsers consume supplied spans. Stage Ops/ModViz/Binary Inspector consume shared identities and payloads.

## R-002 — False L1 completion from synthetic composition
**Severity:** critical

Green A-to-Z synthetic tests, resolver success or structural parsing may be reported as original-game materialization equivalence.

**Mitigation:** [L1 roadmap](../gdspaces/l1-roadmap.md) mandatory gates require direct-retail provenance, real rebuild/reopen and original-game consumption before completion.

## R-003 — Artifact snapshot split regression
**Severity:** high

Future code may again observe archive index metadata, selected member bytes and archive SHA from different physical file states.

**Mitigation:** preserve the artifact-bound serialization/member observation contract promoted through #195–#197; provenance-grade receipts must bind all observations to one stable artifact identity.

## R-004 — No-clobber publication regression
**Severity:** high

New authoring/evidence commands may bypass the shared atomic/no-replace publication primitive.

**Mitigation:** generated artifacts use staged validation + final no-replace publication; concurrency behavior remains a Windows+Ubuntu regression requirement.

## R-005 — Acquisition mutates measured retail tree
**Severity:** critical

An evidence command may publish output inside the same retail tree whose identity/provider surface it measures.

**Mitigation:** fail closed for outputs inside measured retail source trees; keep generated artifacts in explicit output/export locations.

## R-006 — Closed type-0 static boundary accidentally reopened
**Severity:** high

Older docs may still describe the bounded post-`0x0C` type-0 final-open path as the primary unresolved static reverse target.

**Mitigation:** #215/#204 is canonical for that bounded static chain. Protected-process selected identity and cross-build runtime mapping remain separate evidence gates.

## R-007 — Materializer/repacker authority collapse
**Severity:** high

Read/materialization behavior may be treated as proof of original writer/offline-packer behavior.

**Mitigation:** separate product materializer, DMC Rengine writer, retail serialization preservation and Capcom offline-tool equivalence claims.

## R-008 — Retail representation laundering
**Severity:** critical

A transformed or historical corpus may be treated as pristine retail authority before direct-retail member provenance is established.

**Mitigation:** acquire exact retail member bytes first, then classify representation. Only an observed representation inside an evidenced writer domain may advance to writeback.

## R-009 — Pre-guessed archive member identity
**Severity:** high

Documentation or tooling may hard-code `GData*.afs/...` member paths instead of letting the recovered resolver select the actual basename candidate/volume winner.

**Mitigation:** acquisition begins from the game request and records the resolver-selected member/source/volume.

## R-010 — Inferred parent extent treated as intrinsic child EOF
**Severity:** critical

Parent slot ranges may include padding/alignment and cannot automatically become intrinsic editable-child size authority.

**Mitigation:** exact-child authority requires independent intrinsic framing or validated complete-image writer receipts.

## R-011 — Recovered C++ false confidence
**Severity:** high

Readable/compiling recovered code may hide ABI, ownership or lifecycle errors.

**Mitigation:** exact artifact/range evidence, contradiction tracking and controlled original-vs-reconstruction behavioral receipts.

## R-012 — Original/runtime ownership leakage into GDSpaces
**Severity:** high

LoadedResource/FileSlot/cache/lifecycle code may be moved into product resource modules because GDSpaces consumes its behavior.

**Mitigation:** original runtime reconstruction remains in Recovered Game Source Tree. FileSlot byte-read mechanics may support L1, but original request ownership/scheduler/callback lifecycle and LoadedResource states remain L3.

## R-013 — AFS/PACK inference from names/history
**Severity:** high

Logical `.afs/` namespaces or historical product PACK parser code may be promoted as original DMC3 binary backend authority.

**Mitigation:** freeze both absent direct runtime/raw evidence that places them on the supported path.

## R-014 — Stage identity/scene truth collapse
**Severity:** high

Descriptor identity, numeric Stage identity and semantic gameplay identity may collapse, or ModViz may create a second scene truth.

**Mitigation:** preserve distinct identity axes; Stage Ops owns assembly/orchestration; ModViz consumes Stage Ops state.

## R-015 — Historical/active branch truth reported as main truth
**Severity:** high

Stacked or stale PR findings may be described as canonical implementation/evidence.

**Mitigation:** every current status names exact main SHA. A branch remains branch truth until merged; stale branches are superseded explicitly rather than silently reused.

## R-016 — Historical checklist drift
**Severity:** high

Old issues/docs may continue to list promoted work as pending or preserve superseded function labels/addresses.

**Mitigation:** canonical current docs point to merged #228/#230 and the focused materialization-completion packet. Historical evidence remains history with explicit supersession notices.

## R-017 — Public repository contamination
**Severity:** high

Proprietary game bytes or leaked source may be committed.

**Mitigation:** synthetic/public-safe fixtures, sanitized receipts and legal local artifacts only. Canonical EXE raw windows remain private local evidence unless redistribution is lawful.

## R-018 — Premature original-file modification
**Severity:** high

Safe authoring work may evolve into implicit retail file mutation.

**Mitigation:** WorkingCopy + explicit generated output + atomic publication + reopen/validation; retail files remain immutable by default.

## R-019 — Transport callback and higher completion conflation
**Severity:** critical

A reverse pass may treat `0x1400335A0` lower transport completion as equivalent to `0x1401B8DC0` LoadedResource state2 completion.

**Mitigation:** keep the layers separate. `0x1400335A0` writes lower transfer progress/status; normal `0x1401B8DC0` receives only a u32 registry-relative context and does not receive raw transport error/status metadata.

## R-020 — Generic fan-in counter overclaim
**Severity:** critical

The phrase `fan-in/completion` may be read as proof of an original generic outstanding-child counter or `N -> 1` completion object.

**Mitigation:** merged #228 explicitly narrows the claim to **materialization completion ordering / dependency bridge**. A counter remains only one hypothesis until direct bytes prove it.

## R-021 — FIFO-only completion proof
**Severity:** critical

Queue insertion order may be treated as sufficient proof that materialization is terminal before the later `0x1401B8DC0` callback executes.

**Mitigation:** Pass 2 requires completion-aware persistence/retirement, callback-driven terminal state, another scheduler gate, synchronous completion, or another directly evidenced mechanism. FIFO alone is insufficient if the materialization job can retire immediately after async submission.

## R-022 — Historical Pass-90 helper-role laundering
**Severity:** high

Historical derivative evidence around `0x1400333E0` and `0x140033390` may be promoted as fresh canonical function semantics.

**Mitigation:** keep them as **reacquisition anchors** only until exact canonical `e454...` bytes validate status/poll and terminal release roles.

## R-023 — `0x1402EF460` mislabeled as OS cancellation
**Severity:** high

Higher scheduler pending-entry clear/rollback may be described as `CancelIo`/AsyncIO cancellation.

**Mitigation:** use the bounded label **pending scheduler-entry clear/rollback** until direct lower-I/O interaction is recovered. Already-running FileSlot/ReadRequest cancellation remains a separate question.

## R-024 — `.lst` completion semantics inferred before direct-resource closure
**Severity:** high

Loose-container child/recursive failure may be generalized before the direct-resource terminal dependency mechanism is known.

**Mitigation:** close the direct-resource `2EF4D0 -> 2EF790 -> 333E0/333390/3335A0 -> B8DC0` dependency model first; then apply it to `.lst` child failure/recursive ordering without reopening already recovered grammar/layout.
