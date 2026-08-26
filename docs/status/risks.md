# Architecture and Project Risks

**Snapshot date:** 2026-08-27  
**Reconciled canonical main:** through merged PR #242  
**Pending branch truth:** #226, #238, #240, #241

## R-001 — Second resolver/materializer regression
**Severity:** critical

A tool or Stage subsystem may reopen paths, parse archives or rediscover resources independently.

**Mitigation:** GDSpaces remains the only product resource resolver/materializer. Parsers consume supplied spans. Stage Ops/ModViz/Binary Inspector consume shared identities and payloads. Pocket GDS must remain a thin execution/UI surface over its pinned canonical GDSpaces snapshot rather than grow an independent NBZ/PAC/PNST parser.

## R-002 — False L1 completion from synthetic or mobile composition
**Severity:** critical

Green synthetic A-to-Z tests, resolver success, structural parsing, successful Pocket export or crash-free original launch may be reported as original-game materialization equivalence.

**Mitigation:** L1 requires same-lineage selected-source/member provenance, exact materialization, real rebuild/reopen/rematerialization, #209 deterministic original-game consumption, rollback and final V:L1 audit.

## R-003 — Evidence receipt snapshot split
**Severity:** critical

Archive metadata, selected bytes and archive SHA may be observed from different physical states.

