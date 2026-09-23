# DMC3 HD — Euler composition order of `0x140330450` corrected (2026-09-23)

Canonical executable: `dmc3.exe`,
SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Finding

`world_transform::build_local_matrix` expanded `Rz × Ry × Rx`. The executable
builds `Rx × Ry × Rz` (row-vector convention, X applied first). The two agree
for single-axis rotations only, which is why rest poses (mostly zero or single
axis) looked correct while multi-axis cases did not. The first visible
consequence was the Rebellion sheathed record (joint 3, R(-1.658, 0, 3.403)):
with the old order the blade pointed up; with the executable's order it hangs
down along the back.

## Evidence

- `0x140330450` (receipt in the 2026-09-23 MOT note): `rotX(dest, src)`,
  `rotY(dest, dest)`, `rotZ(dest, dest)` with angles `+0`, `+4`, `+8`.
- `0x140030F10` (X), `0x140030FC0` (Y), `0x140031080` (Z): `sinf`/`cosf`
  through thunks `0x140346C26` / `0x140346C20` (IAT `0x14034F4A8` sinf,
  `0x14034F4B0` cosf). Rotation rows written on the stack:
  - X: `(1,0,0) (0,c,s) (0,-s,c)`
  - Y: `(c,0,-s) (0,1,0) (s,0,c)`
  - Z: `(c,s,0) (-s,c,0) (0,0,1)`
- `0x14002FFE0(rcx=out, rdx=A, r8=B)`: `out.row_i = Σ_k B[i][k] · A.row_k`,
  i.e. `out = B × A`. The helpers pass `A = rotation`, `B = source`, so every
  step is `dest = source × R`.

From identity: `Rx × Ry × Rz`.

## Scope

`build_local_matrix` feeds the MOD rest locals (`0x1402FA080`), the animated
locals (`0x140310310`), world/inverse-rest/skin palettes and the weapon attach
offsets (`0x1401FD8F0`). All now use the executable's order.
`tests/mod_euler_order_tests.cpp` reproduces the helper sequence numerically.
