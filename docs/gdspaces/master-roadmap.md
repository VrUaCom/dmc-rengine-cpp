# GDSpaces Master Roadmap — L1 / L2 / L3

**Snapshot:** 2026-08-31  
**Canonical implementation synchronized through:** `main@0d0a604f63eb70706f177e2dde02f7a3586d1e25`  
**L1:** **INCOMPLETE / NOT 100%** — original materialization reverse, naming/type validation and real acceptance open  
**L2:** INCOMPLETE — advanced static/tooling, real selected-identity evidence open  
**L3:** INCOMPLETE — strong static spine, dynamic lifecycle/original-process evidence open  
**Active reconciliation:** PR #269

This is the dependency roadmap for GDSpaces as one resource-runtime program. L1/L2/L3 keep separate authority, but work is scheduled by the evidence dependency that blocks the next real vertical proof.

The historical statement that the L1 internal path was closed is superseded. Product implementation is advanced, but canonical-EXE reverse proved that the original materialization/result/failure contract is not exhaustively recovered. Naming and scoped runtime type evidence are substantially integrated in `main`, but real-corpus/replay validation remains open. Original-game Level-E acceptance also remains open.

## 1. Layer ownership

### L1 — Resource Materialization

```text
[L2 already selected exact identity]
 -> cached/logical/materialized size
 -> capacity/allocation
 -> exact acquisition / transfer / decompression
 -> exact destination bytes + ByteProvenance
 -> PAC/PNST/.lst/nested physical representation
 -> physical child identity + naming/type evidence reconciliation
 -> bounded edit + rebuild/repack
 -> reopen/rematerialize
 -> native terminal byte/result authority
```

L1 owns exact bytes and physical representation. It does not own which provider wins before selection and does not own later LoadedResource/consumer lifecycle publication.

### L2 — Resource Resolution

```text
logical request
 -> candidate construction
 -> normalization
 -> provider/source/volume/member selection
 -> ambiguity/fallback/failure distinction
 -> exact selected ResourceRef / ResourceId
```

L2 owns selection. It may supply the exact parent identity used by L1 naming/materialization, but filenames/display names are not allowed to replace physical identity.

### L3 — Original Runtime / Lifecycle

```text
L1 native terminal byte/result state
 -> queue/callback eligibility
 -> LoadedResource state publication
 -> typed post-load
 -> ready visibility
 -> claims/cache
 -> cancellation/reset/release/shutdown
 -> original consumer behavior
```

Stage Assembly / Stage Ops is downstream domain/tooling, not L3. Validation remains cross-cutting and is not a fourth numbered layer.

## 2. Current vertical acceptance target

```text
real protected DMC3 installation
 -> [L2] exact selected provider/volume/member identity
 -> [L1] artifact-bound exact materialized bytes
 -> [L1] exact child topology + naming/type evidence
 -> [L1] supported bounded edit/rebuild
 -> [L2] authored next-volume winner
 -> [L1] exact authored rematerialization
 -> [bounded L1/L3 seam] native terminal result permits normal callback
 -> [L3] original lifecycle reaches deterministic consumer
 -> observable effect attributable to authored bytes
 -> rollback / original retail immutability
```

A crash-free launch, synthetic round trip or green CI alone is never sufficient.

# Track A — L1 reverse + naming/type validation + final acceptance

Canonical roadmap: `l1-roadmap.md`.

Raw reverse checkpoints:

- `l1-writer-failure-width-reconciliation-2026-08-28.md`;
- `l1-terminal-l3-completion-seam-2026-08-28.md`;
- `../../data/reverse/dmc3-l1-writer-failure-width-2026-08-28.v1.json`;
- `../../data/reverse/dmc3-l1-terminal-l3-completion-seam-2026-08-28.v1.json`.

Naming/type checkpoints:

- `l1-naming-full-integration-20260830.md`;
- `dmc3-runtime-l1-naming-bridge-20260830.md`;
- historical PR #268, landed by fast-forward and followed by instruction-level corrections on `main`;
- primary 3D/render-family reverse documentation synchronized through `main@0d0a604f`.

**Track A status: INCOMPLETE / ACTIVE.**

## A1 — Product capability state

Current bounded capabilities include:

- artifact-bound NBZ/member acquisition and success-bound ByteProvenance;
- STORE/raw-DEFLATE materialization;
- atomic/no-replace publication;
- PAC/PNST sparse/empty/alias-preserving expansion;
- same-size, size-changing and nested relative-slot authoring;
- verified NBZ rebuild and next-volume overlay authoring;
- canonical reopen/rematerialization verification;
- protected build preflight / product closure tooling;
- runtime-synth `.lst` direct `0x800` transfer extents vs recursively synthesized `0x40` complete-image structural extents;
- original zero-filled runtime-synth padding;
- extracted-ordinal `.index` mapping independent of physical slot index;
- sealed separation of external `.index`, embedded aliases, enclosing stored names, semantic evidence, display and safe export;
- exact `ResourceId` runtime-to-L1 naming bridge;
- no-`.index` derived display as presentation-only fallback;
- runtime type evidence separated by original instruction path instead of one global detector.

