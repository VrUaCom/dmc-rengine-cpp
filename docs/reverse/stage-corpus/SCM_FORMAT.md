# DMC3 SCM reverse guide (corpus phase 1)

Status: **confirmed structural pass / in-progress semantic pass**

## Corpus

- Source: `the extracted stage corpus`
- Scope: 556 stage-like `st*` directories.
- SCM files in scope: 1,495.
- Main room files: 284 files matching `_002.scm`.
- Auxiliary model files: 1,211 files under `_005` model slots.
- Independent layout validation: 1,718 SCM files across the complete extracted tree, 1,718 valid, 0 invalid, 0 warnings.

## Confirmed binary layout

The canonical C++ implementation is already present in `the repository`:

- `include/dmc_rengine/formats/scm.hpp`
- `src/formats/scm.cpp`
- `src/formats/scm_validation.cpp`
- `src/formats/scm_hierarchy.cpp`
- `src/formats/scm_transform.cpp`
- `src/formats/scm_writer.cpp`

Structural facts confirmed against the local corpus:

| Offset | Field | Type |
|---:|---|---|
| `0x00` | magic | `SCM ` |
| `0x04` | format version | `f32` |
| `0x10` | object count | `u8` |
| `0x11` | scene-node count | `u8` |
| `0x12` | texture-slot count | `u8` |
| `0x13` | lighting reference node | `u8` |
| `0x14` | legacy resource code | `u32` |
| `0x20` | scene-node block offset | `u64` |
| `0x40` | object table start | — |

Object records are `0x40` bytes. Mesh definitions are `0x50` bytes. Position, normal, fixed-point UV, color/topology, continuation and index-workspace pointers are all validated across the corpus.

## Scene-node block

The scene block header is `0x20` bytes. The block stores four relative offsets:

1. parent-by-order-position;
2. node-at-order-position;
3. object binding by node index;
4. XYZ translation/rotation transforms.

The transform record is `0x20` bytes: translation, translation magnitude, XYZ rotation in radians, and one reserved value.

This is a scene-node hierarchy, **not a MOD skin skeleton**. A reader that exposes `+0x11` as `boneCount` and applies MOD-style skin transforms will misplace room geometry, especially in stages with many nodes such as `st002`.

## Resource variants

`_002.scm` and `_005_###.scm` are role variants inside stage resources. Both retain the same SCM binary family and must use the SCM adapter. `_005` is selected through the stage index/PAC slot table; it is not a replacement for the main `_002` room.

## Validation performed

- Header and object table bounds.
- Mesh table bounds.
- Position/normal/UV/color stream bounds.
- Mesh continuation span.
- Index-workspace bounds and expected capacity.
- SHA-256 recorded for every source file.

Detailed evidence:

- `evidence/stage_inventory.json`
- `evidence/scm_inventory.csv`
- `evidence/scm_layout_validation.json`
- `evidence/resource_profiles.json`
- `guides/stXXX.md`

## Known implementation mismatch

`the JavaScript SCM reader` still models `SCM` and `MOD` through one JavaScript structure and calls the SCM scene-node count `boneCount`. The C++ canonical parser in `dmc-rengine-cpp` has the corrected scene-node model. The next implementation task is to port the canonical SCM scene-node adapter into the Blender/Rengine path before attempting stage alignment or SCM merging.

