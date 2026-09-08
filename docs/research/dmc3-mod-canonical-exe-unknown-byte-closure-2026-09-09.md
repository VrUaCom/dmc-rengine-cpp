# DMC3 HD MOD — canonical EXE unknown-byte closure pass (2026-09-09)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Architecture:** PE32+ x86-64  
**Current bounded MOD corpus:** 38 unique MODs / 166 objects / 180 meshes / 20,976 vertices / 285 transforms

## Purpose

This pass goes beyond external Blender/importer behavior and returns to the canonical retail executable to close the highest-value remaining MOD bytes:

- transform `+0x1C`;
- mesh `+0x0C`;
- mesh `+0x38`;
- mesh `+0x4C`;
- `BLENDINDICES.x`;
- header `+0x14` / manager `+0xE4`.

No field is renamed from analogy alone. Negative evidence is scoped to the audited runtime path and all source bytes remain preservation obligations.

---

## 1. Canonical EXE availability is restored

The project library contains multiple 6,356,432-byte retail executable copies. Three independently materialized copies hash to the canonical value above. Direct disassembly is therefore available again for MOD completion work.

This supersedes the earlier operational assumption that only a tiny byte-record of the executable was currently available.

---

## 2. MOD post-load remains the serialized-field authority

### `0x1402FE3B0` — MOD post-load

For every 0x50 mesh record the function relocates the serialized source pointers:

```text
+0x10 positions
+0x18 normals
+0x20 UV
+0x28 BLENDINDICES
+0x30 packed weights/topology control
```

It does **not** relocate `+0x38`.

`+0x40` is resolved relative to the mesh record itself. The function then reconstructs generated u16 topology into that workspace, checks the observed `0x1212` prefill state, treats source control bit `0x8000` as a strip/topology break, clears that bit with `& 0x7FFF`, and writes the generated word count to `+0x48`.

This continues to validate the distinction:

```text
serialized MOD image != runtime-mutated MOD image
```

---

## 3. Mesh `+0x38` is solved at the stream-role level

This is the strongest new result of the pass.

### MOD runtime builder `0x1402FE6A0`

The MOD runtime mesh object has paired source-stream pointer slots:

```text
raw mesh +0x10 -> runtime +0x110
raw mesh +0x18 -> runtime +0x120
raw mesh +0x20 -> runtime +0x130
raw mesh +0x28 -> runtime +0x140
raw mesh +0x30 -> runtime +0x150
```

At the corresponding next stream position the function does **not** copy serialized mesh `+0x38`.

Instead it explicitly writes zero to both runtime auxiliary-stream pointers:

```text
0x1402FE8C4  runtime +0x168 = 0
0x1402FE8DD  runtime +0x160 = 0
```

It then forwards:

```text
mesh +0x40 -> runtime +0x190
mesh +0x48 -> runtime +0x198
```

Therefore in the canonical MOD runtime path the sixth vertex-stream slot is intentionally disabled.

### EFM positive control

The homologous EFM path proves that this is a real format-family stream position rather than meaningless padding.

`0x1402F7A90` directly relocates serialized EFM mesh `+0x38`:

```text
0x1402F7BBD read  qword [mesh+0x38]
             + file base
0x1402F7BCC write qword [mesh+0x38]
```

Then EFM runtime builder `0x1402F7D60` forwards it:

```text
0x1402F7F88 read  qword [mesh+0x38]
0x1402F7F8C write runtime +0x160/+slot
```

The bound real EFM payload `em000_023.efm` contains non-zero `+0x38` streams in both meshes (`0x06F0`, `0x2350`), and existing EFM shader evidence independently requires an extra per-vertex `COLOR0` input.

### Safe promotion

> Serialized model-family mesh `+0x38` is a **format-specific auxiliary vertex-stream slot**. In EFM it is the active per-vertex `COLOR0` stream. In canonical MOD the runtime builder explicitly disables the corresponding runtime stream and the current 180-mesh MOD corpus serializes `+0x38 = 0` everywhere.

Status:

```text
shared slot role                    EXE_AND_CORPUS_CONFIRMED
EFM +0x38 -> COLOR0 stream          EXE_AND_CORPUS_CONFIRMED
MOD runtime use of +0x38            EXE_CONFIRMED_INACTIVE
MOD corpus value                    MULTI_CORPUS_OBSERVED_ZERO (180/180)
"+0x38 is generic padding"          REJECTED
```

Writer rule: preserve the serialized value. Do not synthesize an EFM COLOR0 pointer into MOD merely because the slot is homologous.

---

## 4. Mesh `+0x0C` is now a strong alignment/reserved candidate

The 0x50 mesh begins:

```text
+0x00 u16 element_count
+0x02 u16 texture_slot
+0x04 u16 GS CLAMP MINU
+0x06 u16 GS CLAMP MAXU
+0x08 u16 GS CLAMP MINV
+0x0A u16 GS CLAMP MAXV
+0x0C u32 unresolved
+0x10 first qword stream pointer
```

