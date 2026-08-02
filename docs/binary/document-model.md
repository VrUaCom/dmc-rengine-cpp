# Binary Inspector Document Model

The Binary Inspector domain model describes byte ownership and structure over a supplied GDSpaces resource. It does not open files or resolve containers.

## Document

A `binary::Document` contains:

- stable `ResourceRef`;
- total byte size;
- structural regions;
- ownership claims.

## Byte ranges

`ByteRange` uses a half-open interval:

```text
[offset, offset + size)
```

A range must be non-empty, non-overflowing, and within the document before it can be accepted.

## Regions

A `Region` contains:

- stable ID;
- display name;
- byte range;
- kind;
- optional recovered type name;
- optional evidence ID.

Current kinds:

- header;
- table;
- record;
- payload;
- padding;
- unknown.

Region IDs are unique. Overlapping regions are allowed because reverse-engineering hypotheses may conflict; overlap is reported rather than silently rejected.

## Ownership claims

An `OwnershipClaim` binds an owner/parser/subsystem ID to a byte range and rationale. Ownership is separate from structural regions because one parser may own multiple regions or a broad range with internal unknowns.

## Coverage

`coverage_bytes()` computes the union of all region ranges. Overlapping bytes are counted once.

This supports metrics such as:

```text
coverage ratio = covered bytes / total bytes
```

## Unknown ranges

`unknown_ranges()` returns gaps not covered by any registered region. Overlaps do not create negative or duplicated gaps.

## Conflicts

`conflicts()` reports pairwise region overlaps with the exact intersection range. This is the seed for the historical Binary Inspector conflict/ownership views.

## Current limits

- no field tree yet;
- no nested parent-child structure;
- no annotation persistence;
- no entropy map;
- no diff model;
- no selection API;
- no evidence packet exporter;
- ownership overlaps are stored but not yet analyzed.

## Planned extension order

1. typed fields and parent-child structure;
2. selection and owner lookup;
3. annotation/evidence links;
4. ownership conflicts;
5. diff regions;
6. entropy and unknown analysis;
7. manifest export;
8. UI adapters.

All future views consume this domain model and supplied bytes; none may introduce a separate source resolver.
