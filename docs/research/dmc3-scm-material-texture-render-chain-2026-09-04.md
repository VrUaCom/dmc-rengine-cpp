# DMC3 HD SCM material / texture / render chain — 2026-09-04

**Status:** EXE_CONFIRMED runtime ownership, alpha control and legacy GS CLAMP projection; one adjacent descriptor field remains semantically open.  
**Canonical target:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Executive result

This pass closes the bridge between serialized SCM geometry and the DMC3 renderer:

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

  +0x04 MINU
  +0x06 MAXU
  +0x08 MINV
  +0x0A MAXV
     -> legacy PS2 GS CLAMP register
        WMS = REGION_REPEAT (3)
        WMT = REGION_REPEAT (3)

SCM object flags (+0x10)
  bit 0x00004000
     -> mesh descriptor field +0x08 = 0 instead of 0x60
```

The remaining caution is the higher-level name of descriptor value `0x60`: its executable behavior is exact, but a downstream semantic decoder has not yet been closed.

## 1. SCM texture companion is a separate runtime resource

`0x140304AE0` binds two independent resource pointers into the SCM manager:

```text
manager +0x108 = SCM resource
manager +0x110 = texture companion
```

`0x1402F9570` copies SCM header/runtime data but obtains the runtime texture count from the companion:

```text
SCM +0x10 -> manager +0xE8  object count
SCM +0x11 -> manager +0xEA  scene-node count
companion +0x00 (u16 view) -> manager +0xEC texture count
SCM +0x13 -> manager +0xFA
SCM +0x14 -> manager +0xE4
```

This path does **not** use SCM header `+0x12` as the runtime texture-table authority. On the confirmed corpus SCM `+0x12` agrees with the companion table size, so it is retained as a serialized mirror/consistency field. Authoring must keep both resources coherent.

## 2. Texture-companion envelope

The companion builder beginning at `0x140304B30` exposes:

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

Texture payload parser `0x1403365B0` directly checks:

```text
*(u32*)payload == 0x00324D54   // "TM2\0"
```

Additional/fallback paths exist, so not every possible companion slot is asserted to be TM2.

## 3. `mesh+0x02` is the external texture-companion slot

`0x1402F9890` performs:

```text
textureIndex = *(u16*)(serializedMesh + 0x02)
textureRecord = runtimeTextureTable + textureIndex * 0x40
runtimeMeshDescriptor.texture = *(textureRecord + 0x20)
```

Thus `mesh+0x02` is an EXE-confirmed slot index into the runtime table derived from the external texture companion.

## 4. Mesh `+0x04..+0x0B` are legacy PS2 GS CLAMP REGION_REPEAT fields

Canonical EXE consumer `0x1402F9890` reads:

```text
+0x04 u16 MINU
+0x06 u16 MAXU
+0x08 u16 MINV
+0x0A u16 MAXV
```

When `MINU == 0`, DMC3 uses a disabled/sentinel convention and emits packed state `0`.

Otherwise it builds:

```text
packed =
    (u64(MINU) << 4)  |
    0x0F              |
    (u64(MAXU) << 14) |
    (u64(MINV) << 24) |
    (u64(MAXV) << 34)
```

Independent PS2 GS references define the CLAMP register as:

```text
WMS  bits 0..1
WMT  bits 2..3
MINU bits 4..13
MAXU bits 14..23
MINV bits 24..33
MAXV bits 34..43
```

and define mode value `3` as `REGION_REPEAT`. Therefore the literal low nibble `0x0F` is exactly:

```text
WMS = 3 = REGION_REPEAT
WMT = 3 = REGION_REPEAT
```

The serialized fields are consequently a legacy GS region-repeat texture-wrapping state. In REGION_REPEAT terminology MIN fields serve mask semantics and MAX fields fix semantics, while the IR keeps the original GS register field names to avoid hiding the binary mapping.

All 481 meshes in the current HD corpus contain zero in these four words. The feature is nevertheless live in the canonical executable and must be preserved for non-stock/legacy-compatible resources.

Clean C++20 representation:

```text
scm::LegacyGsClampRegionRepeat
scm::pack_legacy_gs_clamp_region_repeat()
scm::legacy_gs_clamp_fields_fit_register()
```

The original executable shifts the full serialized `u16` values without masking. The clean parser therefore preserves values above the 10-bit GS hardware width and emits a warning instead of silently truncating them.

`mesh+0x0C..+0x0F` remains separately unresolved/zero on the current corpus.

## 5. Serialized object flag `0x00004000` has a direct render consumer

The caller of `0x1402F9890` passes runtime object `+0x14`, copied verbatim from serialized SCM object `+0x10` by `0x140302F10`.

`0x1402F9890` performs:

```text
if (objectFlags & 0x00004000)
    meshDescriptor.field_08 = 0
