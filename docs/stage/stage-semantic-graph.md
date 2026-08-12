# Stage Semantic Graph v1

**Status:** canonical planned aggregation over Stage Catalog and Stage Workspace contracts  
**Snapshot date:** 2026-08-12

Stage Ops must evolve from format-specific tools into a semantic view over stages selected from the complete DMC3 **Stage Catalog**. `StageCatalog` is the global executable-derived list; `StageBundle`/`GDStageBundle` represents resources resolved for a selected catalog entry; the Stage Semantic Graph adds relationships between those resources and their recovered runtime meaning.

## Target model

```text
StageCatalog
  -> StageCatalogEntry
       -> Stage
            -> Room
            -> Geometry
            -> Collision / HITS
            -> Lighting
            -> Camera
            -> Door / Transition
            -> Script / Event
            -> Effect
            -> Audio
            -> Runtime / EXE reference
```

Cross-stage/shared-resource relationships may connect multiple catalog entries. Unknown resources and unresolved links remain first-class graph nodes rather than being discarded.

## Existing foundation

The project already has the 110 x 4 DMC3 stage-table descriptor, resource-role modeling, StageBundle assembly, Stage Workspace construction, and shared Stage Ops/ModViz views. Format work exists for HITS, DCA, LIG2, and Stage TXT.

## Graph rules

- all stage selection begins from the complete executable-derived `StageCatalog`;
- every resource node retains canonical GDSpaces `ResourceId`;
- every stage node retains its catalog-entry / executable-row identity;
- semantic edges carry confidence/evidence when inferred through reverse engineering;
- shared resources across catalog entries remain shared relations rather than being duplicated into invented per-stage identities;
- partial StageBundle assembly preserves valid nodes and diagnostics;
- format parsers do not open files independently;
- executable/runtime links reference EXE/Reverse Core identities rather than arbitrary unscoped VAs;
- Stage Ops and ModViz consume the same catalog/graph/workspace state;
- filename patterns such as `stNNN` are not stage identity rules unless separately proven.

## Milestone v1

The first milestone is **catalog-driven**, not `st001`-driven:

1. enumerate all 110 executable stage-table rows into a deterministic Stage Catalog;
2. expose a human-readable and machine-readable list of all entries and their four resource roles;
3. select arbitrary entries from that catalog;
4. resolve several representative entry/variant types through GDSpaces;
5. build typed StageBundles from those selected entries;
6. construct semantic nodes for available geometry/collision/light/camera/TXT/effect/audio resources;
7. preserve shared, duplicate, unknown, special, and unresolved cases;
8. emit deterministic graph/workspace manifests;
9. prove identical resource identity is reused by Stage Ops, ModViz, Binary Inspector, and executable evidence links.

A familiar row such as `st001` may be one regression fixture, but successful loading of that one row is not the milestone exit gate.

This milestone is about catalog-wide generic semantics and provenance, not completing every Stage Ops UI panel.
