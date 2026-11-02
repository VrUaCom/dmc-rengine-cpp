# How to Start Modding Devil May Cry 3 HD Collection

This guide is a beginner-oriented orientation for **Devil May Cry 3: Special Edition** in **Devil May Cry HD Collection** on PC, not a promise that every conceivable DMC3 modification is currently automated.

## 1. Start with your own game resources
Use a legitimately obtained copy of Devil May Cry HD Collection. Keep the original installation and source game archives unmodified. Make a separate working directory and back up anything you plan to inspect or modify.

## 2. Identify the operation before choosing a tool
There are distinct tasks: **discover and extract a resource** from an archive; **inspect** its geometry, texture, structure or motion; **author a bounded change**; **rebuild/reintegrate** it; and **verify** what the original game accepts. Success at any one step is not proof the next one works.

If your problem is an NBZ archive or nested PAC/PNST container, start with the GDSpaces documentation in DMC Rengine. If you already have a supported MOD or SCM payload and want to understand it visually, consult the platform-specific DMC Native Reader release and the model/format guide. Blender integration is a developing product, not a substitute for proven authoring or repacking.

## 3. Follow the resource hierarchy
DMC3 HD's useful resource trail can pass from numbered NBZ volumes through contained PAC/PNST slots to leaf payloads. Preserve the containing archive, member, physical slot and provenance. A file name or guessed extension does not necessarily establish its real binary format.

For model/scenes, consult the MOD and SCM readers independently; for texture representations, see DDS and PTX. A reader can recognize a resource without supporting arbitrary conversion or modification.

## 4. Check version, platform and evidence
Verify the actual published release for your OS. The Android and Windows Native Reader offerings have historically differed; future Android/Windows/iOS/macOS parity is planned, not assumed. Treat a parser, preview, canonical writer and original-game acceptance as separate capabilities.

## 5. Escalate safely
When the requested edit is not supported, use the format's canonical documentation and status rather than experimenting on the installed game. Evidence-backed development begins with a reproducible, hash-bound source and ends with reopen/validation and original-game tests where relevant.

## Further reading
- [DMC Rengine](https://github.com/VrUaCom/dmc-rengine-cpp)
- [Native Reader](https://github.com/VrUaCom/DMC-Native-Reader)
- [Canonical format index](../formats/public-index.md)
- [Project status](../status/current.md)
- [First-party tool directory](../discovery/our-tools.md)

Independent fan-made research project, not endorsed by Capcom; does not distribute proprietary game archives.
