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
- triangle vertex replacement
- single- and multi-surface translation
- add / duplicate / delete topology operations
- undo / redo and reset-to-source state
- deterministic writer handoff with automatic fit-bounds fallback when an edit leaves the source grid
- parser round-trip through the existing `hits::writer::SpatialWriter`

The product/UI layer still needs to provide:

- source-aware member 3/source 0 and member 6/source 1 views
- triangle / connected-surface / SCM-mesh selection
- 3D HITS + SCM overlay
- collision-color and raw-flag inspector
- draw-triangle / draw-quad tools
- boundary authoring
- SCM-to-HITS geometry copy
- PAC member replacement and export workflow

Topology-changing output is now structurally supported by the corpus-verified spatial writer. Original-game acceptance of arbitrary authored topology remains a separate validation gate and must not be described as proven.

## Coordination with PR #26

PR #26 owns Custom Build Identity and modifies CMake, Project Graph, ProjectWorkspace and source-integration files. This HITS branch avoids those paths. Later linkage to BuildRecord and IntegrationProject must be added after both branches merge, through a separate conflict-reviewed integration step.