else
    meshDescriptor.field_08 = 0x60
```

This is distinct from the unrelated manager family-mask field at `manager+0xE0`.

The exact higher-level meaning of descriptor value `0x60` remains open. The API exposes only:

```text
scm::mesh_descriptor_field_08(objectFlags)
```

## 6. `object+0x01` is an alpha-control byte

`0x140302F10` copies serialized object `+0x01` to runtime object `+0x07`.

After narrow hard-coded corrections, the common path is:

```text
if effectiveControl <= 0x80:
    runtime +0x178 = 0
else:
    runtime +0x178 = effectiveControl

runtime +0x17C = effectiveControl
```

Packet construction at `0x140304111..0x140304167` computes:

```text
if runtime +0x178 > 0:
    MDL_PARTS_COLOR_PKT.alpha.w = 1.0
else:
    MDL_PARTS_COLOR_PKT.alpha.w = (runtime +0x17C) * (1/255)
```

A non-zero low nibble in serialized object flags forces effective control `0x80`, yielding approximately `0.5019608`.

Therefore `object+0x01` is an **alpha-control byte**, not a plain opacity byte:

- `0x00..0x80` -> `alpha.w = value/255`;
- values `>0x80` -> runtime control codes, `alpha.w = 1.0`, code retained separately;
- non-zero low object mode -> forced control `0x80`.

Clean IR/API:

```text
Object::alpha_control
scm::project_effective_alpha_control()
```

## 7. Hard-coded C4 / EA corrections

Two narrow executable corrections are preserved as target-specific evidence rather than generalized format rules.

### EA correction

Source control `0xEA`, object index `10`, and current plus next three object `totalVertexCount` values:

```text
0x0074, 0x003B, 0x0045, 0x00AD
```

cause rewrite to `0xC5`.

### C4 correction

Source control `0xC4`, object index `16`, runtime scene-node binding `17`, and current plus next three `totalVertexCount` values:

```text
0x074E, 0x0004, 0x000C, 0x0134
```

cause rewrite to `0x80`.

## 8. Shader ABI closes `alpha.w`

The canonical executable embeds:

```hlsl
struct MDL_PARTS_COLOR_PKT
{
    float4 base;
    float4 specular;
    float4 alpha;
};
```

The packet is exactly `0x30` bytes. Embedded `DMC3_STG.hlsl` uses the same packet and explicitly writes:

```hlsl
vo.oColor.w = colors.alpha.w;
```

Generic DMC3 texture/color pixel-shader variants combine `COLOR0` alpha with sampled texture alpha; several variants feed final `color.a` into `AlphaTestFunc(...)` and `clip(...)`.

Thus the stage-geometry CPU-side `alpha.w` lane is vertex-output alpha and can affect final pixel alpha / alpha testing depending on draw variant.

## 9. CDrawSCM packet path

The canonical executable contains RTTI for `CDrawSCM`. Its draw path reaches `0x1402FD040`, which works with arrays at strict `0x30` stride and forwards data into SCM draw submission `0x1402FC850`. This independently ties the recovered color packet ABI to the SCM subsystem.

## 10. Implementation changes on branch `scm`

Implemented:

- `Mesh::gs_clamp_region_repeat` for `+0x04..+0x0B`;
- `Mesh::reserved0c` for still-unresolved `+0x0C..+0x0F`;
- `Object::alpha_control`;
- exact legacy GS CLAMP REGION_REPEAT packer;
- non-destructive 10-bit GS field validation;
- exact `0x4000 -> descriptor field 0/0x60` projection;
- alpha-control projection;
- parser regression accepting/preserving non-zero clamp fields;
- alpha, GS clamp, descriptor, hierarchy and inverse-transform regression coverage.

## Remaining material/render targets

1. Decode downstream meaning of mesh descriptor field `0x60` / `0`.
2. Map exact SCM draw variants to pixel-shader selection.
3. Close remaining observed serialized object flag `0x00200000` through a direct source-flag consumer.
4. Integrate cross-resource texture-companion ownership into canonical writer/rebuild policy.
5. Determine whether any non-stock/legacy corpus exists with non-zero GS region-repeat fields and validate their producer semantics.
