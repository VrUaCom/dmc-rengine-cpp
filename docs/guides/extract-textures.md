# How to Open and Inspect DMC3 DDS and PTX Textures

> **Direct answer:** if you already have an extracted DMC3 HD Collection `DDS` or supported `PTX` resource, open it with **DMC Native Reader Windows v1.0.0 Preview** using **File > Open**, `Ctrl+O`, or drag-and-drop. The reader previews supported texture resources and PTX child-resource structure. It is read-only and does not prove arbitrary texture replacement/repacking.

## Scope

- **Game:** Devil May Cry 3: Special Edition in Devil May Cry HD Collection.
- **OS:** Windows x64.
- **Reader:** DMC Native Reader Windows v1.0.0 Preview.
- **Checked:** 2026-11-02.
- **Inputs:** extracted `DDS` and supported `PTX` resources.
- **Not claimed:** universal PTX conversion, arbitrary texel authoring, automatic model-texture relinking, original-game acceptance.

The public project intentionally distinguishes PTX bundle/container framing, descriptor-bearing texture slots, extracted DDS bytes and runtime texture identities.

## Prerequisites

1. Download the [Windows preview](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview) or use [Downloads](https://vruacom.github.io/dmc-rengine-cpp/downloads/).
2. Extract the target resource from NBZ/PAC/PNST first.
3. Preserve the source path and container/slot identity.
4. Keep the original bytes unchanged.

If you only know the outer archive, start with [NBZ extraction](extract-nbz.md).

## Open DDS

1. Launch `DMC-Native-Reader.exe`.
2. Open the extracted `.dds` file with **File > Open**, `Ctrl+O`, or drag-and-drop.
3. Confirm that the texture preview appears.
4. Inspect the available metadata/structure.

**Expected result:** a recognized DDS displays as an image/texture resource rather than being treated as a generic binary blob.

## Open PTX

1. Open the extracted `.ptx` resource the same way.
2. If the PTX contains supported child texture resources, use the reader's PTX child-resource navigation.
3. Inspect the child/descriptor relationship instead of assuming the whole PTX is one DDS file.

**Expected result:** the reader recognizes the PTX framing supported by the release and lets you inspect relevant texture content/children.

## Verify the texture result

For each resource, record:

- source NBZ/PAC/PNST path;
- file/slot identity;
- reader version;
- recognized family (DDS or PTX);
- visible dimensions when exposed;
- whether the preview is visually plausible;
- any child-resource index/path for PTX.

If the image is clearly corrupted, first re-check extraction boundaries. A wrong offset or container child can still begin with bytes that look superficially texture-like.

## DDS, PTX and model slots are not one identity

When tracing a model's texture:

```text
MOD/SCM texture-facing slot
  -> companion/descriptor relationship
  -> PTX framing or DDS-bearing payload
  -> decoded texture preview
  -> runtime GPU texture state
```

Not every resource follows every step, and the numerical identifiers at different layers are not automatically interchangeable.

## Common problems

### “DDS opens, PTX does not”

Confirm that the PTX is one of the supported DMC3 HD representations and that you extracted the complete resource. Do not rename PTX to DDS to force a preview.

### “The model is untextured in MOD/SCM view”

The model may reference a separate texture-bearing resource. Preserve the model's source context and inspect the related PTX/DDS separately.

### “A normal image editor can open the DDS — why use Native Reader?”

A generic DDS-capable editor is useful for image work. Native Reader is valuable when you want the DMC3 resource context, structural inspection and consistent handling alongside MOD/SCM/PTX without implying edit support.

### “Can I save the edited texture back to the game?”

Not from the current Windows Native Reader. DMC Rengine has texture framing/reflow research in bounded paths, but arbitrary replacement and original-game acceptance remain stronger gates.

### “A mod says to copy PAC files into GData.afs”

That is an installation path for a prepared mod, not proof that every DDS/PTX can be edited directly and dropped into the game.

## Community context

Current Nexus texture mods demonstrate real demand for:

- restoring PS2 texture content in the HD Collection;
- replacing character/weapon textures;
- installing prepared PAC resources through DDMK/Crimson or local `GData.afs`.

Those projects own their assets and installation instructions. This guide does not redistribute their textures or treat their successful mods as first-party DMC Rengine writer evidence.

## What was verified for this guide

- The public Windows Native Reader release documents DDS/PTX preview support and PTX child-resource navigation.
- The repository capability matrix marks the Windows release as read-only and does not claim editing/writer support.
- Public Nexus/PCGamingWiki material corroborates active HD Collection texture-mod workflows.
- No new proprietary game texture or screenshot is included in this documentation pass.

## Sources

- [DMC Native Reader Windows v1.0.0 Preview](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview)
- [DDS/PTX format research](../formats/textures.md)
- [Capability matrix](../discovery/capabilities.md)
- [Current status](../status/current.md)
- [PCGamingWiki — Devil May Cry HD Collection](https://www.pcgamingwiki.com/wiki/Devil_May_Cry_HD_Collection)
- [DMC3 PS2 Texture Pack and Fixes](https://www.nexusmods.com/devilmaycryhdcollection/mods/519) — community context only.

**Author:** DMC Rengine project.  
**Phase-1 evidence review:** 2026-11-02.

## Next

- [Open MOD](inspect-mod.md)
- [Open SCM](inspect-scm.md)
- [Find Dante model/texture resources](dante-model-textures.md)
- [Find Vergil model/texture resources](vergil-model-textures.md)
