# PAC Read-Only Structural Parser

**Reconciled:** 2026-08-17  
**Status:** IMPLEMENTED BOUNDED STRUCTURAL SLICE / SYNTHETIC REGRESSION; HASH-BOUND REAL-CORPUS EXECUTION RECEIPT STILL OPEN

## Purpose

This is the clean-generation read-only PAC structural decoder used to feed the shared `ContainerDocument` contract without introducing Stage/HITS semantics or reviving a PAC-specific product architecture.

It is reconciled against issue #100 / Drive archive-runtime evidence through Pass 34 and supersedes the old PAC half of the stacked #59 implementation path.

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

Phase-15 executable evidence and Pass-17 hash-bound corpus both support this envelope. Real corpus includes sparse PAC/PNST containers, valid zero-count PACs and large declared topologies.

## Product extent rule

This bounded parser does not have an explicit member-size field. GDSpaces therefore derives an extraction extent as:

```text
start = slot offset
end   = next greater distinct populated offset
         or PAC byte-span end
size  = end - start
```

This is a deterministic **product extraction rule**, not a claim that every original DMC3 consumer represents member size through the same algorithm.

Duplicate non-zero offsets remain separate slot identities and share the same inferred span. The current Pass-17 corpus contains no duplicate non-zero offsets, so this behavior is defensive product support rather than newly game-confirmed alias semantics.

## Alignment correction

Universal 16-byte alignment is **not** a PAC validity invariant.

Pass 14 recovered real PAC/PNST counterexamples with populated-offset residues `{0, 8}` modulo 16. Alignment may be reported as corpus/diagnostic information but must not cause structural rejection by this parser.

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

## Synthetic regression

Coverage includes:

- exact magic;
- declared topology and empty-slot preservation;
- duplicate-offset shared extents;
- next-distinct-offset extraction;
- invalid/truncated header/table;
- offsets into table and at file end;
- product slot-count safety limit;
- zero-count structural PAC;
- an offset with residue 8 modulo 16, proving alignment is not used as a reject rule.

No proprietary game bytes are committed.

## Real-corpus gate

Corpus **acquisition is no longer the blocker**.

Issue #100 Passes 14 and 17 reacquired hash-bound PAC/PNST material, including 42 raw stage-drop containers (32 PAC / 10 PNST), sparse cases, six zero-count PACs and large topologies.

The remaining gate is operational:

```text
reacquired hash-bound PAC corpus
  -> this exact current-generation PacParser
  -> sanitized deterministic validation receipts
```

Receipt fields should include artifact SHA-256, byte size, declared slot count, populated/empty counts, slot index, offset, inferred extent, diagnostics and deterministic reopen result.

Until that execution receipt exists, this slice is implemented/evidence-consistent but is not advertised as fully game-validated PAC support.

## Dependency boundary

```text
GDSpaces ResourcePayload
  -> PacParser
  -> ContainerDocument
  -> ContainerExpander / later recursive tree
  -> profile/domain adapter
```

HITS, Stage Ops and other consumers must not add private PAC parsers.

This parser does not infer Stage-CFG slot roles, HITS descriptors/transforms, model/texture/audio meaning or filenames beyond explicit synthetic structural labels.

## Not part of this slice

- PNST parser promotion;
- recursive container-tree policy;
- NBZ provider/materialization;
- `.index` metadata linkage;
- `.lst` original-runtime loose-container synthesis;
- DMC3 provider/source precedence;
- write/repack/export;
- original `LoadedResource` lifecycle/runtime equivalence.

Those are separate reconciliation gates.
