# DMC3 SCM draw order: an enable bit and depth buckets, no CPU culling

Date: 2026-09-24.

Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Findings

- The SCM manager bit `+0xE0` bit 0 is an enable flag, not a per-frame
  visibility result. The object setup sets it once (`0x1402F9641`: `or eax,
  1`), after it reads the object state through `0x1402FD650`. The setters
  near `0x1402F66F4` and `0x1402F7516` rewrite the word as a show/hide state.
- The per-frame dispatcher `0x140300740` skips a manager whose bit 0 is
  clear. For an enabled SCM manager it does the following:
  1. It transforms one point of the object record through the view matrix
     (`0x140031130`, the global matrix block `+0x140`, `r9d = 1`). It stores
     the view-space depth at `manager+0x210`.
  2. For SCM pass mode 4 it quantises that depth into one of 126 buckets
     (`0x140304950` → `0x1402F9680(manager+0x210, 0x7E, 7)`). It stores the
     bucket at `+0x218`.
  3. It queues encoded render commands per object
     (`0x140300150` / `0x140305440`). The GPU draws them in bucket order.
- These functions contain no frustum test and no sphere test. The object
  bounding sphere (object record `+0x30` / `+0x3C`) is not read on this path.
  Hidden surfaces are left to the GPU depth buffer.

## Use in Native Reader

The viewer rasterises on the CPU, so it follows the same order in software:

- Each stage vertex is moved into camera space once per frame.
- Triangles are clipped at a near plane.
- Faces turned away from the camera are dropped.
- The opaque triangles are sorted front to back. The depth test then rejects
  hidden pixels before any texel is sampled.
- Soft-alpha triangles are sorted back to front and blended last.
- The rows are rasterised in bands on several cores.

Together with building the native code at `-O2` in debug APKs as well, this
brings the st000 room frame from 108 ms down to 21 ms at 540×720 on a 4-core
host. At half size, with nearest texels, it takes 7 ms; the viewer uses that
mode while the view moves.
