# Open PR Hygiene — 2026-09-06

**Canonical base for this pass:** `main@c72b7b056517039c815ee4322119f9eb40589601`  
**Rule:** closing a stale PR does not delete its branch or evidence history. Historical branches remain available for semantic salvage, but they are not merge authority.

## Objective

Reduce parallel truth surfaces and prevent stale branches from reintroducing superseded parser, routing, evidence, resolver or status contracts into canonical `main`.

A PR is classified by what is safe to do **now**, not by whether its historical work was useful.

- **CANONICAL / ACTIVE** — current integration surface; may continue after synchronization/review.
- **SALVAGE ONLY** — useful code/evidence may remain, but the branch must not be merged wholesale. Port only reviewed deltas onto current `main`.
- **SUPERSEDED / CLOSED** — current canonical code or a later promotion already carries the required result; keep branch/history only.
- **HOLD / CONTRADICTION REVIEW** — contains a claim that is stronger than current canonical authority or otherwise requires explicit reconciliation before reuse.

## Cleanup wave completed

### L1 naming stack — SUPERSEDED / CLOSED

The following stacked PR heads were compared against current `main` and had no unique commits ahead of canonical history. They were therefore closed without deleting branches:

- #252 — nested DDS index identity;
- #253 — sealed resource name evidence;
- #256 — embedded name-list aliases;
- #259 — embedded name-table `.index` canonicalization;
- #260 — sealed semantic naming authority;
- #261 — sparse PAC/PNST extracted-ordinal mapping;
- #262 — complete naming identity / legacy extraction replay.

#263 was also closed because it was explicitly a temporary integration-sync PR whose head was `main` and whose base was an old naming landing copy.

### L2 successful-mount topology stack — SUPERSEDED / CLOSED

Closed:

- #239;
- #241;
- #246.

These are historical implementations of the same architectural correction now canonically promoted by #287: filename discovery / registration attempts are distinct from explicitly successful linked mount topology, and resolver traversal uses only successful providers.

They must not be merged back into `main`.

### Canonical executable identity correction — SUPERSEDED / CLOSED

#92 was closed. Current `main` already binds canonical SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082` to size `6,356,432` in the active Evidence Packet and preserves an explicit correction record. The old PR is no longer a promotion surface.

### HITS historical review stack — CONSOLIDATED / CLOSED

Closed after checking the active `hits` consolidation surface:

- #82 — Pass 8 wide runtime integration review;
- #83 — Pass 9 ABI/ownership saturation;
- #85 — Pass 10 evidence reacquisition / ABI work;
- #96 — Pass 10 Slice 15 referenced Stage-CFG descriptor census;
- #97 — Pass 10 Slice 16 transform-source provenance plan.

The `hits` branch / #282 now preserves the Pass 8–10 evidence architecture under `evidence/hits/`, provides a canonical reverse synthesis through Pass 10, and retains Slice 15/16 machine-readable packets. Closing the stacked PRs removes parallel integration surfaces; it does not delete their branches or relax unresolved evidence gates.

## Current high-value integration surfaces

### #282 — HITS canonical integration branch — CANONICAL / ACTIVE REVIEW

Keep open as the single current HITS consolidation surface. It intentionally combines the shared HITS format/parser, game-agnostic runtime/writer helpers and DMC3 Pass 8–10 evidence modules.

Before promotion:

1. synchronize with current `main`;
2. run semantic reconciliation against the current modular Native Reader / Format Registry / Tool Registry contracts;
3. verify evidence status has not been strengthened by consolidation;
4. require exact-head Ubuntu + Windows CI;
5. only then promote a bounded slice.

Historical Pass 8–10 PR branches remain available as evidence provenance, but are no longer independent merge/review authorities.

## Salvage-only branches — do not merge wholesale

### #280 — mobile-only format/parser package — SALVAGE ONLY

Useful candidates remain, especially EFM/MOT and selected GDSpaces helpers. However:

- its SHW model is obsolete: it explicitly used a low-confidence/no-corpus SHW contract and is superseded by canonical #288;
- MOD/PTX overlap later canonical implementations;
- its registry/classifier edits predate the current cross-registry drift protections.