**Mitigation:** canonical NBZ acquisition uses artifact-bound serialization/member observation (#195–#197). Final acceptance still requires all cross-tool receipts belong to one reconciled lineage.

## R-004 — False no-clobber publication
**Severity:** critical

`exists() -> ofstream` can race and violate claimed no-clobber behavior.

**Mitigation:** shared atomic/no-replace publication (#194) and Windows/Ubuntu concurrency regressions remain mandatory.

## R-005 — Acquisition mutates measured retail tree
**Severity:** critical

Evidence tooling may publish into the same retail tree whose identity it measures.

**Mitigation:** fail closed for outputs inside measured retail source trees. Level-E may temporarily add only the explicit next-volume overlay followed by verified rollback.

## R-006 — Product physical lookup mistaken for original Win32 semantics
**Severity:** high

Product `LocalDirectorySource` behavior may be described as exact DMC3 type-0 provider behavior.

**Mitigation:** #215 closes only the bounded evidenced original edge. Keep product filesystem behavior and unobserved failure cases distinct.

## R-007 — Materializer/repacker authority collapse
**Severity:** high

Read/materialization behavior may be treated as proof of original writer/offline packer behavior.

**Mitigation:** separate original materializer behavior, DMC Rengine writer, retail serialization preservation and Capcom offline-tool equivalence.

## R-008 — Retail representation laundering
**Severity:** critical

A filename-familiar or transformed resource may be treated as pristine retail writer authority before exact source/member provenance and representation are established.

**Mitigation:** acquire exact bytes + ByteProvenance first, then classify representation. Only an observed representation inside an evidenced writer domain advances to writeback.

## R-009 — Pre-guessed archive member identity
**Severity:** high

Tooling/docs may hard-code assumed paths or treat `obj\em000.pac` as the winner without preserving actual selection evidence.

**Mitigation:** protected-install acquisition begins from the game request and records actual selected provider/member. Pocket evidence joins final acceptance only after independent selected-source binding; filename equality is insufficient.

## R-010 — Inferred parent extent treated as intrinsic child EOF
**Severity:** critical

Parent slot ranges may include padding/alignment and cannot automatically become intrinsic editable-child size authority.

**Mitigation:** exact-child authority requires independent framing or validated complete-image writer receipts.

## R-011 — Recovered C++ false confidence
**Severity:** high

Readable/compiling recovered code may hide ABI, ownership or lifecycle errors.

**Mitigation:** exact artifact/range evidence, contradiction tracking and controlled original-vs-reconstruction behavioral receipts.

## R-012 — Original/runtime ownership leakage into GDSpaces
**Severity:** high

LoadedResource/FileSlot/cache/lifecycle code may be moved into product resource modules because GDSpaces consumes its behavior.

**Mitigation:** original runtime reconstruction remains separate. #230/#242 preserve the L1/L3 materialization boundary.

## R-013 — AFS/PACK inference from names/history
**Severity:** high

Logical `.afs/` namespaces or historical PACK parser code may be promoted as original DMC3 binary-backend authority.

**Mitigation:** freeze absent direct runtime/raw evidence.

## R-014 — Stage identity/scene truth collapse
**Severity:** high

Descriptor identity, numeric Stage identity and semantic gameplay identity may collapse, or ModViz may create a second scene truth.

**Mitigation:** preserve distinct identity axes; Stage Ops owns downstream assembly; ModViz consumes Stage Ops state.

## R-015 — Branch truth reported as main truth
**Severity:** high

An active PR/local APK/operator report may be described as canonical before merge or receipt review.

**Mitigation:** current status names merged authority. #226/#238/#240/#241 remain branch truth until merged.

## R-016 — Historical checklist drift
**Severity:** high

Old issues/docs may keep already-promoted work pending or preserve superseded environment assumptions.

**Mitigation:** current status/roadmaps are synchronized together. Historical pass/audit docs remain chronology and receive explicit supersession/reconciliation rather than silent rewrite.

## R-017 — Repository contamination
**Severity:** high

Proprietary game bytes or leaked source may be committed.

**Mitigation:** synthetic/public-safe fixtures, byte-free receipts and legal local artifacts only.

## R-018 — Premature original-file modification
**Severity:** high

Safe authoring may evolve into implicit retail mutation.

**Mitigation:** WorkingCopy + explicit generated output + atomic publication + reopen validation. Retail originals remain immutable; Level-E temporarily adds only a known overlay and removes it after verification.

## R-019 — Transport ceiling misreported as artifact absence
**Severity:** high

Failure to ingest the 960,358,951-byte `DMC3-0.nbz` may be reported as archive absence.

**Mitigation:** preserve separate facts: archive locatable, observed size 960,358,951 bytes, connected raw materialization ceiling 268,435,456 bytes. Treat as transport/execution scope.

## R-020 — Mobile receipt authority laundering
**Severity:** critical

A valid Pocket `gdspaces.l1.member-acquisition-receipt.v1` may be promoted into protected executable authority, original resolver winner, successful original mount topology or original-game consumption.

**Mitigation:** mobile receipts are local snapshot/member/materialization evidence only. Final promotion independently binds selected source and #209 consumption/rollback.

## R-021 — Filename discovery collapsed into successful mount topology
**Severity:** critical

`first_missing_archive_volume` or filename presence may be interpreted as proof that every discovered archive mounted successfully. #235 proves bootstrap can continue after an existing archive registration failure.

**Mitigation:** keep discovery/attempt plan and actual successful mounted set distinct. Issue #237 tracks correction; PR #241 is pending. Successful mounts may be sparse. Successful archive registration prepends, so precedence is higher successful volume -> lower successful volume -> physical. Provider/backend failure must not become a clean miss.

## R-022 — Independent PASS receipts composed by filename
**Severity:** critical

Mobile member PASS, desktop authoring PASS, L2 selected-identity PASS and original-game observation from different lineages may be combined because they share a path string.

**Mitigation:** V/LV requires one reconciled selected provider/archive/member -> L1 bytes -> authored/rebuilt/overlay identity -> original consumer effect -> rollback lineage. Filenames are descriptive, not evidence joins.

## R-023 — PID reuse / runtime mapping instance confusion
**Severity:** critical

A protected-process mapping packet may combine windows from different launches that reused a PID or trust child-declared canonical hashes.

**Mitigation:** #235 R2B v2 requires PID + non-zero OS-derived process creation FILETIME + module identity across all windows and independently derives seven canonical anchor hashes from the canonical EXE. Legacy v1 is not promotion authority.

## R-024 — Archive normalized-key collision ambiguity
**Severity:** critical

Original archive indexing sorts/searches `0x0E` normalized strings with no equal-key secondary tie-break. Collisions could make a deterministic winner claim unsafe.

**Mitigation:** require exact archive-SHA-bound retail member surface and run the `0x0E` collision census before uniqueness-dependent selection claims.

## R-025 — Completion callback publishes state2 before transport is terminal
**Severity:** critical

Older wording could imply scheduler FIFO or a generic child/outstanding-work counter guarantees safe materialization completion. #230/#242 prove normal `0x1401B8DC0` receives only one u32 registry-relative context and no lower transport status/error/byte count/FileSlot/child metadata.

If the earlier scheduler job merely submits asynchronous work and retires, a later normal completion callback could publish state2 too early.

**Mitigation:** treat this as materialization completion ordering / dependency bridge. Reacquire exact bytes in order `0x1402EF4D0` -> relevant `0x1402EF790` -> historical `0x1400333E0`/`0x140033390` hypotheses -> `0x1400335A0` -> normal `0x1401B8DC0` suppression/block condition -> `0x1402EF460` -> `.lst` recursive failure ordering. Do not promote FIFO-only or generic fan-in models without direct evidence.

## R-026 — Static L3 bounded closure reported as full L3 completion
**Severity:** high

Pending #240 may promote exact canonical `LoadedResource +0x04` writer census to `STATIC BOUNDED-CLOSED / APPROVED / CONTRADICTION-GATED` and be misreported as Layer 3 complete.

**Mitigation:** until merge it is branch truth. After promotion it closes only R1 for its exact artifact/scope; R2-R5 and V1-V7 remain open.

## R-027 — Control plane or validation architecture becomes accidental L4
**Severity:** high

Draft RCP/V-LV work may absorb ownership from L1/L2/L3 or be described as a fourth execution layer.

**Mitigation:** RCP is orthogonal orchestration; V/LV is cross-cutting validation. L1 owns materialization/authoring, L2 resolution/selection, L3 original runtime/lifecycle. Neither changes completion accounting by itself.
