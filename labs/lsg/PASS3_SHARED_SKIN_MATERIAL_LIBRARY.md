# Pass 3 — Shared Skin Material Library

Status: **IMPLEMENTED / CI_REQUIRED**

## Goal

Make skin a shared Human DNA material system rather than a character-owned shader or texture package.

## Architecture

`SkinGenome + MicroDetailGenome + PhysiologyGenome + runtime PhysiologyState`
→ `SkinPhenotype`
→ `SkinMaterialGpuV0`
→ shared GLSL material library
→ Detail-on-Demand evaluation.

The current packed GPU state is 80 bytes and contains:

- pigments: melanin, haemoglobin, carotene, age;
- surface: oiliness, hydration, roughness bias, coat strength;
- pores: density, scale, depth, follicle density;
- features: freckle density, meso strength, micro strength, wrinkle bias;
- physiology: perfusion, sweat, normalized temperature, subsurface strength.

## Shared shader modules

- `skin_material.glsl`: material UBO, base reflectance, roughness, coat/specular response.
- `skin_microstructure.glsl`: freckles, follicles and wrinkle microgeometry.
- `skin_transport.glsl`: bounded real-time subsurface/terminator transport.

These modules are character-agnostic. Male Base, Female Base, Ada and future profiles use the same shader family. A future specialized high-quality transport model may be added to the library, but it must remain a reusable material family rather than character-specific code.

## Detail-on-Demand

The existing pixel-footprint scheduler controls cost:

- MACRO: pigment/material response only;
- MESO: vascular/meso variation and freckles;
- MICRO: pores and follicles;
- MICRO_HIGH: subpixel microdetail and wrinkles.

No generated pore/freckle/follicle/wrinkle texture is stored on disk.

## Seam rule

Procedural phase is anchored only to deterministic object-space cells and surface seed. Semantic `BodyRegion` labels may influence future continuous phenotype fields, but may not alter noise phase. CPU reference sampling now follows the same rule as GLSL.

## Physiology

Runtime physiology and genome physiology biases are combined into the same phenotype. No shader variant is selected for exercise, cold, hot, Ada, sex, or carrier.

## Acceptance

- all SkinGenome and MicroDetailGenome channels are consumed by the shared phenotype/material path;
- `SkinMaterialGpuV0` remains 80 bytes and std140-compatible;
- one material UBO is updated for the active profile; no per-profile material buffer is required;
- Male/Female/Ada use the same shader modules;
- semantic region changes do not phase-shift procedural detail;
- macro detail band evaluates no generated microdetail;
- generated skin texture storage remains 0 bytes;
- profile-specific carrier duplicate bytes remain 0;
- Android + Windows shader/build/test/storage gates PASS.

## Deferred

True GPU timestamp benchmarking, multi-lobe/high-order diffusion, hair scattering and physical-device shimmer/visual acceptance remain separate evidence gates. They must not be marked PASS from CI alone.
