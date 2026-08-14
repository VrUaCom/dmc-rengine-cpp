# DMC3 Recovered Resource Runtime

This area belongs to the **Recovered Game Source Tree**. It represents reconstructed DMC3 game-runtime evidence and stays separate from `dmc_rengine_core`.

The compiled Wave-2 model in `resource_lifecycle.*` preserves the first recovered `363 x 0x48` load-pool ABI and its `0 -> 1 -> 2 -> 3` lifecycle.

Wave 3 extends the recovered runtime through `../wave3_runtime.hpp` with additional directly evidenced architecture:

- corrected Stage cell ABI: `u16 kind16 @ +0x00`, path pointer `@ +0x08`, stride `0x10`;
- two Stage descriptor banks: 110 + 79 = 189 descriptors;
- 193-entry selector space + 10 group-base pointers + numeric Stage resolver;
- 13 confirmed `st600..st612` cross-stage effect/sound aliases;
- `.lst` recursive manifest fallback with `dummy`, nested lists and `.pac` rewrite;
- owner-local 32-node resource cache keyed by `(group,index)` with observed refcount at `+0x2C`;
- seven resource domains: stage script/config/effect, demo/event, localized message, enemy object and enemy sound;
- player and enemy factory evidence, including the 64-entry external enemy mapping boundary;
- root/nested scene-manager evidence and ten-scene factory map;
- demo asset type registry (`MOT/MCV/CAM/HID/CLT/TSC`) and `CMotion` ABI anchors;
- D3D11 graphics-object pointer offsets and the `2 x 450` `gfxTexture` pool;
- HD ADX->OGG lookup/loop metadata, SFD->WMV translation, legacy `VAGp` sample path and FMOD codec export;
- 4,039-entry master resource-name catalog and numeric semantic resource resolver;
- central memory-arena and registry-lookup anchors.

Wave 3 is still an **evidence model**, not a claim that the whole game is reconstructed. It deliberately does not invent unknown Stage `kind16` semantics, full `.lst` error/lifetime behavior, complete enemy AI/combat logic, post-load algorithms, factory implementations, or final shutdown/transition ownership.

Authority:
- `docs/research/dmc3-vanilla-deep-research-wave-2.md`
- `docs/research/dmc3-vanilla-deep-research-wave-3.md`
- canonical executable SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
