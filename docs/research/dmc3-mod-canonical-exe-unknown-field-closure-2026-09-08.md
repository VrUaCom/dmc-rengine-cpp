# DMC3 HD MOD — canonical EXE unknown-field closure pass (2026-09-08)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

Close the remaining MOD unknown-byte questions against the canonical executable directly. External Blender/importer behavior is treated only as secondary corroboration and is not semantic authority.

This pass focuses on:

- transform `+0x1C`;
- mesh `+0x0C`;
- mesh `+0x38`;
- mesh `+0x4C`;
- `BLENDINDICES.x`.

## 1. Transform `+0x1C`

Canonical MOD/EFM transform initializer: `0x1402FA080`.

For each serialized 0x20-byte transform record, the initializer performs two 16-byte loads:

```text
+0x00..+0x0F -> translation block
+0x10..+0x1F -> rotation block
```

It then passes the second block to rotation helper `0x140330450`.

The helper reads only:

```text
rotation block +0x00 -> X
rotation block +0x04 -> Y
rotation block +0x08 -> Z
```

and never reads rotation block `+0x0C`, which is serialized transform `+0x1C`.

Therefore `+0x1C` is present only because the serialized rotation block occupies a 16-byte lane. In the canonical MOD/EFM transform-construction path it is ignored.

Status:

```text
serialized position        STRUCTURAL_CONFIRMED
canonical transform use    EXE_CONFIRMED_UNUSED_IN_INITIALIZER
multi-corpus value          0.0f in 285/285 current MOD transforms
safe semantic              reserved fourth lane / preservation field
writer rule                preserve source byte pattern; do not synthesize meaning
```

This is stronger than the previous `PRESERVED_UNDECODED` state, but it still does not claim that no alternate engine path or future revision could ever consume the lane.

## 2. Mesh `+0x0C`

The common material helper `0x1402F9890` reads the serialized mesh material prefix as:

```text
+0x02 texture_slot
+0x04 MINU
+0x06 MAXU
+0x08 MINV
+0x0A MAXV
```

and then stops. It does not read `+0x0C`.

The primary post-load handlers likewise do not consume `+0x0C`:

```text
MOD  0x1402FE3B0
EFM  0x1402F7A90
SCM  0x1403051B0
```

The first stream pointer starts at aligned offset `+0x10`, so the physical layout is:

```text
0x00..0x0B  typed material/CLAMP prefix
0x0C..0x0F  inactive u32 lane
0x10..      aligned pointer/stream region
```

Across the current 180-MOD-mesh corpus, `+0x0C == 0` in 180/180 meshes.

Safe classification:

> reserved/alignment candidate u32 in the currently confirmed MOD ABI.

Do not remove it from the ABI and do not force-zero unknown future source bytes.

## 3. Mesh `+0x38`

`+0x38` is definitively **not generic padding** in the shared 0x50-byte model-family record.

### MOD

`0x1402FE3B0` relocates:

```text
+0x10 +0x18 +0x20 +0x28 +0x30
```

but deliberately does not relocate `+0x38`.

### EFM

`0x1402F7A90` additionally relocates:

```text
+0x38
```

as a live stream pointer. The bound retail EFM payload and embedded `DMC3_EFM*.hlsl` bind that stream to the EFM per-vertex `COLOR0` channel.

### SCM

`0x1403051B0` also relocates `+0x38`; its topology reconstruction reads byte `+3` from each 4-byte entry there, consistent with the known SCM colour/topology stream.

Therefore the strongest cross-family classification is:

```text
mesh +0x38 = format-specific auxiliary vertex-stream slot
```

For MOD specifically, the slot is dormant in every currently confirmed layout and zero in 180/180 meshes.

Do **not** rename MOD `+0x38` to `COLOR0`: EFM owns that semantic in its own adapter.

## 4. Mesh `+0x4C`

The canonical MOD handler writes generated topology count to `+0x48` but does not read or rewrite trailing dword `+0x4C`.

The runtime mesh builder `0x1402FE6A0` transfers the live workspace/count from `+0x40/+0x48` and does not transfer `+0x4C`.

The EFM and SCM post-load paths also leave the homologous trailing dword untouched in their recovered primary handlers.

Current MOD corpus:

```text
+0x4C == 0 in 180/180 meshes
```

Safe classification remains:

```text
trailing reserved/preservation dword candidate
```

A global xref census is still required before declaring it permanently unused across every engine path/revision.

## 5. `BLENDINDICES.x`

The canonical executable embeds all three named MOD HLSL source families:

```text
DMC3_MOD.hlsl
DMC3_MOD_SP.hlsl
DMC3_MOD_STX.hlsl
```

They declare:

```hlsl
uint4 matIndex : BLENDINDICES;
```

but the embedded source corpus uses `matIndex.y`, `matIndex.z`, and `matIndex.w` for the three skin influences. A direct canonical-executable string/source census finds:

```text
matIndex.x / vi.matIndex.x / matIndxX : 0 occurrences
```

while y/z/w usages are present repeatedly.

CPU evidence at `0x1402F3D0A..0x1402F3D0F` independently reads byte lane `+1` and divides by four to recover the model node/bone index. The MOD post-load special path also probes blend bytes `+1/+2`; it does not assign a lane-0 meaning.

Current multi-family MOD corpus:

```text
BLENDINDICES.x == 0 in 20,976 / 20,976 vertices
```

Safe classification:

```text
lane x = strong reserved/compatibility-lane candidate for the known MOD shader ABI
```

This is now based on the canonical EXE plus corpus, not on Blender-tool behavior. We still preserve lane x and do not hard-code a zero writer rule until the CPU-side blend-stream consumer census is globally closed.

## 6. Updated unresolved-field table

| Field | Current status | Strongest current interpretation |
|---|---|---|
| transform `+0x1C` | EXE + multi-corpus constrained | ignored fourth float of 16-byte rotation block in canonical initializer |
| mesh `+0x0C` | EXE-negative + multi-corpus zero | reserved/alignment candidate before aligned pointer block |
| mesh `+0x38` | cross-family EXE-confirmed role | format-specific auxiliary vertex-stream slot; dormant in MOD |
| mesh `+0x4C` | EXE-negative + multi-corpus zero | trailing reserved/preservation dword candidate |
| `BLENDINDICES.x` | shader + CPU + multi-corpus constrained | unused/reserved compatibility lane candidate in known MOD skin ABI |

## 7. What remains genuinely open

The unknown-byte problem is now much narrower. Remaining closure work:

1. global CPU xref census for all reads from the four-byte MOD blend stream to rule out a non-shader lane-0 use;
2. global raw-mesh xref census for `+0x4C` outside primary post-load/build paths;
3. header `+0x14 -> manager +0xE4` downstream consumer identification;
4. object-flag `0x00100000` / `0x00200000` final render/material effect;
5. broaden retail MOD corpus further to seek non-zero counterexamples.

External importers remain useful only as corroboration. Canonical executable and retail corpus evidence remain authoritative.
