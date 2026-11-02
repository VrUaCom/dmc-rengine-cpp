# How to Start Modding Devil May Cry 3 HD Collection

> **Direct answer:** for modern PC modding, first confirm that you are working with **Devil May Cry 3: Special Edition inside Devil May Cry HD Collection**, keep the original game data untouched, use an archive/container workflow to locate resources, and use **DMC Native Reader** only for read-only inspection of supported extracted `MOD`, `SCM`, `DDS` and `PTX` resources. Do not mix instructions for the 2006 Ubisoft PC port, PS2, or HD Collection unless a guide explicitly says they are compatible.

## Scope and verified baseline

- **Game:** Devil May Cry 3: Special Edition in Devil May Cry HD Collection.
- **Primary platform for this guide:** Windows x64.
- **Checked:** 2026-11-02.
- **Reader release referenced:** [DMC Native Reader Windows v1.0.0 Preview](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview).
- **Reader scope:** read-only inspection of supported `MOD`, `SCM`, `DDS` and `PTX` resources.
- **Not the same game build:** Devil May Cry 3 Special Edition 2006 Ubisoft PC release.
- **Not covered as an interchangeable source:** PS2 retail resources.
- **Canonical technical authority:** this repository's `main`, format documentation and [current status](../status/current.md).

The HD Collection currently stores DMC3 data under its own archive/resource layout. Community instructions for the 2006 PC port often refer to different paths, executables and tools. Treat edition mismatch as the first troubleshooting question.

## Before you change anything

1. Use a legitimately obtained copy of Devil May Cry HD Collection.
2. Make a backup of any archive or game file you plan to touch.
3. Prefer a separate workspace for extracted resources.
4. Record the source path of each resource you extract. At minimum, keep the outer archive name and nested PAC/PNST slot or path when known.
5. Do not use the Native Reader as an editor. The current Windows preview is intentionally read-only.

Steam updates can replace `dmc3-0.nbz` and other game files. A September 2026 update changed the DMC3 archive and executables, so backup/restore guidance is not theoretical. Community discussion also shows that users who modify the original NBZ directly can lose those changes when the game updates.

## Choose the task before choosing the tool

### I only want to install an existing mod

Follow the mod author's instructions and confirm the **edition**. Many current HD Collection mods use a local `data/dmc3/GData.afs` path through DDMK/Crimson or replace/archive files directly. That is an installation workflow, not the same thing as reverse-engineering a resource.

Useful independent projects include:

- [DDMK](https://github.com/serpentiem/ddmk) — gameplay/mod framework; governed by its own compatibility requirements.
- [DMC3 Crimson](https://github.com/berthrage/DMC3-Crimson) — DMC3 gameplay/QoL project based on DDMK.

DMC Rengine does not own those tools and does not use their successful installation as evidence for DMC Rengine writer support.

### I want to find a model, scene or texture

Use this sequence:

```text
HD Collection DMC3 data
  -> NBZ archive/member
  -> PAC or PNST when present
  -> leaf resource
  -> MOD / SCM / DDS / PTX inspection
```

Start with [How to extract files from DMC3 NBZ archives](extract-nbz.md). Preserve provenance as you descend through nested containers.

### I already have an extracted MOD or SCM

Use the Windows Native Reader preview:

1. Download the Windows package from the [release page](https://github.com/VrUaCom/DMC-Native-Reader/releases/tag/windows-v1.0.0-preview) or the project's [Downloads](https://vruacom.github.io/dmc-rengine-cpp/downloads/) page.
2. Extract the ZIP into a normal writable folder.
3. Launch `DMC-Native-Reader.exe`.
4. Use **File > Open**, `Ctrl+O`, or drag the resource onto the window.
5. For `MOD`, follow [Open and inspect DMC3 MOD files](inspect-mod.md).
6. For `SCM`, follow [Open and inspect DMC3 SCM files](inspect-scm.md).

**Expected result:** the reader accepts a supported resource and presents a read-only structural/visual view. If the file is rejected, do not rename it to force a parser; first verify that you extracted the correct leaf resource.

### I already have DDS or PTX

Follow [Inspect DDS/PTX textures](extract-textures.md). The reader can preview supported texture resources, but a successful preview does not prove that arbitrary replacement/repacking is supported.

### I want Blender

The first-party **DMC Native Blender Reader** does not currently have a public add-on release that can be promoted as a verified workflow. For present-day Blender import, use the [Blender community-tool guide](dmc3-blender-import.md), which attributes independent tools to their maintainers and keeps their support contracts separate from DMC Rengine.

## How to verify your first successful inspection

A useful first milestone is deliberately small:

1. Start from an extracted resource whose source is known.
2. Open it without changing the original bytes.
3. Confirm the reader recognizes the intended family (`MOD`, `SCM`, `DDS` or `PTX`).
4. For models/scenes, confirm geometry is visible and the Inspector exposes structure.
5. For textures, confirm the image/metadata view is populated.
6. Record the resource's original path and the reader version.

That proves **inspection of that resource with that reader build**. It does not prove writer support or in-game acceptance.

## Common mistakes

### The guide I found refers to the 2006 PC version

Stop and re-check the edition. A path, tool or file name from the 2006 Ubisoft port may not map to HD Collection.

### The mod instructions say to use `GData.afs`

That is common in HD Collection community workflows, especially with DDMK/Crimson. It does not mean `GData.afs` is a single binary archive you should parse as one format; it is also used as a logical/local resource path in community installation workflows.

### The Native Reader says the file is unsupported

Do not change the extension as a workaround. Verify that you extracted the correct leaf payload and that the release actually supports the format.

### The game update broke an installed mod

Restore from backup or reinstall/verify the original game files, then follow the current mod's compatibility instructions. Treat updates as a separate compatibility axis from file-format support.

## Limits of this beginner workflow

- Native Reader Windows v1.0.0 Preview is not a universal editor, converter, repacker or archive extractor.
- DMC Rengine contains bounded writer/reintegration research, but that is not equivalent to universal original-game acceptance.
- DMC3 2006, HD Collection and PS2 are not interchangeable targets.
- Android, iOS and future macOS/Linux builds must be evaluated from their own release/status evidence.

## Sources and verification

**Project sources**

- [DMC Rengine current status](../status/current.md)
- [First-party tools](../discovery/our-tools.md)
- [Public downloads](../discovery/downloads.md)
- [Capability matrix](../discovery/capabilities.md)
- [DMC Native Reader releases](https://github.com/VrUaCom/DMC-Native-Reader/releases)

**Independent compatibility/context sources checked 2026-11-02**

- [PCGamingWiki — Devil May Cry HD Collection](https://www.pcgamingwiki.com/wiki/Devil_May_Cry_HD_Collection)
- [DDMK](https://github.com/serpentiem/ddmk)
- [DMC3 Crimson](https://github.com/berthrage/DMC3-Crimson)
- Steam community discussion about the September 2026 DMC3 update and `dmc3-0.nbz` replacement.

**Author:** DMC Rengine project.  
**Phase-1 evidence review:** repository evidence + public-source review, 2026-11-02. No new physical-device test was performed for this documentation pass.

## Next

- [Extract files from a DMC3 NBZ archive](extract-nbz.md)
- [Open and inspect MOD](inspect-mod.md)
- [Open and inspect SCM](inspect-scm.md)
- [Inspect DDS/PTX textures](extract-textures.md)
- [Choose a first-party tool](../discovery/our-tools.md)
