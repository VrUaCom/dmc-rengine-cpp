# DMC3 HD SCM hierarchy/world-transform reverse — 2026-09-03

**Status:** EXE_CONFIRMED hierarchy indexing, local/world propagation and rigid inverse-world cache; corpus-confirmed on 68 unique SCM resources.  
**Canonical target:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Result

The SCM scene block is no longer modeled as four generic parallel arrays. The canonical executable proves that the first two arrays form an **evaluation-order pair**, while object bindings and transforms are indexed by the actual scene-node index.

```text
scene block +0x00 -> parentByOrderPosition[]
scene block +0x04 -> nodeAtOrderPosition[]
scene block +0x08 -> objectBindingByNodeIndex[]
scene block +0x0C -> transformByNodeIndex[]
```

This distinction is required for correct world-matrix reconstruction.

## 1. Runtime pointer binding — 0x1402F1DB0

`0x1402F1DB0` resolves the four relative offsets from the serialized scene block and stores the resulting pointers in the SCM runtime manager:

```text
serialized scene +0x00 -> manager +0x08
serialized scene +0x04 -> manager +0x10
serialized scene +0x08 -> manager +0x18
serialized scene +0x0C -> manager +0x20
```

The later consumers establish the semantics below.

## 2. Evaluation order and parent indexing

`0x1402FA080` and `0x1402F9700` iterate an order-position `i` and obtain the actual scene-node index from:

```text
currentNode = manager+0x10[i]
```

For non-root positions, the parent is read independently from:

```text
parentNode = manager+0x08[i]
```

Therefore `manager+0x08` is **not** `parent[nodeIndex]`. It is `parentByOrderPosition[i]`, paired with `nodeAtOrderPosition[i]` from `manager+0x10`.

The relation is:

```text
position i
  node   = nodeAtOrderPosition[i]
  parent = parentByOrderPosition[i]
```

This also reconciles the historical Blender prior-art assignment:

```text
bone[hierarchyOrder[i]].parent = hierarchy[i]
```

without making that importer canonical authority.

## 3. Corpus invariants

A combined sweep of the 67 unique SCM resources in the preserved stage-drop ZIP plus the independently supplied `st001.scm` / `st114.scm` identities yields **68 unique SCM files**.

On 68/68:

1. `nodeAtOrderPosition` is an exact permutation of `0..nodeCount-1`;
2. `parentByOrderPosition[0] == -1`;
3. there is exactly one `-1` parent and it occurs at evaluation position 0;
4. every non-root parent references a scene node that has already appeared at an earlier evaluation position;
5. `objectBindingByNodeIndex` contains each geometry object index `0..objectCount-1` exactly once;
6. additional scene/helper nodes use object binding `-1`.

This is now enforced by the clean C++ structural validator.

## 4. Object binding is node-indexed

The object initializer at `0x140302F10` scans `manager+0x18` directly by scene-node index and looks for the entry equal to the geometry object index. The matching node index is stored in the runtime object at `+0x0E`.

This is consistent with the corpus. For example, in `st001.scm` the order permutation is highly non-linear, while `objectBindingByNodeIndex` remains aligned to node identity and marks helper nodes with `-1`.

Safe interpretation:

```text
objectBindingByNodeIndex[node] = geometry object index or -1
```

Do not reorder this array into evaluation order.

## 5. Local transform construction

The serialized transform record remains:

```text
+0x00 f32 translationX
+0x04 f32 translationY
+0x08 f32 translationZ
+0x0C f32 translationMagnitude
+0x10 f32 rotationX radians
+0x14 f32 rotationY radians
+0x18 f32 rotationZ radians
+0x1C f32 reserved = 0
```

The rotation path was previously closed through `0x140330450`: X, then Y, then Z, producing the DMC3 matrix product `Rz * Ry * Rx` from identity.

The translation helper `0x140031200` closes the remaining local-matrix part:

- rows 0..2 are copied from the rotation matrix;
- serialized translation XYZ is added to matrix row 3 XYZ;
- homogeneous W is preserved from the pre-existing matrix;
- serialized `translationMagnitude` is **not** added to W.

The mask used by `0x140031200` lives at runtime address `0x1405D9F30` and is initialized by `0x140001920` from static constant `0x14035D340`:

```text
u32 lanes = {0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF}
```

This makes the helper preserve the original W lane while accepting only translation XYZ from the serialized vec4. Starting from the identity-backed rotation matrix, local W remains `1.0`.

Clean implementation:

```text
scm::build_local_transform()
```

## 6. Matrix multiplication contract

`0x1400312B0` is the canonical 4x4 multiply. Its assembly computes every output row as the weighted sum of rows from the right operand using one row of the left operand.