The common material helper `0x1402F9890` consumes texture slot and the four GS CLAMP values through `+0x0A`; it does not consume `+0x0C`.

The audited format-specific load paths also provide no positive consumer:

```text
MOD post-load 0x1402FE3B0   no +0x0C use
EFM post-load 0x1402F7A90   no +0x0C use
SCM post-load 0x1403051B0   no corresponding +0x0C semantic promotion
```

Corpus:

```text
MOD: 180 / 180 meshes -> +0x0C == 0
EFM:   2 /   2 bound meshes -> +0x0C == 0
```

The dword exactly fills the gap between the last 16-bit GS field and the next 8-byte-aligned pointer.

Safe status:

```text
ALIGNMENT_OR_RESERVED_CANDIDATE
MULTI_CORPUS_OBSERVED_ZERO
PRESERVED_UNDECODED
```

This is intentionally **not** promoted to universal padding yet. A future writer still preserves source bytes.

---

## 5. Mesh `+0x4C` is a strong trailing reserve/alignment candidate

Layout tail:

```text
+0x40 qword generated topology workspace relative offset
+0x48 u32   generated topology word count (serialized zero, runtime-produced)
+0x4C u32   unresolved
+0x50       next record boundary
```

`+0x4C` is not consumed by the audited MOD/EFM post-load or runtime mesh builders.

Corpus:

```text
MOD: 180 / 180 meshes -> +0x4C == 0
EFM:   2 /   2 bound meshes -> +0x4C == 0
```

Safe status matches `+0x0C`:

```text
ALIGNMENT_OR_RESERVED_CANDIDATE
MULTI_CORPUS_OBSERVED_ZERO
PRESERVED_UNDECODED
```

It remains part of exact source preservation until a wider ABI/version census authorizes a stronger reserved declaration.

---

## 6. Transform `+0x1C` is inactive in canonical matrix construction

Serialized transform:

```text
+0x00 float translation.x
+0x04 float translation.y
+0x08 float translation.z
+0x0C float translation_magnitude
+0x10 float rotation.x
+0x14 float rotation.y
+0x18 float rotation.z
+0x1C float unresolved
```

### MOD/EFM initializer `0x1402FA080`

For every node, the initializer copies two full 16-byte blocks into scratch:

```text
serialized +0x00..+0x0F -> translation scratch
serialized +0x10..+0x1F -> rotation scratch
```

The 16-byte copy alone previously left open whether the fourth rotation float was meaningful.

### Rotation helper `0x140330450`

The called rotation helper reads exactly:

```text
0x14033045A  scratch +0x00  rotation X
0x14033046A  scratch +0x04  rotation Y
0x14033047A  scratch +0x08  rotation Z
```

It never reads scratch `+0x0C`, which corresponds to serialized transform `+0x1C`.

Therefore:

> Serialized transform `+0x1C` has **no effect on the canonical MOD/EFM local rotation-matrix construction path**.

This is direct executable evidence, not an inference from zero-valued corpus data.

### Positive control: translation `+0x0C` really is live

The same initializer passes the first 16-byte transform block to translation helper `0x140031200`.

That helper executes a full 16-byte load from the supplied vector and combines it with matrix row 3. Thus the fourth translation float (`translation_magnitude`) is a live part of the recovered helper contract, whereas the homologous fourth rotation float is not read by `0x140330450`.

Corpus:

```text
MOD: 285 / 285 transforms -> +0x1C == 0.0f
EFM:   5 /   5 bound transforms -> +0x1C == 0.0f
```

Safe status:

```text
EXE_CONFIRMED_UNCONSUMED_IN_LOCAL_MATRIX_BUILD
MULTI_CORPUS_OBSERVED_ZERO
PRESERVED_UNDECODED
```

We still do **not** claim globally unused, because another subsystem could theoretically inspect the serialized/node-domain source outside the matrix initializer.

---

## 7. `BLENDINDICES.x` is unused by every canonical MOD/EFM skin shader found

The canonical executable embeds source/evidence for these MOD vertex-shader families:

```text
DMC3_MOD.hlsl
DMC3_MOD_SP.hlsl
DMC3_MOD_STX.hlsl
```

and related EFM families:

```text
DMC3_EFM.hlsl
DMC3_EFM_SP.hlsl
DMC3_EFM_STX.hlsl
DMC3_EFM_VA.hlsl
DMC3_EFM_VA_SP.hlsl
```

A whole-executable source-string census returns:

```text
matIndex.x  0 occurrences
matIndxX    0 occurrences
matIndex.y  128 occurrences
matIndex.z   16 occurrences
matIndex.w   16 occurrences
```

The explicit temporary names likewise occur for Y/Z/W and never X.

The known skin code uses `y/z/w` as the three matrix-row starts. Independent CPU code at `0x1402F3D0A` reads byte lane `[1]` from the 4-byte blend-index stream and shifts it right by two, confirming the active node-index encoding.

