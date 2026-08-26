# Architecture and Project Risks

**Snapshot date:** 2026-08-26  
**Reconciled main base:** `main@a90b017ab29171e00174f2a56c719c32241a63f1`

## R-001 — Second resolver/materializer regression
**Severity:** critical

A tool or Stage subsystem may reopen paths, parse archives or rediscover resources independently.

**Mitigation:** GDSpaces remains the only product resource resolver/materializer. Parsers consume supplied spans. Stage Ops/ModViz/Binary Inspector consume shared identities and payloads. Pocket GDS must remain a thin execution/UI surface over its pinned canonical GDSpaces snapshot rather than grow an independent NBZ/PAC/PNST parser.

## R-002 — False L1 completion from synthetic or mobile composition
**Severity:** critical

Green A-to-Z synthetic tests, resolver success, structural parsing, a successful Pocket archive open/export or a crash-free original-game launch may be reported as original-game materialization equivalence.

**Mitigation:** [L1 roadmap](../gdspaces/l1-roadmap.md) requires same-lineage selected-source/member provenance, exact materialization, real rebuild/reopen/rematerialization, #209 deterministic original-game consumption, rollback and final V:L1 audit before completion.

## R-003 — Evidence receipt snapshot split
**Severity:** critical

Archive index metadata, selected member bytes and archive SHA may be observed from different physical file states.

