# Specification 006 — Binary Inspector Domain Model

## Status

Initial region/ownership foundation implemented.

## Problem

Binary Inspector historically accumulated valuable structure, ownership, unknown, and conflict views. The clean C++ generation needs a source-independent domain model before any GUI migration.

## Goals

- represent a binary document over a stable `ResourceRef`;
- represent overflow-safe half-open byte ranges;
- register uniquely identified structural regions;
- preserve overlapping hypotheses rather than silently rejecting them;
- represent ownership claims independently from regions;
- calculate union coverage;
- expose unknown gaps;
- report exact overlap conflicts;
- attach recovered type and evidence IDs;
- remain independent from filesystem/container resolution.

## Non-goals

- hex rendering;
- opening files;
- format parsing;
- editing bytes;
- entropy analysis;
- diff visualization;
- GUI selection state;
- automatic conflict resolution.

## Current public types

- `binary::ByteRange`;
- `binary::RegionKind`;
- `binary::Region`;
- `binary::OwnershipClaim`;
- `binary::RegionConflict`;
- `binary::Document`.

## Invariants

1. Every accepted range is non-empty, non-overflowing, and within the document.
2. Region IDs are unique.
3. Regions may overlap; overlaps are reported as conflicts.
4. Coverage counts overlapping bytes once.
5. Unknown ranges are the complement of the union of known regions.
6. Ownership is explicit and carries a rationale.
7. The document stores resource identity but never resolves the source.

## Acceptance criteria

- out-of-range regions and ownership claims are rejected;
- duplicate region IDs are rejected;
- deterministic region ordering by offset/ID;
- union coverage is correct with overlap;
- unknown gaps are correct;
- exact conflict intersections are returned;
- Windows/Linux tests pass.

## Planned extensions

1. fields and nested structures;
2. owner lookup for selection;
3. annotations and Evidence Packet links;
4. ownership overlap diagnostics;
5. diff documents;
6. entropy/unknown analyzers;
7. manifest export;
8. UI adapters.

## Risks

- treating region overlap as always invalid;
- conflating parser ownership with resource ownership;
- making UI nodes the canonical structure;
- storing raw pointers into temporary payloads;
- bypassing GDSpaces to reopen source bytes.
