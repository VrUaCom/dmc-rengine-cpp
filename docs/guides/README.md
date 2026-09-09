# DMC3 HD Guides — DMC Rengine

Practical, evidence-bounded entry points for people trying to unpack, extract, browse, inspect or understand Devil May Cry 3: Special Edition resources from the HD Collection.

These guides implement the DMC Rengine discovery funnel:

```text
Problem -> Guide -> Tool / Capability -> Technical Research -> Brand / Ecosystem
```

Canonical technical truth remains in the format/evidence documentation and `docs/status/current.md`.

## First wave

- [How to Unpack Devil May Cry 3 HD Collection Files](unpack-dmc3-hd.md)
- [How to Extract Files from DMC3 NBZ Archives](extract-nbz.md)
- [How to Extract Models from Devil May Cry 3 HD](extract-models.md)
- [How to Extract Textures from Devil May Cry 3 HD](extract-textures.md)
- [How to Open and Inspect DMC3 MOD Files](inspect-mod.md)
- [How to Open and Inspect DMC3 SCM Files](inspect-scm.md)

## Second wave

- [How to Extract DMC3 PAC Files](extract-pac.md)
- [How to Browse DMC3 PNST Containers](browse-pnst.md)
- [How to Find Dante Models and Textures in DMC3 HD](find-dante-model-textures.md)
- [How to Extract DMC3 Stage Models](extract-stage-models.md)
- [How to Inspect DMC3 Animations and MOT Research](inspect-animations.md)

## Third wave

- [How to Find DMC3 Character Models](character-models.md)
- [How to Find and Inspect DMC3 Enemy Models](enemy-models.md)
- [DMC Native Reader for Android — DMC3 Resource Inspection](native-reader-android.md)

The Android guide is intentionally product-state aware: current `DMC-Native-Reader/main` promotes MOD, SCM, DDS and PTX. It does not advertise Windows, Web or iOS as shipped Native Reader platforms without implementation/build evidence.

## Fourth wave

- [How to Find Vergil Models and Textures in DMC3 HD](find-vergil-model-textures.md)
- [How to Find DMC3 Weapon Models](find-weapon-models.md)
- [DMC3 Blender Import and Community Tools](dmc3-blender-import.md)

The Blender guide is deliberately ecosystem-aware. `deshayu/DMC3HDC-Import-Tools` and `HansLichtner/DMC3-Blender-Import-Addon` are independent community projects. DMC Rengine can provide extraction, provenance and canonical format evidence without claiming that those Blender import/export capabilities belong to DMC Rengine itself.

## Search problems covered

The guide system deliberately targets practical language people use before they know DMC Rengine or the underlying binary format names: `DMC3 unpacker`, `Devil May Cry 3 extractor`, `DMC3 NBZ extractor`, `DMC3 PAC extractor`, `DMC3 PNST`, `DMC3 model viewer`, `DMC3 texture extractor`, `Dante model`, `Dante textures`, `Vergil model`, `Vergil textures`, `DMC3 character models`, `DMC3 enemy models`, `DMC3 Yamato model`, `DMC3 Rebellion model`, `DMC3 Beowulf model`, `DMC3 weapon models`, `DMC3 stage models`, `DMC3 animations`, `DMC3 Blender import`, `DMC3HDC Import Tools`, `DMC3 MOD Blender`, `DMC3 SCM Blender`, `DMC3 MOT Blender`, `DMC3 Android viewer`, `DMC3 model viewer Android`, `DMC3 texture viewer Android`, `open DMC3 MOD`, and `open DMC3 SCM`.

Each problem page links deeper into the canonical technical layer instead of creating several near-duplicate pages for spelling variants of the same search intent.

## Ecosystem routing

- **DMC Rengine** — canonical C++20 parsers, reverse evidence, resource architecture and guarded authoring/rebuild research.
- **DMC Native Reader** — current Android-facing reader/viewer application. Product/platform claims follow the app's promoted `main` surface rather than roadmap intent.
- **DMC Rengine Native Reader integration registry** — a broader C++ integration surface than the current Android app; registry presence must not be misrepresented as an already-promoted application feature.
- **GDSpaces / PocketGDS** — archive/resource navigation, nested materialization and provenance-oriented browsing.
- **Community Blender tools** — independent DCC workflows that may consume DMC3 resources; their supported formats and Blender behavior are governed by their own repositories rather than DMC Rengine claims.

The projects are presented as one ecosystem with distinct roles, not as duplicate all-in-one tools.

## Capability language

`unpack`, `extract`, `browse`, `open`, `view`, `inspect`, `import`, `export`, `convert`, `edit`, `replace`, `repack` and `rebuild` are different capabilities. A guide must never imply that recognition or parsing proves authoring/reintegration support, or that compatibility with an external Blender addon makes DCC import/export a native DMC Rengine feature.

For exact current maturity, always check [`../status/current.md`](../status/current.md).
