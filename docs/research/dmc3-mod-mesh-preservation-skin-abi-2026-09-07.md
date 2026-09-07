# DMC3 HD MOD mesh preservation and skin ABI — 2026-09-07

## Scope

This pass closes two previously separate evidence boundaries:

1. serialized MOD mesh bytes that must be preserved even though the confirmed runtime load path does not consume them; and
2. the exact MOD skin-index/weight ABI across serialized streams, CPU runtime consumers, and the runtime-selected DMC3 MOD vertex shader.

Canonical executable:

- `dmc3.exe`
- SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

No writer or edited-game acceptance authority is claimed by this pass.

## 1. Serialized mesh preservation boundary

The recovered MOD inner-mesh record is `0x50` bytes.

The canonical parser already materializes the live stream/state fields and preserves the whole source image in `Document::source_bytes`. Three serialized fields remain semantically unresolved:

| Field | Width | Current safe status |
|---|---:|---|
| `+0x0C` | u32 | serialized-preserved, runtime-unconsumed in confirmed load path |
| `+0x38` | u64 | serialized-preserved, runtime-unconsumed in confirmed load path |
| `+0x4C` | u32 | serialized-preserved, runtime-unconsumed in confirmed load path |

`+0x38` and `+0x4C` were already surfaced by the structural parser. `+0x0C` was only recoverable indirectly through the retained source image, leaving a typed preservation gap for future authoring work.

This pass adds a dedicated preservation view for all three fields without inventing semantics.

### 1.1 Canonical post-load evidence

MOD post-load `0x1402FE3B0` relocates/consumes the confirmed runtime-live mesh fields:

- `+0x10` positions;
- `+0x18` normals;
- `+0x20` UV;
- `+0x28` blend indices;
- `+0x30` packed weight/topology stream;
- record-relative `+0x40` generated-topology workspace;
- generated count at `+0x48` is produced by post-load.

It does **not** relocate or rewrite `+0x0C`, `+0x38`, or `+0x4C`.

The same post-load consumes bit `0x8000` from each `+0x30` word as topology state and clears that bit in place before the remaining 15-bit payload continues into runtime use.

### 1.2 Runtime mesh construction evidence

Runtime mesh builder `0x1402FE6A0` transfers the serialized streams/state into its runtime mesh block. The confirmed mappings include:

```text
serialized +0x10 -> runtime positions
serialized +0x18 -> runtime normals
serialized +0x20 -> runtime UV
serialized +0x28 -> runtime blend-index stream
serialized +0x30 -> runtime packed/control stream
serialized +0x40 -> runtime topology workspace
serialized +0x48 -> runtime generated count
```

There is no transfer of `+0x0C`, `+0x38`, or `+0x4C` in this canonical construction path.

This is strong negative runtime evidence, but it is deliberately phrased as **unconsumed in the confirmed path**, not “globally unused”. A future writer must preserve these bytes until broader evidence proves a stronger invariant.

## 2. Blend-index ABI — direct CPU proof

The serialized mesh `+0x28` stream becomes runtime mesh blend-index storage.

At `0x1402F3CE6..0x1402F3D12`, the canonical runtime:

1. fetches the runtime blend-index stream;
2. reads byte lane `[vertex * 4 + 1]` at `0x1402F3D0A`;
3. shifts it right by two at `0x1402F3D0F`;
4. uses the result as a node index.

The resulting index is then used with:

- runtime node stride `0xA0` at `0x1402F3E59..0x1402F3E73`;
- current-world matrix stride `0x40` at `0x1402F3E76..0x1402F3E8E`.

Therefore the serialized blend-index lane is a float4-row offset:

```text
node_or_bone_index = raw_blend_index / 4
```

This upgrades the existing `matrix_row_stride = 4` decoder contract to direct `EXE_CONFIRMED` runtime evidence.

## 3. MOD vertex shader registry — runtime linkage

The canonical executable contains a vertex-shader descriptor table:

```text
base   = 0x1405612D0
count  = 29
stride = 0x20
```

Initializer `0x140045D00` iterates all 29 descriptors. For each entry it passes the descriptor bytecode pointer and bytecode size to the graphics-device vertex-shader creation path, with the created shader object stored in descriptor output storage.

Lookup helper `0x1400461B0` scans the table by descriptor tag.

Renderer setup beginning at `0x1400473F0` executes:

```text
0x140047409  edx = 5
0x140047424  call 0x1400461B0
```

Tag `5` resolves to:

