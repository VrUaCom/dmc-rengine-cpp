# PAC Read-Only Structural Parser

**Date:** 2026-08-17  
**Status:** BOUNDED IMPLEMENTED / SYNTHETIC REGRESSION / REAL PAC CORPUS VALIDATED

## Purpose

This is the production-oriented read-only PAC structural parser for the clean C++ generation.

It feeds the shared `ContainerDocument` / `ContainerEntry` contracts without interpreting Stage, HITS, model, texture, gameplay or other domain semantics.

## Accepted binary contract

```text
+0x00  char[4]  "PAC\0"
+0x04  u32 LE   declared slot count
+0x08  u32 LE[] slot-offset table, one entry per declared slot
```

Rules:

- offset `0` preserves an empty physical slot;
- non-zero offset is a container-base-relative PAC byte offset;
- populated offsets may not point into the header/table;
- populated offsets must lie strictly inside the supplied PAC byte span;
- declared slot space is preserved exactly, including empty entries;
- valid `slot_count == 0` containers are accepted;
- alignment is diagnostic evidence only, not an unconditional validity rule.

## Extent inference

The promoted structural subset has offsets but no per-slot size field. A populated extent is therefore bounded by the next greater **distinct** populated offset, or PAC end for the final distinct offset.

Slot-table order is not used as size authority. Duplicate non-zero offsets remain separate physical slot identities and receive the same bounded extent; no semantic alias meaning is inferred.

This is a conservative GDSpaces extraction policy, not a claim that every original DMC3 consumer conceptualizes member size through the same algorithm.

## Structural slot names

The shared `ContainerEntry` invariant requires a non-empty `logical_name` for populated entries, while PAC itself carries no filename table in the promoted structure.

Therefore the parser assigns deterministic positional presentation names:

- populated: `slot_NNNN.bin`;
- empty: `slot_NNNN.empty`.

Every such name has `synthetic_name=true`. These names are **not source filenames and not semantic labels**. Physical slot index remains the authority.

This rule fixes a reconciliation defect found before the real-corpus run: the earlier #99 implementation left populated `logical_name` empty, which caused the shared `ContainerDocument::valid()` contract to reject any PAC containing populated entries as `invalid_document`.

## Fail-closed rules

The parser rejects:

- input shorter than 8 bytes;
- magic other than exact `PAC\0`;
- declared slot count above the parser safety limit;
- slot-table arithmetic that cannot be represented safely;
- an offset table extending beyond the supplied bytes;
- a populated slot offset pointing into the PAC header/table;
- a populated slot offset at or beyond PAC end;
- a final document that violates the shared container invariants.

The current safety limit is `1 << 20` declared slots; actual table/file bounds normally limit this much earlier.

## Identity and semantic boundary

A PAC slot index is **not globally semantic**. Matching slot numbers across different resources do not imply matching schemas.

The parser preserves only:

- declared slot index;
- populated/empty state;
- container-relative byte offset;
- bounded inferred byte size;
- explicit synthetic presentation identity;
- container format/version/size.

It does **not** infer Stage names, HITS roles, transforms, model/texture/audio semantics, or original filenames. Those interpretations belong to separately evidenced profile/domain adapters.

## Validation

Synthetic regression covers valid parsing, sparse slots, duplicate offsets, next-distinct-offset extent inference, bad magic, truncation, offsets into the table, out-of-range offsets, slot-count safety and zero-slot PAC.

### Real-corpus receipt — 2026-08-17

The exact #99 C++ parser was executed against all `PAC\0` files under `analysis_inputs/stage_drops` in the reacquired exact historical `DMC 3 RENGINE (6).zip` corpus.

Result:

- **32 PAC artifacts tested**;
- **32/32 parse success**;
- **0 failures**;
- **32/32 deterministic second parse**;
- **6 valid zero-slot PACs**;
- **6 sparse PACs**;
- maximum declared topology **415 slots**;
- observed artifact size range **16 .. 2,495,392 bytes**.

No proprietary game bytes are committed. The sanitized hash/topology receipt is stored at:

`docs/evidence/pac-real-corpus-validation-2026-08-17.md`

Duplicate non-zero offset behavior remains defensive product support because this real corpus does not exercise populated duplicate offsets.

## GDSpaces / Stage / HITS integration boundary

Target dependency direction:

```text
GDSpaces resource bytes
  -> PacParser
  -> ContainerDocument
  -> canonical container expansion / child ResourceIds
  -> profile/domain adapter
```

For HITS Slice 15:

```text
Stage-CFG ResourcePayload
  -> PacParser / canonical ContainerDocument
  -> Stage-CFG entry+descriptor adapter
  -> referenced-descriptor census
```

HITS must not parse PAC independently. PAC structural success also does not resolve Slice-16 transform-source provenance or make Stage-CFG slot 38 a transform table.

## Not implemented here

- PNST;
- NBZ;
- AFS;
- recursive container expansion policy;
- `.lst` behavior;
- archive/provider priority;
- write/repack/export;
- semantic slot naming;
- full DMC3 resource-runtime equivalence.

## Completion boundary

The bounded responsibility is now validated as:

> decode the evidenced PAC header/relative-slot structure into a deterministic, bounds-checked, slot-preserving `ContainerDocument`, including real sparse/zero/large-topology PAC corpus cases.

This does **not** make issue #3 complete and does not make PAC/PNST/NBZ/AFS production support complete.
