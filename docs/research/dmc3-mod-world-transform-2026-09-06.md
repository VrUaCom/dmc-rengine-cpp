# DMC3 HD MOD hierarchy/world-transform reverse — 2026-09-06

**Status:** EXE_CONFIRMED MOD/EFM hierarchy indexing + world propagation; serialized local-transform shell is corpus-confirmed on the three bound MOD payloads already recorded by Model Family evidence.  
**Canonical target:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Size:** `6,356,432` bytes

## Result

The previous MOD evidence boundary can be closed for **model-space spatial hierarchy**.
The canonical executable proves that MOD/EFM uses the same serialized node-domain
shape already parsed by `formats::mod::transform_domain`, and it also proves the
world-matrix propagation relation. MOD 3D hierarchy preview no longer needs to infer
bone positions from mesh vertices.

Safe canonical result:

```text
serialized MOD node domain
  +0x00 -> parentByOrderPosition[]
  +0x04 -> nodeAtOrderPosition[]
  +0x08 -> adapter domain (semantics still unresolved)
  +0x0C -> localTransformByNodeIndex[]

local[node] = Rz * Ry * Rx + translation XYZ

root:
  world[root] = local[root] * rootBase

child at evaluation position i:
  node   = nodeAtOrderPosition[i]
  parent = parentByOrderPosition[i]
  world[node] = local[node] * world[parent]
```

With an identity `rootBase`, the result is the canonical **MOD model-space world
hierarchy**. Translation row XYZ of each resulting world matrix is the node position
for the 3D hierarchy overlay.

## 1. Canonical artifact verification

The supplied executable was re-hashed before analysis:

```text
size      6356432
sha256    e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
imageBase 0x140000000
```

No result in this addendum is taken from an unverified EXE variant.

## 2. Family identity: MOD/EFM path is distinct from SCM

The family classifier at `0x1402FD650` directly compares the first four bytes and
returns:

```text
"MOD " -> 0x10000000
"EFM " -> 0x20000000
"SCM " -> 0x30000000
```

Exact function receipt:

```text
VA          0x1402FD650
file offset 0x2FCA50
size        0x273
sha256      a31a8c1e225bc62c07dea05921c42eeff85c28b2f4872713594262e579b91961
```

The shared MOD/EFM construction function `0x1403039C0` calls the node-domain binder
`0x1402F1DB0`, then calls `0x1402FA080`, and later branches on manager `+0xE0`
against exactly `0x10000000` and `0x20000000` for MOD and EFM specialization.

```text
VA          0x1403039C0
file offset 0x302DC0
size        0x11D
sha256      31f6b1074d387aea377ef166506b472135380cd288859f803be6d938049cc427
```

SCM uses the separate construction path `0x140303C10`, which calls
`0x1402FA360` instead of `0x1402FA080`.

```text
SCM constructor VA    0x140303C10
SCM constructor size  0xDB
SCM constructor hash  0fc48cd414b3b1c11323a4652f19e12eff4e999d302d2bc31555fedc571b8838
SCM initializer VA    0x1402FA360
SCM initializer size  0x217
SCM initializer hash  9dbab5f1584d8ed55b1a9d00ecbd7aa2fa3cbc37e480994e4169b3b33a70e13c
```

This closes the provenance mistake in the 2026-09-03 SCM notes: the world/hierarchy
logic observed in `0x1402FA080` belongs to the MOD/EFM manager path.

## 3. Serialized node-domain binding — 0x1402F1DB0

`0x1402F1DB0` reaches the serialized node-domain block through the loaded resource,
resolves its four relative pointers, and stores them in manager state:

```text
serialized +0x00 -> manager +0x08
serialized +0x04 -> manager +0x10
serialized +0x08 -> manager +0x18
serialized +0x0C -> manager +0x20
```

The same helper assigns the runtime world-matrix array to `manager+0x188` and the
runtime-node array to `manager+0x00`.

```text
VA          0x1402F1DB0
file offset 0x2F11B0
size        0xF1
sha256      48f4603ce59f443d74eed4266e5e43788e338121bd285918d236dc17d453e175
```

The earlier inverse-permutation parent heuristic is therefore rejected. Parent bytes
already name node indices and are paired by evaluation position with the order array.

## 4. MOD/EFM initializer — 0x1402FA080

`0x1402FA080` first walks every node by node index. Runtime nodes have stride `0xA0`;
the local matrix lives at runtime-node `+0x40`. The serialized transform pointer is
`manager+0x20`, and records advance by `0x20` bytes.

For each record, the function:

