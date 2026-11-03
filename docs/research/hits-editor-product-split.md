# HITS Editor product split

Status: implementation contract for `feature/hits-editor-mvp`.

## One core, two product surfaces

The HITS authoring implementation remains a single native core:

`hits::editor::Session`

No frontend is allowed to fork the HITS parser, writer, topology reconstruction,
spatial-grid rebuild, collision presets, SCM import, undo/redo or mesh-group
logic.

### 1. Standalone HITS Editor

Public boundary:

`hits::standalone::Session`

The standalone product owns file dialogs, local files and any future standalone
PAC/file-shell workflow. Its native session accepts raw HITS bytes and returns
raw rebuilt HITS bytes. It deliberately has no GDSpaces dependency.

This is the correct base for a separately packaged Windows/Linux/mobile HITS
Editor.

### 2. Embedded Rengine HITS tool

Public boundary:

`hits::gdspaces_tool::Session`

The embedded tool does not search archives and does not resolve resources.
GDSpaces materializes the exact HITS resource and passes a
`gdspaces::ResourcePayload` into the tool. On commit the tool returns a
`ResourcePayload` with:

- the same canonical resource identity;
- the same diagnostics/provenance/name/semantic evidence envelope;
- rebuilt HITS bytes;
- an updated resource byte size.

GDSpaces remains responsible for putting that resource back into its parent
PAC/NBZ/working-copy path.

## Routing

`ToolTarget::hits_editor` is the primary route for HITS resources, including
when the resource belongs to a stage. Stage Ops remains a companion consumer of
stage context rather than the owner of HITS authoring.

ModViz remains a visual companion. The upcoming HITS + SCM overlay/picking layer
should be shared presentation code consumed by both product surfaces.

## PAC boundary

The existing source-aware `hits::pac_session::PacSession` and
`PacHitsWriter` remain valid native container integration components.

For the embedded product, GDSpaces should normally hand the editor an already
materialized HITS child and own parent reintegration.

For the standalone product, a future shell adapter may use the PAC components
to support opening/saving a whole PAC, but that shell concern must not leak
GDSpaces ownership into `hits::standalone::Session`.

## Data flow

Standalone:

`file bytes -> standalone::Session -> editor::Session -> rebuilt HITS bytes -> file`

Embedded:

`GDSpaces ResourcePayload -> gdspaces_tool::Session -> editor::Session -> ResourcePayload -> GDSpaces reintegration`

The editor core is therefore shared, while ownership of resource transport is
explicit and non-overlapping.