Required approach: extract individual EFM/MOT/helper deltas onto fresh current-main branches. Never merge #280 as one package.

### #284 — model-family reader hardening — SALVAGE ONLY

Contains useful MOD hardening and transform-domain work, but it is based on the old `model-family` line. Canonical MOD/SCM structural readers already exist on current `main`.

Port only independently useful hardening after a field-by-field diff against canonical readers and current parser-validation contracts.

### #278 — SCM branch — SALVAGE / RESEARCH

The branch contains deep SCM reverse, layout/topology/runtime-flag and authoring experiments. Canonical read-only SCM already exists on `main`.

Do not infer writer authority from layout reconstruction. Any writer/authoring salvage requires a new current-main branch, explicit preserved-unknown policy, round-trip validation and original-game acceptance gates.

### #277 — L1/L2/L3 reconciliation — SALVAGE

Do not merge the old documentation branch wholesale. Its important remaining candidate is the L3-R1 conclusion (`STATIC BOUNDED-CLOSED / CONTRADICTION-GATED`) and associated current-main semantic promotion work. Current canonical status still records that promotion as open.

Port the L3-R1 conclusion only after a fresh contradiction sweep against current `main`.

### #254 — animation/effect/naming reverse pass — SALVAGE ONLY

Large mixed branch with MOT/animation dispatch, content-tag census, effect naming and other findings. Treat as a source of bounded evidence/deltas, never as a merge unit.

## Verify-before-closing queue

These PRs may contain unique evidence or tooling and require an exact current-main content check before closure:

- #274 — EXE format/type census propagation;
- #269 — L1 writer-failure/width and L1→L3 seam evidence;
- #264 — AFS namespace vs binary AFS/PACK authority separation;
- #236 — process-instance-bound R2B/R3 tooling;
- #232 — raw EXE R3 structural identity evidence;
- #223 — V/LV validation architecture/code;
- #218 — L3 lifecycle receipt validator;
- #212 / #210 — acquisition receipt binding and nested slot-path reflow;
- older texture/PTX/DDS authoring/reverse stack — preserve until one canonical salvage head is selected and evidence lineage is checked.

No PR in this queue should be mechanically rebased/merged merely to reduce the open-PR count.

## Hold / contradiction review

### #181 — protected/unpacked executable identity

The PR used stronger language that the protected and canonical analysis executables are representations of the same linked image. Current canonical status deliberately keeps the protected distribution candidate and canonical analysis executable as separate authorities and states that no global build equivalence is claimed.

Therefore #181 is **HOLD** until its bounded PE-section observations are separated from the stronger global-equivalence wording. Do not use it to transfer canonical VAs/RVAs into a protected process.

## Anti-regression rules

1. `main` is the only implementation authority.
2. A stale branch may provide evidence or a patch candidate, but never silently overwrite newer canonical registry/routing/workspace contracts.
3. No old SHW implementation may replace `formats.shw-structural-v1` from #288.
4. No old resolver branch may reintroduce `discovered archive == mounted archive` assumptions after #287.
5. No read-only structural parser branch may grant writer authority by implication.
6. No stale status/documentation PR may downgrade the current proof-gated `L1/L2/L3 incomplete` truth.
7. Salvage is performed as small semantic ports onto a fresh branch from current `main`, followed by exact-head cross-platform CI.
8. Close PRs, not history: branches/evidence remain recoverable until deliberately retired under a separate repository-history policy.

## Next hygiene wave

Priority order:

1. synchronize and review #282 as the single active HITS surface against current `main`;
2. split #280 into EFM, MOT and helper salvage candidates while rejecting its stale SHW path;
3. extract the L3-R1 current-main promotion candidate from #277;
4. audit #269/#264/#236/#232 for unique evidence already absent or present on `main`;
5. consolidate the old texture/PTX/DDS stack into one evidence-preserving salvage plan;
6. rerun open-PR inventory and leave only active integration surfaces, explicit salvage sources and true research blockers.
