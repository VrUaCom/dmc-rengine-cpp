# DMC3 Vanilla Deep Research Wave 3

**Status:** CANONICAL RESEARCH ADDENDUM (pre-roadmap)  
**Artifact:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Purpose:** preserve the broad direct-EXE reverse wave that extends Wave 2 into scene/gameplay/resource ownership, media, animation, rendering and process/runtime architecture.

## Promoted findings

### Resource ownership and lifetime

Wave 3 confirms that the `363 x 0x48` global load pool is not the only lifetime layer.

An owner-local cache was recovered with:
- observed capacity 32 nodes;
- lookup key `(group,index)`;
- ready and pending lists;
- refcount field at node `+0x2C`;
- duplicate/reuse behavior through refcount increment;
- release path that decrements refcount and can return group-6 payloads to the global load manager.

The seven observed acquisition domains are:
0. stage script;
1. stage config;
2. stage effect;
3. demo/event;
4. localized message;
5. enemy object/model PAC;
6. enemy sound PAC.

### Stage dependency preload

Stage config is a dependency source, not merely passive configuration. The observed load flow is:

```text
stage cfg
-> READY
-> scan cfg records
-> extract enemy IDs
-> map enemy IDs to resource-set selectors
-> deduplicate
-> preload enemy object PAC + enemy sound PAC
-> schedule stage script + stage effect
-> wait pending dependencies
```

### Player and enemy factories

Player factory `0x1401DE820` exposes four concrete selectors:
- 0 `CPlDante`, allocation `0xB8C0`;
- 1 `CPlVergil`, allocation `0xB680`;
- 2 `CPlLady`, allocation `0x8280`;
- 3 `CPlNewVergil`, allocation `0xB8C0`.

Enemy factory `0x1401AC6D0` has 46 selector slots, with null slots `9,15,24,26,38,41,42,43`. Shared construction cases include `17..20 -> selector 17` and `29..31 -> selector 28`.

A separate 64-entry external enemy mapping boundary feeds both class-selection and resource-set selection. Examples of paired resource sets include:
- selector 12 -> `obj\em017.pac` + `se\snd_em17.pac`;
- selector 20 -> `obj\em029.pac` + `se\snd_em29.pac`;
- selector 27 -> `obj\em037.pac` + `se\snd_em37.pac`;
- selector 28 -> `obj\em000.pac` + `se\snd_em00b.pac`;
- selector 29 -> `obj\em000.pac` + `se\snd_emsr.pac`.

### Scene control flow

`CSceneFactoryApp` is a process-global object at `0x140C8F970`, size `0x14F0`, with a root scene manager embedded at `+0x1478`.

The ten-scene factory remains:
`Boot, Opening, StartMenu, MisSelect, Game, GameMain, Demo, MisStart, Result, Ending`.

Wave 3 adds direct transition behavior: exit current scene, destroy it, create the requested scene through the factory, enter it, then continue update-driven transition handling. Gameplay contains a nested gameplay scene manager rather than using only the root application router.

### Numeric resource authority

A master catalog begins at `0x140553050` and contains exactly 4,039 consecutive resource-name pointers before the null terminator:
- 3,398 PAC;
- 424 TXT;
- 154 ADX;
- 24 BIN;
- 19 SFD;
- remaining entries in smaller MOD/TM2/PTX/FON/etc. families.

Separate function `0x1402C07F0` implements numeric semantic resource lookup. A deterministic read-only scan of IDs `0..10000` produced 932 valid IDs grouped into 145 numeric ranges. This layer is separate from the master string catalog.

### `.lst` manifest layer

For the evidenced `kind16 == 0` path, missing primary resources can fall back to `.lst`.

The list path supports:
- `#` comments;
- `dummy`;
- nested `.lst`;
- ordinary entry rewrite to `.pac`;
- directory-relative resolution;
- recursive loading.

This is a manifest/packing indirection layer, not merely filename fallback.

### Animation and demo registry

Registry function `0x1402E01A0` uses:
- 0 MOT
- 1 MCV
- 2 CAM
- 3 HID
- 4 CLT
- 5 TSC

with capacity 1024 entries.

`CMotion` size is `0x220` with eight observed channels/tracks; `CEm029` embeds a `CMotion` object at `+0x4D10`.

### Rendering/effects

Graphics global `0x140C0B410` contains:
- `ID3D11Device* @ +0x08`;
- `ID3D11DeviceContext* @ +0x10`;
- `IDXGISwapChain* @ +0x18`.

A recovered effect/graphics pool contains `2 x 450 = 900` `gfxTexture` objects, each `0x60`. Effect decode paths index this pool as `bank * 450 + slot`.

### Audio/video

HD music/demo translation:
- 154-entry OGG descriptor table at `0x14055C610`, stride `0x10`;
- path pointer `+0x00`;
- loop-start ms `+0x08`;
- loop-end ms `+0x0C`;
- `0xFFFFFFFF` sentinel for the no-explicit-loop path;
- OGG basename lookup around `0x140031D80`;
- FMOD loop-point application around `0x140032A80`.

Legacy SFD paths are translated to WMV at runtime around `0x14032C10D`.

Audio is not OGG-only: sound PACs contain `VAGp` sample blocks decoded through the game's FMOD codec path; the EXE exports `FMODGetCodecDescription`.

Eleven observed FMOD channel groups:
`system, common, player-style, weapon1, weapon2, weapon3, weapon4, enemy, room, music, demo`.

### Process / memory

A central 256-MiB runtime memory arena is observed, with at least 64-MiB, 5-MiB and 4-MiB subordinate domains. Registry/type lookup anchor: `0x1402C6150`.

## Code integration boundary

Wave 3 is compiled into the **Recovered Game Source Tree** evidence target, not GDSpaces and not the DMC Rengine product core.

The model records proved ABI, mappings and control boundaries. It does not claim:
- full executable decompilation;
- final semantic name for `StageResourceCell.kind16`;
- full `.lst` grammar/error ownership;
- complete player combat or enemy AI;
- complete MOD/EFM/SCM/SHW algorithms;
- complete render pipeline;
- complete animation blending/IK;
- full scene/stage unload/shutdown graph.

Those remain reverse targets.

## Supersession

Where this document is stronger, it supersedes earlier assumptions that:
- the Stage universe is only the first 110-row bank;
- resource loading ends at byte/container materialization;
- cache/refcount behavior is wholly unknown;
- stage cfg is only passive configuration;
- ADX/OGG and SFD/WMV are merely parallel filename catalogs.
