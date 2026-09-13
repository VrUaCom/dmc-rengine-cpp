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

## 1. Header `+0x13` is no longer an undecoded carry-only byte

Canonical header initialization proves:

```text
0x1402F960E  read u8 SCM header+0x13
0x1402F9616  write zero-extended value to manager+0xFA (u16)
```

The old state stopped here and therefore classified the source byte as runtime-carried but semantically undecoded.

The supplied canonical executable closes a deeper consumer chain.

### Scene-node matrix ownership

`0x1402F1DB0` resolves the four scene arrays and also installs the scene-node matrix-array base:

```text
manager+0x188 = matrixArrayBase
matrix stride = 0x40
matrix count  = manager+0xEA scene-node count
```

After reserving `nodeCount * 0x40`, the same setup proceeds to the next runtime scene-node region.

`0x1402F9700` independently confirms this ownership. For each scene node in evaluation order it computes:

```text
matrix = manager+0x188 + nodeIndex * 0x40
```

and updates the world matrix through the established hierarchy path.

### CDrawSCM consumer

At `0x1402FD040`:

```text
0x1402FD0A3  read manager+0xFA
0x1402FD0AA  selector *= 0x40
0x1402FD0B5  load manager+0x188 matrix-array base
0x1402FD0BC  add selector
0x1402FD0C2  add +0x30
0x1402FD0D6  copy selected float4
```

Thus serialized `header+0x13` is an **EXE-confirmed scene-node matrix index used by the CDrawSCM path**.

The selected matrix lane `+0x30` is passed as a spatial input to `0x1402EE560`, which performs downstream draw-state selection. The safe technical name is therefore:

```text
draw_reference_node_index
```

This does **not** justify artistic names such as camera/root/light bone. No producer or non-zero retail specimen currently proves such a label.

Current bounded retail samples serialize zero, so node zero is selected in these files.

## 2. Header `+0x14` observed domain expands

The runtime carry remains:

```text
SCM+0x14 -> 0x1402F95C2 -> manager+0xE4
```

A direct model-core scan still does not recover a type-proven downstream read of this specific manager field. Raw `[reg+0xE4]` matches on unrelated layouts are not promoted.

The fresh semantic change is corpus-side:

```text
st002: 813800
       = 8 * 100000 + 138 * 100 + 0
```

Therefore:

- the decimal structural decomposition remains valid on this new sample;
- family class `8` is now observed;
- the previous 3/4-only observed-shape predicate is obsolete;
- high-level runtime meaning of `manager+0xE4` remains unproven.

## 3. Source object flag `0x00200000` — expanded whole-image negative census

`st001` provides three real source-positive objects.

Fresh whole-image bit-21 search recovered two candidates absent from the previous bounded report.

### Rejected `0x1402F2CDD`

The function walks records with stride:

```text
0x380
```

and performs:

```text
BTS runtimeRecord+0x304, bit 21
```

SCM objects created by `0x140302F10` use runtime stride `0x3C0`. Pointer/record-layout provenance therefore rejects this as an SCM source-object bit-21 consumer.

### Rejected `0x140212B08`

This site performs `BTR bit21` on an unrelated large actor/global-state word around `+0x3FF8`. There is no SCM manager/object provenance edge.

Together with the already rejected `0x1402F4C21`, `0x140303F2F` and manager setters `0x140302CF9/0x140302D59`, the correct result remains:

```text
PRESERVED_UNDECODED_WITH_EXPANDED_NEGATIVE_EVIDENCE
```

The bit is real and runtime-preserved; it must not be called globally unused or be given a speculative editor toggle.

## 4. Mesh preservation lanes — deeper indirect-consumer closure

The target lanes are:

```text
mesh+0x0C u32
mesh+0x30 u64
mesh+0x4C u32
```

The new canonical pass follows the runtime backreference beyond the primary materializer.

### `0x1402F9BB0` primary SCM runtime-mesh materialization

It installs:

```text
runtimeMesh+0x10 = serializedMesh
runtime mesh stride = 0x1A0
```

and directly consumes serialized:

```text
+0x00 vertex count
+0x02 texture index
+0x10 positions
+0x18 normals
+0x20 UV
+0x28 continuation span
+0x38 RGB/topology stream
```

No read of `+0x0C/+0x30/+0x4C` occurs.

### `0x140308C00` post-init

This helper initializes runtime-only fields:

```text
runtimeMesh+0xF0
runtimeMesh+0xF4
runtimeMesh+0xF8
```

It never follows `runtimeMesh+0x10` back to the serialized mesh.

### `0x1402F9890` material helper

This function does follow the backreference and consumes:

```text
serialized +0x02        texture slot
serialized +0x04..0x0A legacy GS CLAMP fields
```

plus effective object flags for TEX1 filtering. It skips all three preservation lanes.

### `0x1402F9A80` follow-up state

The follow-up path works on generated runtime descriptor state and does not dereference the serialized backreference.

### `0x1402F9F20` SCM-specialized allocation

The SCM-specific path composes:

```text
0x1402F9BB0
 -> 0x1402F9890
 -> 0x1402F9A80
```

without an additional serialized read of `+0x0C/+0x30/+0x4C`.

This materially strengthens the preservation result. It is now a negative census across primary materialization, post-init, material construction, common follow-up and SCM-specific allocation, not merely one function.

A runtime field at `runtimeMesh+0x4C` does exist elsewhere, but it is written from runtime object state. Offset equality is explicitly rejected as evidence for serialized `mesh+0x4C`.

## 5. Helper-mode dispatch

The previously recovered source-flag helper `0x140302640` chooses numeric modes. The deeper target `0x1402F17C0` is now resolved:

```text
pointer = table_0x1405D0550[mode]
*destination = pointer
```

Therefore modes such as `1/2/4/9` are proven lookup-table selector IDs. This does not by itself give them artistic/material names, and there is no hidden bit-21 decode in this helper.

## 6. Texture companion — real DDS fallback confirmed

The direct executable texture parser at `0x1403365B0` has:

```text
TM2\\0 direct branch
else -> generic fallback parser
```

All freshly supplied PTX companions use the alternate framing:

```text
u32 count
u32 blockCount[count]
payload base 0x800
per-slot descriptor
DDS magic at slot +0x70
```

Observed:

```text
st001.ptx: 17 slots, 17/17 DDS@+0x70, 0 TM2-at-start
st002.ptx: 14 slots, 14/14 DDS@+0x70, 0 TM2-at-start
st003.ptx: 17 slots, 17/17 DDS@+0x70, 0 TM2-at-start
```

The PTX counts match their SCM serialized texture-slot mirror counts exactly.

This independently proves on real retail resources that TM2 is not mandatory for an SCM texture-companion slot and that descriptor + DDS fallback is a live ownership path.

## Current deeper frontier

The highest-value unresolved items are now narrower:

1. recover the producer/intended high-level label for `draw_reference_node_index`; the technical consumer is already closed;
2. recover a type-proven downstream role for `manager+0xE4` / SCM `+0x14`;
3. find a real semantic consumer or producer for object flag `0x00200000`;
4. find a non-zero specimen/producer for mesh `+0x0C/+0x30/+0x4C`, or retain their strengthened preservation result;
5. finish exact CDrawSCM draw-variant -> shader/resource selection mapping.

The structural SCM reverse remains intact. This pass improves **reader/runtime semantic depth** and corrects two stale assumptions (`+0x13` carry-only and `+0x14` class 3/4-only corpus shape).