1. reads serialized translation vec4 at `+0x00`;
2. reads rotation XYZ at `+0x10`;
3. calls `0x140330450` to build the XYZ Euler rotation basis;
4. calls `0x140031200` to apply translation XYZ while preserving homogeneous W.

It then initializes hierarchy state:

- root node is `nodeAtOrderPosition[0]`;
- root local matrix is copied to its world slot;
- root runtime-node `+0x80` points to `manager+0x1B0` (root-base matrix);
- every child runtime-node `+0x80` points to `world[parentByOrderPosition[i]]`;
- every child world matrix is composed from local and parent world through
  `0x140030E40`.

Exact function receipt:

```text
VA          0x1402FA080
file offset 0x2F9480
size        0x2DE
sha256      7eff4f2f947fff6d152a714d1a14d6ee72162186304a9d0169df12ccfe5a9e32
```

## 5. Runtime world update — 0x1402F9700

`0x1402F9700(manager, rootBase)` first copies the caller-provided root matrix to
`manager+0x1B0`. It then iterates evaluation positions and resolves the current node
through `manager+0x10`.

For every node it calls:

```text
0x140030E40(
    world[current],
    runtimeNode[current]+0x80,   // rootBase for root; parent world for child
    runtimeNode[current]+0x40    // local
)
```

The wrapper calls `0x1400312B0` with operands in the order that yields:

```text
world[current] = local[current] * parentWorld
```

Because `0x1402FA080` pre-binds root `+0x80` to `manager+0x1B0`, this gives:

```text
world[root]  = local[root] * rootBase
world[child] = local[child] * world[parent]
```

Exact function receipt:

```text
VA          0x1402F9700
file offset 0x2F8B00
size        0xBA
sha256      314ee245ca08ce03c4d566af9cadb487532a020c3be4829a51a433328c5fd9d8
```

## 6. Matrix-helper receipts

```text
0x140030D00  0x23 bytes  sha256 f0d1aefc30b98239cf61c3401b93a59a48da903d9e113d4e7824323fe56da43c
  copy 0x40-byte matrix: destination RCX <- source RDX

0x140030E40  0x40 bytes  sha256 c18b8371c729ed8704135accd9a3ebdcc8b29d2a1159b919266fb84ea6c4b189
  composition wrapper; delegates to 0x1400312B0

0x1400312B0  0x13A bytes sha256 8173912ec4d56de74d60c0573ba6408c27c99a57d2d9696b2b08d17d54cfef3c
  canonical 4x4 multiply, result = left * right

0x140031200  0x34 bytes  sha256 5d34ed9f5d4659c17c26079cf5786b188d0572ebd83e0e701ab1b9ac428ea557
  copies rotation basis and adds translation XYZ to row 3; +0x0C is excluded from W

0x140330450  0x44 bytes  sha256 f11c66a7b9910dea4feb1072dd84efbafd0ff605c8c144ac30b9469cff07ac65
  consumes serialized rotation XYZ through the recovered axis-rotation sequence
```

## 7. Canonical implementation boundary

The clean C++20 implementation is `formats::mod::world_transform`:

- `supports_spatial_hierarchy()` — instance-level capability gate;
- `build_local_matrix()` — MOD local matrix;
- `build_world_matrices(domain, rootBase)` — exact runtime world relation;
- `build_model_space_world_matrices(domain)` — identity-root model-space hierarchy;
- `world_position()` — row-3 XYZ position for overlay.

The capability gate requires all of the following for the concrete MOD document:

```text
recognized parser result
complete node permutation
valid topological parent/order relation
complete local-transform array
finite transform values
matching node-array sizes
```

If any condition fails, spatial hierarchy is unavailable and the renderer must not
invent positions.

## 8. Overlay authorization after this pass

Promoted:

- ✅ MOD local transform records;
- ✅ MOD parent/order indexing;
- ✅ MOD model-space world matrices with identity root;
- ✅ MOD node world positions from world-matrix row 3 XYZ;
- ✅ capability-driven 3D hierarchy overlay when the concrete document passes the
  spatial gate;
- ✅ caller-supplied trusted root matrix can reproduce runtime placement composition.

Still not promoted:

- ❌ semantics of serialized adapter array `+0x08`;
- ❌ authoring/writer safety for MOD transforms;
- ❌ arbitrary game/world placement when no trusted external root transform is
  available;
- ❌ deriving any bone position from mesh vertices.

This means the viewer may now draw a truthful MOD skeleton/hierarchy in model space.
If a later runtime/game placement matrix is available from another canonical source,
the same builder can place that hierarchy through the explicit root-base overload.
