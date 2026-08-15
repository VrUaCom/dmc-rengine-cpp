# PAC Read-Only Structural Parser

**Date:** 2026-08-15  
**Status:** IMPLEMENTED / SYNTHETIC REGRESSION; REAL LOCAL PAC CORPUS VALIDATION PENDING

## Purpose

This is the first production-oriented read-only PAC structural parser in the clean C++ generation.

It exists to remove the architectural gap where higher-level systems had a `ContainerDocument` contract but no canonical PAC decoder feeding it.

The parser is intentionally structural. It does not interpret Stage, HITS, model, texture, gameplay or filename semantics.

## Accepted binary contract

The currently promoted PAC structure is:

```text
+0x00  char[4]  "PAC\0"
+0x04  u32 LE   declared slot count
+0x08  u32 LE[] slot-offset table, one entry per declared slot
```

For an offset-table entry:

- `0` means the slot is structurally empty;
- non-zero means a populated slot begins at that absolute PAC byte offset;
- populated offsets must begin at or after the end of the slot-offset table;
- populated offsets must be strictly inside the supplied PAC byte span.

Slot count is preserved exactly, including empty slots.

## Extent inference

PAC does not carry an explicit size beside each slot-table offset in this promoted subset.

The parser therefore derives a populated slot extent as:

```text
start = slot offset
end   = next greater distinct populated offset
         or PAC byte-span end for the final distinct offset
size  = end - start
```

Important details:

- slot-table order is not used as the extent authority;
- zero/empty slots are ignored when finding the next extent boundary;
- duplicate non-zero offsets are preserved as separate slot identities and receive the same bounded extent;
- no semantic alias meaning is inferred from duplicate offsets.

The resulting generic representation is `formats::ContainerDocument` / `ContainerEntry`.

## Fail-closed rules

The parser rejects:

- input shorter than 8 bytes;
- magic other than exact `PAC\0`;
- declared slot count above the parser safety limit;
- slot-table arithmetic that cannot be represented safely;
- an offset table extending beyond the supplied bytes;
- a populated slot offset pointing into the PAC header/table;
- a populated slot offset at or beyond the end of the supplied PAC span;
- a final `ContainerDocument` that fails the shared container invariants.

The current safety limit is `1 << 20` declared slots. The actual byte-span/table bound normally limits this much earlier.

## Identity and semantic boundary

A PAC slot index is **not globally semantic**.

HITS Pass-10 Slice 13 provides a concrete negative control: matching PAC slot numbers across different PAC resources do not imply matching schemas.

Therefore this parser only preserves:

- slot index;
- populated/empty state;
- absolute byte offset;
- bounded byte size;
- container format/version/size.

It does **not** invent:

- Stage domain names;
- `stNNN` meaning;
- HITS/collision identities;
- transform/entry/descriptor roles;
- model/texture/audio semantics;
- filenames for unnamed slots.

Those interpretations belong to separately evidenced profile/domain adapters.

## Empty-slot preservation

`declared_slot_count` and `entries.size()` retain the declared PAC slot space, including empty offsets.

This is required because slot identity is positional even when a slot has no payload. Higher-level container expansion must not collapse the slot namespace to only populated children.

## Current validation state

Synthetic regression covers:

- exact `PAC\0` magic;
- declared slot count;
- empty-slot preservation;
- non-zero populated offsets;
- next-distinct-offset size inference;
- duplicate-offset preservation;
- invalid magic;
- truncated header/table;
- offset into header/table;
- offset at file end/outside payload;
- slot-count safety limit;
- zero-slot structural PAC.

The implementation was also built locally against the real clean-repository `ContainerDocument` API before promotion to the branch.

No proprietary PAC bytes are committed.

## Real-corpus gate

This parser must **not** be advertised as fully game-validated PAC support yet.

The next validation gate requires legally supplied local PAC resources through the canonical GDSpaces path, with sanitized receipts recording at minimum:

- resource identity/hash;
- PAC byte size;
- declared slot count;
- populated/empty slot counts;
- per-populated-slot index/offset/size;
- duplicate-offset groups if present;
- parser diagnostics;
- reopen/determinism result.

Representative validation should include more than one PAC family. A Stage-CFG PAC alone cannot establish globally semantic slot roles.

## GDSpaces / Stage / HITS integration boundary

This parser is the structural layer required before higher-level PAC consumers can run without a private parser.

Target dependency direction:

```text
GDSpaces resource bytes
  -> PacParser
  -> ContainerDocument
  -> canonical container expansion / child ResourceIds
  -> profile/domain adapter
```

For HITS Slice 15 specifically:

```text
Stage-CFG ResourcePayload
  -> PacParser / canonical ContainerDocument
  -> Stage-CFG entry+descriptor adapter
  -> referenced-descriptor census
```

HITS must not parse PAC independently.

For Slice 16, this parser does not resolve `entry+0x01` transform provenance. PAC structural success does not make Stage-CFG slot 38 a transform table.

## Not implemented here

- PNST;
- NBZ;
- AFS;
- recursive container expansion policy;
- `.lst` behavior;
- archive source priority;
- write/repack/export;
- semantic slot naming;
- full DMC3 resource runtime equivalence.

Those remain separate gates under the production container/resource workstream.

## Completion boundary

This parser may only be called complete at the bounded responsibility:

> decode the currently evidenced PAC header/slot-offset structure into a deterministic, bounds-checked, slot-preserving `ContainerDocument`.

It does not make issue #3 complete and does not make PAC/PNST/NBZ/AFS production support complete.
