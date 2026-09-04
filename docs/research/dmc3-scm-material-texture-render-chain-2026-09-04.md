# DMC3 HD SCM material / texture / render chain — 2026-09-04

**Status:** EXE_CONFIRMED runtime ownership and bounded render projection; semantic decoding of one descriptor field remains open.  
**Canonical target:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Executive result

This pass closes the previously missing bridge between serialized SCM geometry and the DMC3 renderer:

```text
SCM object
  alpha_control (+0x01)
     -> runtime control/override
     -> MDL_PARTS_COLOR_PKT.alpha.w
     -> DMC3_STG vertex COLOR0.a

SCM mesh
  texture_index (+0x02)
     -> external texture-companion slot
     -> runtime texture record (stride 0x40)
     -> texture pointer/descriptor

  render_words (+0x04,+0x06,+0x08,+0x0A)
     -> packed 64-bit render state

SCM object flags (+0x10)
  bit 0x00004000
     -> mesh descriptor field +0x08 = 0 instead of 0x60
```

The remaining caution is semantic naming of the packed mesh render words and descriptor value `0x60`: the executable behavior is exact, but their higher-level blend/cull/depth/sampler meanings are not yet proven.

## 1. SCM texture companion is a separate runtime resource

`0x140304AE0` binds two independent resource pointers into the SCM manager:

```text
manager +0x108 = SCM resource
manager +0x110 = texture companion
```

The texture companion is therefore not an embedded region of SCM.

`0x1402F9570` copies SCM header/runtime data but obtains the runtime texture count from the companion:

```text
SCM +0x10 -> manager +0xE8  object count
SCM +0x11 -> manager +0xEA  scene-node count
companion +0x00 (u16 view) -> manager +0xEC texture count
SCM +0x13 -> manager +0xFA
SCM +0x14 -> manager +0xE4
```

Notably, this path does **not** use SCM header `+0x12` as the runtime texture-table authority. On the confirmed corpus SCM `+0x12` agrees with the companion table size, so it remains a useful serialized consistency value, but authoring must keep both resources coherent.

## 2. Texture-companion envelope

The companion builder beginning at `0x140304B30` exposes the physical envelope:

```text
+0x000 u32 textureCount
+0x004 u32 blockCount[textureCount]
...
+0x800 payload[0]
       size = blockCount[0] * 0x800
       payload[1]
       size = blockCount[1] * 0x800
       ...
```

The loop advances each payload pointer by exactly:

```text
blockCount[i] * 0x800
```

and advances the block-count table by four bytes per slot.

Texture payload parser `0x1403365B0` has a direct check:

```text
*(u32*)payload == 0x00324D54   // "TM2\0"
```

There are additional/fallback paths, therefore this evidence does not justify the stronger claim that every possible companion slot must be TM2.

## 3. `mesh+0x02` is the external texture-companion slot

`0x1402F9890` closes the mesh texture mapping directly:

```text
textureIndex = *(u16*)(serializedMesh + 0x02)
textureRecord = runtimeTextureTable + textureIndex * 0x40
runtimeMeshDescriptor.texture = *(textureRecord + 0x20)
```

Thus `mesh+0x02` is no longer merely a candidate material index. It is an EXE-confirmed index into the runtime table derived from the external texture companion.

The current corpus independently agrees with this ownership model: large outer stage groups show SCM `textureSlotCount` equal to the number of extracted textures in the associated texture folder/table, while SCM resources nested under PNST can reuse the outer resource group's companion rather than owning a separate texture folder.

## 4. Mesh `+0x04..+0x0B` are not reserved

A previous corpus-only interpretation labeled `mesh+0x04..+0x0B` reserved because all 481 confirmed stock SCM meshes use zero there. Canonical EXE consumer `0x1402F9890` disproves that label.

The four words are read as:

```text
+0x04 u16 renderWord0
+0x06 u16 renderWord1
+0x08 u16 renderWord2
+0x0A u16 renderWord3
```

If `renderWord0 == 0`, the derived runtime field is zero. Otherwise the executable builds:

```text
packed =
    (u64(renderWord0) << 4)  |
    0x0F                     |
    (u64(renderWord1) << 14) |
    (u64(renderWord2) << 24) |
    (u64(renderWord3) << 34)
```

The clean C++20 reconstruction is:

```text
scm::MeshRenderWords
scm::pack_mesh_render_words()
```

The names deliberately remain operational. No blend/shader/GS-state vocabulary is assigned until the downstream decoder is closed.

`mesh+0x0C..+0x0F` remains separately unresolved/zero on the current corpus.

## 5. Serialized object flag `0x00004000` has a direct render consumer

The caller of `0x1402F9890` passes runtime object `+0x14`, which is copied verbatim from serialized SCM object `+0x10` by `0x140302F10`.

`0x1402F9890` then performs:

```text
if (objectFlags & 0x00004000)
    meshDescriptor.field_08 = 0
else
    meshDescriptor.field_08 = 0x60
```

This is a direct render-path consumer of the serialized object flags and is distinct from the unrelated manager family-mask field at `manager+0xE0`.

