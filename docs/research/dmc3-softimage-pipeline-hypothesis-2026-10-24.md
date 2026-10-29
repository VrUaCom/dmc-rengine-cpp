# Softimage pipeline: a research hypothesis

> Ported 2026-10-29 from the DMC Native Reader (`NR-Luna-v73`, `docs/research/softimage-pipeline-hypothesis.md`). Paths below that start with `app/` or `tools/research/` are the Reader's. In Rengine: a hypothesis; no code depends on it.

Status: open. Nothing in the Reader depends on it.

## The hypothesis

Capcom made PS2-era games with Softimage (3D, then XSI), its SDK, scripts
and in-house exporters. If DMC3's assets came through that pipeline, the
way Softimage stores scenes could explain parts of SCM, MOD, MOT, COLSHAPE,
`.cam` and the cutscene clip texts that are still guessed.

Softimage pieces that would matter:

- The SDK and the Game Development Kit (C++ exporters).
- VBScript / JScript automation.
- dotXSI and the File Transfer Kit (geometry, hierarchies, animation).
- Animation Mixer clips.
- Custom properties and UserData (per-object data, collision perhaps).
- The scene graph (hierarchies, transforms, constraints).

Sources to read:

- dotXSI Converter Tool Kit: https://sourceforge.net/projects/dotxsictk/files/
- Softimage C++ plug-ins: https://github.com/ReinhardC/softimage
- UserData API: https://download.autodesk.com/global/docs/softimage2012/en_us/sdkguide/si_om/UserDataMap.html

That Capcom used Softimage is history. That DMC3's formats follow it is not
shown.

## What the data says so far (2026-10-24)

- No tool name, file path or exporter tag in the assets. 26 files (98 MB):
  cutscene archives m00-m07, stages st000-st006 with their configurations
  and effects. The few "XSI" / "MAYA" byte runs are inside texture and
  palette data (surrounded by binary values), not text.
- The cutscene texts come from an in-house tool. They are Shift-JIS with
  Japanese comments that document every field: `SetFrame`, `SkipFrame`,
  `CutFrame`, `ClipScale`, `Life`, `Model string`, `Texture string`, `Cloth
  string`, `HideKind (A=...)`. Cloth texts carry `SpringForce`, `Wind`,
  `WindType`, `MaxSpeed`, `ResetCltLevel` and per-bone `Bone N Y` lines.
- `.cam` tracks are cubic Hermite curves with an in and an out slope per key
  (CUTSCENES.md). Softimage and Maya both keep their curves this way, so this
  fits the hypothesis without proving it.
- The motions have a key on most frames, as captured motion does. That says
  how they were made, not with which program.

- The EXE (the HD Collection's PC `dmc3.exe`) has no tool string either:
  no Softimage, XSI, dotXSI, `.hrc`, `SI_`, Maya, 3ds Max or LightWave. It
  is the port's build (`C:\dev\dmc\dmc3\...`, MSVC, HLSL shaders), so the
  PS2 production tools could not be expected in it. Its 67 shader sources
  are read in dmc3-hd-shaders-exe.md.

## How to test it

1. **The EXE.** Done (above): the HD build has no trace of the tools. The
   PS2 executable (`SLUS_209.64` and its kin) is the next place to look;
   an original build is likelier to keep a path of the production tree.
2. **Curves.** Compare dotXSI's `SI_FCurve` Hermite keys (value, in and out
   tangents) with `.cam` kind 1 keys: the same slope convention (per frame,
   or tangent handles), the same channel order (`EyeX` ... `Fov`, the group
   offsets)?
3. **Hierarchies.** Softimage chains end in an effector. Do MOD skeletons
   have leaf joints of zero length at the end of every limb, where an
   effector would have been exported?
4. **Rotation order and units.** Softimage defaults to XYZ order in
   degrees, Y up. MOT and `.cam` rotations: which order and unit, which axis
   up?
5. **Collision.** If COLSHAPE came from custom properties on objects, the
   shapes would be named or numbered as the objects they sat on; compare
   the attack index with the joints the shapes follow.

Each answer is recorded here with the data it came from (ids, offsets and
layouts only, no game files).
