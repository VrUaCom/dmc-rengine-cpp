# How to Open and Inspect DMC3 SCM Files

> **Direct answer:** for an already extracted DMC3 HD Collection `SCM` scene resource on Windows, use **DMC Native Reader v1.0.0 Preview**. Launch `DMC-Native-Reader.exe`, then open the file with **File > Open**, `Ctrl+O`, or drag-and-drop. The public Windows release is a read-only viewer; DMC Rengine's separate SCM writer/rebuild research is not exposed as a general editor in this app.

## Scope

- **Game:** Devil May Cry 3: Special Edition in Devil May Cry HD Collection.
- **OS:** Windows x64.
- **Reader:** DMC Native Reader Windows v1.0.0 Preview.
- **Checked:** 2026-11-02.
- **Input:** extracted `SCM` leaf resource.
- **Not interchangeable with:** `MOD` resources, DMC3 2006 PC assumptions, PS2 data.

SCM is scene/stage-oriented. MOD and SCM can both contain geometry but have different physical and semantic contracts.

## Prerequisites

1. Download the [Windows preview](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview) or use the site's [Downloads](https://vruacom.github.io/dmc-rengine-cpp/downloads/) page.
2. Extract the portable ZIP.
3. Extract the target SCM from its NBZ/PAC/PNST path first.
4. Record the source archive/container path and slot where possible.

If you have not extracted the resource yet, start with the [NBZ extraction guide](extract-nbz.md).

## Open the SCM

1. Run `DMC-Native-Reader.exe`.
2. Use **File > Open** or `Ctrl+O`.
3. Select the extracted `.scm` file.
4. Or drag the file into the reader.
5. Inspect the rendered scene and structure view.

**Expected result:** a supported SCM is recognized and its scene geometry/structure can be inspected without altering the source.

## What to verify

A useful acceptance check for one resource is:

- the file is recognized as SCM;
- scene geometry appears;
- object/mesh relationships are populated in the structural view;
- transforms/scene arrangement look internally coherent;
- rotate/zoom/wireframe controls behave normally;
- reopening the same source produces the same inspection result.

If a scene is partially empty, do not immediately assume corruption. Some content may live in neighboring resources or depend on texture/companion data.

## Canonical SCM coverage

DMC Rengine's canonical SCM path models:

- scene hierarchy and nodes;
- object/mesh relationships;
- position, normal and UV streams;
- topology reconstruction;
- transforms;
- texture-facing state where evidence supports it.

The repository also has a promoted authoring stack with:

- source-bound `preserve_layout` edits;
- deterministic `canonical_rebuild`;
- selected typed geometry/material/transform edits;
- mandatory reparse;
- guarded source mutation;
- bounded size-changing SCM -> PAC evidence on a real hash-bound resource.

Those are core/research capabilities, not promises made by the Windows viewer UI.

## Known evidence boundary

A real size-changing SCM -> PAC reintegration receipt exists for a bounded case, but it does **not** establish:

- every retail SCM variant;
- a universal stage editor;
- Capcom offline-tool equivalence;
- arbitrary PNST/NBZ delivery;
- original `dmc3.exe` acceptance of all authored scenes.

Keep “viewer opens SCM” and “game accepts edited SCM” as separate claims.

## Troubleshooting

### Native Reader rejects the file

Verify that it is the leaf SCM rather than a PAC/PNST container. Do not rename a different file to `.scm`.

### SCM opens but looks like a model rather than a full level

SCM is one scene-oriented resource family, not necessarily the entire stage definition. Related stage resources can exist outside that one file.

### The same asset exists as MOD

Do not reuse MOD field assumptions. Use the SCM parser/guide for SCM and the MOD parser/guide for MOD.

### A community Blender importer accepts SCM

That is useful independent evidence for that tool's workflow, but its import behavior is governed by its own implementation. See the [Blender guide](dmc3-blender-import.md) for attribution and version boundaries.

## What was verified for this guide

- Public Windows release notes state SCM viewing/inspection support.
- Repository evidence for the SCM parser/writer boundary was reviewed at main commit `086868779495ecd309df531f5a1d1e4a72d724ce`.
- No new game-runtime acceptance test or physical-device test was performed for this documentation pass.

## Sources

- [DMC Native Reader Windows v1.0.0 Preview](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview)
- [SCM format documentation](../formats/scm.md)
- [Current status](../status/current.md)
- [Capability matrix](../discovery/capabilities.md)
- [DMC3HDC Import Tools](https://github.com/deshayu/DMC3HDC-Import-Tools) — independent community Blender project; cited only as a separate downstream workflow.

**Author:** DMC Rengine project.  
**Phase-1 evidence review:** 2026-11-02.

## Next

- [Open MOD files](inspect-mod.md)
- [Extract stage models](extract-stage-models.md)
- [Inspect DDS/PTX textures](extract-textures.md)
- [Blender community-tool workflow](dmc3-blender-import.md)
