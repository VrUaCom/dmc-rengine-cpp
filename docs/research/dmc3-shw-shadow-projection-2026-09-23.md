# DMC3 SHW: model pairing, per-frame posing and shadow drawing

Date: 2026-09-23
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Builds on `dmc3-real-mod-shw-payload-binding-2026-09-01.md` (layout and
selector semantics). Samples (analysed only, not committed): `pl000.pac`
(SHA-256 `b73b6e8ee2f2088a088f4215a5a13a50700fb4e4f0bef54dc0bcce11ae7b9c5d`)
slots 8 and 14.

## 1. Pairing with a model

Header byte `+0x11` equals the node count of the MOD the SHW shadows:

| Slot | Hulls | Vertices | Triangles | `+0x11` | Model |
| --- | --- | --- | --- | --- | --- |
| 8 | 17 | 152 | 236 | 24 | slot 1 body (24 nodes) |
| 14 | 17 | 200 | 332 | 33 | slot 12 coat (33 nodes) |

Every selector is below that count (body max 21, coat max 19) and each hull
uses one joint (head 5, chest 3, hips 14, legs 15/16/17 and 19/20/21, arms
7-13). All 34 hulls are closed (`T = 2V - 4`). Byte `+0x12`/`+0x14` (1, 2)
is not interpreted.

Vertices are in the model's rest space (body hulls span y 0..182 like the
body mesh; coat hulls hang from the coat root), so the selected matrix must
map rest space to the current pose: `inverseRest x world` of that joint,
i.e. single-influence skinning.

## 2. Per-frame code (`0x1403200D0`)

- Copies `u16 [shw+6] * 0x40` bytes of matrices from the owning model
  (`[shw+0x10] + 0x178 + buffer*8`) into a scratch palette (memcpy thunk
  `0x140346BE4`).
- Light vector: `[shw+0x60]` minus the translation row of the model joint
  `word [model+0xFA]` (`[model+0x188] + joint*0x40 + 0x30`), then combined
  with the camera matrices at global `+0x180` and `+0x140` (`0x140030E40`).
- For each hull with flag bit `+2 & 1`: every vertex is transformed by
  `palette[selector]` (`0x1403202F0` -> `0x140030A70`), then
  `0x1403204F0` runs.

## 3. Silhouette data (`0x1403204F0`)

For every triangle it builds the face normal (`0x140320BB0`) from the three
transformed vertices and dots it with the light vector (`0x140030D30`);
facing triangles pass two further tests (`0x140320950`, `0x1403206F0`) and
are marked. It then copies the transformed vertices to the draw buffer and,
for each marked triangle, writes the marks of its three neighbours from the
adjacency stream (unmarked triangles write `-1`). This is silhouette
extraction for a stencil shadow volume; the extrusion happens in the draw.

## 4. Shader

The embedded `DMC3_SHW.hlsl` (vs_5_0, source in the PDB blob at file offset
`0x4AF800`) takes a `float3 POSITION`, outputs
`pos * screenScale + screenOffset * pos.w` (y flipped) and colours it with the
constant `sdwColor`. Positions arrive already transformed, so the shape of the
shadow is decided entirely by the CPU steps above.

## 5. Viewer rule

On a flat floor a closed hull's stencil shadow equals the hull projected along
the light direction onto the floor. The light position comes from the stage
(`[shw+0x60]`), so a viewer without a stage uses a fixed direction and says so.

## 6. Open

- Where `[shw+0x60]` is written (stage light or per-character light).
- `0x140320950` / `0x1403206F0` (likely view/extent culling).
- Hull flag bit at `+2` of the runtime record and the meaning of `+0x12`.
