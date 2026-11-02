# How to Open and Inspect DMC3 MOD Files

> **Direct answer:** if you already have an extracted DMC3 HD Collection `MOD` resource, the simplest current first-party viewer on Windows is **DMC Native Reader v1.0.0 Preview**. Extract the portable ZIP, run `DMC-Native-Reader.exe`, then use **File > Open**, `Ctrl+O`, or drag the `.mod` file into the window. The release is read-only: it does not edit, export or repack the model.

## Scope

- **Game:** Devil May Cry 3: Special Edition in Devil May Cry HD Collection.
- **OS:** Windows x64.
- **Reader:** DMC Native Reader Windows v1.0.0 Preview.
- **Release:** https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview
- **Checked:** 2026-11-02.
- **Input:** already extracted `MOD` resource.
- **Not covered:** 2006 Ubisoft PC MOD assumptions unless separately proven; PS2 resources; arbitrary editing/repacking.

If your file is still inside NBZ/PAC/PNST, extract it first. Native Reader does not replace that step.

## Prerequisites

1. Download the Windows preview from the [release page](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview) or [Downloads](https://vruacom.github.io/dmc-rengine-cpp/downloads/).
2. Extract the ZIP to a writable folder.
3. Keep your game resources in a separate read-only/source folder.
4. Have one known DMC3 HD `MOD` resource with its source path recorded.

The reader is portable; no traditional installer is required.

## Open the MOD

1. Launch `DMC-Native-Reader.exe`.
2. Choose **File > Open** or press `Ctrl+O`.
3. Select the extracted `.mod` file.
4. Alternatively, drag and drop the file onto the application window.
5. Wait for parsing and the 3D/Inspector view to populate.

**Expected result:** supported geometry is displayed and structural information is available for inspection. The release also exposes interaction such as rotate/zoom/wireframe/reset according to its published notes.

## Verify that the result is meaningful

Do not stop at “a window opened.” Check:

1. the reader identifies the resource as MOD rather than an unknown format;
2. geometry appears rather than an empty/garbled scene;
3. the structure/Inspector view contains model/object/mesh information;
4. rotate and zoom work on the loaded model;
5. if wireframe is enabled, topology remains consistent with the visible geometry;
6. record the reader version and the source path of the resource.

If the model renders but some material/texture information is absent, that does not necessarily mean the MOD parse failed. Texture relationships cross into companion/DDS/PTX resources.

## What the canonical MOD research understands

The DMC Rengine MOD authority covers the serialized model structure and a large runtime-facing surface, including:

- document/object/mesh records;
- hierarchy and transform relationships;
- positions, normals and fixed-point UV streams;
- topology controls;
- packed skin weights and blend indices;
- texture slots and companion relationships;
- bounds and selected runtime-facing flags;
- preservation contracts for bytes whose artistic meaning is not proven.

The project treats **reverse completeness** and **writer completeness** as different gates. Preserved/undecoded fields can be a valid terminal reverse outcome when their storage/runtime carriage is known but a semantic label is not justified.

## Reader versus writer

The Windows Native Reader is read-only.

The DMC Rengine core has a bounded preserve-layout MOD writer and evidence-backed reintegration receipts for selected fields/containers. That does not make this viewer a general MOD editor and does not prove arbitrary edited models are accepted by `dmc3.exe`.

Do not advertise:

- free-form mesh editing;
- skeleton authoring;
- texture replacement;
- general MOD export;
- universal PNST/NBZ repacking;
- original-game acceptance

as features of the Windows reader.

## Troubleshooting

### The file is rejected

Check that:

- it is an extracted leaf resource, not the containing PAC/PNST/NBZ;
- it belongs to DMC3 HD Collection;
- the extension was not guessed/renamed;
- the file is complete and non-zero length.

Unknown/unsupported resources are intentionally rejected rather than forced through the wrong parser.

### Geometry is visible but textures are missing/wrong

Follow the texture relationship separately. Use [DDS/PTX inspection](extract-textures.md) and preserve the source container/slot information. A MOD texture slot, companion entry and DDS/PTX payload are related but not one universal ID.

### A Blender tool imports the file differently

That is a separate implementation. Compare the DMC Rengine canonical structure with the independent Blender tool's own documented support; do not use one tool's successful import to silently promote another tool's semantics.

### The resource came from DMC3 2006 or PS2

Treat it as a different target until binary compatibility is proven. This guide's verified scope is HD Collection.

## Evidence and limitations

The canonical MOD reverse is complete for the project's defined reverse scope, while the public reader remains intentionally read-only. Exact field semantics, writer receipts and unresolved authoring gates live in the format/status documents.

No new model screenshot or physical-device test was fabricated for this phase. The documentation uses the published Windows release contract and repository evidence.

## Sources

- [Windows v1.0.0 Preview release](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview)
- [DMC3 MOD format](../formats/mod.md)
- [Current project status](../status/current.md)
- [Capability matrix](../discovery/capabilities.md)
- [Nexus Native Reader page](https://www.nexusmods.com/devilmaycryhdcollection/mods/533) — independent distribution/community surface for the same public Windows tool.

**Author:** DMC Rengine project.  
**Phase-1 evidence review:** 2026-11-02.

## Next

- [Inspect SCM stage/scene resources](inspect-scm.md)
- [Inspect DDS/PTX textures](extract-textures.md)
- [Find character models](character-models.md)
- [Downloads](../discovery/downloads.md)
