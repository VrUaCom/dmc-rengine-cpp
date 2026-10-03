# HITS Editor migration plan

## Existing state

The repository contains an existing HITS format scanner and Binary Inspector adapter, but no separate finished GUI editor. The existing scanner used the obsolete `HITS$` / `0x18060001` marker model.

## Decision

Migrate the existing module in place. Do not create a competing parser or a second tool-local resource resolver.

## Preserved integration points

- GDSpaces format registration and resource session
- ResourceAnalyzer parser dispatch
- Binary Inspector document attachment
- stage-resource provenance from PAC container chains

## First implementation boundary

- canonical read-only parser
- structural validation
- spatial cell lists
- triangle-plane semantic fields
- raw flag preservation
- synthetic tests

## Editor boundary after parser merge

The editor consumes the same parsed model and must not introduce a private HITS parser.

The first implemented native authoring slice lives on `feature/hits-editor-mvp` and adds a reusable `hits::editor::Session` over the canonical parser and spatial writer. The session currently provides:

- stable surface IDs derived from the parsed triangle order
- raw flag editing
- evidence-gated collision preset switching for the four project-observed viewer variants: blue `0x00000001`, orange `0x00000009`, green `0x0000000A`, red `0x18060001`
- triangle vertex replacement
- single- and multi-surface translation
- add / duplicate / delete topology operations
- editor-side logical mesh groups built from stable surface IDs
- merge-mesh, translate-mesh and whole-mesh collision-preset operations
- undo / redo and reset-to-source state
- deterministic writer handoff with automatic fit-bounds fallback when an edit leaves the source grid
- parser round-trip through the existing `hits::writer::SpatialWriter`

The product/UI layer still needs to provide:

- 3D HITS + SCM overlay
- visual triangle / connected-surface / SCM-mesh picking
- final platform UI wiring for collision-color and raw-flag inspector
- final platform UI wiring for draw / boundary tools
- file-dialog/export UX and packaged editor application shell

The native C++ authoring core now additionally provides:

- connected-surface discovery by exact shared edges
- UI-facing surface inspection with raw flags, split upper/lower flag lanes, preset and logical-mesh membership
- SCM mesh -> world-space HITS conversion using the canonical SCM topology and hierarchy/world-transform paths
- whole-SCM-object -> one logical collision-mesh import
- draw-quad authoring as two HITS triangles
- rectangular four-wall boundary authoring as eight HITS triangles
- explicit PAC physical-slot HITS replacement through the existing packed relative-slot reflow writer
- source-aware PAC editing sessions for member 3/source 0 and member 6/source 1, including rebuilding both dirty sources into one PAC output

Topology-changing output is now structurally supported by the corpus-verified spatial writer. Original-game acceptance of arbitrary authored topology remains a separate validation gate and must not be described as proven.

### Collision-type naming rule

The editor exposes the four currently project-observed color/raw-flag presets as convenience controls, but it does not promote the colors into invented gameplay semantics. The UI should always keep the raw value visible and retain the evidence boundary.

### Mesh-merge naming rule

HITS has no serialized mesh table. An editor `Mesh` is therefore a logical authoring group of stable HITS surface IDs. `merge_meshes()` unions those groups so they can be moved or retyped together; it deliberately does not collapse triangles or claim a runtime mesh object. Triangle reduction/welding/simplification is a separate future operation.

## Coordination with PR #26

PR #26 owns Custom Build Identity and modifies CMake, Project Graph, ProjectWorkspace and source-integration files. This HITS branch avoids those paths. Later linkage to BuildRecord and IntegrationProject must be added after both branches merge, through a separate conflict-reviewed integration step.
