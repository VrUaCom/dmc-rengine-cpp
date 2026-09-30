# GDSpaces PR layer audit — #1 through #137

Canonical audit date: 2026-08-19.

This is an exhaustive audit of repository number positions #1..#137. GitHub issues and pull requests share one number space, so not every number is a PR. Coverage check: **137 positions = 120 actual PRs + 17 non-PR number positions**. Every actual PR was inspected.

Rule used during audit: if a PR has no GDSpaces/resource-runtime decompilation ownership or boundary, it is recorded as **OUTSIDE GDSpaces** and its PR conversation is not modified. GDSpaces-relevant PRs receive/retain a layer tag. Layer definitions live in `docs/gdspaces/decompilation-layer-classification.md`.

## GDSpaces/resource-runtime PRs

- **Cross-layer governance:** #54
- **GDS tooling OUTSIDE runtime layers:** #60
- **Governance + V:** #43, #94
- **L1:** #57, #58, #59, #63, #64, #65, #66, #101, #102, #104, #109, #112, #115, #118
- **L1 + L2:** #137
- **L1 + L2 + L3:** #80
- **L1 + L2 + L3 + V:** #74
- **L1 + L2 + V:** #77
- **L1 + V:** #1, #6, #99
- **L1 product hardening:** #105, #116, #125
- **L1 superseded:** #106, #107, #108, #110, #111, #113, #114, #117
- **L2:** #62, #67, #68, #69, #70, #71, #120, #122, #129, #133, #135, #136
- **L2 + L3:** #84
- **L2 + L3 + V:** #79
- **L2 product hardening:** #124
- **L2 superseded:** #119, #121, #123, #126, #128, #131, #132, #134
- **L2 superseded architecture:** #127
- **L2 superseded/WIP:** #130
- **L3:** #81, #87, #89
- **V:** #14, #15, #19
- **V supporting L1:** #103
- **V supporting L2 consumer:** #9

## Inspected PRs outside GDSpaces — no PR mutation

#7, #8, #10, #11, #12, #16, #17, #18, #20, #21, #22, #23, #24, #26, #27, #28, #29, #30, #31, #32, #33, #34, #35, #37, #39, #41, #42, #44, #45, #46, #47, #49, #50, #56, #61, #72, #73, #75, #76, #78, #82, #83, #85, #86, #91, #92, #93, #95, #96, #97, #98

These PRs belong primarily to Evidence infrastructure, Binary Inspector, HITS/collision, Item/Trial Chamber/Patch Engine, save ABI, custom-build/source-modification architecture, Stage Catalog/Stage Ops, EXE Editor/Reverse Core acquisition, branding, build maintenance or other non-GDSpaces scopes. Their existence is recorded here only to prove audit coverage; they are not rewritten into the GDSpaces layer taxonomy.

## Number positions with no pull request

#2, #3, #4, #5, #13, #25, #36, #38, #40, #48, #51, #52, #53, #55, #88, #90, #100

These are issue/non-PR positions in GitHub's shared issue/PR numbering space, so there is no PR body/conversation to classify or mutate.

## Audit invariants

- L1 = resource materialization: physical/container bytes -> parse/read/transform -> exact materialized bytes -> nested extraction -> editable WorkingCopy -> future writer/rebuild/repack/round-trip.
- L2 = resource resolution: request/candidate/path normalization -> provider/source/volume selection -> exact ResourceRef/ambiguity.
- L3 = original resource runtime/lifecycle: FileSlot/scheduler/callback/state/post-load/claims/reset/unload; recovered original code remains Recovered Game Source Tree ownership, not GDSpaces product ownership.
- V = validation/equivalence; it never substitutes for L1/L2/L3 implementation.
- `GDS tooling OUTSIDE runtime layers` is GDSpaces-owned product metadata such as `.index`, but not original-runtime evidence.
- Historical/superseded PRs keep their historical status; a layer tag does not restore them as promotion authority.
- Current GDSpaces execution priority remains L1. L2/L3 progress is preserved but is not counted as L1 completion unless it directly unblocks materialization/edit/rebuild/repack/round-trip.
