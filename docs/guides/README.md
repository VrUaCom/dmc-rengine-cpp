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

## Search problems covered

The guide system deliberately targets practical language people use before they know DMC Rengine or the underlying binary format names: `DMC3 unpacker`, `Devil May Cry 3 extractor`, `DMC3 NBZ extractor`, `DMC3 PAC extractor`, `DMC3 PNST`, `DMC3 model viewer`, `DMC3 texture extractor`, `Dante model`, `Dante textures`, `DMC3 stage models`, `DMC3 animations`, `open DMC3 MOD`, and `open DMC3 SCM`.

Each problem page links deeper into the canonical technical layer instead of creating several near-duplicate pages for spelling variants of the same search intent.

## Ecosystem routing

- **DMC Rengine** — canonical C++20 parsers, reverse evidence, resource architecture and guarded authoring/rebuild research.
- **DMC Native Reader** — user-facing reader/viewer product direction; platform and feature claims must follow the current build state rather than roadmap intent.
- **GDSpaces / PocketGDS** — archive/resource navigation, nested materialization and provenance-oriented browsing.

The projects are presented as one ecosystem with distinct roles, not as duplicate all-in-one tools.

## Capability language

`unpack`, `extract`, `browse`, `open`, `view`, `inspect`, `convert`, `edit`, `replace`, `repack` and `rebuild` are different capabilities. A guide must never imply that recognition or parsing proves authoring/reintegration support.

For exact current maturity, always check [`../status/current.md`](../status/current.md).
