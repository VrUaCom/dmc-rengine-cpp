# Architecture and Project Risks

**Snapshot date:** 2026-08-25  
**Canonical base:** `main@8e67235fd26cf7af94146f4dc660eb49e3c1d133`

## R-001 — Second resolver/materializer regression
**Severity:** critical

A tool or Stage subsystem may reopen paths, parse archives or rediscover resources independently.

**Mitigation:** GDSpaces remains the only product resource resolver/materializer. Parsers consume supplied spans. Stage Ops/ModViz/Binary Inspector consume shared identities and payloads.

## R-002 — False L1 completion from synthetic composition
**Severity:** critical

Green A-to-Z synthetic tests, resolver success or structural parsing may be reported as original-game materialization equivalence.

**Mitigation:** [L1 roadmap](../gdspaces/l1-roadmap.md) mandatory gates require direct-retail provenance, real rebuild/reopen and original-game consumption before completion.

## R-003 — Composite evidence receipt detachment
**Severity:** critical

Artifact-bound acquisition is merged, but a higher-level closure receipt can still be detached from or falsely paired with the acquisition sidecar unless it binds and validates that exact sidecar.

**Mitigation:** #212 makes the sidecar path, size and SHA-256 mandatory closure fields and validates the sidecar's request, output path, provenance, member size and member SHA against the closure bytes. A real receipt remains invalid when the sidecar is missing, unbound or mismatched.

## R-004 — False no-clobber publication
**Severity:** critical / product mitigation merged

The historical `exists() -> ofstream` race could replace evidence outputs.

**Mitigation:** #194 introduced shared atomic/no-replace publication. Preserve Windows/Ubuntu regressions and do not bypass the primitive in new artifact paths.

## R-005 — Acquisition mutates measured retail tree
**Severity:** critical

An evidence command may publish output inside the same retail tree whose identity/provider surface it measures.

**Mitigation:** fail closed for outputs inside measured retail source trees; keep generated artifacts in explicit output/export locations.

## R-006 — Product physical lookup mistaken for original Win32 semantics
**Severity:** high

Current `LocalDirectorySource`/physical index behavior may be described as exact DMC3 type-0 provider behavior.

**Mitigation:** keep the recovered `0x0C` normalization boundary separate from unresolved exact filename/case/CreateFile/open/failure semantics.

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

**Mitigation:** acquisition begins from the game request (for example `obj\\em000.pac`) and records the actual resolver-selected member.

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

**Mitigation:** original runtime reconstruction remains in Recovered Game Source Tree; validation receipts bridge behavior without collapsing ownership.

## R-013 — AFS/PACK inference from names/history
**Severity:** high

Logical `.afs/` namespaces or historical product PACK parser code may be promoted as original DMC3 binary backend authority.

**Mitigation:** freeze both absent direct runtime/raw evidence that places them on the supported path.

## R-014 — Stage identity/scene truth collapse
**Severity:** high

Descriptor identity, numeric Stage identity and semantic gameplay identity may collapse, or ModViz may create a second scene truth.

**Mitigation:** preserve distinct identity axes; Stage Ops owns assembly/orchestration; ModViz consumes Stage Ops state.

## R-015 — Branch truth reported as main truth
**Severity:** high

Active #210/#211/#212 or historical stacked PR findings may be described as canonical implementation.

**Mitigation:** every current status names exact main SHA; branch work stays branch truth until merged.

## R-016 — Historical checklist drift
**Severity:** high

Old issues/docs may continue to list already-promoted PNST/NBZ/provider work as pending or preserve superseded target paths.

**Mitigation:** current docs point to the canonical L1 roadmap; historical material remains history but receives explicit supersession/reconciliation notices when it can misdirect work.

## R-017 — Public repository contamination
**Severity:** high

Proprietary game bytes or leaked source may be committed.

**Mitigation:** synthetic/public-safe fixtures, sanitized receipts and legal local artifacts only.

## R-018 — Premature original-file modification
**Severity:** high

Safe authoring work may evolve into implicit retail file mutation.

**Mitigation:** WorkingCopy + explicit generated output + atomic publication + reopen/validation; retail files remain immutable by default.