**Mitigation status:** implemented for canonical NBZ acquisition through artifact-bound serialization/member observation (#195–#197). Final acceptance still requires that every cross-tool receipt belongs to the same reconciled source lineage.

## R-004 — False no-clobber publication
**Severity:** critical

`exists() -> ofstream` can race and replace/create evidence outputs contrary to claimed no-clobber behavior.

**Mitigation status:** implemented through the shared atomic/no-replace publication contract (#194) and adopted authoring/acquisition seams. Preserve concurrency regression coverage on Windows and Ubuntu.

## R-005 — Acquisition mutates measured retail tree
**Severity:** critical

An evidence command may publish output inside the same retail tree whose identity/provider surface it measures.

**Mitigation:** fail closed for outputs inside measured retail source trees; keep generated artifacts in explicit output/export locations. The only allowed retail-tree write in Level-E is the explicit temporary next-volume overlay copy followed by verified rollback.

## R-006 — Product physical lookup mistaken for original Win32 semantics
**Severity:** high

Product `LocalDirectorySource` behavior may be described as exact DMC3 type-0 provider behavior.

**Mitigation status:** the bounded type-0 final-open static contract is closed by #215, including the specific resolver edge to the shared low-level `CreateFileA` helper. Continue to distinguish that evidenced original edge from broader product filesystem behavior and from unobserved runtime failure cases.

## R-007 — Materializer/repacker authority collapse
**Severity:** high

Read/materialization behavior may be treated as proof of original writer/offline-packer behavior.

**Mitigation:** separate product materializer, DMC Rengine writer, retail serialization preservation and Capcom offline-tool equivalence claims.

## R-008 — Retail representation laundering
**Severity:** critical

A transformed or filename-familiar resource may be treated as pristine retail writer authority before exact source/member provenance and representation are established.

**Mitigation:** first acquire exact materialized bytes and ByteProvenance from an accepted source lineage, then classify representation. Only an observed representation inside an evidenced writer domain may advance to writeback. A Pocket receipt can supply member/classification evidence but cannot self-declare original resolver selection.

## R-009 — Pre-guessed archive member identity
**Severity:** high

Documentation or tooling may hard-code `GData*.afs/...` member paths or assume `obj\\em000.pac` is a particular archive member instead of preserving actual selection evidence.

**Mitigation:** protected-install acquisition begins from the game request and records the actual selected provider/member. A Pocket member receipt is joined to final acceptance only after independent selected-source binding; filename equality is insufficient.

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

**Mitigation:** original runtime reconstruction remains in Recovered Game Source Tree; L2/L3 and V receipts bridge behavior without collapsing ownership. #228 explicitly preserves this separation for the materialization-completion boundary.

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

An active PR, local APK or operator report may be described as canonical implementation/evidence before merge or receipt review.

**Mitigation:** current status names a reconciled main SHA; branch work stays branch truth until merged. Pocket GDS PR #2 tooling is not a real member receipt until an operator actually runs the merged/bound build against the real archive.

## R-016 — Historical checklist drift
**Severity:** high

Old issues/docs may continue to list already-promoted PNST/NBZ/provider work as pending or preserve superseded environment assumptions.

**Mitigation:** current docs point to the canonical L1 roadmap and post-audit reconciliation. Historical material remains history, while explicit addenda supersede stale current-state claims such as “retail NBZ absent” when later evidence shows it is locatable but transport-blocked.

## R-017 — Repository contamination
**Severity:** high

Proprietary game bytes or leaked source may be committed.

**Mitigation:** synthetic/public-safe fixtures, byte-free receipts and legal local artifacts only. Pocket evidence JSON stores hashes/identity/provenance, never resource payload bytes.

## R-018 — Premature original-file modification
**Severity:** high

Safe authoring work may evolve into implicit retail file mutation.

**Mitigation:** WorkingCopy + explicit generated output + atomic publication + reopen/validation; retail files remain immutable by default. Level-E temporarily adds only one known next-volume overlay and then removes it after SHA verification.

## R-019 — Transport ceiling misreported as artifact absence
**Severity:** high

A failed connected download/materialization of the 960,358,951-byte `DMC3-0.nbz` may be reported as “the retail archive is unavailable/not present”. That can restart pointless acquisition work or distort blockers.

**Mitigation:** preserve separate facts: artifact locatable, observed archive size 960,358,951 bytes, connected raw-transfer/materialization ceiling 268,435,456 bytes. Treat this as transport/execution scope. Use a local real-device/PC execution surface for member evidence instead of weakening acceptance.

## R-020 — Mobile receipt authority laundering
**Severity:** critical

A valid Pocket `gdspaces.l1.member-acquisition-receipt.v1` may be promoted into claims it cannot prove: protected executable authority, original resolver winner, successful original mount topology or original-game consumption.

**Mitigation:** receipt schema explicitly says `original_game_consumption = not-claimed`; producer/core revisions are bound; materialization requires ResourceIdentity + ByteProvenance + exact size/hash equality. Final promotion independently binds the same member/archive identity to protected selection and #209 original consumption.

## R-021 — Filename discovery collapsed into successful mount topology
**Severity:** critical

`first_missing_archive_volume` or pre-gap filename presence may be interpreted as proof that every discovered archive mounted successfully. #235 confirms original bootstrap ignores archive-registration return and continues discovery after an existing archive mount failure.

**Mitigation:** keep discovery and successful-mounted set distinct. Open #237 corrects the product model. Clean successful precedence remains higher successful index -> lower successful index -> physical. Failure cases must not be modeled as ordinary lookup misses or inferred mounts.

## R-022 — Independent PASS receipts composed by filename
**Severity:** critical

A mobile member PASS, desktop authoring PASS, L2 selected-identity PASS and original-game observation from different source/process lineages may be combined because they share `em000.pac` or another path string.

**Mitigation:** final V/LV audit requires one cryptographically/structurally reconciled lineage: exact selected provider/archive/member -> exact L1 materialized bytes -> authored/rebuilt/overlay identity -> original consumer effect -> rollback. Filenames are descriptive, not evidence joins.

## R-023 — PID reuse / runtime mapping instance confusion
**Severity:** critical

A protected-process mapping packet may combine windows from different launches that reused a PID, or trust child-declared canonical window hashes.

**Mitigation status:** #235 R2B v2 tooling requires PID + non-zero OS-derived process creation FILETIME + module identity across all child windows and independently derives seven canonical anchor hashes from the supplied canonical EXE artifact. Legacy v1 is not promotion authority. A real v2 packet is still required.

## R-024 — Archive normalized-key collision ambiguity
**Severity:** critical

Original archive indexing sorts and searches `0x0E` normalized strings with no equal-key secondary tie-break. Two retail central entries with one normalized key would make a deterministic winner claim unsafe.

**Mitigation:** require an exact archive-SHA-bound retail member surface and run the `0x0E` collision census before uniqueness-dependent original-selection claims. A single Pocket member receipt is insufficient; a complete bound derivative could support this later.
