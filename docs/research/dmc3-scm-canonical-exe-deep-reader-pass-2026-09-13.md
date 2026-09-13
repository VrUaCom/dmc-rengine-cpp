# DMC3 HD SCM — canonical EXE deep-reader pass 2026-09-13

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Machine receipt:** `data/reverse/dmc3-scm-canonical-exe-deep-reader-20260913.json`

## Scope

This pass uses the actual canonical executable together with freshly supplied retail stage resources. The goal is deeper **read/runtime provenance**, not writer promotion.

The proof standard is:

```text
serialized SCM byte
  -> canonical executable read/copy
  -> typed runtime owner
  -> downstream consumer/effect
```

Numeric offset or immediate equality without pointer provenance is rejected.

## Fresh retail material

### st001

```text
st001.scm
size    887760
sha256  3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
objects 44
meshes  77
nodes   50
textures 17
header+0x13 = 0
header+0x14 = 300100 -> family=3 model_set=1 sub_index=0
```

Objects `8`, `39` and `40` carry source flag `0x00200000`, giving direct retail positives for the bit-21 investigation.

### st002

```text
st002.scm
size    860928
sha256  670ded6de6bd57ecc75118eaf89e9918dbb4b92ce271c96526b9e758b5ba7f44
objects 66
meshes  107
nodes   67
textures 14
header+0x13 = 0
header+0x14 = 813800 -> family=8 model_set=138 sub_index=0
```

This is a real corpus extension. The previous observed-shape assumption limiting SCM `LegacyResourceCode.family_class` to `3/4` is stale. The arithmetic decomposition still holds, but class `8` is now observed and must not trigger an unconfirmed-shape warning.

### st003

```text
st003.scm
size    670816
sha256  d573034fc4fd5496e3ffd1d84830f748cc70af2dbcc4d442d8af8694ac410be5
objects 13
meshes  45
nodes   14
textures 17
header+0x13 = 0
header+0x14 = 300300 -> family=3 model_set=3 sub_index=0
```

## 1. Header `+0x13` — lighting reference node index

Canonical header initialization proves:

```text
0x1402F960E  read u8 SCM header+0x13
0x1402F9616  write zero-extended value to manager+0xFA
```

### Scene-node world-matrix ownership

`0x1402F1DB0` installs the runtime scene-node world-matrix array:

```text
manager+0x188 = matrixArrayBase
matrix stride = 0x40
matrix count  = manager+0xEA scene-node count
```

`0x1402F9700` independently proves the index contract by updating:

```text
manager+0x188 + nodeIndex * 0x40
```

through the recovered hierarchy/world-composition path.

### CDrawSCM selection

At `0x1402FD040`:

```text
0x1402FD0A3  read manager+0xFA
0x1402FD0AA  selector *= 0x40
0x1402FD0B5  load manager+0x188
0x1402FD0C2  select matrix lane +0x30
```

The selected world-space `float4` is passed to `0x1402EE560` as the spatial query input.

### Lighting constant-buffer closure

The chain continues beyond generic draw-state selection:

- `0x1402EE560` produces four records with `0x30` stride from the spatial query;
- `CDrawSCM` gathers/transposes record lane `+0x00` into one 4x4 matrix and lane `+0x10` into a second 4x4 matrix;
- `0x1402FC850` writes the resulting two matrices into the next `0x80` constant-buffer block beginning at `+0x280`;
- `0x1402FAB70` independently establishes the preceding `MDL_GLOBAL_MAT` block at `+0x200/+0x240`;
- the embedded DMC3 shader ABI declares the following `0x80` block as:

```hlsl
struct MDL_LIGHT_MAT
{
    matrix Lc;
    matrix Lv;
};
```

The stage vertex shader consumes `light.Lv` to derive saturated normal-dependent light intensity and consumes `light.Lc` to derive light color before multiplying by material/base color.

Therefore the evidence-safe technical semantic is now:

```text
header +0x13 = lighting_reference_node_index
```

This is `EXE_CONFIRMED_TECHNICAL_SEMANTIC`. It does not claim recovery of Capcom's original source symbol or an artistic authoring label such as "light bone". Current supplied and previously bounded retail specimens serialize zero, therefore they select scene node zero.

## 2. Header `+0x14` observed domain expands

The runtime carry remains:

```text
SCM+0x14 -> 0x1402F95C2 -> manager+0xE4
```

A direct model-core scan still does not recover a type-proven downstream read of this specific manager field. Raw `[reg+0xE4]` matches on unrelated layouts are not promoted.

Fresh st002 provides:

```text
813800 = 8 * 100000 + 138 * 100 + 0
```

