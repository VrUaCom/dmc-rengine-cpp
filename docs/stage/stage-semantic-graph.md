# Stage Semantic Graph v1

**Status:** canonical planned aggregation over existing Stage Workspace contracts  
**Snapshot date:** 2026-08-08

Stage Ops must evolve from a collection of format-specific tools into a semantic view of one stage. `StageBundle`/`GDStageBundle` remains the typed resource input; the Stage Semantic Graph adds relations between those resources and their recovered runtime meaning.

## Target model

```text
Stage
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

Unknown resources and unresolved links remain first-class graph nodes rather than being discarded.

## Existing foundation

The reviewed repository already contains stage identity, the 110 x 4 DMC3 stage-table descriptor, resource matching, typed StageBundle assembly, Stage Workspace construction, and shared Stage Ops/ModViz views. Format work exists for HITS, DCA, LIG2, and Stage TXT.

## Graph rules

- every resource node retains canonical GDSpaces `ResourceId`;
- semantic edges carry confidence/evidence when inferred through reverse engineering;
- partial StageBundle assembly must preserve valid nodes and diagnostics;
- format parsers do not open files independently;
- executable/runtime links reference EXE/Reverse Core identities rather than embedding arbitrary VAs as unscoped constants;
- Stage Ops and ModViz consume the same graph/workspace state.

## Milestone v1

The first graph milestone should use legal local `st001` data and demonstrate:

1. one game-backed StageBundle from GDSpaces;
2. typed nodes for available geometry/collision/light/camera/TXT/effect/audio resources;
3. known door/transition and runtime relationships represented as evidence-backed edges;
4. unknown/unparsed resources preserved;
5. deterministic graph/workspace manifest;
6. identical resource identity reused by Stage Ops, ModViz, Binary Inspector, and executable evidence links.

This milestone is about cross-format semantics and provenance, not about completing every Stage Ops UI panel.
