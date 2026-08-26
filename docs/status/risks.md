# Architecture and Project Risks

**Snapshot date:** 2026-08-26  
**Canonical base:** `main@c147facb310d32ef084c56ba82d1e4b6b9b1b496`

## R-001 — Second resolver/materializer regression
**Severity:** critical

A tool or Stage subsystem may reopen paths, parse archives or rediscover resources independently.

**Mitigation:** GDSpaces remains the only product resource resolver/materializer. Parsers consume supplied spans. Stage Ops/ModViz/Binary Inspector consume shared identities and payloads.

## R-002 — False L1 completion from synthetic composition
**Severity:** critical

Green A-to-Z synthetic tests, resolver success or structural parsing may be reported as original-game materialization equivalence.

**Mitigation:** [L1 roadmap](../gdspaces/l1-roadmap.md) mandatory gates require direct-retail provenance, real rebuild/reopen and original-game consumption before completion.

## R-003 — Evidence receipt snapshot split regression
**Severity:** high

Future code may accidentally separate archive index metadata, selected member bytes and archive SHA into different physical-file observations even though artifact binding is now canonical.

**Mitigation:** preserve the promoted artifact-bound serialization/member observation contract from #195–#197 and reject new provenance paths that reopen stale observations without revalidation.

## R-004 — No-clobber publication regression
**Severity:** high

New authoring/evidence commands may bypass the canonical atomic/no-replace publication primitive and reintroduce `exists() -> write` races.

**Mitigation:** all generated artifacts use the shared staged validation + final no-replace publication contract promoted through #194.

## R-005 — Acquisition mutates measured retail tree
**Severity:** critical

An evidence command may publish output inside the same retail tree whose identity/provider surface it measures.

**Mitigation:** fail closed for outputs inside measured retail source trees; keep generated artifacts in explicit output/export locations.

## R-006 — Closed type-0 physical boundary accidentally reopened
**Severity:** high

Old docs/branches may still describe type-0 post-`0x0C` final filename/open/miss semantics as unresolved and drive duplicate reverse work.

**Mitigation:** #215/#204 is canonical for the bounded static physical-provider contract. Reopen only on contradictory direct evidence or a newly claimed scope beyond that contract.

## R-007 — Materializer/repacker authority collapse
**Severity:** high

Read/materialization behavior may be treated as proof of original writer/offline-packer behavior.

**Mitigation:** separate product materializer, DMC Rengine writer, retail serialization preservation and Capcom offline-tool equivalence claims.

## R-008 — Retail representation laundering
**Severity:** critical

The preserved transformed DDS-bearing corpus may be treated as pristine retail authority before direct-retail member provenance is established.

**Mitigation:** first acquire exact retail member bytes, then classify representation. Only an observed representation inside an evidenced writer domain may advance to writeback.

## R-009 — Pre-guessed archive member identity
**Severity:** high

Documentation or tooling may hard-code `GData*.afs/...` member paths instead of letting the recovered resolver select the actual basename candidate/volume winner.

**Mitigation:** acquisition begins from the game request and records the actual resolver-selected member.

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

LoadedResource/cache/lifecycle code may be moved into product resource modules because GDSpaces consumes its behavior.

**Mitigation:** original runtime reconstruction remains in Recovered Game Source Tree; validation receipts bridge behavior without collapsing ownership. FileSlot transport used for L1 byte acquisition does not imply wholesale lifecycle ownership by GDSpaces.

## R-013 — AFS/PACK inference from names/history
**Severity:** high

Logical `.afs/` namespaces or historical product PACK parser code may be promoted as original DMC3 binary backend authority.

**Mitigation:** freeze both absent direct runtime/raw evidence that places them on the supported path.

## R-014 — Stage identity/scene truth collapse
**Severity:** high

Descriptor identity, numeric Stage identity and semantic gameplay identity may collapse, or ModViz may create a second scene truth.

**Mitigation:** preserve distinct identity axes; Stage Ops owns assembly/orchestration; ModViz consumes Stage Ops state.

## R-015 — Historical branch truth reported as main truth
**Severity:** high

Old stacked L1 PRs may remain open and be mistaken for current implementation authority.

**Mitigation:** every current status names exact `main` SHA. Historical open PRs such as #162/#170/#175/#176/#178/#181 are evidence history only unless cleanly respun from current `main` and reviewed against current canonical boundaries. Active #218/#219 remain branch truth until merged.

## R-016 — Historical checklist drift
**Severity:** high

Old issues/docs may continue to list promoted work as pending or preserve superseded function labels.

**Mitigation:** canonical current docs point to the L1 roadmap and 2026-08-26 EXE boundary review. Historical evidence remains immutable but receives explicit reconciliation notices where it can misdirect work.

## R-017 — Public repository contamination
**Severity:** high

Proprietary game bytes or leaked source may be committed.

**Mitigation:** synthetic/public-safe fixtures, sanitized receipts and legal local artifacts only. Guarded EXE window acquisition writes proprietary raw bytes only to private local output when explicitly requested.

## R-018 — Premature original-file modification
**Severity:** high

Safe authoring work may evolve into implicit retail file mutation.

**Mitigation:** WorkingCopy + explicit generated output + atomic publication + reopen/validation; retail files remain immutable by default.

## R-019 — Transport callback and resource completion conflation
**Severity:** critical

A reverse pass may treat raw transport completion as equivalent to resource materialization completion, hiding a scheduler/fan-in/error layer.

**Mitigation:** keep `0x1400335A0` as transport/whole-file completion and `0x1401B8DC0` as resource-level scheduler/materialization completion to state2. Reverse the bridge explicitly before making fan-in/error-equivalence claims.

## R-020 — `0x1402EF4D0` overclaim regression
**Severity:** high

The materialization submission wrapper may again be mislabeled as exact-path resolver, final provider open, synchronous reader or OS-read wrapper.

**Mitigation:** canonical safe label is **resource materialization submission/scheduling wrapper** until stronger direct evidence exists.

## R-021 — `.lst` synchronous-wrapper identity laundering
**Severity:** high

Because `.lst` text acquisition is synchronous and a separate synchronous-style whole-file wrapper exists around `0x1402EF920`, future work may equate them by proximity rather than evidence.

**Mitigation:** keep the identities separate until a direct caller/callee edge is reacquired. Current open target is temporary allocation/free/failure cleanup plus child fan-in semantics.
