# DMC3 Stage Catalog

**Status:** canonical stage identity and selection model  
**Source authority:** canonical DMC3 executable stage-resource table

## Core rule

DMC Rengine does not treat `st001` as the canonical stage target. The canonical object is the complete **Stage Catalog** reconstructed from the executable stage-resource table.

The currently confirmed table shape is:

- 110 rows;
- four resource-role cells per row;
- 440 total resource references;
- column roles: script, room configuration, room effects, room sound.

A concrete stage such as `st001` may be used as a test fixture, regression case, or debugging sample, but it must never define the architecture, naming rules, resource-plan algorithm, or completion gate for Stage Ops/GDSpaces.

## Canonical model

```text
StageCatalog
  -> StageCatalogEntry[0..109]
       -> executable row identity
       -> raw cell metadata
       -> four resource-role references
            -> script
            -> room configuration
            -> room effects
            -> room sound
       -> observed names / aliases / shared paths
       -> resolved GDSpaces ResourceIds
       -> evidence-backed stage/variant metadata
       -> diagnostics
```

The row identity is primary. A filename-derived number such as `001` is presentation/derived metadata unless executable/runtime evidence proves it is the canonical stage identity.

## Stage names, variants, and shared resources

The catalog must preserve what the executable actually contains rather than forcing all rows into a `stNNN` template.

Therefore it must support:

- numbered stage names where observed;
- rows whose resource names do not follow the usual numeric pattern;
- shared resources reused by multiple rows;
- aliases;
- repeated paths;
- stage/room variants;
- special or non-standard rows;
- missing/empty/unresolved references where observed;
- future semantic classification of row variants only when supported by evidence.

Variant labels are not inferred from filenames alone. They are confidence/evidence-backed metadata over catalog entries.

## Selection contract

All Stage Ops, ModViz, testing, and game-backed integration workflows select from the `StageCatalog`.

Correct:

```text
canonical EXE
  -> StageCatalog
  -> select StageCatalogEntry
  -> resolve four role references
  -> StageBundle
  -> Stage Semantic Graph
```

Incorrect:

```text
stage number 001
  -> invent st001 paths
  -> assume all other stages follow the same pattern
```

## Test fixtures

A fixture may select any catalog entry. `st001` is allowed as one regression fixture because it is familiar and useful for debugging, but tests must include non-pattern/shared/variant rows so the implementation cannot accidentally hard-code `st001` or `stNNN` assumptions.

At minimum, the test corpus should eventually cover:

1. one conventional numbered row;
2. one row with non-pattern or aliased resource names;
3. one row sharing at least one resource with another entry;
4. one row containing unresolved/special metadata when such a row is confirmed;
5. cross-row enumeration proving all 110 rows are preserved in deterministic order.

## Required list artifact

The project must expose the full catalog as both:

- a human-readable list/table for inspection in Stage Ops/diagnostics;
- a deterministic machine-readable manifest generated from the canonical executable observation.

The list should include, per row:

- row index / executable table position;
- any evidence-backed stage identifier/name;
- all four raw resource references;
- normalized presentation paths;
- resolved `ResourceId` values when available;
- shared/duplicate relation flags;
- variant/classification metadata with confidence;
- diagnostics/unresolved fields.

## Relationship to StageBundle

`StageBundle` represents one selected catalog entry after GDSpaces resolution/materialization. It is not the global stage list.

Thus:

```text
StageCatalog = all executable stage rows
StageCatalogEntry = one row / stage-selection record
StageBundle = resolved resources for one selected entry
Stage Semantic Graph = semantic graph built from one or more selected bundles plus cross-stage relations
```

## Completion gate

Stage resource reconstruction is not considered complete because one fixture loads successfully.

The relevant gate is:

1. enumerate all 110 executable rows deterministically;
2. preserve their four resource roles without filename-template assumptions;
3. expose the complete list/catalog;
4. resolve representative different row/variant types through GDSpaces;
5. prove the same generic pipeline works across the catalog;
6. preserve unresolved/special cases explicitly rather than coercing them into `st001` behavior.
