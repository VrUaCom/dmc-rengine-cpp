# PAC Read-Only Structural Parser

**Reconciled:** 2026-08-17  
**Status:** IMPLEMENTED / SYNTHETIC + 32-ARTIFACT HASH-BOUND REAL-CORPUS VALIDATION; BOUNDED STRUCTURAL SLICE ONLY

## Purpose

This is the clean-generation read-only PAC structural decoder used to feed the shared `ContainerDocument` contract without introducing Stage/HITS semantics or reviving a PAC-specific product architecture.

It is reconciled against issue #100 / Drive archive-runtime evidence through Pass 34, the clean #102 promotion, and the real-corpus execution receipt promoted from the superseded #99 development branch.

## Evidence-backed structural envelope

```text
+0x00  char[4]  "PAC\0"
+0x04  u32 LE   declared slot count
+0x08  u32 LE[] container-base-relative slot offsets
```

Rules:

- `offset == 0` preserves an explicit empty physical slot;
- non-zero offset is relative to the PAC/container base;
- populated offsets must begin after the complete header/table and remain strictly inside the supplied byte span;
- the full declared slot namespace is preserved, including sparse and zero-count containers;
- slot number is structural identity inside the resource/schema context, not a global semantic type.

Phase-15 executable evidence, Pass-17 hash-bound corpus and the 32-artifact parser receipt all support this envelope.

## Product extent rule

This bounded parser does not have an explicit member-size field. GDSpaces therefore derives an extraction extent as:

```text
start = slot offset
end   = next greater distinct populated offset
         or PAC byte-span end
size  = end - start
```

This is a deterministic **product extraction rule**, not a claim that every original DMC3 consumer represents member size through the same algorithm.

Duplicate non-zero offsets remain separate slot identities and share the same inferred span. The current 32-PAC validation corpus contains no duplicate non-zero offsets, so this behavior remains defensive product support rather than newly game-confirmed alias semantics.

## Alignment correction

Universal 16-byte alignment is **not** a PAC validity invariant.

Pass 14 recovered real PAC/PNST counterexamples with populated-offset residues `{0, 8}` modulo 16. Clean synthetic regression also locks this rule by accepting an offset with residue 8. Alignment may be reported as corpus/diagnostic information but must not cause structural rejection.

## Product safety boundary

`PacParser::k_max_slot_count` is a denial-of-service/safety bound. It is not an original DMC3 ABI capacity claim.

The parser fails closed on:

- header shorter than 8 bytes;
- non-`PAC\0` magic;
- product safety-limit breach;
- unrepresentable/truncated offset table;
- populated offset inside the header/table;
- populated offset at or beyond the supplied PAC span;
- resulting invalid shared `ContainerDocument`.

## Shared-contract naming rule

`ContainerEntry::valid()` requires populated entries to have a non-empty presentation name. PAC does not supply original filenames in this structural envelope, so the parser assigns deterministic synthetic structural labels:

```text
slot_NNNN.bin
slot_NNNN.empty
```

with `synthetic_name=true`. These labels are presentation only; physical slot index remains authority and no semantic/source filename is inferred.

## Validation

Synthetic CTest covers exact magic, declared topology, sparse/empty slots, duplicate-offset shared extents, bounded extent inference, malformed/truncated tables, safety limit, zero-count PAC and the non-16-byte-alignment regression.

The real-corpus execution receipt is committed at:

`docs/evidence/pac-real-corpus-validation-2026-08-17.md`

Summary:

- **32 PAC artifacts** parsed;
- **32/32 success**;
- **0 failures**;
- **32/32 deterministic second parse**;
- **6 valid zero-count PACs**;
- **6 sparse PACs**;
- maximum declared topology **415 slots**;
- byte-size range **16 .. 2,495,392 bytes**.

No proprietary game bytes are committed; the receipt preserves sanitized artifact hashes/topology.

### Gate status

The bounded **PAC structural real-corpus execution gate is closed** for this parser behavior.

This does **not** close PNST, NBZ, `.lst`, archive/provider behavior, semantic slot schemas, write/repack support or full DMC3 runtime equivalence.

## Dependency boundary

```text
GDSpaces ResourcePayload
  -> PacParser
  -> ContainerDocument
  -> ContainerExpander / later recursive tree
  -> profile/domain adapter
```

HITS, Stage Ops and other consumers must not add private PAC parsers.

This parser does not infer Stage-CFG slot roles, HITS descriptors/transforms, model/texture/audio meaning or original filenames.

## Not part of this slice

- PNST parser promotion;
- recursive container-tree policy;
- NBZ provider/materialization;
- `.index` metadata linkage;
- `.lst` original-runtime loose-container synthesis;
- DMC3 provider/source precedence;
- write/repack/export;
- original `LoadedResource` lifecycle/runtime equivalence.

Those remain separate reconciliation gates.