The exact higher-level meaning of descriptor value `0x60` is still open. The current API therefore exposes only the neutral reconstruction:

```text
scm::mesh_descriptor_field_08(objectFlags)
```

## 6. `object+0x01` is an alpha-control byte

`0x140302F10` copies serialized object `+0x01` to runtime object `+0x07`.

For the common path, after two narrow hard-coded corrections, the effective byte is copied to runtime `+0x17C` and classified:

```text
if effectiveControl <= 0x80:
    runtime +0x178 = 0
else:
    runtime +0x178 = effectiveControl
```

Packet construction at `0x140304111..0x140304167` then computes:

```text
if runtime +0x178 > 0:
    MDL_PARTS_COLOR_PKT.alpha.w = 1.0
else:
    MDL_PARTS_COLOR_PKT.alpha.w = (runtime +0x17C) * (1/255)
```

The exact constant at `0x14035D558` is the single-precision representation of `1/255`.

A non-zero low nibble in serialized object flags bypasses the normal correction path and forces effective alpha control `0x80`, yielding approximately `0.5019608`.

Therefore `object+0x01` is not safely modeled as a plain opacity byte. It is an **alpha-control byte**:

- common values `0x00..0x80` map to `alpha.w = value / 255`;
- values `>0x80` are runtime control codes and force `alpha.w = 1.0` while the code is retained separately;
- non-zero low object mode forces control `0x80`.

The clean IR field is now:

```text
Object::alpha_control
```

and the post-correction projection is reconstructed by:

```text
scm::project_effective_alpha_control()
```

## 7. Hard-coded C4 / EA corrections

The executable contains two narrow title-specific corrections before the generic alpha projection. They should be preserved as executable evidence, not generalized into format rules.

### EA correction

If the source control is `0xEA`, current object index is `10`, and the current plus next three object records have `totalVertexCount` values:

```text
0x0074, 0x003B, 0x0045, 0x00AD
```

the runtime control byte is rewritten to `0xC5`.

### C4 correction

If the source control is `0xC4`, current object index is `16`, the runtime scene-node binding is `17`, and current plus next three object records have `totalVertexCount` values:

```text
0x074E, 0x0004, 0x000C, 0x0134
```

the runtime control byte is rewritten to `0x80`.

These signatures inspect the SCM object table (`object + 0x02` at successive 0x40-byte records), not mesh texture indices.

## 8. Shader ABI closes `alpha.w`

The canonical executable embeds the HLSL definition:

```hlsl
struct MDL_PARTS_COLOR_PKT
{
    float4 base;
    float4 specular;
    float4 alpha;
};
```

The packet is exactly `0x30` bytes, matching the per-pass runtime color-packet stride.

The embedded DMC3 stage vertex shader source (`DMC3_STG.hlsl`) declares:

```hlsl
MDL_PARTS_COLOR_PKT colors;
```

and explicitly writes:

```hlsl
vo.oColor.w = colors.alpha.w;
```

The fog/stage shader family uses the same packet ABI. Generic DMC3 texture/color pixel shaders accept `COLOR0` and combine its alpha with sampled texture alpha; several variants feed the resulting `color.a` through `AlphaTestFunc(...)` and `clip(...)`.

This closes the stage-geometry meaning of the CPU-side `alpha.w` lane: it is vertex-output alpha and can participate in final pixel alpha / alpha testing depending on the selected pixel-shader variant.

## 9. CDrawSCM packet path

The canonical executable contains RTTI for `CDrawSCM` and its draw path reaches `0x1402FD040`.

`0x1402FD040` works with arrays at a strict `0x30` stride, copies 16-byte lanes from those packets and forwards them into the SCM draw submission (`0x1402FC850`). This independently matches `MDL_PARTS_COLOR_PKT` size and ties the recovered color-packet ABI to the SCM draw subsystem rather than relying only on similarly named MOD/EFM shader sources.

## 10. Implementation changes on branch `scm`

Implemented in this pass:

- `Mesh::render_words` replaces the incorrect reserved interpretation of `+0x04..+0x0B`;
- `Mesh::reserved0c` preserves the still-unresolved `+0x0C..+0x0F` lane;
- `Object::alpha_control` replaces `unresolved01`;
- `scm::pack_mesh_render_words()`;
- `scm::mesh_descriptor_field_08()` for exact `0x4000` behavior;
- `scm::project_effective_alpha_control()`;
- parser regression accepting/preserving non-zero render words;
- alpha-control, descriptor and inverse-transform regression coverage.

## Remaining material/render targets

1. Decode the downstream meaning of mesh descriptor field `0x60` / `0`.
2. Decode semantic meaning/ranges of the four packed mesh render words from a non-zero producer or downstream consumer.
3. Map exact SCM draw variants to pixel-shader selection rather than only the shared stage/color packet ABI.
4. Close the remaining observed serialized object flag `0x00200000` through a direct source-flag consumer.
5. Integrate texture-companion ownership into the canonical writer/rebuild plan so SCM and companion counts/slots cannot drift.
