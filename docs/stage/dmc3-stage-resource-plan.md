# DMC3 Phase 12 Stage Resource Catalog

This layer represents the confirmed executable stage-resource table as a complete catalog. A concrete stage such as `st001` may be used as a test fixture, but it is not the canonical architecture target and does not define filename-generation rules for the other rows.

## Stage table descriptor

`phase12_stage_resource_table()` records:

- canonical executable SHA-256;
- public Evidence Packet ID;
- file offset `0x005C30A8`;
- RVA `0x005C4AA8`;
- VA `0x1405C4AA8`;
- 110 rows;
- four roles per row;
- 440 total entries.

Column order:

1. script;
2. room configuration;
3. room effects;
4. room sound.

The descriptor validates its artifact hash against the profile-specific known executable target and checks the VA/ImageBase/RVA relationship.

## Canonical Stage Catalog

The product-facing target is a complete `StageCatalog` generated from all executable rows:

```text
StageCatalog
  -> StageCatalogEntry[0..109]
       -> row identity
       -> script reference
       -> room configuration reference
       -> room effects reference
       -> room sound reference
       -> evidence-backed identifiers / variants
       -> resolved GDSpaces ResourceIds
       -> diagnostics
```

The catalog preserves the exact observed row/resource references. It must not derive the remaining rows from a `stNNN` filename template.

See [Stage Catalog](stage-catalog.md).

## Concrete fixtures

`st001` may remain one convenient regression fixture where its observed paths are known:

```text
scr/st001.pac
room/st001cfg.pac
room/st001_effect.pac
se/snd_r001.pac
```

Those four names describe that fixture only. They are not the generic Stage Catalog algorithm.

Tests must also include non-pattern, aliased, shared, or otherwise distinct rows as executable evidence exposes them, so a hidden `st001`/`stNNN` assumption cannot satisfy the suite.

## Matching / resolution model

Stage resource resolution consumes:

- a selected `StageCatalogEntry` derived from the executable table;
- the central GDSpaces/runtime resolver.

It does not:

- invent stage paths from a numeric stage ID;
- open files directly in Stage Ops;
- create a stage-local archive resolver;
- assume every row uses the same naming pattern.

Normalized paths are presentation/lookup data. Canonical identity remains tied to the executable row observation and resolved GDSpaces `ResourceId`.

## Diagnostics

The catalog/resolution layer must preserve:

- missing references;
- ambiguous normalized matches;
- duplicate/shared resources across rows;
- unresolved row metadata;
- non-pattern names;
- partial resolution without discarding the rest of a row.

## Stage categories

The four top-level table roles remain:

- script → scripts;
- room configuration → container/resource group whose nested children receive evidence-backed classification;
- room effects → effects;
- room sound → sounds.

The room-configuration resource is intentionally not overclassified. Nested resources may later expose cameras, lighting, collision, events, positions, and unknown categories through GDSpaces expansion and Stage Semantic Graph construction.

## Current completion boundary

Implemented/researched foundations include the table descriptor, row/resource-role model, resource matching/resolution infrastructure, StageBundle assembly, and synthetic integration coverage.

The completion target is not “load `st001`”. It is:

1. enumerate all 110 rows from the canonical executable;
2. expose the complete human/machine-readable Stage Catalog;
3. preserve all four observed references per row without path-template invention;
4. resolve representative row/variant classes through production GDSpaces;
5. build `StageBundle` objects from arbitrary selected catalog entries;
6. preserve shared, duplicate, special, and unresolved rows explicitly;
7. validate the generic pipeline across multiple different catalog entries with legal local game data.
