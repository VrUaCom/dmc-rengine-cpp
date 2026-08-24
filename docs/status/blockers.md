# Current Blockers

**Snapshot date:** 2026-08-24  
**Canonical base:** `main@c4920c8602dd7492b6c89e9fc8ecf8a6d8397ee0`

This file lists unresolved gates against the current reviewed tree. The canonical execution order is [GDSpaces L1 Roadmap](../gdspaces/l1-roadmap.md).

## P0 — GDSpaces L1 closure blockers

### B-L1-01 — Atomic/no-replace publication is not unified

**Status:** CODE CORRECTION REQUIRED

Retail-NBZ repack already has a no-replace staging/publication seam, but CLI artifact paths still contain `exists() -> ofstream` publication. That is a TOCTOU race and cannot support a no-clobber claim.

Close by introducing one shared atomic/no-replace publication primitive and migrating overlay/acquisition/evidence outputs to it, with Windows and Ubuntu concurrency regressions.

### B-L1-02 — Provenance-grade NBZ artifact stability

**Status:** CODE/EVIDENCE REQUIRED

`NbzZipSource` indexes one archive open and reopens the path during member `read()`. A provenance receipt can therefore bind stale index metadata, newer member bytes and a later archive SHA unless stability is explicitly enforced.

Close by proving index + selected member + archive identity refer to one stable artifact observation or by fail-closed revalidation of the complete observation interval.

### B-L1-03 — PR #191 promotion blockers

**Status:** ACTIVE / DO NOT PROMOTE

The retail-member acquisition seam uses the correct canonical resolver/materializer path, but currently requires:

- B-L1-01 atomic publication;
- B-L1-02 artifact-stability binding;
- rejection of output inside the measured retail game tree.

Fresh exact-head Windows + Ubuntu validation is required after correction.

### B-L1-04 — Direct-retail representative member receipt

**Status:** EVIDENCE REQUIRED

After acquisition correction, obtain the first exact retail receipt using game request `obj\\em000.pac`. The resolver-selected member/volume must be recorded; no archive member identity is predeclared from filename intuition.

Required evidence includes archive SHA/size, volume index, central-entry identity, compression metadata, CRC, materialized SHA/size and ByteProvenance.

### B-L1-05 — Retail representation classification

**Status:** EVIDENCE REQUIRED

The preserved transformed DDS-bearing texture corpus has strong product/writer evidence, but direct-retail provenance remains the deciding boundary. The exact retail member must be classified before any writer is promoted as appropriate for that resource.

### B-L1-06 — Representative real edit + bottom-up rebuild

**Status:** VALIDATION REQUIRED

Required chain:

```text
retail-selected member
 -> exact editable child
 -> bounded edit in evidenced writer domain
 -> PAC/PNST bottom-up rebuild
 -> exact edited child verification
 -> byte-exact untouched sibling checks
```

If the retail representation is outside current writer authority, this gate stops rather than forcing an unsupported serializer.

### B-L1-07 — Real next-volume publication + canonical reopen

**Status:** VALIDATION REQUIRED

The synthetic/product composition is strong. A real receipt must publish the rebuilt resource in the next contiguous numbered NBZ with atomic no-replace semantics, then reopen/select/rematerialize it through the canonical resolver path.

### B-L1-08 — Original DMC3 consumption receipt

**Status:** FINAL ACCEPTANCE REQUIRED

At least one representative authored resource must be consumed successfully by the exact protected distribution execution authority. Product reopen/reparse cannot substitute for this gate.

### B-L1-09 — Final cross-stack acceptance audit

**Status:** OPEN

Before `L1 COMPLETE`, issue #100, issue #182, code, docs, CI and receipts must agree and no unresolved contradiction may alter the claimed representation/materialization boundary.

## Supporting EXE reverse blockers

These are not reasons to redo already-closed archive reverse, but they remain relevant to exact GDS parity claims:

- exact type-0 physical-provider final Win32 filename/case/open/failure behavior after `0x0C` normalization;
- complete `0x140328540` ZIP stream initializer/lifetime;
- complete `0x140328FE0` compressed seek/reset/reinflate behavior;
- malformed/partial-read error equivalence where needed;
- dynamic `.lst` lifetime/error/cycle behavior if real loose-container use becomes an acceptance dependency.

## Evidence-gated non-blockers

### Binary AFS

`.afs/` strings remain logical namespace evidence. No binary AFS backend is promoted or required for current L1 without new direct evidence.

### PACK

Historical product parser evidence does not establish original DMC3 PACK runtime use. PACK is not an L1 blocker absent a directly observed dependency.

## Downstream blockers

These remain real project work but do not block L1 closure directly:

- full original FileSlot/AsyncIO/LoadedResource dynamic lifecycle equivalence;
- Stage Ops gameplay/runtime semantic links;
- HITS source2/transform-provider/lifecycle closure;
- whole-game decompilation/recompilation/equivalence.

## No longer primary L1 blockers

Do not reopen these merely because older documents still mention them:

- NBZ STORE/raw-DEFLATE materialization;
- PAC structural parsing;
- PNST structural parsing/classification on current main;
- recursive PAC/PNST expansion;
- ByteProvenance;
- same-size PAC/PNST authoring;
- bounded size-changing relative-slot reflow;
- synthetic nested A-to-Z composition;
- transformed DDS-bearing safe texture writer/runtime-relocation composition;
- numbered-volume precedence;
- next-volume STORE overlay generation and resolver selection composition.
