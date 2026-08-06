# Binary Inspector Document Model

The Binary Inspector domain describes byte structure, fields, ownership, evidence, selection, and analysis over a resource supplied by GDSpaces. It never opens game files or resolves containers independently.

## Architectural boundary

```text
GDSpaces ResourceRef + supplied bytes
                ↓
       binary::Document
                ↓
regions / fields / ownership / annotations
                ↓
selection / coverage / conflicts / diff / entropy
                ↓
CLI, desktop UI, tests, manifests, Evidence links
```

The C++20 implementation ports behavior and workflows from the web generation without copying React state, browser file access, or editor-specific resource resolution.

## Document

A `binary::Document` contains:

- stable `ResourceRef`;
- total byte size;
- structural regions;
- typed fields and parent-child relationships;
- ownership claims;
- annotations and Evidence links.

The document stores analysis metadata, not the resource bytes themselves. Byte-consuming analyses receive explicit `std::span<const std::byte>` inputs.

## Byte ranges

`ByteRange` uses a half-open interval:

```text
[offset, offset + size)
```

A range must be non-empty, non-overflowing, and within the document before it can be accepted.

Supported relations:

- contains an offset;
- contains another range;
- overlaps another range;
- validates against total byte size.

## Regions

A `Region` contains:

- stable ID;
- display name;
- byte range;
- kind;
- optional recovered type name;
- optional Evidence ID.

Current kinds:

- header;
- table;
- record;
- payload;
- padding;
- unknown.

Region IDs are unique. Overlapping regions are allowed because reverse-engineering interpretations may conflict; overlap is reported rather than silently rejected.

## Fields

A `Field` contains:

- stable ID and name;
- byte range;
- typed `FieldKind`;
- recovered type name;
- display value;
- optional parent field ID;
- optional Evidence ID.

Child fields must fit completely inside their parent. This supports structure trees, arrays, nested records, and Field Inspector adapters.

## Ownership claims

An `OwnershipClaim` binds an owner, parser, or subsystem ID to a byte range and rationale.

Ownership is separate from regions because one parser may own multiple non-contiguous ranges, and multiple interpretations may overlap. Cross-owner overlaps are exposed through `ownership_conflicts()`.

## Annotations

Annotations contain:

- stable ID;
- byte range;
- text;
- optional Evidence ID;
- normalized tags.

Empty tags and duplicate tags are removed during insertion.

## Selection

Two selection modes are available.

### Offset selection

`selection_at(offset)` returns every region, field, ownership claim, and annotation containing one byte position.

### Range selection

Wave 1 adds `selection_for_range(range)` plus typed overlap queries:

- `regions_overlapping()`;
- `fields_overlapping()`;
- `owners_overlapping()`;
- `annotations_overlapping()`.

This ports the web workflow where drag-selection in the hex view drives the Structure, Field, Ownership, and Annotation panels.

Invalid or out-of-document selection ranges return an empty context.

## Coverage and unknown ranges

`coverage_bytes()` computes the union of all region ranges. Overlapping bytes are counted once.

```text
coverage ratio = covered bytes / total bytes
```

`unknown_ranges()` returns gaps not covered by registered regions. Unknown does not mean unused; it means the current document model has not assigned a structural region.

## Conflicts

`conflicts()` reports pairwise region overlaps with exact intersection ranges.

`ownership_conflicts()` reports overlapping claims from different owners. Claims from the same owner may cover multiple adjacent or overlapping ranges without becoming cross-owner conflicts.

## Byte diff

Wave 1 adds `aligned_byte_diff()`.

It produces deterministic spans classified as:

- equal;
- modified;
- inserted;
- removed.

The first implementation is deliberately offset-aligned. It does not guess resynchronization after a middle insertion. This avoids presenting heuristic alignment as fact. A future resynchronizing diff may be added as a separate explicitly heuristic mode.

## Entropy map

Wave 1 adds `entropy_map()` using configurable window and step sizes.

Each window reports:

- byte range;
- Shannon entropy in bits per byte;
- zero-byte ratio;
- unique-byte count;
- visualization band: zero-fill, low, medium, or high.

Entropy bands are UI heuristics only. They must not be treated as proof of compression, encryption, executable code, padding, or any specific format.

## Manifest and adapters

The document model supports deterministic manifest JSON and format adapters such as HITS. Future UI and CLI layers consume the same domain objects; they must not create a parallel Binary Inspector state model.

## Current remaining gaps

- persistent Analysis Cache keyed by resource hash, adapter version, and analysis options;
- resynchronizing or structure-aware diff mode;
- entropy/unknown correlation view;
- duplicate-offset and suspicious-table diagnostics as a generic service;
- reusable binary template schema and template registry;
- RVA/VA bridge view over EXE workspace mappings;
- Binary Inspector to guarded-patch safety bridge;
- production desktop hex/structure interaction layer.

See [Web-to-C++20 cross-port plan](web-crossport.md).
