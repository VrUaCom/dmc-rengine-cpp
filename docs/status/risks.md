# Architecture and Project Risks

**Snapshot date:** 2026-08-20  
**Snapshot base:** `main` at `4cf6b34258e95bc6fde19979036c82ba0104d270`

## R-001 — Second resolver regression

**Severity:** critical

A subsystem may reintroduce direct path/container loading for convenience.

Mitigation: all tools consume GDSpaces identities/payloads/shared workspaces; parsers consume supplied spans; no editor owns a second source resolver.

## R-002 — Layer mixing / false completion

**Severity:** critical

L2 resolver progress or L3 runtime/lifecycle reverse may be counted as L1 completion, or synthetic tests may be promoted into game-equivalence claims.

Mitigation: canonical L1/L2/L3/V separation, [reverse-progress scale](../gdspaces/reverse-progress-scale.md), and strict `100% / COMPLETE` gate requiring zero mandatory blockers plus real receipts.

## R-003 — Semantic leakage into GDSpaces

**Severity:** high

Resource materialization may accumulate gameplay-specific Stage/HITS semantics.

Mitigation: GDSpaces owns bytes, resource identity, provenance, parsing/materialization/rebuild contracts. Gameplay/runtime semantics remain in recovered runtime/domain layers.

## R-004 — Materializer/repacker authority collapse

**Severity:** high

`NbzZipSource` may be treated as sufficient authority for lossless retail repack even though materialization does not preserve the full source serialization envelope.

Mitigation: keep ordinary NBZ materialization separate from on-demand raw serialization metadata and metadata-preserving retail repack authority. STORE-overlay writing remains a distinct product mode.

## R-005 — Inferred packed extent laundered as intrinsic child EOF

**Severity:** critical

A bounded parent slot span may include alignment/padding and cannot automatically become intrinsic child length authority for size-changing rebuild.

Mitigation: typed exact-child authority only; independently intrinsic resources or verified complete-image writer results; no self-declared generic writer receipt.

## R-006 — Missing artifact mistaken for reverse uncertainty

**Severity:** high

Absent `.lst`/PNST/raw evidence may trigger redundant parser redesign instead of being tracked as an artifact-acquisition gate.

Mitigation: classify missing raw corpus as `ARTIFACT REQUIRED`; preserve already-strong reverse authority until contradictory evidence appears.

## R-007 — Decompiled-code false confidence

**Severity:** high

Readable recovered C++ may hide wrong ABI, ownership, lifetimes or behavior.

Mitigation: hash-bound evidence, bounded promotion, behavioral receipts, contradiction tracking and no whole-game equivalence claims without runtime validation.

## R-008 — Stage identity collapse

**Severity:** high

Resource descriptor identity, numeric Stage selector identity and semantic gameplay Stage identity may be conflated; `st001` may incorrectly become the central architecture model.

Mitigation: preserve distinct identity layers and use `st001` only as a regression/compatibility fixture.

## R-009 — ModViz second scene truth

**Severity:** high

ModViz may recreate discovery/parsing/scene membership independently from Stage Ops.

Mitigation: ModViz consumes Stage Ops snapshots/projections and sends revision-guarded edits back through Stage Ops.

## R-010 — Authored-byte provenance corruption

**Severity:** high

Authored output may incorrectly inherit source `ByteProvenance`.

Mitigation: source provenance remains immutable; writers return authored bytes + receipts; new source provenance begins only after persistence/reopen.

## R-011 — Premature original-file write support

**Severity:** high

Write-back without explicit output/reopen/validation contracts may corrupt legal user files.

Mitigation: WorkingCopy, bounded writer modes, copied/generated outputs, deterministic receipts, reopen/reparse checks and no implicit original-file mutation.

## R-012 — Branch truth reported as main truth

**Severity:** high

Active branches or PRs may be described as canonical implementation.

Mitigation: every status document names the canonical main SHA; branch work remains branch truth until merge and required CI/promotion gates pass.

## R-013 — Public repository contamination

**Severity:** high

Proprietary game bytes or leaked source may be committed accidentally.

Mitigation: clean-room policy, synthetic fixtures, legal local artifacts only, `.gitignore`, review and removal procedures.

## R-014 — Architecture monolith

**Severity:** medium

GDSpaces, Reverse Core, recovered runtime, Stage Ops, ModViz, Binary Inspector or EXE Editor boundaries may collapse into one module.

Mitigation: preserve explicit ownership contracts and keep evidence/reconstruction authority separate from product/editor authority.

## R-015 — Historical document drift

**Severity:** medium

Root README/status/navigation may remain pinned to obsolete implementation snapshots after rapid promotion.

Mitigation: status/navigation sync is part of material promotion; machine-readable status and linked docs must share the same snapshot boundary or explicitly declare a different scope.
