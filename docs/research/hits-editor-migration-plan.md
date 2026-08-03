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

The future editor will consume the same parsed model and provide:

- source-aware member 3/source 0 and member 6/source 1 views
- triangle selection and 3D preview
- raw flag bit view
- vertex editing
- normal and plane-D recomputation
- winding and degenerate-triangle validation
- byte-preserving save-copy mode

Topology changes, arbitrary cross-cell moves and spatial-index rebuild remain blocked until the exact deterministic builder is implemented and runtime-validated.

## Coordination with PR #26

PR #26 owns Custom Build Identity and modifies CMake, Project Graph, ProjectWorkspace and source-integration files. This HITS branch avoids those paths. Later linkage to BuildRecord and IntegrationProject must be added after both branches merge, through a separate conflict-reviewed integration step.