MOD runtime builder forwards the whole 4-byte-per-vertex stream pointer to runtime `+0x140`; lane X physically exists but is not consumed by the canonical render-skin shaders.

Corpus:

```text
BLENDINDICES.x == 0 in 20,976 / 20,976 current MOD vertices
```

Safe promotion:

```text
BLENDINDICES.x render-skin use = EXE_CONFIRMED_UNUSED
BLENDINDICES.y/z/w              = EXE_CONFIRMED_ACTIVE_SKIN_LANES
lane X corpus                   = MULTI_CORPUS_OBSERVED_ZERO
lane X global CPU semantic      = OPEN / PRESERVED_UNDECODED
```

Therefore Blender importer behavior is no longer the authority for this conclusion; the canonical executable is.

A remaining whole-EXE CPU dataflow census is required before calling lane X globally reserved or authorizing a writer to force it to zero.

---

## 8. Header `+0x14` remains the stubborn field

### Exact transfer

Manager initializer `0x1402F9570` performs:

```text
0x1402F95C2  read  u32 [serialized header +0x14]
0x1402F95C5  write u32 [manager +0xE4]
```

The same function also transfers already-known neighboring authority:

```text
header +0x10 -> manager +0xE8
header +0x11 -> manager +0xEA
TM2 texture count -> manager +0xEC
header +0x13 -> manager +0xFA
```

### New negative consumer evidence

A direct displacement census over the core model subsystem address range:

```text
0x1402F9000 .. 0x14030D000
```

finds one manager `+0xE4` access in this manager path: the write at `0x1402F95C5`. No direct read of manager `+0xE4` was found in this bounded core model load/build/material range.

This means `+0x14` is **runtime-carried but not consumed by the core model construction path we just audited**.

It does not prove the field is unused globally; manager pointers escape into other subsystems and an external consumer may read the value.

The multi-corpus values remain decisive against the old over-strong decimal semantic assumption:

```text
em000 examples: 100407, 202900, 601715, 700601, ...
pl000 main MOD: 217
pl000 cloth-associated MOD: 217
id100 MOD: 1000000
```

Therefore:

```text
serialized u32 -> manager +0xE4    EXE_CONFIRMED
core model direct consumer         NOT FOUND in bounded scan
universal decimal resource code    REJECTED / NOT PROVEN
higher-level semantic name         PRESERVED_UNDECODED
```

Next closure requires type-aware whole-EXE tracing from the manager pointer rather than a naive global grep for displacement `+0xE4`, because many unrelated runtime structures also have fields at that offset.

---

## 9. Current unknown-field classification

| Field | Current strongest status | What is still open |
|---|---|---|
| transform `+0x1C` | EXE-confirmed unconsumed by local matrix build; 290/290 known MOD+EFM zero | any external/non-matrix consumer; historical name |
| mesh `+0x0C` | strong alignment/reserved candidate; MOD 180/180 + EFM 2/2 zero | global version/family proof |
| mesh `+0x38` | format-specific auxiliary vertex-stream slot; EFM COLOR0, MOD disabled | whether any MOD revision activates it |
| mesh `+0x4C` | strong trailing reserve/alignment candidate; MOD 180/180 + EFM 2/2 zero | global version/family proof |
| `BLENDINDICES.x` | unused by canonical MOD/EFM render skin shaders; 20,976/20,976 MOD zero | non-render CPU consumers |
| header `+0x14` | EXE-confirmed manager-carried u32; no direct core-model read found | external manager consumer / semantic identity |

---

## 10. What this changes for writer research

This pass materially reduces the unknown writer surface, but does not authorize a writer yet.

The preservation policy becomes more precise:

```text
transform +0x1C
    preserve source value
    canonical matrix build ignores it

mesh +0x0C
    preserve source value
    likely alignment/reserved, not yet universal padding

mesh +0x38
    preserve source value
    MOD runtime auxiliary stream disabled
    EFM meaning must never be copied into MOD blindly

mesh +0x4C
    preserve source value
    likely trailing reserve/alignment

BLENDINDICES.x
    preserve source byte
    render skin uses y/z/w only

header +0x14
    preserve exactly
    semantic still open
```

The only field in this group that remains a major semantic blocker rather than mostly a preservation/classification question is header `+0x14` / manager `+0xE4`.

---

## 11. Next direct-EXE work

Priority order:

1. type-aware manager `+0xE4` whole-EXE consumer tracing;
2. whole-EXE CPU dataflow census for BLENDINDICES lane 0;
3. global raw-MOD access census for `mesh +0x0C/+0x4C` outside known post-load paths;
4. look for additional model-family revisions/corpora that make any candidate field non-zero;
5. only after those gates, decide whether `+0x0C`, `+0x4C`, transform `+0x1C`, or lane X can be promoted from preservation fields to formal reserved fields.

## Promotion rule

No external tool is treated as semantic authority. External importers remain useful independent observations, but canonical executable dataflow + retail corpus evidence determines promotion.
