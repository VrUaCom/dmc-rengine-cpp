# Урок 4 — Skinning, topology і shader ABI

## Skin stream

На вершину MOD має:

- `BLENDINDICES`: 4 bytes;
- packed control/weights: 1 × `u16`.

## Blend indices

Serialized stream — це повний `u8x4` ABI:

```text
lane X
lane Y
lane Z
lane W
```

Canonical CPU consumer `0x1402F3D0A..0x1402F3D0F` читає `BLENDINDICES.y` і ділить на 4:

```text
bone_index = raw_blend_index / 4
```

Причина: raw index адресує початок 4-row matrix у matrix array.

Shader skin path використовує `matIndex.y/z/w` для трьох influences.

### BLENDINDICES.x — новий closure

Multi-corpus census:

```text
20,976 / 20,976 vertices -> lane X == 0
```

Але головний доказ тепер не corpus zero, а canonical executable + compiled shader ABI.

Serialized mesh `+0x28` переходить:

```text
serialized +0x28
 -> runtime mesh +0x140
 -> draw/input descriptor +0x30
 -> BLENDINDICES uint4 input
```

Whole-image DXBC census знайшов 8 input signatures із `BLENDINDICES`:

```text
register        3
Mask            0xF
ReadWriteMask   0xE
component type  uint32
```

`ReadWriteMask = 0xE` означає, що compiled shaders читають Y/Z/W і **не читають X**. Це узгоджується з embedded HLSL source census для `DMC3_MOD`, `DMC3_MOD_SP`, `DMC3_MOD_STX` family.

Provenance-confirmed CPU census також не знайшов direct lane-X consumer; unrelated scaled-four lane-0 candidates були відхилені після pointer-provenance reconstruction.

Отже canonical status:

```text
serialized u8x4 ABI                    STRUCTURAL_CONFIRMED
serialized -> runtime/input chain      EXE_CONFIRMED
compiled shader X consumption          EXE_CONFIRMED: not read
proven direct CPU X consumption        EXE_CONFIRMED: none found
bounded corpus X == 0                  CORPUS_CONFIRMED
semantic label                         PRESERVED_UNDECODED
writer policy                          preserve raw X exactly
```

Критично: **“runtime does not read X” ≠ “X is padding”**. Reader має зберігати byte; semantic skin decoder ігнорує X; writer не має права force-zero non-zero source data.

## Packed weights

Lower 15 bits = 3 × 5-bit weight:

```text
q0 = (packed >>  0) & 0x1F
q1 = (packed >>  5) & 0x1F
q2 = (packed >> 10) & 0x1F

w0 = q0 / 31
w1 = q1 / 31
w2 = q2 / 31
```

High bit:

```text
0x8000 = topology/control break bit
```

Це доведено двома незалежними шляхами:

- CPU post-load consumes/clears high bit для topology generation;
- runtime-selected MOD vertex shader використовує lower 15 bits для weights.

## Canonical skin decoder guards

Decoder повинен перевіряти:

- active index кратний matrix-row stride 4;
- `bone_index < node_count`;
- немає duplicate active bone;
- quantized sum відповідає очікуваному contract.

Невалідні bytes не треба «виправляти».

## DMC3_MOD.hlsl

Vertex shader registry:

```text
descriptor table base = 0x1405612D0
count = 29
stride = 0x20
initializer = 0x140045D00
lookup = 0x1400461B0
```

Renderer запитує tag 5:

```text
0x140047409 -> tag 5
0x140047424 -> lookup
```

Tag 5:

```text
descriptor = 0x140561370
DXBC       = 0x14048A8B0
size       = 0x8D84
source     = DMC3_MOD.hlsl
```

Input signature містить:

`POSITION`, `NORMAL`, `TEXCOORD`, `BLENDINDICES`, `PSIZE`.

Adjacent descriptors:
- tag 6: `DMC3_MOD_SP.hlsl`;
- tag 7: `DMC3_MOD_STX.hlsl`.

## Runtime post-load topology

Helper `0x1402FE3B0`:

1. читає serialized control words;
2. detects `0x8000`;
3. clears bit in-place;
4. generates u16 topology command words;
5. writes them у workspace, на який вказує mesh `+0x40`;
6. записує generated count у `+0x48`.

`+0x40` є **mesh-record-relative**, не resource-base-relative.

## Важлива межа

Post-load-mutated MOD memory — це runtime representation. Вона не є canonical serialized MOD і не може стати input writer-а через простий memory dump.