These are product capabilities, not an L1 completion certificate.

## A2 — Original materialization reverse gate L1-R

Canonical-EXE reverse confirms/corrects:

- FileSlot cached-size source for physical and NBZ entries;
- direct whole-file `0x800` transfer extent in the safe positive domain;
- lower EOF/short-read behavior;
- `.lst` direct-vs-recursive placement semantics;
- original synthesized-image zero initialization;
- `0x1402EF4D0` is type-2 queue admission, not byte completion;
- `0x1402EF790` consumes admitted materialization jobs;
- `0x1401B85C0` ignores direct child enqueue and recursive-writer failures;
- `0x1401B8CA0` has branch-dependent boolean semantics;
- `0x1401B84E0` ignores type-3 completion enqueue failure;
- chunk/planner arithmetic is 32-bit and wrap-prone;
- scanner/token ceilings are bounds rather than clean original error enums;
- admitted type-2 materialization and admitted normal type-3 completion use the same per-lane FIFO;
- status `2` remains pending, status `4` retries without retirement, status `3` retires and advances;
- original status `3` does not independently prove actual bytes equal planned bytes;
- cancellation queued-work suppression is L3 ownership.

Therefore the old model:

```text
upstream materializer/writer true == exact bytes completed
```

is rejected.

### Bounded static L1/L3 seam — CLOSED

For successfully admitted canonical normal-path jobs:

```text
current type-2 materialization
 -> status 2: pending / no FIFO advance
 -> status 4: retry same job / no FIFO advance
 -> status 3: retire type-2 / advance FIFO
 ===== native L1 byte/result terminal =====
 -> later admitted type-3 callback becomes eligible
 -> 0x1401B8DC0 publishes state 1 -> 2
 ===== L3 lifecycle =====
```

Qualifications:

- FIFO ordering proves nothing about jobs that failed admission;
- original status `3` is not an independent planned-length equality proof;
- queued normal work can be suppressed by cancellation;
- dynamic current-slot cancellation/concurrency remains L3 breadth.

### Remaining L1-R frontier

```text
A-R1 recursive .lst cycle/depth semantics
 -> A-R2 recursive allocation/free lifetime
 -> A-R3 residual allocator/backend failure branches
 -> A-R4 representative real .lst corpus only if loose-list equivalence is claimed
 -> A-R5 final original-L1 contradiction sweep
```

### Current evidence constraint for A-R1..A-R3

The complete raw canonical analysis executable (`e454...`, 6,356,432 bytes) is not exposed in the current connected session. The accessible raw EXE is the protected/distribution build (`81c7...`, 6,567,320 bytes) and is not instruction authority for canonical analysis VAs. Therefore no new instruction-level `.lst` cycle/depth/allocation/free claim may be promoted from that protected image without independent mapping/equivalence evidence.

The next reverse step is to obtain canonical bytes, trustworthy bounded disassembly, or an equivalent evidence packet. Product recursion/depth guards may still be reviewed as product hardening, but they cannot be mislabeled as recovered original semantics.

## A3 — Naming/type gate L1-N

Naming is main-landed but not complete.

Canonical physical/naming authority separation:

```text
ResourceId
physical_slot_index
extracted_ordinal
external .index evidence
embedded alias evidence
enclosing-container stored-name evidence
semantic/type evidence
canonical display
historical extraction representation
safe host export projection
```

`.index` authority is:

```text
entry N == extracted ordinal N == N-th populated payload
```

not physical slot N.

The runtime-to-L1 naming bridge accepts exact complete `ResourceId` equality only. Basename, display, alias, semantic extension or `.index` label cannot substitute for physical identity.

The no-`.index` derived display remains presentation-only and cannot create historical extraction evidence or write authority.

### Scoped runtime type correction

The old global shorthand “the runtime compares exactly five payload tags” is superseded. Current direct canonical-EXE evidence separates at least three paths:

1. **Registry/resource-registration path** — three-byte content probe with the five-tag registry boundary;
2. **PAC/PNST child dispatcher** — independent container materialization/dispatch evidence, including currently bounded EFW/EFE sentinel/prefix observations;
3. **Higher-level family-mask classifier** — four-byte comparisons where byte 3 matters and MCV evidence is separately classified.

Do not collapse these into one magic/type detector.

### Remaining L1-N frontier