Therefore the decimal structural decomposition remains valid on the new sample, family class `8` is now observed, the previous `3/4`-only observed-shape predicate is obsolete, and the high-level runtime meaning of `manager+0xE4` remains unproven.

## 3. Source object flag `0x00200000` — expanded whole-image negative census

`st001` provides three real source-positive objects: `8`, `39`, `40`.

Fresh whole-image bit-21 search recovered two candidates absent from the previous bounded report.

### Rejected `0x1402F2CDD`

This function walks records with stride `0x380` and performs a bit-21 operation on runtime record `+0x304`. SCM objects materialized by `0x140302F10` use stride `0x3C0`, so record-layout provenance rejects this as an SCM source-object bit-21 consumer.

### Rejected `0x140212B08`

This site clears bit 21 on unrelated large actor/global state around `+0x3FF8`; there is no SCM manager/object provenance edge.

Together with previously rejected `0x1402F4C21`, `0x140303F2F` and manager bit setters `0x140302CF9/0x140302D59`, the correct status remains:

```text
PRESERVED_UNDECODED_WITH_EXPANDED_NEGATIVE_EVIDENCE
```

The bit is real runtime-carried state and must be preserved exactly, but no gameplay/render semantic name is justified yet.

## 4. Mesh preservation lanes — deep indirect-consumer closure

Target serialized lanes:

```text
mesh+0x0C u32
mesh+0x30 u64
mesh+0x4C u32
```

The fresh canonical pass follows the runtime serialized-mesh backreference substantially farther than the original census.

### `0x1402F9BB0`

Primary SCM runtime-mesh materialization installs:

```text
runtimeMesh+0x10 = serializedMesh
runtime mesh stride = 0x1A0
```

and directly reads serialized `+0x00/+0x02/+0x10/+0x18/+0x20/+0x28/+0x38`, but not the three target lanes.

### `0x140308C00`

Post-init initializes runtime-only `+0xF0/+0xF4/+0xF8` and never follows `runtimeMesh+0x10` back to serialized data.

### `0x1402F9890`

The common material helper does follow the backreference and consumes texture slot `+0x02` plus GS CLAMP `+0x04..+0x0A`; it skips `+0x0C/+0x30/+0x4C`.

### `0x1402F9A80`

Common follow-up works on generated runtime descriptor state without dereferencing the serialized backreference.

### `0x1402F9F20`

SCM-specific allocation composes `0x1402F9BB0 -> 0x1402F9890 -> 0x1402F9A80` and adds no extra read of the preservation lanes.

The result is therefore stronger than the original primary-function census:

```text
PRESERVED_UNDECODED_WITH_DEEP_NEGATIVE_EVIDENCE
```

A runtime mesh field at numeric `+0x4C` exists elsewhere but is produced from runtime object state. Numeric offset equality is explicitly rejected as evidence for serialized `mesh+0x4C`.

## 5. Helper-mode dispatch

The source-flag helper `0x140302640` selects numeric modes. `0x1402F17C0` resolves them as lookup-table indices:

```text
pointer = table_0x1405D0550[mode]
*destination = pointer
```

Modes such as `1/2/4/9` are therefore proven table-selector IDs. This does not justify artistic/material names and exposes no hidden `0x00200000` decode.

## 6. Texture companion — real DDS fallback confirmed

The canonical texture parser around `0x1403365B0` contains a direct `TM2\0` branch and a non-TM2 fallback.

All freshly supplied PTX companions exercise the alternate descriptor+DDS framing:

```text
st001.ptx: 17 slots, 17/17 DDS@+0x70, 0 TM2-at-start
st002.ptx: 14 slots, 14/14 DDS@+0x70, 0 TM2-at-start
st003.ptx: 17 slots, 17/17 DDS@+0x70, 0 TM2-at-start
```

Each PTX count equals the paired SCM serialized texture-slot mirror count exactly. Thus TM2 is not mandatory for SCM texture companion slots; descriptor + DDS is a live retail ownership path.

## Current deeper frontier

The remaining high-value reverse targets are now narrower:

1. recover Capcom's original producer/content-authoring label for `lighting_reference_node_index` or locate a retail SCM where it is non-zero;
2. recover a type-proven downstream role for `manager+0xE4` / SCM `+0x14`;
3. recover a semantic consumer or producer for object flag `0x00200000`;
4. locate a non-zero canonical producer/specimen for mesh `+0x0C/+0x30/+0x4C`, otherwise retain the strengthened preservation classification;
5. complete the exact CDrawSCM branch/resource/shader-variant selection map beyond the now-closed lighting constant ABI.

The structural SCM reverse remains intact. This pass materially deepens the **reader/runtime semantic reverse** and corrects stale assumptions for both header `+0x13` and the observed `+0x14` resource-code domain.
