# DMC3 HD dynamic geometry budget — direct canonical EXE pass

Date: 2026-10-04  
Branch: `reverse/dmc3-resource-render-limits-20261004`  
Canonical executable: `dmc3.exe`  
SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Scope

This pass isolates concrete D3D11 geometry-buffer capacities in the canonical DMC3 HD executable and separates:

- the SCM-specific stream/index path;
- generic HD streaming geometry;
- the static quad-index helper;
- serialized SCM count ceilings.

The report does not equate a buffer capacity with a guaranteed original-game acceptance maximum. Rollover/reset scheduling and multi-draw sharing remain separate gates.

## Renderer initialization

The renderer zeroes the stream-buffer state beginning at renderer `+0x1400` and creates two ping-pong resource sets.

### SCM-compatible stream slots

Sixteen stream descriptors exist at a physical stride of `0x38`. Seven have non-zero element strides in the canonical initializer:

| stream | stride | ByteWidth per buffer | element capacity |
|---:|---:|---:|---:|
| 0 | 12 | `0x120000` = 1,179,648 B | 98,304 |
| 2 | 12 | `0x120000` = 1,179,648 B | 98,304 |
| 3 | 16 | `0x180000` = 1,572,864 B | 98,304 |
| 4 | 16 | `0x180000` = 1,572,864 B | 98,304 |
| 6 | 4 | `0x60000` = 393,216 B | 98,304 |
| 7 | 4 | `0x60000` = 393,216 B | 98,304 |
| 8 | 8 | `0xC0000` = 786,432 B | 98,304 |

Creation formula recovered at `0x140042E20..0x140042E67`:

```text
ByteWidth = stream_stride * 3 * 0x8000
          = stream_stride * 0x18000
```

Therefore every active stream has exactly `0x18000 = 98,304` elements of capacity.

The seven active streams total 7,077,888 bytes (6.75 MiB) per resource set, or 13.5 MiB across the two ping-pong sets.

Status: EXE_CONFIRMED.

## SCM stream uploader

`0x140043830`:

1. selects the stream descriptor from the requested stream index;
2. selects the current ping-pong buffer from renderer `+0x1828`;
3. maps it through the D3D11 context;
4. uses WRITE_DISCARD when the stream's current byte offset is zero and WRITE_NO_OVERWRITE otherwise;
5. converts/copies the input stream into the HD stream representation;
6. unmaps;
7. binds through `IASetVertexBuffers`;
8. advances that stream's byte offset by `element_count * stream_stride`.

The SCM compatibility converter `0x140044310` directly calls `0x140043830` for stream 0, stream 2, stream 8 and additional format-dependent streams among the initialized set.

This closes the previous uncertainty over whether the separate generic `0x3C0000` streaming vertex buffer should be used as SCM authority: it should not. SCM has source-proven access to the per-stream `0x18000`-element buffers above.

### Consequence for one SCM mesh

SCM mesh `vertex_count` is u16, so the serialized single-mesh domain ends at 65,535 vertices.

A single mesh therefore cannot exceed the per-stream 98,304-element capacity solely through its serialized vertex count:

```text
65,535 < 98,304
```

This means per-stream vertex capacity is not the first single-mesh ceiling.

However, stream offsets accumulate across uploads inside the same renderer reset window. Multi-draw/shared-window exhaustion remains open until caller-side rollover/split/reset behavior is fully closed.

Status: EXE_CONFIRMED capacity + STRUCTURAL_CONFIRMED comparison.

## SCM index path

`0x140044310` calls `0x1400425B0` with index-element width 2.

The renderer creates two ping-pong R16 dynamic index buffers:

```text
element size = 2
index capacity = 0x27000 = 159,744
ByteWidth = 0x4E000 = 319,488
format = DXGI_FORMAT_R16_UINT
```

`0x1400425B0` maps, copies, unmaps, binds the selected buffer through `IASetIndexBuffer`, advances the current index offset, selects primitive topology and reaches D3D11 `DrawIndexed`.

Status: EXE_CONFIRMED_SCM_TO_D3D11_R16_INDEXED_DRAW.

## Derived no-break single-mesh threshold

Canonical SCM index reconstruction uses u16 indices and reserves:

```text
align16(6 * (vertexCount - 2)) bytes
```

For an unbroken strip, generated index count is:

```text
3 * (vertexCount - 2)
```

Combining that with the R16 dynamic buffer's 159,744-index capacity:

```text
3 * (v - 2) <= 159,744
v <= 53,250
```

So 53,250 vertices is a **derived single-mesh/no-break capacity threshold**, not a final engine maximum.

Required assumptions:

- the R16 dynamic buffer begins the relevant reset window empty;
- no previous indexed draws consume the same current buffer;
- no caller-side split/rollover occurs;
- topology is one unbroken strip;
- the game reaches the draw path normally.

Retail comparison:

- maximum observed mesh: 10,196 vertices;
- worst-case no-break indices: 30,582;
- 30,582 / 159,744 = about 19.1%.

This shows the retail corpus stays far below the recovered R16 dynamic-index capacity.

## Generic HD geometry path

The generic streaming vertex uploader at `0x140043190` uses two dynamic vertex buffers:

```text
ByteWidth = 0x3C0000 = 3,932,160 bytes = 3.75 MiB
Usage = D3D11_USAGE_DYNAMIC
BindFlags = D3D11_BIND_VERTEX_BUFFER
CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
initializer stride field = 40
```

Its output stride is runtime-derived from vertex-format flags. It is real backend evidence but not used as the SCM stream-capacity authority.

The generic dynamic R32 index set has:

```text
capacity = 0x27000 = 159,744 indices
ByteWidth = 0x9C000 = 638,976 bytes
format = DXGI_FORMAT_R32_UINT
```

A separate static quad-index buffer contains:

- 159,744 quads;
- 958,464 u32 indices;
- 3,833,856 bytes;
- index references covering 638,976 vertices.

The generic draw wrapper `0x140043F90` can therefore use 32-bit indices, proving that R16 is not a global HD-renderer limitation. It is the confirmed SCM path's index format.

## Reset state

`0x140042270` switches the ping-pong selector at renderer `+0x1828` and zeroes:

- generic dynamic vertex byte offset `+0x17A0`;
- R16 index offset `+0x17D8`;
- R32 index offset `+0x1810`;
- per-stream current offsets in the stream-state array.

`0x1400335C0` directly invokes this reset and also resets adjacent renderer/compatibility state.

The exact scheduler semantic is not yet promoted to "per frame". Until a direct scheduling proof is recovered, the evidence ledger calls this interval a **renderer reset window**.

## Remaining gates

1. Determine whether a pre-call capacity test rolls/splits SCM streams before a current-offset + incoming-count overflow.
2. Bind `0x1400335C0` / `0x140337FA0` to an exact scheduler/frame lifecycle semantic.
3. Stress-test geometry around the derived 53,250 no-break threshold.
4. Test multi-mesh accumulation that approaches 98,304 stream elements and 159,744 R16 indices in one reset window.
5. Separate graceful D3D11 Map failure from game-side memory corruption/overrun behavior on deliberately invalid inputs.

Until these gates close, 53,250 is a derived budget boundary and 65,535 remains only the serialized u16 ceiling.