Safe recovered contract:

```text
result = left * right
```

with DMC3's recovered row-major / row-vector convention.

`0x140030E40(dest, second, third)` calls the multiplier as:

```text
0x1400312B0(temp, third, second)
```

then copies `temp` to `dest`.

Therefore:

```text
0x140030E40(dest, parentWorld, local)
    => dest = local * parentWorld
```

Clean implementation:

```text
scm::multiply_dmc3_matrices()
```

## 7. Canonical world propagation — 0x1402F9700

`0x1402F9700` first copies its external/root base matrix to manager `+0x1B0`, then walks all scene nodes in evaluation order.

For each order position:

```text
currentNode = nodeAtOrderPosition[i]
local       = runtimeNode[currentNode] +0x40
parentWorld = runtimeNode[currentNode] +0x80
worldOut    = worldMatrices[currentNode]

0x140030E40(worldOut, parentWorld, local)
```

Thus the exact update relation is:

```text
root:
  world[root] = local[root] * rootBase

non-root:
  world[current] = local[current] * world[parentByOrderPosition[i]]
```

`0x1402FA080` establishes the parent-world pointers:

- the root runtime node points to manager `+0x1B0`;
- every non-root runtime node points to the already allocated world matrix of `parentByOrderPosition[i]`.

The clean C++20 reconstruction is:

```text
scm::build_world_matrices()
```

and rejects malformed/non-topological hierarchy data rather than guessing a fallback.

## 8. Inverse-world cache — 0x140030DC0

`0x1402FA080` calls `0x140030DC0` with each current world matrix and stores the returned 0x40-byte matrix in the first 0x40 bytes of the corresponding runtime-node record.

The helper is now structurally closed. It is a rigid-transform inverse for the SCM transform family:

1. transpose the upper-left 3x3 rotation basis;
2. zero the fourth element of the first three rows;
3. compute inverse translation as `-T * R^T` in the recovered row-vector convention;
4. set homogeneous W to `1.0`.

Equivalent relation:

```text
inverseWorld.rotation    = transpose(world.rotation)
inverseWorld.translation = -world.translation * inverseWorld.rotation
inverseWorld.W           = 1
```

No scale inversion is performed, which is correct for the recovered SCM node matrices because serialized transforms contain translation + Euler rotation and no scale channel.

Clean implementation:

```text
scm::invert_dmc3_rigid_transform()
```

Regression verifies both:

```text
world * inverseWorld == identity
inverseWorld * world == identity
```

within float tolerance for translation-only and combined XYZ-rotation/translation cases.

## 9. Object +0x01 additional narrowing

The serialized `object+0x01` byte is copied to runtime object `+0x07`, then duplicated to runtime `+0x17C`. Runtime setup paths at `0x140304140` and `0x140304509` convert `+0x17C` to float and multiply by the exact constant at `0x14035D558`:

```text
0.003921568859... ~= 1 / 255
```

Operationally:

```text
if runtime +0x178 > 0:
    downstream float = 1.0
else:
    downstream float = runtime +0x17C / 255.0
```

`+0x178` is populated when the corrected control/classification byte is greater than `0x80`.

This proves the byte participates in a normalized 8-bit render-facing parameter/control path, but does **not** by itself prove a final name such as opacity/alpha/material class. The clean IR therefore remains semantically neutral.

## 10. Source-object flag lineage correction

Serialized `object+0x10` is copied verbatim into runtime object `+0x10` and `+0x14` by `0x140302F10` before the known operational bit projection runs.

A same-value `0x00200000` test exists at `0x140303F2F`, but provenance review shows that site reads **manager `+0xE0`**, not runtime object `+0x10/+0x14`. Manager `+0xE0` is independently initialized from family classifier `0x1402FD650` plus constructor flags in `0x1402F9570`.

Therefore that site is **not evidence** for the observed serialized object flag `0x00200000`. The object-bit lineage remains open until a direct runtime-object consumer or a proven propagation path is recovered.

## 11. Remaining hierarchy boundary

The following are now closed for the canonical target:

- exact scene-array indexing contract;
- topological evaluation order;
- root and parent relation;
- object-binding node indexing;
- local rotation matrix;
- local translation placement;
- translation-magnitude exclusion from homogeneous W;
- matrix multiply order;
- root/non-root world propagation;
- rigid inverse-world cache form.

Still separate/open:

- exact semantic purpose of `translationMagnitude` outside matrix construction;
- external engine coordinate-system naming/handedness conversion for third-party tools;
- final semantic name of object `+0x01`;
- direct lineage/meaning of serialized object flag `0x00200000`.