```text
A-N1 representative retained effect-corpus replay/reconciliation
 -> A-N2 global representative PAC/PNST naming coverage + collision census
 -> A-N3 historical .index producer/extractor lineage or explicit unresolved bound
 -> A-N4 real-retail selected runtime identity -> exact L1 parent identity receipt
 -> A-N5 historical extraction replay/export/reopen validation
 -> A-N6 final naming/type-evidence contradiction audit
```

## A4 — Real L1 acceptance

After the reverse/naming boundaries needed by the selected resource are clean:

```text
direct-retail selected identity + provenance receipt
 -> exact retail representation classification
 -> one supported real edit/rebuild/rematerialization receipt
 -> #209 original-game deterministic consumption + rollback
 -> final L1 cross-stack audit
 -> L1 COMPLETE / 100%
```

Unsafe original behavior is evidence, not a product requirement. GDSpaces remains fail-closed for overflow, rejected queue work, malformed bounds and exactness receipts.

# Track B — L2 closure

**Status: INCOMPLETE.**

Strong/integrated L2 slices include:

- canonical request/candidate/normalization behavior;
- type-0 physical-provider static reverse and native product path;
- numbered-volume discovery/mount precedence model;
- bounded protected-runtime address-window/multi-anchor mapping tooling;
- selected-identity content-candidate normalization and artifact-binding tooling.

Current evidence order:

```text
B-R2A real-retail normalized-key/member collision census

B-R2B real protected-process multi-anchor mapping receipt

B-R3 trusted original-process selected-provider identity
  -> mapped anchors
  -> trusted publisher/origin
  -> zero-loss trace
  -> exact observer + NBZ artifact binding

[R2A + R2B + R3]
 -> contradiction/docs/evidence reconciliation
 -> exact-head CI
 -> final L2 audit
```

Provider/backend failure must remain distinct from a clean miss.

Canonical analysis executable VAs/RVAs may not be transferred to a different protected build without independent runtime mapping.

For the L1 vertical proof, only the L2 identity evidence needed to bind the selected and authored resources is mandatory; broader L2 completion remains independent.

# Track C — L3 closure

**Status: INCOMPLETE.**

The static LoadedResource/typed-ready/release-reset spine is strong. The bounded static L1-terminal → normal-completion seam is closed at its stated scope.

Do not restart that seam or already bounded static writer areas absent contradictory evidence.

Current dynamic order:

```text
C-V1 initial load
 -> C-V2 room/stage transition
 -> C-V3 restart/reload
 -> C-V5 in-flight cancellation
 -> C-V4 return-to-menu/full reset
 -> C-V6 shutdown
 -> C-V7 family/build breadth
 -> final L3 audit
```

For the first L1 vertical proof, L3 only needs enough original-process evidence to attribute the deterministic consumer effect to the exact authored L1 resource. Broader L3 completion remains separate.

# Cross-layer dependency matrix

| Acceptance question | Primary | Required support |
|---|---|---|
| Which resource wins for a real request? | L2 | real retail/protected identity evidence |
| Are selected bytes exact? | L1 | exact L2 selection + artifact binding |
| Is original materialization terminal semantics understood? | L1 | bounded queue/completion seam evidence |
| How are nested children physically identified/named? | L1 | topology + sealed naming/type evidence |
| Can the selected representation be edited safely? | L1 | representation-specific writer authority |
| Will the authored overlay win? | L2 | exact L1 generated artifact |
| Are authored bytes rematerialized exactly? | L1 | exact authored L2 winner |
| Did original DMC3 consume those bytes? | L3 + validation | same L2→L1 identity chain |
| Was rollback clean? | validation | exact artifact identities |

# Current priority queue

1. Land PR #269 so canonical status/evidence matches the confirmed reverse and current naming/type main history.
2. Obtain canonical-analysis bytes/trustworthy bounded disassembly for the residual `.lst` frontier.
3. Finish recursive `.lst` cycle/depth/allocation/free lifetime and residual allocator/backend failure branches.
4. Run final original-L1 contradiction sweep.
5. Finish naming/type real-corpus, historical producer-lineage and replay/export/reopen validation.
6. Obtain one representative real-retail selected identity + L1 acquisition receipt.
7. Classify its exact representation and perform one supported edit/rebuild/rematerialization chain.
8. Execute #209 original-game consumption + rollback.
9. Run final L1 audit.
10. Continue L2/L3 broader evidence programs without displacing the L1 vertical critical path.

# Completion rule

No percentage alone can mark a layer complete. Completion requires the applicable mandatory gates, canonical code/docs, exact-head Windows+Ubuntu validation, representative real-corpus/original-process receipts and no unresolved contradiction that changes the declared scope.

**Current canonical statuses:**

- **L1: INCOMPLETE / NOT 100%**
- **L2: INCOMPLETE**
- **L3: INCOMPLETE**

Percentages are planning indicators only.
