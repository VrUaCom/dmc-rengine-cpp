# DMC3 / PlayStation 2 graphics hardware limit baseline — 2026-10-04

## Purpose

This document establishes the **PlayStation 2 hardware domain** that DMC Rengine must keep separate from:

- DMC3 serialized-format limits;
- DMC3 HD Collection x64/D3D11 runtime limits;
- retail HD asset observations;
- DMC Rengine product-safety limits.

It does **not** claim a DMC3-PS2 engine-specific maximum unless the original PS2 executable/microprogram path has been recovered.

## Source status

The baseline below is drawn from PS2 hardware documentation/community reverse references and current emulator implementations that reproduce the GS/VU register model.

External references:

- PS2 Developer Wiki, Memory Map: https://www.psdevwiki.com/ps2/Memory_Map
- PS2 Developer Wiki, Graphics Synthesizer: https://www.psdevwiki.com/ps2/Graphics_Synthesizer
- ps2tek, PS2 internals / GS TEX0: https://psi-rockin.github.io/ps2tek/
- PCSX2 GSState implementation, TEX0 dimension handling: https://github.com/PCSX2/pcsx2/blob/master/pcsx2/GS/GSState.cpp
- SCEA GDC2000 PS2 Graphics Architecture white paper mirror: https://lukasz.dk/mirror/research-scea/research/pdfs/GDC2000-DOM-white-paper.pdf

Classification in this file:
- PS2_HARDWARE_CONFIRMED: hardware/register/memory fact supported by the references above;
- HARDWARE_THROUGHPUT_SPEC: peak/theoretical throughput, not a DMC3 content limit;
- DMC3_PS2_OPEN: requires original DMC3 PS2 executable/assets/microcode.

## Memory hierarchy relevant to rendering

Retail PS2:

- EE main RDRAM: **32 MiB**.
- EE scratchpad: **16 KiB**.
- VU1 instruction memory: **16 KiB**.
- VU1 data memory: **16 KiB = 1024 qwords**.
- GS embedded local video memory: **4 MiB eDRAM**.

Status: PS2_HARDWARE_CONFIRMED.

The GS 4 MiB is shared by the live graphics allocations required by the game: frame/depth buffers, textures, CLUTs and other local-memory surfaces. Therefore "4 MiB texture memory" is not a usable per-texture budget in a normal rendered scene.

## GS sampled-texture dimensions

GS TEX0 encodes:

- TBP0: 14 bits;
- TBW: 6 bits;
- PSM: 6 bits;
- TW: 4 bits;
- TH: 4 bits;
- remaining texture/CLUT state in the rest of the 64-bit register.

For TW/TH the hardware/spec domain is:

```text
texture width  = min(2^TW, 1024)
texture height = min(2^TH, 1024)
```

PCSX2 independently documents the same distinction: the bitfield can encode values beyond 10, but the GS specification maximum is 10, corresponding to **1024 x 1024**.

Therefore the PS2 GS sampled-texture dimension ceiling is:

```text
1024 x 1024
```

Status: PS2_HARDWARE_CONFIRMED.

This is a **dimension ceiling**, not a statement that a 1024x1024 texture with arbitrary format/mips is practical in a DMC3 scene.

## Why 1024x1024 can still be impractical

Ignoring GS page/alignment overhead, a single 1024x1024 base image contains:

- 32-bit: 4,194,304 bytes = **4 MiB**;
- 16-bit: 2,097,152 bytes = **2 MiB**;
- 8-bit indexed: 1,048,576 bytes = **1 MiB**, plus CLUT;
- 4-bit indexed: 524,288 bytes = **0.5 MiB**, plus CLUT.

A 32-bit 1024x1024 base level alone equals the entire 4 MiB GS local-memory capacity before framebuffer, depth buffer, mip levels or other resources. Thus the register dimension maximum is much higher than the practical normal-scene allocation for many formats.

These byte counts are arithmetic base-image sizes only; they do not model GS swizzle/page alignment or full mip allocation.

## Geometry is streamed, not resident as one giant mesh

VU1 has only 16 KiB of data memory. Geometry pipelines therefore split geometry into batches that are transferred through VIF/DMA, processed by VU1 and kicked to GS/GIF. Double buffering lets one batch be uploaded while another is processed.

Consequences:

1. There is no useful PS2 hardware constant equivalent to "maximum polygons in a level".
2. A very large scene can contain far more geometry in main memory/storage than fits VU1 at once.
3. The important hard unit is the **batch**, whose exact vertex count depends on the VU1 program, vertex stride, constants, clipping output and double-buffer partition.
4. The frame/scene ceiling is mainly throughput, bandwidth, culling, memory residency and frame-time rather than a single polygon-count register.

Status: PS2_HARDWARE_CONFIRMED architectural model.

## Polygon throughput numbers are not engine limits

The GS is documented with headline rates up to roughly **75 million small polygons per second** in the simplest conditions. Texturing, Z/alpha, larger screen coverage, multipass work, VU transforms, clipping, CPU/game logic and memory transfers reduce achievable game throughput.

Rengine policy:

> Never store a PS2 peak polygon-rate figure as "DMC3 maximum polygons".

Classification: HARDWARE_THROUGHPUT_SPEC only.

## DMC3 PS2-specific limit remains open

The current canonical reverse target in DMC Rengine is the HD Collection x64 executable:

`dmc3.exe`
SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

That executable preserves substantial legacy GS/VIF/GIF compatibility state but ultimately renders through D3D11. It cannot, by itself, prove the original PS2 build's:

- VU1 microprogram memory partition;
- vertices per geometry batch;
- original texture residency policy;
- stage geometry streaming/culling budget;
- DMA/VIF packet batching limits chosen by Capcom;
- frame-time ceiling on retail PS2 hardware.

A search of the currently connected project Drive did not locate an original DMC3 PS2 SLUS/SLES executable artifact. Therefore these values remain DMC3_PS2_OPEN rather than inferred from HD.

## Cross-generation comparison

| Domain | Texture dimension | Geometry unit |
|---|---:|---|
| PS2 GS hardware | 1024x1024 sampled texture | VU1/DMA streamed batches; no single scene polygon-count constant |
| DMC3 HD retail observed (bounded corpus) | at least 1024x2048 DDS | largest observed SCM mesh 10,196 vertices |
| DMC3 HD canonical loader | 16384x16384 Texture2D guard | SCM R16 dynamic index path, 159,744 indices/reset buffer |
| DMC Rengine current authoring safety | 1024 per DDS axis | product policy, not engine ABI |

The 1024x2048 HD retail DDS is direct evidence that HD resources must **not** be projected backward onto PS2 GS capability.

## What is required to close DMC3-PS2 itself

Acquire a hash-bound original DMC3 PS2 executable/ELF and its matching resource set, then:

1. identify DMC3's VIF1/VU1 microprogram uploads;
2. recover VU1 data-memory partition/high-water marks;
3. recover input/output vertex strides per pipeline;
4. derive exact vertices/triangles per VU1 batch;
5. trace DMA chain construction and batch splitting;
6. census original PS2 texture dimensions/formats/mips;
7. recover GS local-memory allocation/residency policy;
8. measure scene/frame throughput on original hardware or cycle-faithful instrumentation.

Until then, Rengine must expose **PS2 hardware limits** and **DMC3 HD limits** separately.