```text
descriptor       = 0x140561370
DXBC pointer     = 0x14048A8B0
DXBC size        = 0x8D84
descriptor tag   = 5
input count      = 5
shader path      = DMC3_MOD.hlsl
```

The same descriptor is then used while creating the associated renderer input-layout/pipeline state.

Two adjacent canonical MOD variants are also present:

- tag 6: `DMC3_MOD_SP.hlsl`, DXBC `0x140493640`, size `0x96B0`;
- tag 7: `DMC3_MOD_STX.hlsl`.

This establishes that the MOD shader source/debug payload is attached to a runtime-created and runtime-selected shader descriptor rather than being an unrelated embedded text artifact.

## 4. DXBC/SPDB evidence

The tag-5 MOD shader is a valid DXBC container:

```text
raw file offset  = 0x489AB0
VA               = 0x14048A8B0
size             = 0x8D84
chunk count      = 6
chunks           = RDEF, ISGN, OSGN, SHEX, STAT, SPDB
```

The input-signature chunk explicitly contains:

```text
POSITION
NORMAL
TEXCOORD
BLENDINDICES
PSIZE
```

The SPDB chunk contains the canonical source path:

```text
C:\dev\dmc\dmc3\code\dmc3\source\dmc3nb\shaders\hlsl\vs\DMC3_MOD.hlsl
```

and the HLSL source used for the recovered skinning math.

## 5. Exact weight packing

The runtime-linked `DMC3_MOD.hlsl` takes:

```hlsl
uint4 matIndex : BLENDINDICES;
float flags : PSIZE;
```

and reconstructs three weights through repeated division/fraction extraction by 32 and scaling by `32/31`.

This is mathematically equivalent to:

```text
q0 = (packed >>  0) & 0x1F
q1 = (packed >>  5) & 0x1F
q2 = (packed >> 10) & 0x1F

w0 = q0 / 31
w1 = q1 / 31
w2 = q2 / 31
```

The shader builds the first four-row bone matrix from `matIndex.y`, then, when remaining weights exist, blends rows from matrices beginning at `matIndex.z` and `matIndex.w`.

This directly matches the canonical C++ decoder's mapping:

```text
weights 0..2 <-> blend_indices[1..3]
raw blend index / 4 <-> node/bone matrix index
quantized weight denominator = 31
```

## 6. Topology bit is independent from the 15-bit weight payload

Serialized `+0x30` uses high bit `0x8000` as topology-break state in the CPU post-load path. `0x1402FE3B0` consumes it and clears it in place.

The shader weight reconstruction uses only the three lower 5-bit groups. Therefore:

```text
bit 15        = topology/control state
bits 0..14    = q0/q1/q2 packed skin weights
```

CPU and runtime shader evidence independently converge on the same separation.

## 7. Canonical implementation

Added:

- `include/dmc_rengine/analysis/mod/mesh_serialized.hpp`
- `src/analysis/mod/mesh_serialized.cpp`

The API exposes:

- `MeshSerializedPreservationAbi`;
- `MeshSerializedPreservation`;
- `decode_mesh_serialized_preservation()`;
- `read_mesh_serialized_preservation()`.

The compile-time regression uses non-zero sentinels for all three unresolved fields and verifies truncation fails closed.

`include/dmc_rengine/formats/mod_skin.hpp` keeps the existing decoder behavior but now documents its direct CPU and runtime-shader authority.

## 8. Evidence status after this pass

- mesh `+0x0C`: `PRESERVED_UNDECODED`, confirmed unconsumed in canonical post-load/runtime-mesh construction path;
- mesh `+0x38`: same;
- mesh `+0x4C`: same;
- blend-index `/4` node/bone mapping: `EXE_CONFIRMED`;
- three 5-bit weight fields: `EXE_CONFIRMED`;
- denominator `/31`: `EXE_CONFIRMED`;
- weights mapped to BLENDINDICES y/z/w: `EXE_CONFIRMED`;
- topology bit `0x8000` separate from weight payload: `EXE_CONFIRMED`;
- `blend_indices[0]` exact semantic purpose: still `PRESERVED_UNDECODED` beyond the observed/reserved corpus behavior.

## Explicit non-claims

This pass does not establish:

- semantic names for mesh `+0x0C/+0x38/+0x4C`;
- that those three fields are globally unused outside every possible executable path;
- MOD writer authority;
- safe edited values for the preserved fields;
- original-game acceptance of edited skinning data;
- complete animation/current-pose ownership;
- byte-identical no-edit rebuild.
