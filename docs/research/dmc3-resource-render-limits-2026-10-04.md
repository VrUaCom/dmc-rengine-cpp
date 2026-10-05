# DMC3 HD / PS2 — Resource & Rendering Limits Full Reverse

**Project:** DMC Rengine  
**Date:** 2026-10-04  
**Status:** Current consolidated reverse snapshot  
**Repository:** `VrUaCom/dmc-rengine-cpp`  
**Research branch:** `reverse/dmc3-resource-render-limits-20261004`  
**Saved v21 evidence commit:** [cf3c0a6](https://github.com/VrUaCom/dmc-rengine-cpp/commit/cf3c0a6e9045ed9594450409fb9442bf01e55714)  
**Research base commit (v22):** `cf3c0a6e9045ed9594450409fb9442bf01e55714`  
**Main machine ledger schema:** `dmc-rengine.dmc3-resource-render-limits.v22`

**v22 continuation:** Sections 65–69 recover effect 11/12 initializer defaults, raw parameter admission, the 512×256 startup viewport, cached type 11 regeneration and checksum invalidation. 38 new instruction fixtures pass. Earlier evidence remains as history; later continuation sections and the current ledger supersede intermediate open questions.

## Canonical executable

**Target:** `dmc3.exe` — Devil May Cry 3 Special Edition, HD Collection  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Size:** `6,356,432` bytes  
**Image base:** `0x140000000`

This document consolidates the current reverse of DMC3 geometry, polygon, texture, runtime allocator, D3D11 backend and PS2 hardware limits. It intentionally separates:

1. what a serialized format can represent;
2. what retail assets actually use;
3. what the canonical HD executable accepts or allocates;
4. what the HD renderer can submit in one rendered frame;
5. what original PlayStation 2 hardware can represent;
6. what is still unknown until controlled runtime stress testing or an original PS2 executable is available.

A single number such as “maximum polygons supported by DMC3” is not technically valid without specifying which of these domains is meant.

---

# 1. Evidence policy

DMC Rengine uses evidence states rather than guessed semantics.

| Status | Meaning |
|---|---|
| `EXE_CONFIRMED` | Directly proven from canonical executable behavior |
| `CORPUS_CONFIRMED` | Observed across a bounded/hash-bound real-resource corpus |
| `EXE_AND_CORPUS_CONFIRMED` | Executable behavior and corpus agree |
| `STRUCTURAL_CONFIRMED` | Binary structure/field width/layout is confirmed |
| `SEMANTIC_CANDIDATE` | Plausible interpretation, not yet promoted |
| `PRESERVED_UNDECODED` | Must be preserved exactly; semantics intentionally not invented |
| `RESERVED_OBSERVED_ZERO` | Zero in bounded corpus, not permission to normalize |
| `REJECTED` | Hypothesis disproven |
| `REJECTED_SUPERSEDED` | Earlier conclusion explicitly replaced by stronger evidence |

For limits, the following categories must remain distinct:

- `PS2_HARDWARE`
- `DMC3_SERIALIZED`
- `DMC3_HD_RUNTIME`
- `RETAIL_CORPUS`
- `PRODUCT_SAFETY`

A representable limit is not automatically a safe runtime limit.

---

# 2. Executive summary

The strongest current conclusions are:

- SCM can represent **65,535 vertices per mesh** and **65,535 total vertices per object** because both fields are `u16`.
- The canonical SCM normalizer does not introduce a lower explicit per-mesh vertex cap.
- Retail SCM is far below that ceiling: the largest observed mesh has **10,196 vertices**, while the largest observed file has **27,057 vertices**.
- The normal SCM runtime allocation route uses a dedicated contiguous pool whose empty usable payload is **5,236,736 bytes**; an alternate live route reduces that to **4,185,600 bytes**.
- Under a deliberately geometry-dense, empty-pool scenario, the allocation planner can mathematically describe around **1.18 million serialized vertices** on the normal route or around **0.92 million** on the alternate route. These are allocator envelopes, not gameplay-safe polygon counts.
- The HD renderer uses a shared dynamic vertex ring of exactly **0x3C0000 = 3,932,160 bytes** per normal rendered-frame lifecycle.
- SCM stage geometry reaches the HD backend as legacy GS compatibility geometry with **GS PRIM=5 = TRIANGLE_FAN**, then is converted to independent **3-vertex D3D11 triangle-list draws**.
- Therefore a large SCM mesh is not one giant GPU draw. Its rendered triangles consume the shared dynamic ring cumulatively.
- SCM-compatible output layouts observed in the EXE use 20, 28 or 36 bytes per output vertex, so one rendered triangle costs 60, 84 or 108 ring bytes.
- Empty-ring theoretical SCM triangle capacities are therefore **65,536**, **46,811** or **36,408** triangles depending on layout.
- The shared ring is not SCM-private. `0x140043F90` has **49 direct callsites** across the executable.
- The remaining generic draw `0x140044CDF` is SHW shadow-volume batching: **72 × (active faces + silhouette-side groups)** ring bytes per invocation.
- Cropped execution reproduces neighbor-variable writes beyond the 64/32/128 KiB separations for subcommand 11, subcommand 12 and SHW. These are conditional conflict boundaries, not enforced gameplay caps.
- SHW constructor/planner count fields are sign-extended 16-bit values, and SHW runtime allocation uses the same routed contiguous pool as SCM.
- Effect 11 initializer shifts are 4/3; the recovered startup compatibility viewport is 512×256. That combination generates 47,600 bytes and completes before the known scalar neighbor.
- The raw parameter importer permits values beyond that combination. Cached effect 11 also regenerates geometry; its checksum change detector is not an exact capacity guard.
- The executable has separate dynamic 16-bit and 32-bit index buffers with **159,744 entries each**. A 16-bit index buffer may contain more than 65,535 entries; only each stored index value is limited to `0..65535`.
- DMC3 HD's recovered DDS/Texture2D path explicitly accepts width and height up to **16,384 × 16,384** before `CreateTexture2D`, and the game explicitly requests **D3D feature level 11_0**.
- The preserved bounded retail DDS archive already contains **1024×2048 DXT5**, disproving the old assumption that `1024×1024` is a universal DMC3 HD maximum.
- Original PS2 GS hardware is a different domain: its specified texture size ceiling is **1024×1024**, with only **4 MiB GS local memory**.
- Exact DMC3-specific PS2 allocator/mesh/texture limits remain open until a hash-bound original `SLUS/SLES/ELF` is obtained and reversed.

---

# 3. SCM serialized structural ceilings

| Field | Serialized type | Representable range |
|---|---:|---:|
| `header.object_count` | `u8` | `0..255` |
| `header.scene_node_count` | `u8` | `0..255` |
| `header.texture_slot_count` | `u8` | `0..255` |
| `object.mesh_count` | `u8` | `0..255` |
| `object.total_vertex_count` | `u16` | `0..65535` |
| `mesh.vertex_count` | `u16` | `0..65535` |

**Status:** `STRUCTURAL_CONFIRMED`

These are representation ceilings only. They do not guarantee original-game acceptance at the maximum values.

---

# 4. Retail SCM corpus

Current hash-bound SCM corpus:

- **68 unique SCM payloads**
- **254 objects**
- **481 meshes**
- **328 scene nodes**
- **182,612 total serialized vertices**

Largest observed SCM: `st114.scm`

- file size: **1,038,816 bytes**
- vertices: **27,057**
- objects: **41**
- meshes: **72**

Observed per-mesh vertex domain: **3..10,196**

Observed mesh count per object: **1..10**

Observed texture-slot counts: **6 / 16 / 17**

**Status:** `CORPUS_CONFIRMED`

---

# 5. Reconstructed retail triangle workload

Applying the confirmed SCM topology-break reconstruction rule across the 68-file corpus produces:

- **114,051 reconstructed non-degenerate triangles**

Largest file workload — `st114.scm`:

- 27,057 vertices
- 72 meshes
- **16,248 triangles**

Largest individual mesh workload — `m20_b00_004_000.scm`, object 17, mesh 1:

- 10,196 vertices
- **5,696 triangles**

---

# 6. SCM generated-index workspace

`mesh+0x40` is a mesh-relative runtime-generated `u16` index workspace.

```text
capacityBytes = align16(6 * (vertexCount - 2))
```

for `vertexCount >= 3`.

Serialized state:

```text
first workspace word = 0x1212
mesh+0x48 = 0
```

The canonical normalizer at `0x1403051B0` reconstructs the index sequence and writes the generated index count to `mesh+0x48`.

**Status:** `EXE_AND_CORPUS_CONFIRMED`

---

# 7. SCM normalizer vertex-count domain

The normalizer `0x1403051B0` reads `mesh.vertex_count` as `u16`, zero-extends it, and compares a 32-bit loop counter directly against the resulting count.

At:

```text
vertex_count = 65535
```

the largest source vertex index is:

```text
65534
```

which remains valid in `u16`.

No lower explicit vertex-count cap has been recovered inside this normalizer.

> The SCM normalizer itself supports the full serialized `u16` mesh-count domain.

This does **not** prove a 65,535-vertex mesh is universally safe to render because allocation and per-frame renderer budgets are separate domains.

**Status:** `EXE_CONFIRMED_NORMALIZER_U16_DOMAIN`

---

# 8. SCM runtime memory planner

For SCM family mask `0x30000000`:

```text
0x1402FD8D0
 -> common planner 0x1402FD9C0
 -> SCM planner 0x1402FDD10
 -> align16(total)
```

SCM-specific term:

```text
scmSpecific =
    0xA0
  + 0x740 * objectCount
  + 0x220 * totalMeshCount
  + 0x100 * Σ ceil(meshVertexCount / 60)
  + 2 * auxiliarySize
```

Common term:

```text
common =
    0xB0
  + 0xE0 * sceneNodeCount
  + 0x40 * textureCount
  + optionalTextureState
```

For a one-object, one-mesh SCM with `vertexCount = 65535`, the SCM-specific contribution before common/auxiliary terms is approximately:

```text
0x44F00 = 282,368 bytes
```

Thus the size planner by itself does not reduce the one-mesh `u16` ceiling.

**Status:** `EXE_CONFIRMED`

---

# 9. Game arena and real SCM pool route

Startup `0x140030190` requests:

```text
0x10400000 = 260 MiB
```

The game initializes a:

```text
0x10000000 = 256 MiB
```

master arena.

SCM does not receive the whole master arena.

At `0x1400899FB..0x140089A12`, SCM requests selector `-2` through allocator router `0x1402C6150`.

## 9.1 Normal route — pool 1

- pool span: **5 MiB**
- block size: **1024 bytes**
- alignment: **16 bytes**
- slots: **5,114**
- empty-pool usable payload: **5,236,736 bytes (`0x4FE800`)**

Allocation requires a contiguous free bitmap run.

## 9.2 Alternate route — pool 2

When:

```text
byte[0x140CA8AB1] == 1
```

the route is forced to pool 2:

- pool span: **4 MiB**
- block size: **512 bytes**
- alignment: **64 bytes**
- slots: **8,175**
- empty-pool usable payload: **4,185,600 bytes (`0x3FDE00`)**

**Status:** `EXE_CONFIRMED_SCM_RUNTIME_ALLOCATION_LIMIT_PATH`

---

# 10. Geometry-dense allocator envelope

Under an intentionally artificial maximum-density scenario:

- empty pool;
- one mesh per object;
- each object/mesh contains 65,535 vertices;
- no texture overhead;
- zero auxiliary allocation;
- minimal node overhead;

the planner gives:

| Route | Empty usable pool | Full-u16 objects | Serialized vertices | Planned bytes |
|---|---:|---:|---:|---:|
| normal pool 1 | 5,236,736 | 18 | **1,179,630** | 5,084,336 |
| alternate pool 2 | 4,185,600 | 14 | **917,490** | 3,954,608 |

This is a **mathematical allocation envelope**, not a playable-scene guarantee.


# 11. `ceil(vertexCount / 60)` reverse

An important ambiguous term was:

```text
ceil(meshVertexCount / 60)
```

Initially this could have been interpreted as a 60-vertex render batch. That interpretation is now rejected.

## 11.1 Planner behavior

`0x1402FDD10` computes per mesh, per one of two runtime/frame domains:

```text
0x40 + 0x80 * ceil(vertexCount / 60)
```

The exact division pattern is:

```text
(vertexCount + 0x3B) / 0x3C
```

## 11.2 Constructor behavior

The actual SCM construction sequence is:

```text
0x1402F9570
 -> 0x1402F1DB0
 -> 0x1402FA360
 -> 0x14030D040 / 0x14030D430
 -> 0x1402F9120
 -> 0x1402F92B0
 -> 0x140302F10
 -> 0x1402F9F20
 -> 0x140309C60
```

The constructor creates, per mesh and per command domain:

```text
one 0x40 material block
one 0x80 geometry command block
```

The geometry block contains the fixed HD compatibility descriptor:

```text
0x5B00001C
```

carried by a seven-qword inline command.

No runtime mesh consumer was recovered that walks additional `chunkIndex * 0x80` geometry blocks.

Therefore additional planned bytes are headroom:

```text
plannerHeadroom(mesh)
 = 2 * 0x80 * (ceil(vertexCount/60) - 1)
 = 0x100 * (ceil(vertexCount/60) - 1)
```

**Conclusion:** the `/60` term is planner allocation headroom, not D3D11 draw batching.

**Status:** `EXE_CONFIRMED_PLANNER_HEADROOM`

---

# 12. Retail census of `/60` planner headroom

Across 67 unique SCM payloads in the bounded historical archive:

- 61 have non-zero planner headroom;
- 6 have zero headroom;
- aggregate headroom: **610,816 bytes**
- median: **1,792 bytes/file**
- mean: approximately **9,116.66 bytes/file**

Largest observed headroom — `st001_002.scm`:

- vertices: **23,049**
- meshes: **77**
- planner headroom: **87,808 bytes**

Other large values:

- `m20_b00_004_000.scm`: 77,056 B
- `m20_s00_004_032.scm`: 65,024 B
- `st445_005_040.scm`: 58,880 B
- `st445_002.scm`: 57,088 B

Rengine should preserve this allocation contract for parity unless a deliberate runtime compatibility patch changes it.

---

# 13. `runtimeObject+0x98` producer closure

Serialized source:

```text
SCM object+0x10 flags
```

Path:

```text
0x140302F10
 -> 0x140302640
 -> runtimeObject+0x98
```

Formula:

```text
runtimeObject+0x98 =
    0x1C
  | ((flags & 0x00010000) == 0 ? 0x20 : 0)
  | ((flags & 0x0000000F) != 0 ? 0x40 : 0)
```

Possible runtime values:

```text
0x1C
0x3C
0x5C
0x7C
```

The SCM geometry tag builder `0x14030A320..0x14030A38D` projects:

```text
((runtimeObject+0x98 & 0x7F8) | 5)
```

into a PRE-enabled GS PRIM/control field.

Recovered technical projections:

- `IIP = 1`
- `TME = 1`
- `FGE = 1` when serialized flag `0x00010000` is clear
- `ABE = 1` when the serialized flags low nibble is non-zero

**Status:** `EXE_CONFIRMED_SERIALIZED_TO_RUNTIME_GS_PRIM_CONTROL`

---

# 14. Correction: GS PRIM 5 is TRIANGLE_FAN

An earlier intermediate pass labeled GS `PRIM=5` as `TRIANGLE_STRIP`. That was wrong and is formally superseded.

Correct GS primitive domain:

```text
0 = point
1 = line
2 = line strip
3 = triangle list
4 = triangle strip
5 = triangle fan
6 = sprite
```

The canonical EXE independently agrees:

- threshold table: `0x14002BA98`
- primitive-5 fan anchor setup: `0x14002B20C`
- second dispatch table: `0x14002BAB4`
- primitive-5 handler: `0x14002B56E`

Therefore:

```text
SCM HD compatibility PRIM=5 = TRIANGLE_FAN
```

The old `TRIANGLE_STRIP` label for this HD render path is `REJECTED_SUPERSEDED`.

This correction does **not** rename the separate serialized SCM topology workspace reconstruction domain.

---

# 15. SCM TRIANGLE_FAN → D3D11 conversion

The HD path is now provenance-clean:

```text
SCM object flags
 -> 0x140302640
 -> runtimeObject+0x98
 -> 0x14030A320 geometry GIFtag
 -> GS PRIM=5 TRIANGLE_FAN
 -> XYZF2 handler 0x14002D6F6
 -> primitive dispatcher 0x14002B160
 -> fan handler 0x14002B56E
 -> D3D11 draw wrapper 0x140043F90
```

## 15.1 Fan state

Primitive-5 setup stores the first fan vertex as an anchor.

Anchor-related compatibility state includes:

```text
0x1405D9638
0x1405D9658
0x1405D9678
```

Current/previous state includes:

```text
0x1405D9620
0x1405D9628
0x1405D9630
```

The XYZF2 handler shifts the current/previous state on each drawable vertex.

Handler `0x14002B56E` composes the saved anchor plus the current/previous vertex state into one rendered triangle.

For a continuous drawable fan:

```text
triangleCount = N - 2
```

## 15.2 GPU submission

Each emitted fan triangle reaches `0x140043F90` with exactly:

**3 output vertices**

and is submitted as a non-indexed D3D11 triangle-list draw.

Therefore a large serialized SCM mesh does **not** become one giant GPU draw. It becomes a sequence of independent 3-vertex triangle draws.

**Status:** `EXE_CONFIRMED_SCM_TRIANGLE_FAN_DRAW_CONVERSION`

---

# 16. D3D11 draw wrapper

Main draw wrapper:

`0x140043F90..0x140044114`

Confirmed operations include:

- input-layout / vertex binding;
- topology selection;
- `Draw`;
- `DrawIndexed`;
- index buffer selection.

Indexed mode 5 uses:

```text
DXGI_FORMAT_R32_UINT
```

and calls `DrawIndexed`.

The count arguments in the D3D11 API path are 32-bit `UINT`.

This proves the HD backend itself is not limited to the serialized SCM `u16` workspace representation.

---

# 17. Dynamic index buffers

## 17.1 16-bit dynamic index buffer

```text
ByteWidth = 0x4E000 = 319,488 bytes
element size = 2 bytes
entries = 159,744
```

## 17.2 32-bit dynamic index buffer

```text
ByteWidth = 0x9C000 = 638,976 bytes
element size = 4 bytes
entries = 159,744
```

## 17.3 Important 16-bit clarification

A 16-bit index buffer is **not** limited to 65,535 elements.

It is limited to:

```text
index value = 0..65535
```

A buffer may therefore contain **159,744 `uint16` entries** as long as each entry references a representable vertex index.

## 17.4 Prebuilt quad index buffer

The EXE also creates:

```text
ByteWidth = 0x3A8000 = 3,833,856 bytes
format = R32_UINT
indices = 958,464
```

Generated as:

```text
159,744 quad groups * 6 indices
```

covering up to 638,976 sequential quad vertices in the generated pattern.

---

# 18. Dynamic vertex ring — corrected result

An intermediate pass incorrectly generalized **98,304 vertices** as a renderer-wide limit. That is rejected.

The actual fixed resource is:

```text
ByteWidth = 0x3C0000
          = 3,932,160 bytes
          = 3.75 MiB
```

Relevant ranges:

- initialization: `0x140042E79..0x140042EC7`
- upload/conversion: `0x140043190..0x14004378A`
- draw wrapper: `0x140043F90..0x140044114`

The ring is byte-limited, not vertex-count-limited.

---

# 19. Output stride formula

The uploader derives output stride from compatibility vertex flags:

```text
base = 16 if (flags & 0x40000002) == 0x40000002 else 12

+8  if flags & 0x00000100
+16 if flags & 0x00000040
+4  if flags & 0x00020000
```

Confirmed examples:

| Flags | Output stride |
|---:|---:|
| `0x00000002` | 12 B |
| `0x40000002` | 16 B |
| `0x00000102` | 20 B |
| `0x00000042` | 28 B |
| `0x00000142` | 36 B |

Additional registered layouts include 40, 48 and 56-byte strides.

Thus the old **98,304 vertices** figure was simply:

```text
0x3C0000 / 40
```

for one 40-byte layout.

There is no universal 98,304-vertex backend ceiling.

---

# 20. Dynamic vertex upload lifecycle

Uploader `0x140043190` performs:

```text
Map
 -> compatibility vertex conversion/copy
 -> Unmap
 -> IASetVertexBuffers
 -> renderer+0x17A0 += vertexCount * outputStride
```

Map mode:

```text
offset == 0  -> discard-style
offset != 0  -> no-overwrite-style
```

No local guard was recovered in this uploader equivalent to:

```text
if currentOffset + uploadBytes > 0x3C0000
    wrap/reset/fail
```

Therefore ring safety is a caller/lifecycle property.

**Status:** `EXE_CONFIRMED_FIXED_BYTE_CAPACITY`

---

# 21. Normal rendered-frame reset

The normal renderer reset clears:

- `renderer+0x17A0` — dynamic vertex-ring byte offset;
- `renderer+0x17D8` — dynamic u16-index offset;
- `renderer+0x1810` — dynamic u32-index offset.

Normal render scheduling chain:

```text
dmc3_main 0x1402C5DF0
 -> mode setup 0x140337F70 = 1
 -> timed loop 0x1402C5EB0..0x1402C5FE2
 -> render dispatch
 -> 0x140337EC0
 -> mode-1 target 0x140337FA0
 -> 0x1400335C0
 -> dynamic ring reset
 -> render work
```

This supports modeling `0x3C0000` as a **shared normal per-rendered-frame byte budget**.

**Status:** `EXE_CONFIRMED_NORMAL_RENDERED_FRAME_RESET`

---

# 22. SCM triangle cost inside the ring

Recovered SCM-compatible output layouts:

| Flags | Stride | 3-vertex triangle cost | Empty-ring full triangles |
|---:|---:|---:|---:|
| `0x102` | 20 B | 60 B | **65,536** |
| `0x42` | 28 B | 84 B | **46,811** |
| `0x142` | 36 B | 108 B | **36,408** |

These are arithmetic empty-ring capacities only.

They are **not** stage polygon limits because the ring is shared, callsites may repeat, and other resources render in the same frame.

---

# 23. Maximum-size `u16` fan projection

For:

```text
source vertices = 65,535
triangles = 65,533
```

20-byte layout:

```text
65,533 * 60 = 3,931,980 bytes
ring size = 3,932,160 bytes
remaining = 180 bytes
```

28-byte layout:

```text
65,533 * 84 = 5,504,772 bytes
overflow = 1,572,612 bytes
```

36-byte layout:

```text
65,533 * 108 = 7,077,564 bytes
overflow = 3,145,404 bytes
```

This is one of the strongest demonstrations that:

```text
serialized u16 maximum != safe renderer maximum
```

for every output layout.

---

# 24. Retail `st114.scm` ring projection

If all 16,248 reconstructed triangles of `st114.scm` were rendered in one frame and no other dynamic draws used the ring:

| Layout | Bytes | Fraction of 3.75 MiB |
|---:|---:|---:|
| 20 B | 974,880 | 24.79% |
| 28 B | 1,364,832 | 34.71% |
| 36 B | 1,754,784 | 44.63% |

Actual occupancy depends on visibility, scheduling and simultaneous renderer work.


# 25. Shared dynamic-ring ownership

The dynamic uploader `0x140043190` has one direct executable caller:

```text
0x140043F90
```

The draw wrapper itself has:

**49 direct callsites**

across the canonical EXE.

SCM compatibility examples include:

```text
0x14002B407
0x14002B555
0x14002BA5B
```

Many callsites lie outside the SCM primitive cluster.

Therefore the ring is conclusively:

> a shared renderer resource, not an SCM-private allocation.

**Status:** `EXE_CONFIRMED_SHARED_DYNAMIC_VERTEX_RING`

---

# 26. Static draw-call ring census

A bounded local constant-propagation pass currently resolves flags, mode and byte count completely at:

**42 / 49 direct draw-wrapper callsites**

Fixed signatures:

| Flags | Mode | Ring bytes/invocation | Direct callsites |
|---:|---:|---:|---:|
| `0x102` | 5 | 80 | 20 |
| `0x102` | 3 | 60 | 17 |
| `0x142` | 5 | 144 | 1 |
| `0x102` | 4 | 60 | 1 |
| `0x42` | 4 | 84 | 1 |
| `0x102` | 4 | 80 | 1 |
| `0x142` | 5 | 96 | 1 |

If each of those 42 static sites executed exactly once:

```text
total = 3,084 bytes
```

This is **not** a frame occupancy claim because callsites may be inside loops and conditional paths.

Three compatibility primitive callsites have layout-dependent exact sets:

- `0x14002B407`: **40 / 56 / 72 B**
- `0x14002B555`: **60 / 84 / 108 B**
- `0x14002BA5B`: **80 / 112 / 144 B**

Four callsites retained genuinely runtime-derived counts during this pass:

```text
0x14003F4C8
0x140040344
0x140044CDF
0x1400457CD
```

Three of them have since been substantially bounded.

**Status:** `EXE_CONFIRMED_LAYOUT_STRIDE_MAPPING + EXE_CONFIRMED_STATIC_CALLSITE_ARGUMENT_CENSUS`

---

# 27. Custom HD A+D command protocol

The lower compatibility A+D dispatcher exposes a custom three-register command protocol.

## Address `0x63`

Handler `0x14002D200`:

- stores command/control value;
- resets argument count:

```text
0x1405D96A0 = 0
```

## Address `0x64`

Handler `0x14002D224`:

- appends one 64-bit argument to:

```text
0x1405D96F0[count]
```

- increments argument count.

## Address `0x65`

Handler `0x14002D23F`:

- interprets payload as subcommand ID `1..15`;
- dispatches through:

```text
0x14002D968
```

**Status:** `EXE_CONFIRMED`

---

# 28. A+D subcommand 11 → draw `0x14003F4C8`

Producer:

`0x140318EC0`

creates generated `0x28`-byte records beginning at:

`0x140CC1AD0`

At `0x1403194D8` it registers the backing pointer and serializes:

```text
arg0.low32  = registry index for 0x140CC1AD0
arg0.high32 = (generatedEnd - 0x140CC1AD0) >> 2
```

Packet terminator:

```text
A+D address 0x65
payload = 0x0B
```

selects subcommand **11**.

Immediately before draw callsite `0x14003F4C8`:

```text
ringByteCount = 4 * arg0.high32
```

Therefore, for this aligned producer:

```text
ringByteCount = generatedEnd - backingBase
```

Ring consumption is the exact generated backing byte count, not an estimated vertex count.

The producer is screen/display-derived non-SCM geometry and competes with SCM for the shared ring.

## 28.1 Candidate backing extent

The next referenced distinct global begins exactly:

```text
0x10000 = 65,536 bytes
```

after `0x140CC1AD0`.

This strongly suggests a 64 KiB backing region.

At `0x28` bytes per record, such a region could contain:

- 1,638 complete records
- 65,520 used bytes

But no explicit bounds check or authoritative size-bearing initializer has yet promoted this to a hard engine limit.

**Extent status:** `STRUCTURAL_CANDIDATE`

---

# 29. A+D subcommand 12 → draw `0x140040344`

Producer:

`0x140319D60`

creates `0x50`-byte records beginning at:

`0x140CD1AF0`

At `0x14031A263` it serializes:

```text
arg1.low32  = registry index
arg1.high32 = generatedEnd - 0x140CD1AF0
```

Packet terminator:

```text
A+D address 0x65
payload = 0x0C
```

selects subcommand **12**.

The dispatcher passes the high dword unchanged to draw site `0x140040344`.

Thus ring-byte count is exactly:

```text
generatedEnd - 0x140CD1AF0
```

The draw uses flags `0x102`:

```text
output stride = 20 bytes
```

Each `0x50` producer record corresponds to exactly:

**4 rendered vertices**

## 29.1 Candidate backing extent

Next referenced distinct global:

```text
0x140CD9AF0
```

Difference:

```text
0x8000 = 32,768 bytes
```

If this is the exact backing extent:

- 409 complete `0x50` records
- 1,636 rendered vertices
- 32,720 used bytes
- 48 bytes remain

Again, no explicit producer-side hard bound has been found.

**Extent status:** `STRUCTURAL_CANDIDATE`

---

# 30. Generic draw helper `0x1400457CD`

This formerly generic runtime-count draw is now bounded.

At `0x140045631`:

```text
N = (state+0x160) & 0x7FF
```

Therefore:

```text
0 <= N <= 2047
```

`N == 0` skips draw.

Mode selection from `source+0x04`:

- source mode 0 → draw mode 1
- source mode 1 → draw mode 3
- other → draw mode 5

Helper `0x140043110` maps:

```text
mode 1 -> 2*N vertices
mode 3 -> 3*N vertices
mode 5 -> 4*N vertices
```

This callsite uses:

```text
flags = 0x40000002
stride = 16 bytes
```

Maximum per invocation:

| Mode | Max vertices | Max ring bytes |
|---:|---:|---:|
| 1 | 4,094 | 65,504 |
| 3 | 6,141 | 98,256 |
| 5 | 8,188 | **131,008** |

**Status:** `EXE_CONFIRMED_BOUNDED_DYNAMIC_DRAW`

---

# 31. Draw helper `0x140044CDF` and scratch candidate

Batching helper:

`0x1400446F0`

writes `0x48`-byte groups beginning at:

`0x140BEB3F0`

Each group contributes:

```text
6 * 12-byte source vertices
```

An outer item produces one mandatory group plus up to three conditional groups.

Whole-image referenced-global analysis finds the next referenced global at:

`0x140C0B3F0`

Difference:

```text
0x20000 = 131,072 bytes
```

This is strongly consistent with a dedicated **128 KiB scratch region**.

If exact:

- 1,820 complete `0x48` groups
- 10,920 generated vertices
- 131,040 used bytes
- 32 bytes remain

However, an explicit bound/initializer for the exact object extent has not yet been recovered.

Therefore:

**Status:** `STRUCTURAL_CANDIDATE`

Rengine must not advertise 10,920 as a hard engine limit yet.

---

# 32. Current shared-ring research boundary

Static analysis has now established:

- ring size;
- exact stride formula;
- normal frame reset;
- direct caller population;
- 42 fully static callsites;
- exact layout-dependent compatibility callsites;
- bounded generic helper `0x1400457CD`;
- exact generated-byte propagation for subcommands 11 and 12;
- strong but unpromoted backing-region candidates.

What static reverse still does **not** provide is the exact number of times every draw path executes in a representative gameplay frame.

Therefore true per-frame occupancy still requires:

- full dynamic scheduler/draw-order reconstruction; or
- runtime instrumentation.

---

# 33. PTX runtime budget

Canonical runtime PTX payload:

```text
0x208 = 520 bytes
```

Contains:

**64 texture-record pointers**

Global PTX pool:

- **128 records**
- stride: `0x50 = 80 bytes`
- record-array span: `0x2800 = 10,240 bytes`

**Status:** `EXE_CONFIRMED`

---

# 34. PTX preflight reservation gate

Functions:

```text
0x1402FE1B0
0x1402FE030
```

were initially candidates for a hidden geometry limiter.

The reverse shows that they operate on PTX texture records.

`0x1402FE030` iterates `0x50`-byte records and counts active primary allocation plus eligible secondary allocation.

Terminal comparison is equivalent in the normal domain to:

```text
active PTX spans <= configured reservation blocks
```

Relevant pool field:

```text
pool+0xCB14 = 0x140D6C684
```

Producer `0x140331D90` stores:

```text
blocks << 5
```

Therefore this gate is a texture-placement/reservation gate, not an SCM vertex/polygon limit.

**Status:** `EXE_CONFIRMED_PTX_RESERVATION_GATE`

---

# 35. Full bounded DDS census

Archive:

`DMC 3 RENGINE (6).zip`

Size:

`237,658,858 bytes`

SHA-256:

`7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`

Population:

- **243 DDS paths**
- **154 unique DDS by SHA-256**
- **60 unique DXT1**
- **94 unique DXT5**
- **15 unique dimension/compression combinations**

Observed dimensions:

- minimum width: 128
- minimum height: 64
- maximum width: **1024**
- maximum height: **2048**
- mip counts: **8..12**

Largest observed texture:

`plwp_grenade_000_000.dds`

- `1024×2048`
- DXT5
- 12 mip levels
- **2,796,368 bytes**
- SHA-256: `00f03a616da7e8ae2a44f5a9a386a21b705ff74118a64bd72734504db23e40c1`

This directly disproves the old assumption that `1024×1024` is a universal DMC3 HD texture ceiling.

**Status:** `CORPUS_CONFIRMED_BOUNDED_ARCHIVE`

---

# 36. DDS dimension distribution

| Width | Height | Compression | Unique count |
|---:|---:|---|---:|
| 128 | 64 | DXT5 | 6 |
| 128 | 128 | DXT1 | 1 |
| 128 | 128 | DXT5 | 16 |
| 256 | 128 | DXT1 | 2 |
| 256 | 128 | DXT5 | 11 |
| 256 | 256 | DXT1 | 23 |
| 256 | 256 | DXT5 | 23 |
| 256 | 512 | DXT1 | 2 |
| 256 | 512 | DXT5 | 9 |
| 512 | 256 | DXT1 | 1 |
| 512 | 256 | DXT5 | 3 |
| 512 | 512 | DXT1 | 28 |
| 512 | 512 | DXT5 | 25 |
| 1024 | 1024 | DXT1 | 3 |
| 1024 | 2048 | DXT5 | 1 |


# 37. DMC3 HD DDS → D3D11 Texture2D path

Recovered path:

```text
0x1403365B0 PTX parser
 -> texture resource
 -> vtable 0x1404C5388
 -> GPU realization 0x140046AF0
 -> DDS loader 0x1400499C0 / 0x140049BA0
 -> creation helper 0x140049490
 -> Texture2D branch 0x1400495DC
 -> ID3D11Device::CreateTexture2D at 0x140049642
 -> optional CreateShaderResourceView
```

Recovered explicit validation:

```text
mipCount <= 15
arraySize <= 2048
width <= 16384
height <= 16384
```

Therefore:

> The recovered DMC3 HD descriptor-wrapped DDS Texture2D path has an EXE-side pre-create dimension ceiling of **16,384 × 16,384**.

This is not a guarantee that such a texture can actually be allocated alongside the game's other resources.

**Status:** `EXE_CONFIRMED`

---

# 38. D3D11 feature-level requirement

The canonical executable imports:

`D3D11CreateDeviceAndSwapChain`

and the recovered callsite supplies exactly one requested feature level:

```text
0xB000 = D3D_FEATURE_LEVEL_11_0
```

Feature-level count:

```text
1
```

The canonical path therefore does not negotiate down through a list of 10.x/9.x levels.

This is important because D3D11 feature level 11_0 defines a Texture2D U/V resource dimension limit of:

**16,384 texels**

matching the executable's own:

```text
width <= 0x4000
height <= 0x4000
```

checks.

**Status:** `EXE_CONFIRMED_D3D_FEATURE_LEVEL_11_0`

---

# 39. Historical Rengine DDS safety envelope

The previous `Dmc3DdsSafety` product policy used roughly:

```text
64..1024 per dimension
```

This must be treated as:

`PRODUCT_SAFETY_LIMIT`

not:

`DMC3_HD_ENGINE_MAX`

because retail evidence already contains 1024×2048 and the EXE validates up to 16,384 per axis.

Writer safety policy may remain intentionally conservative until authored stress tests are completed.

---

# 40. PS2 hardware envelope

The original PS2 domain must remain separate from HD Collection runtime.

Cross-check authority includes:

- PCSX2 `PCSX2/pcsx2`
- commit `81526d4dc7cc70e4ae75abb35a789417456c6d43`
- `pcsx2/GS/GSRegs.h`
- `pcsx2/GS/GSState.cpp`
- PS2 hardware/memory documentation

## 40.1 GS local memory

```text
4 MiB = 4,194,304 bytes
```

## 40.2 GS TEX0 fields

- `TBP0`: 14 bits
- `TBW`: 6 bits
- `PSM`: 6 bits
- `TW`: 4 bits
- `TH`: 4 bits

The specified valid maximum exponent used by PCSX2 is:

```text
TW/TH = 10
```

therefore:

**1024 × 1024**

specified maximum texture dimensions.

The raw 4-bit fields can encode larger exponent values, but those must not be advertised as valid GS texture support.

---

# 41. PS2 GS CLAMP domain

GS CLAMP fields:

- `MINU`: 10 bits
- `MAXU`: 10 bits
- `MINV`: 10 bits
- `MAXV`: 10 bits

Domain:

```text
0..1023
```

This directly matches the 10-bit `REGION_REPEAT` lanes recovered from SCM, strongly supporting the preserved legacy GS material ABI interpretation.

---

# 42. PS2 MTBA nuance

PCSX2 documents a narrower automatic mip-base (`MTBA`) domain:

- 32-bit swizzled texture formats: up to **512**
- 16-bit and ordinary 8/4-bit formats: up to **1024**

This is an automatic mip-base generation constraint.

It is **not** a replacement for the general TEX0 1024×1024 dimension ceiling.

---

# 43. Why PS2 1024×1024 is not a practical residency promise

Ignoring GS swizzle/alignment/CLUT/mip overhead:

| Format | Raw 1024² bytes |
|---|---:|
| 32 bpp | **4,194,304 B** |
| 16 bpp | 2,097,152 B |
| 8 bpp | 1,048,576 B |
| 4 bpp | 524,288 B |

The entire GS local memory is only:

**4,194,304 B**

Therefore one 1024×1024 32-bpp texture would consume the full GS local-memory capacity before framebuffer, depth buffer or any other resident resource.

A full mip chain further increases storage requirement.

Thus:

```text
1024×1024 = addressing/format ceiling
```

not:

```text
1024×1024 = practical DMC3 resident-texture budget
```

---

# 44. PS2 EE / VU memory context

| Resource | Capacity |
|---|---:|
| EE main RAM | **32 MiB** |
| VU0 instruction | 4 KiB |
| VU0 data | 4 KiB |
| VU1 instruction | **16 KiB** |
| VU1 data | **16 KiB** |
| EE scratchpad | **16 KiB** |

VU memory is a working-set/micro-batch domain, not a whole-scene polygon count.

PS2 engines stream geometry through DMA/VIF/VU/GIF rather than requiring an entire scene to fit in VU memory.

---

# 45. PS2 throughput is not a resource limit

Published GS peak figures include approximately:

- 2.352 Gpixel/s untextured peak fill
- 1.2 Gpixel/s textured peak
- 75 million/s small-polygon peak

These are idealized throughput metrics.

They do not specify:

- stage polygon capacity;
- mesh vertex count;
- resource file size;
- scene residency;
- DMC3-specific allocation.

Do not convert peak polygons/s into a DMC3 mesh or stage limit.

---

# 46. DMC3-specific PS2 limit boundary

No hash-bound original DMC3 PS2 executable:

```text
SLUS / SLES / ELF
```

is currently present in the connected project sources.

Therefore current PS2 evidence can establish:

- hardware/register limits;
- GS memory domains;
- compatibility correspondence with HD legacy-GS state.

It cannot yet establish:

- original DMC3 PS2 heap/pool sizes;
- DMC3 PS2 mesh allocator ceilings;
- DMC3 PS2 VU packet batching;
- DMC3 PS2 practical texture residency;
- DMC3 PS2 stage polygon maximum.

Required future evidence:

> Acquire a hash-bound original DMC3 PS2 executable and perform a separate allocator/VU/VIF/GIF reverse.

---

# 47. PS2 vs HD — do not merge these limits

| Domain | Texture dimension result |
|---|---:|
| PS2 GS specified TEX0 | **1024×1024** |
| Retail HD bounded archive | **1024×2048 observed** |
| HD canonical DDS loader guard | **16384×16384** |
| Old Rengine safety profile | 1024 per dimension — **product policy only** |

Therefore:

- do not project HD `16384×16384` backward onto PS2;
- do not project PS2 `1024×1024` forward as the HD engine maximum.

---

# 48. Key rejected or superseded claims

## REJECTED: “98,304 vertices is the engine limit”

Correct result:

```text
dynamic vertex ring = 3,932,160 bytes
```

Vertex capacity depends on stride.

## REJECTED: “GS PRIM 5 = TRIANGLE_STRIP”

Correct:

```text
GS PRIM 5 = TRIANGLE_FAN
```

## REJECTED: “1024×1024 is DMC3 HD maximum texture size”

Retail already contains:

```text
1024×2048 DXT5
```

and the EXE checks:

```text
<= 16384 per axis
```

## REJECTED: “ceil(vertexCount/60) means 60-vertex D3D11 draw chunks”

Correct:

```text
planner-side runtime allocation headroom
```

## REJECTED: “16-bit index buffer can contain at most 65,535 index elements”

Correct:

```text
each index value <= 65535
```

The buffer itself can contain more elements.

## REJECTED: “PTX preflight is another geometry limiter”

Correct:

```text
PTX texture-placement/reservation gate
```

---

# 49. Current strongest geometry model

The real HD geometry budget is a chain:

```text
SCM serialized field widths
        ↓
SCM normalizer
        ↓
runtime allocation planner
        ↓
contiguous SCM pool availability
        ↓
legacy GS compatibility command generation
        ↓
TRIANGLE_FAN primitive conversion
        ↓
independent 3-vertex D3D11 draws
        ↓
shared 3.75 MiB dynamic vertex ring
        ↓
other dynamic draws in same frame
        ↓
GPU/system resource residency
        ↓
frame-time / original-game stability
```

This is why no single “max polygons” integer can correctly describe DMC3 HD.

---

# 50. Current strongest texture model

```text
DDS / companion serialized dimensions
        ↓
PTX record/reservation budget
        ↓
DDS loader validation
        ↓
D3D11 feature-level 11_0 resource domain
        ↓
CreateTexture2D
        ↓
GPU/system memory residency
        ↓
simultaneous textures/render targets
        ↓
practical runtime success
```

Current strongest numbers:

```text
retail observed      = 1024×2048
HD EXE validation    = 16384×16384
PS2 GS specified max = 1024×1024
```

---

# 51. Recommended Rengine Resource Budget Analyzer

The reverse is now strong enough to justify a dedicated Rengine limit inspector.

For each SCM, Rengine should report at least:

```text
FORMAT DOMAIN
object count
mesh count
vertices per mesh
vertices per object
scene nodes
texture slots

RETAIL COMPARISON
vs observed corpus maxima

SCM ALLOCATOR
planned runtime bytes
pool route
empty-pool percentage
planner /60 headroom

TOPOLOGY
reconstructed triangles
triangle breaks
largest mesh workload

HD RENDER RING
candidate output stride
bytes per triangle
projected SCM ring bytes
fraction of 0x3C0000 ring

TEXTURE
slot count
largest width/height
mips
DXT type
PTX record pressure

STATUS
FORMAT_SAFE
CORPUS_NORMAL
ALLOCATOR_WARNING
RING_WARNING
TEXTURE_WARNING
UNTESTED_ABOVE_RETAIL
```

Suggested severity policy:

- **green:** inside retail-observed envelope;
- **yellow:** above retail but below proven structural/runtime boundaries;
- **orange:** projected shared-ring or pool pressure;
- **red:** proven structural/runtime ceiling exceeded;
- **purple/experimental:** EXE allows it statically but original-game stress acceptance remains untested.


# 52. Open gates (updated by v22)

The current machine ledger leaves the following important gates open:

1. Determine retail input reachability for the three generic producers. SHW ownership of `0x140044CDF`, its workload formula, and the specific neighbor-write boundaries are closed; see sections 59–64.
2. Determine retail payload values and dynamic execution frequency/order for effects 11/12; initializer defaults, local raw import and cache mechanics are now closed (sections 65–69).
3. Recover exact static-object extents. The distances below are now proven interference boundaries, but exact array declarations and enforced admission caps remain unproven:
   - `0x140CC1AD0` — candidate 64 KiB
   - `0x140CD1AF0` — candidate 32 KiB
   - `0x140BEB3F0` — candidate 128 KiB
4. Measure exact shared dynamic-ring occupancy/order in representative retail frames.
5. Test actual overflow behavior near projected SCM ring boundaries.
6. Quantify non-SCM per-frame ring consumption dynamically rather than only statically.
7. Determine practical texture allocation/residency failure below the EXE `16384×16384` validation ceiling.
8. Run original-game authored geometry stress tests.
9. Run original-game authored DDS stress tests.
10. Measure simultaneous stage residency:
    - stage geometry
    - textures
    - actors
    - effects
    - shadows
    - UI / other dynamic renderer paths
11. Acquire and reverse original DMC3 PS2 `SLUS/SLES/ELF` for game-specific PS2 limits.

---

# 53. Proposed geometry stress ladder

A controlled original-game test should not jump directly to the ceiling.

Recommended sequence per output-layout class:

```text
10k vertices
15k
20k
25k
30k
35k
40k
45k
50k
55k
60k
65,000
65,535
```

For each build record:

- original SCM SHA;
- authored SCM SHA;
- object/mesh;
- source vertex count;
- reconstructed triangle count;
- output layout flags;
- projected ring bytes;
- pool allocation bytes;
- game boot/load result;
- room load result;
- first rendered frame;
- visual corruption;
- crash address;
- GPU removal/device error;
- FPS/frame time.

Topology must be varied independently because 65k vertices with frequent topology breaks can produce much less rendered triangle traffic than one continuous fan.

---

# 54. Proposed texture stress ladder

Test separately from geometry:

```text
1024×2048   retail-observed control
2048×2048
4096×4096
8192×8192
16384×16384
```

For DXT1 and DXT5 where descriptor/path permits.

Record:

- DDS dimensions;
- mip count;
- compressed bytes;
- PTX slot;
- resource creation result;
- SRV creation result;
- level/room load;
- corruption;
- GPU/system memory usage if instrumented;
- crash/failure address.

The static EXE guard only proves the loader permits the dimensions to reach the D3D11 creation layer.

---

# 55. Evidence artifacts in Rengine

Primary canonical ledger:

```text
docs/research/dmc3-resource-render-limits-2026-10-04.md
data/reverse/dmc3-resource-render-limits-20261004.json
```

Important dedicated evidence:

```text
data/reverse/dmc3-scm-runtime-object-98-producer-20261004.json
data/reverse/dmc3-scm-triangle-fan-draw-conversion-20261004.json
data/reverse/dmc3-scm-geometry-budget-envelope-20261004.json
data/reverse/dmc3-scm-vertex60-planner-headroom-20261004.json
data/reverse/dmc3-dds-full-corpus-census-20261004.json
data/reverse/dmc3-hd-dynamic-vertex-ring-caller-census-20261004.json
```

The obsolete intermediate:

```text
data/reverse/dmc3-scm-triangle-strip-draw-chunking-20261004.json
```

is retained only as correction history and marked `REJECTED_SUPERSEDED_NAMING`.

---

# 56. Current final conclusions

## Geometry

The strongest exact per-mesh serialized limit is:

**65,535 vertices**

but this is not a universal rendering-safe limit.

The HD render path converts SCM legacy geometry into independent triangle draws that consume a shared 3.75 MiB per-frame ring.

Practical geometry safety therefore depends more directly on:

```text
rendered triangle count
× output stride
+ other dynamic frame work
```

than on raw file vertex count alone.

## Allocation

SCM runtime resources are constrained by a dedicated contiguous pool, not the whole 256 MiB master arena.

Normal empty-pool usable capacity:

**5,236,736 B**

Alternate:

**4,185,600 B**

## Indexing

- serialized/runtime SCM generated index workspace: `u16`
- HD generic GPU backend also supports `R32_UINT`
- dynamic u16 and u32 buffers each contain **159,744 entries**

## Textures — HD

- largest bounded retail observation: **1024×2048 DXT5**
- EXE pre-create width/height checks: **16384×16384**
- canonical D3D feature level: **11_0**
- practical residency ceiling remains unproven.

## Textures — PS2

- GS specified dimension maximum: **1024×1024**
- GS local memory: **4 MiB**
- practical original-DMC3 PS2 texture budget is still open.

## Main unresolved quantity

The largest remaining obstacle to a defensible **practical polygon budget** is not the serialized format.

It is:

> exact shared dynamic-ring occupancy and execution order in a real gameplay frame, followed by controlled original-game stress tests.

---

# 57. Canonical rule going forward

No future DMC Rengine documentation should state:

```text
"DMC3 maximum polygons = X"
```

or:

```text
"DMC3 maximum texture = Y"
```

without naming the evidence domain.

Use explicit wording such as:

```text
SCM serialized maximum
SCM normalizer maximum
empty-pool allocator envelope
empty shared-ring arithmetic capacity
retail observed maximum
HD EXE validation ceiling
PS2 hardware ceiling
original-game stress-tested safe maximum
```

Only the final category — **original-game stress-tested safe maximum** — should be presented to end users as a practical authoring ceiling.

---

# 58. Snapshot authority

This document is a consolidated snapshot of:

```text
repository: VrUaCom/dmc-rengine-cpp
branch: reverse/dmc3-resource-render-limits-20261004
v22 research base: cf3c0a6e9045ed9594450409fb9442bf01e55714
```

Main current ledger schema:

```text
dmc-rengine.dmc3-resource-render-limits.v22
```

Where this document conflicts with older intermediate notes, the current branch ledger and explicit `REJECTED/SUPERSEDED` corrections are authoritative.


---

# 59. Continuation v21 — SHW ownership and three producer collision boundaries

**Date:** 2026-10-04. **Base:** `a6a740a27cc125bb92a2dd86e3c8c7458156140d`.
This pass uses the same hash-bound canonical EXE. The machine authority is now
`dmc-rengine.dmc3-resource-render-limits.v21`.

The earlier sections retain the v20 evidence history. Sections 59–64 supersede the
remaining generic-draw ownership frontier and qualify the three extent candidates.
No original-game runtime measurements were made in this pass.

## 59.1 Last generic draw is SHW shadow-volume rendering

The ownership chain is recovered directly:

```text
CDrawShadow attach 0x14008BC60
 -> binder 0x1403204B0
 -> allocation planner 0x14031FC40
 -> constructor 0x14031FD30
 -> compatibility command 0x58000000 at 0x14031FE4E
 -> packet command decoder 0x140032CC0
 -> command 0x58 selection at 0x140032D86
 -> call 0x140032D93 -> shadow batching 0x1400446F0
 -> draw 0x140044CDF -> shared-ring uploader
```

The constructor copies each SHW hull from `SHW+0x20+hullIndex*0x40` to a
`0xA0` runtime hull. It sign-extends vertex and triangle counts from source
`+0x00/+0x02` to runtime `+0x04/+0x08`.
`0x140320600` copies runtime triangle count to compatibility descriptor `+0x18`.

The serialized fields remain 16-bit raw fields. The recovered constructor and
planner interpret them as **signed 16-bit values**; `0x8000..0xFFFF` must not be
advertised as positive supported counts just because the current reader preserves
them in `uint16_t` containers. Positive count domain on this route is `0..32767`.

## 59.2 Exact shadow output formula

The visibility producer `0x1403204F0` sets a negative first marker for inactive
faces. Active faces receive three neighbor-visibility markers, each 0 or 1.
The draw helper skips negative first markers, emits one mandatory 72-byte group
per active face, and emits one additional 72-byte group for each zero marker.

Define:

- `A`: active faces;
- `E`: zero neighbor-visibility lanes across those active faces (`0..3*A`).

Then:

```text
groups       = A + E
vertices     = 6 * (A + E)
source bytes = 72 * (A + E)
ring bytes   = 72 * (A + E)
```

Draw flags are `0x2`, mode is `3`, and output stride is 12 bytes.
A face therefore contributes 72–288 bytes. These optional side groups follow
the light-dependent silhouette; they do not mean the SHW mesh has open topology.
Invocation counts and visibility vary with the frame and light.

# 60. SHW allocation route — separate from scratch and GPU ring

`CDrawShadow` also requests allocator selector `-2` at `0x14008BC95..0x14008BC97`.
It therefore competes for the routed pool used by SCM; it does not allocate its
runtime data from the static shadow batching scratch.

For `H` hulls and normal positive signed counts `V_i`, `T_i`, the planner is:

```text
plannedBytes = 0x280 + 0xA0*H + 0x20*sum(V_i + T_i + 6)
```

This includes two command/runtime domains. The constructor's per-hull, per-domain
term is `16*V + 16*T + 0x30 + 0x20 + 0x10`.
Pool alignment/block rounding remain allocator-router properties.

Machine-code controls: no hulls -> 640 B; one hull `(3,1)` -> 1,120 B;
two hulls `(4,4),(8,12)` -> 2,240 B; three hulls adding `(12,20)` -> 3,616 B.
The high-bit control `(0x8000,0x8000)` returns unsigned `4,292,871,136` after
signed arithmetic wraps; that is a rejection signal for positive authoring,
not a usable large allocation guarantee.

# 61. 128 KiB shadow scratch — protected-neighbor boundary proven

Scratch begins at `0x140BEB3F0`; the live renderer pointer is at `0x140C0B3F0`.
The distance is exactly `0x20000`.

Fresh cropped EXE execution proves:

- 1,820 groups use 131,040 bytes, leaving 32 bytes before that pointer;
- group 1,821 writes to the pointer's first dword at `0x140044A15`;
- with three side groups per active face, 455 active faces fit before the
  pointer and face 456 produces the first overlapping group;
- with only mandatory groups, 1,820 active faces fit and face 1,821 overlaps.

Additional fixtures use the visibility pattern of closed tetrahedra: one active
face and three inactive neighbors per tetrahedron. The 455/456-active-face
boundary is reproduced with 1,820/1,824 total input faces. This checks a marker
pattern consistent with closed topology; geometric classification and real-game
reachability of that pattern were not executed in these fixtures.

**Status:** `EXE_CONFIRMED` for the write conflict and producer formula.
The exact linker-declared array size remains `STRUCTURAL_CANDIDATE`.
`A+E <= 1820` is a necessary condition to avoid this specific neighbor conflict,
not an enforced engine cap or a sufficient whole-game safety condition.

# 62. Subcommand 12 — exact workload and missing lower guard

The caller's dispatch-table slot for effect type 12 selects `0x140319D60`.
This producer first replaces unsigned field values above 200 with 100. For the
resulting `a=record+0x2C`, `b=record+0x30`:

```text
sx = floor(64*a/100)
sy = floor(32*b/100)
```

For positive steps, the post-tested grid loops generate:

```text
records   = ceil(640/sx) * ceil(360/sy)
vertices  = 4 * records
ringBytes = 80 * records
```

There is no local lower-bound or backing-span guard in the recovered producer.
`a=0..1` gives `sx=0`; `b=0..3` gives `sy=0`. The corresponding loop has no
forward coordinate progress and continues writing; the isolated verifier stops
at its first known neighboring-pointer conflict.

| Input a/b | Steps sx/sy | Full formula records | Full formula bytes | Cropped EXE result |
|---|---|---:|---:|---|
| 100/100 | 64/32 | 120 | 9,600 | Completes |
| 60/60 | 38/19 | 323 | 25,840 | Completes |
| 50/50 | 32/16 | 460 | 36,800 | Neighbor conflict at record 410 |
| 200/200 | 128/64 | 30 | 2,400 | Completes |
| 201/201 | 64/32 after reset | 120 | 9,600 | Completes |

Record 410 starts at offset 32,720. Its write at record `+0x30`, instruction
`0x14031A153`, hits live pointer `0x140CD9AF0` exactly.
Therefore the 32 KiB separation is now an EXE-proven interference boundary.
409 complete records avoid that specific neighbor; the producer does not enforce
409 as a hard limit. The exact array declaration and retail parameter reachability
remain open. The `(100,100)` fixture is a control, not a claim of retail default.

# 63. Subcommand 11 — display grid and 64 KiB write conflict

Effect type 11 selects `0x140318EC0`; cached update `0x14031A830` also writes the
same backing. Both must be considered for ownership/instrumentation.
Display dimensions are copied by `0x140316360` from signed 16-bit fields in the
display context to `context+0x15D58/+0x15D5C`.

In the ordinary positive-step domain (verified shifts 3–6):

```text
sx = 1 << record[+0x58]
sy = 1 << record[+0x5C]
rows = floor(height/sy) + 2
recordsPerRow = floor(width/sx) + 3
ringBytes = 40 * rows * recordsPerRow
```

The formula includes the initial negative-coordinate row/column and one extra
terminal record per row. It applies to nonnegative dimensions and positive
signed strides without overflow; the x86 shift-count masking domain must be
handled separately for arbitrary raw values.

At display 640×360:

| x/y shifts | Records | Generated/ring bytes | Cropped EXE result |
|---|---:|---:|---|
| 6/6 | 91 | 3,640 | Completes |
| 5/5 | 299 | 11,960 | Completes |
| 4/4 | 1,032 | 41,280 | Completes |
| 3/3 | 3,901 | 156,040 | Neighbor conflict at record 1,639 |

At record 1,639, instruction `0x14031928F` writes `record+0x10` to scalar
`0x140CD1AD0`. That scalar is read by both the producer and cached updater.
1,638 complete 40-byte records stop before the scalar; there is no local guard
enforcing that count. The exact 64 KiB array declaration remains a candidate.

# 64. Validation, gate changes and next reverse targets

**27/27 cropped machine-code cases pass their assertions.** The verifier maps
the canonical PE and executes unmodified instruction ranges using Unicorn 2.1.4.
RNG/state helpers and the shadow matrix transform are explicitly stubbed; SHW
allocation-planner code executes without external stubs. These are machine-code
fixtures, not original-game gameplay or GPU stress tests.

Reproduce with Python, `pefile==2024.8.26`, and `unicorn==2.1.4`:

```sh
python tools/reverse/verify_dmc3_render_producer_boundaries.py \
  /path/to/canonical/dmc3.exe /path/to/output.json
```

Evidence and test receipt:

- `data/reverse/dmc3-hd-render-producer-boundaries-20261004.json`
- `data/reverse/dmc3-hd-render-producer-verification-20261004.json`
- `data/reverse/render-producer-boundaries-20261004/*.asm`

Closed in this pass:

- ownership/source-count provenance of `0x140044CDF`;
- exact SHW group/ring formula and allocation planner;
- conditional subcommand 11/12 workload formulas;
- specific machine-code write conflicts at all three known neighbors.

Still open:

- exact compiler/linker static-object extents;
- retail parameter, hull and light census proving which witnesses are reachable;
- cached-update call order, rebuild frequency and packet reuse per frame;
- original-game writes, crash behavior and actual shared-ring occupancy;
- authored geometry/DDS stress acceptance and practical residency;
- original PS2 executable reverse.

Next static work: trace effect-record initializers/serialized parameter admission
for types 11/12, inspect cached updater shape stability, and census SHW hulls plus
light-dependent silhouette counts. Next dynamic work: instrument producer end
pointers, the protected neighbor variables, and ring `+0x17A0` in retail frames.

---

# 65. Continuation v22 — effect defaults and parameter admission

**Date:** 2026-10-04. **Research base:** `cf3c0a6e9045ed9594450409fb9442bf01e55714`.
This pass retains the canonical executable hash and promotes the current ledger
to `dmc-rengine.dmc3-resource-render-limits.v22`. No original-game frame was run.

`0x140315BD0` selects a free record among 16 records of stride `0x280`, then
calls initializer `0x140316E10`. The initializer zeroes `0x280` bytes and dispatches
on `effectType-1` through the table at `0x140317398`.

| Effect | Initializer handler | Confirmed initial fields | Initial checksum span |
|---|---|---|---:|
| 11 | `0x1403171B7` | `+0x58=4`, `+0x5C=3`, `+0x60=0` | 80 B |
| 12 | `0x140317218` | `+0x2C=100`, `+0x30=100` | 120 B |

These are EXE initializer defaults, not a retail asset census. The v21 `(100,100)`
control for effect 12 is now independently connected to an actual initializer.

The parameter importer `0x14031F050` selects a typed handler through
`0x14031F778` and uses actual lookup `0x14031EC60` to obtain an existing record.

| Effect | Import handler | Source payload → runtime record | Write sites |
|---|---|---|---|
| 11 | `0x14031F0E5` | source `+0x1C/+0x20` → record `+0x58/+0x5C` | `0x14031F125/0x14031F12B` |
| 12 | `0x14031F373` | source `+0x00/+0x04` → record `+0x2C/+0x30` | `0x14031F38D/0x14031F393` |

Both copy complete raw dwords with no local minimum, shift-domain or backing-span
validation. Fixtures confirm values including zero and `0xFFFFFFFF` survive this
import. Effect 12's later `>200 ->100` producer normalization remains separate;
its zero-progress values still pass the importer. Effect 11's shift fields are
also unbounded here; x86 masking remains relevant for arbitrary raw values.

The recovered stream route `0x14031EE00` resolves a source payload through a
relative-offset table and reaches importer call `0x14031EFF9`. Calls at
`0x14031E67D/0x14031E722` provide additional importer routes.
This closes local static admission provenance. Which payload values occur in
retail files, and complete source-container validation, remain open.

# 66. Internal viewport provenance — startup is 512×256

Viewport selector `0x140337CD0` reads one of two three-entry pointer tables:
`0x1405D1B08` and `0x1405D1B50`. Each descriptor stores three u16 dimensions at
`+0x08/+0x0A/+0x0C`; the selector copies them to the display context at
`+0x20/+0x22/+0x24`.

| Table | Index | Descriptor | Width | Height | Third dimension |
|---|---:|---|---:|---:|---:|
| first | 0 | `0x1405D1AD8` | 512 | 256 | 224 |
| first | 1 | `0x1405D1AE8` | 512 | 256 | 224 |
| first | 2 | `0x1405D1AF8` | 512 | 512 | 448 |
| alternate | 0 | `0x1405D1B20` | 512 | 256 | 224 |
| alternate | 1 | `0x1405D1B30` | 512 | 256 | 224 |
| alternate | 2 | `0x1405D1B40` | 512 | 512 | 512 |

Startup call `0x140337C4C` supplies index 1 and selector 0, establishing
**512×256** on that route. `0x140316360` subsequently copies width/height into
effect context `+0x15D58/+0x15D5C`. The third dimension is not this grid's height.
These are compatibility dimensions; they must not be equated with the user's
HD window/output resolution. Selecting table index 2 in a fixture does not prove
that retail gameplay enters that mode.

At initializer shifts `4/3`, effect 11's formula gives:

| Internal dimensions | Rows × records per row | Theoretical records | Bytes | Cropped producer result |
|---|---:|---:|---:|---|
| 512×256 | 34×35 | 1,190 | 47,600 | Completes before known scalar |
| 512×512 | 66×35 | 2,310 | 92,400 | Scalar conflict at record 1,639 |
| 640×360 fixture | 47×43 | 2,021 | 80,840 | Scalar conflict at record 1,639 |

At 512×256, imported shifts `3/3` instead produce 2,278 records / 91,120 bytes
theoretically and reach the same conflict. Thus the default startup combination
is below that specific neighbor boundary; accepted raw parameters need not be.
No universal game-safe ceiling follows from either observation.

# 67. Cached effect 11 regenerates geometry; cache is not a draw-size limiter

The main effect processor begins at `0x1403153C0`. Its cached dispatch table
`0x140315708` uses `effectType-3`. Type 11 reaches `0x140315676` and calls
`0x14031A830` at `0x14031567C`; type 12 reaches the no-updater continuation
`0x14031568E` in this table.

For ordinary nonnegative dimensions and positive strides, the type 11 updater
writes the same 40-byte records as the initial producer:

```text
sx = 1 << field58
sy = 1 << field5C
bytes = 40 * (floor(height/sy)+2) * (floor(width/sx)+3)
```

It adds `field20/field24` to phase accumulators `field70/field74` and changes the
generated values. Phase and drift fixtures confirm these changes do not alter
the generated record count at fixed dimensions/strides.

The updater again starts at `0x140CC1AD0`. The first scalar overlap is now
independently proven at instruction **`0x14031AB97`**, record **1,639**, writing
four bytes to `0x140CD1AD0`. This differs from the initial producer's write site
`0x14031928F`, while sharing the same record boundary.

The original producer encodes generated byte length divided by four in a
`0x64` command at `0x1403194E0..0x140319511`. A separate indexed patch slot is
recorded for the updater. The updater rewrites only that patch command's two
dwords plus its command qword. It does not update the earlier registry/length
command or independently register a new generated span.

Fixtures execute the actual count-writer slice and then the actual updater:
the count command remains unchanged while geometry is regenerated. They use a
synthetic cache/packet and registry ID, not the complete packet allocation route.
Changing dimensions or strides behind a still-valid cache can therefore make
generated extent differ from its cached count. Ordinary parameter changes are
handled by the invalidation mechanism below; real mode-transition behavior is
still an open runtime/lifecycle question.

# 68. Cache checksum and its precise assurance boundary

`0x1403162E0` computes a wrapping 32-bit sum of rotated parameter dwords:

```text
N = floor(record.u16[+0x06] / 4)
checksum = sum(ROR32(record.u32[+0x18+4*i], i&31), i=0..N-1) mod 2^32
```

For type 11's initialized 80-byte span this covers `+0x18..+0x67`, including
both shifts, but excludes accumulators `+0x70/+0x74`. Effect-context width/height
are also outside this record checksum.

The processor compares the checksum with record `+0x08`. A mismatch sets dirty
mask `+0x0C=3`. For the current display lane, `0x140316A90` releases the cached
packet via `0x1402C6260` and clears its pointer at cache `+0x40+8*lane`. The
current lane's dirty bit is cleared; the other lane remains dirty for a later
visit. An absent current-lane packet selects rebuilding. A matching checksum and
present packet selects cache reuse. Both lanes and both effect types are tested.

This is a change detector, not an exact size guard. A deliberate type 11 fixture
changes shift `+0x58:4->3` and amplitude `+0x18:32->65568`. The two checksum
contributions cancel exactly: the shift change subtracts `0x10000`, while the
amplitude change adds `0x10000`. The checksum remains equal and the existing
cache is reused despite a changed shape parameter.

**Status:** `EXE_CONFIRMED` for this raw-input collision and branch outcome.
There is no claim that this combination occurs in retail assets or causes a
retail crash. For an Rengine authoring validator, checksum equality cannot stand
in for explicit dimensions, strides, byte count and backing-capacity validation.

# 69. v22 verification and remaining gates

**38/38 new machine-code fixtures pass.** The retained v21 receipt contains a
separate 27 passing cases; the counts are not presented as gameplay tests.

The new fixtures cover initializer defaults (2), viewport table selection (6),
raw parameter admission (8), cache invalidation (8), a checksum collision (1),
initial producers (7), and cached effect 11 (6).

External stubs are explicit: initializer memset, viewport state calls, cache
deallocation, effect 12 RNG, and security-cookie checking. The cached updater's
depth helper executes its actual no-camera path. Source parameters, cache objects,
display state and packet storage are controlled fixtures. No original-game frame,
retail asset corpus, GPU draw, allocation-failure behavior or crash was tested.

Reproduce with Python, `pefile==2024.8.26`, and `unicorn==2.1.4`:

```sh
python tools/reverse/verify_dmc3_effect_parameter_admission.py \
  /path/to/canonical/dmc3.exe /path/to/output.json
```

The adjacent v21 `verify_dmc3_render_producer_boundaries.py` provides the shared
hash-bound PE fixture helpers and initial producer runners.

Evidence:

- `data/reverse/dmc3-hd-effect-parameter-admission-20261004.json`
- `data/reverse/dmc3-hd-effect-parameter-verification-20261004.json`
- `data/reverse/effect-parameter-admission-20261004/*.asm`

Closed: initializer defaults; raw field import; viewport descriptors and startup
selection; cached effect 11 count/formula/conflict; normal two-lane invalidation;
the checksum's exact input domain and a reproducible collision.

Still open: retail effect parameter census; full source-container semantics;
mode/lifecycle transitions and invocation order; exact static object extents;
SHW hull/light silhouette corpus; actual shared-ring occupancy and original-game
stress/failure behavior; original PS2 executable limits.

Next useful static target: identify the imported effect stream's resource owner
and census its real type 11/12 parameter records. Next dynamic target: record
internal viewport, initial/cached producer end pointers, cached encoded length,
neighbor scalar/pointers and shared-ring occupancy together in original frames.


# 70. Continuation v23 — bounded retail SHW topology and render-budget census

**Date:** 2026-10-05. This pass closes the bounded SHW corpus side of the shadow scratch question without requiring a guessed light direction.

The exact historical archive `DMC 3 RENGINE (6).zip` contains **16 unique SHW payloads**. The corpus totals are:

- **158 hull records**;
- **1,882 vertices**;
- **3,104 triangles**;
- **165 disconnected closed components**.

Every triangle edge in every hull belongs to exactly two triangles, and every serialized adjacency triplet equals the complete three-edge neighbor set. All hulls satisfy the stronger component-aware identity:

```text
T = 2*V - 4*C
```

where `C` is the number of disconnected components in that hull record. Five hull records contain multiple components (2 or 3); this explains the earlier apparent failures of the single-component `T=2V-4` rule. In this bounded corpus the components are closed genus-zero triangulated surfaces.

## 70.1 Light-independent SHW draw bound

The EXE-confirmed shadow helper uses:

```text
groups = A + E
ringBytes = 72 * (A + E)
```

where `A` is active face count and `E` is the count of active/inactive neighbor edges that generate silhouette-side groups.

For a closed triangular adjacency graph:

```text
E <= 3*A
E <= 3*(T-A)
```

Therefore, regardless of the binary active/inactive face assignment:

```text
A + E <= 2*T
ringBytes <= 144*T
```

This is a topological upper bound; real light-facing states can only reduce it.

## 70.2 Retail maxima

The largest SHW file by triangles is `em035_021.shw`:

- 17 hulls;
- 251 vertices;
- **434 triangles**;
- EXE-planner runtime allocation: **28,544 bytes**;
- whole-file shared-ring upper bound, if all hull draws occur in one rendered frame: **62,496 bytes** = about **1.59%** of the 3.75 MiB dynamic vertex ring.

The largest single observed hull contains **116 triangles / 60 vertices**. Because the command descriptor and `0x1400446F0` invocation are hull-scoped, its light-independent per-invocation bound is:

```text
max groups <= 232
scratch/ring bytes <= 16,704
```

That is only **12.74%** of the 128 KiB scratch-neighbor separation.

The first known neighboring-pointer conflict occurs at generated group **1,821**. Since any closed hull obeys `groups<=2*T`, a hull needs at least **911 triangles** even to be structurally capable of reaching that conflict. The largest retail hull observed here has only 116 triangles.

Therefore:

> No SHW hull in this bounded 16-file retail corpus can reach the known 128 KiB scratch-neighbor conflict under any binary face-activity pattern, independent of light direction.

This is stronger than a sampled light-silhouette census, but it remains bounded to this archive and does not define a universal authored-SHW maximum.

**Status:** `CORPUS_CONFIRMED_BOUND_BELOW_CONFLICT + EXE_CONFIRMED_FORMULA_DERIVED_BOUND`.

Reproducer:

`scripts/reverse/census_shw_render_budget.py`

Machine evidence:

`data/reverse/dmc3-shw-retail-render-budget-20261005.json`


# 71. Continuation v24 — EXE effect tags and bounded retail HTH parameter witness

**Date:** 2026-10-05. This pass connects the numeric type 11/12 runtime work to
their EXE-confirmed resource tags and to a real hash-bound retail text witness.

The tag mapper at `0x14031E730..0x14031E9FD` compares four-byte NUL-terminated
IDs from `0x140507CE4..0x140507D1C`. The complete recovered mapping is:

| Tag | Runtime type |
|---|---:|
| DOF | 2 |
| HTH | **11** |
| BGA | 8 |
| BGO | 9 |
| CCT | 5 |
| SGA | 7 |
| RDN | **12** |
| ATF | 1 |
| GFG | 4 |
| DGO | 10 |
| FCS | 3 |
| LGA | 6 |
| PDM | 13 |
| EDT | 14 |
| EAA | 15 |

**Status:** `EXE_CONFIRMED`.

For HTH/type 11, importer handler `0x14031F0E5` copies source dwords
`+0x1C/+0x20` directly to runtime record `+0x58/+0x5C`. The bounded archive
contains one exact text witness:

`analysis_inputs/stage_drops/m20_c00/m20_c00_002/m20_c00_002_083.txt`

SHA-256:
`afc99521aa3b44e25b9af3fc24623c1f421e85bb4b102d4c4124d7256a287ea3`

Its HTH block contains:

```text
Id HTH
StartZ 300
EndZ 360
Color 1082163328
AmplitudeH 16
AmplitudeV 16
SpeedH 12
SpeedV 12
DetailH 6
DetailV 3
Reduction 0
OtType 0
```

The source-field order and importer writes connect:

```text
DetailH = 6 -> runtime +0x58 = 6
DetailV = 3 -> runtime +0x5C = 3
```

Using the EXE-confirmed startup compatibility viewport **512×256** and the
already recovered type-11 producer formula:

```text
sx = 1 << 6 = 64
sy = 1 << 3 = 8
rows = floor(256/8) + 2 = 34
recordsPerRow = floor(512/64) + 3 = 11
records = 374
ringBytes = 40 * 374 = 14,960
```

Thus this real retail HTH witness uses **14,960 generated/ring bytes**, about
**22.83%** of the first known 64 KiB interference separation and leaves 50,576
bytes before that neighbor boundary. This is not a universal HTH maximum; it is
an exact retail witness projected through the EXE-confirmed importer, viewport
and producer formula.

No exact `Id RDN` text witness was found in the bounded archive text population.
That is a bounded negative only. It does **not** prove RDN/type 12 is absent from
the full retail game or from packed resources outside this archive.

**Status:** HTH witness `EXE_AND_CORPUS_CONFIRMED`; RDN absence
`BOUNDED_CORPUS_NEGATIVE`.

Reproducer:

`scripts/reverse/census_dmc3_effect_retail_parameters.py`

Evidence:

`data/reverse/dmc3-hd-effect-retail-parameter-census-20261005.json`

Next target: identify the exact serialized/packed owner feeding the stream table
at runtime owner `+0xF8`, then expand the census beyond loose text witnesses and
recover any packed RDN/type-12 records.


# 72. Continuation v25 — demo clip effect-stream manager ownership

The runtime owner behind the effect-stream `+0xF8` pointer is now closed one
level further.

Binder `0x14031EDB0` performs:

```text
if resource == null:
    fail

manager+0xF8 = resource
manager.u16+0x00 = resource.u16+0x04

require resource.dword+0x04 >= 1
require resource.dword+0x08 != 0
firstRecord = resource + resource.dword+0x08
require firstRecord.byte+0x06 == 1

otherwise:
    manager+0xF8 = null
    fail
```

The two direct stream-admission callsites in the complete executable are:

```text
0x14023BA03 -> 0x14031EE00
0x14023CBD5 -> 0x14031EE00
```

Both initialize the same manager object at caller `+0x1B0A0` through binder
calls `0x14023B9C3` / `0x14023CBC7` immediately before admission.

Separately, parser `0x1402DA750` initializes the canonical TXT parser helpers
`0x140322CB0/0x140322CA0`, consumes `Clip`, `SetFrame`, `SkipFrame`,
`CutFrame`, `ClipScale`, `ChangeType`, `Life`, `Id` and `Param`,
and uses the `/demo/%s/%s` resource domain. Its `Id` path calls the recovered
effect-tag mapper `0x14031E730`.

Therefore the `+0xF8` stream consumed by `0x14031EE00` is no longer an
anonymous auxiliary stream: it belongs to the **demo clip screen-effect**
processing domain and is installed from an offset-backed resource object.

**Status:** `EXE_CONFIRMED_DEMO_CLIP_EFFECT_STREAM_OWNER`.

Still open: the exact outer PAC/PNST/NBZ slot/container that materializes this
resource pointer. Closing that boundary is required before claiming a complete
resource path from archive slot to HTH/RDN runtime record.

Evidence:

`data/reverse/dmc3-hd-demo-effect-stream-owner-20261005.json`


## 0x140044CDF exact producer-to-draw byte propagation

A direct canonical-EXE pass closes the previously generic byte-count propagation at
`0x140044CDF`.

Producer `0x1400446F0` initializes:

```text
r15 = 0x140BEB3F0
r12d = 0
```

For every emitted scratch group it writes exactly `0x48 = 72` bytes, advances:

```text
r15 += 0x48
r12d += 6
```

and may emit one mandatory group plus up to three conditional groups for each
outer input item. At the terminal draw setup:

```text
eax = 3 * r12d
eax <<= 2
r9  = sign_extend(eax)
source = 0x140BEB3F0
call 0x140043F90
```

Therefore:

```text
drawByteCount = 12 * generatedVertexCount
              = 72 * generatedGroupCount
              = generatedEnd - 0x140BEB3F0
```

for the aligned producer. This is now **EXE_CONFIRMED_EXACT_BYTE_PROPAGATION**,
not a generic unknown-count draw.

The outer iteration count is read from `input+0x18`. Static analysis has not
yet recovered a producer-side clamp for that field. In the worst branch pattern
one outer item can emit four groups:

```text
maxGeneratedBytesPerInputItem = 4 * 0x48 = 288
maxGeneratedVerticesPerInputItem = 24
```

If the previously inferred `0x20000` contiguous referenced-global interval
were the true scratch extent, 455 worst-case outer items would consume 131,040
bytes and 456 would require 131,328 bytes. However the base
`0x140BEB3F0` is also referenced by multiple legacy compatibility handlers, so
absence of another referenced global inside that interval is **not sufficient
ownership evidence** for a dedicated 128 KiB array.

Correction to the earlier candidate wording:

- exact generated-byte propagation: **EXE_CONFIRMED**;
- `0x20000` dedicated scratch extent: remains **STRUCTURAL_CANDIDATE**, with
  weaker ownership confidence than a true symbol/initializer/bounds proof;
- 10,920 generated vertices / 1,820 groups: **not an engine limit**;
- hard maximum remains open until the upstream producer of `input+0x18` or an
  explicit scratch bound is recovered.


## SHW triangle-count provenance and render expansion budget

A canonical-EXE pass closes the upstream count used by
`0x1400446F0`.

### Serialized -> runtime -> render-count chain

SHW runtime builder `0x14031FD30` reads each record:

```text
source +0x00 -> movsx WORD -> runtimeHull+0x04  (vertex count)
source +0x02 -> movsx WORD -> runtimeHull+0x08  (triangle count)
```

The use of `movsx` is significant: the canonical HD runtime interprets these
two physical 16-bit lanes as signed values when building the SHW runtime
record. Positive runtime counts therefore occupy the domain `0..32767`, not
the full unsigned `0..65535` range.

During SHW runtime update `0x1403204F0`, the code loads:

```text
eax = runtimeHull+0x08
rcx = runtimeHull+(frame-domain 0x88/0x90) descriptor
descriptor+0x18 = eax
```

at `0x1403205F0..0x140320600`.

The compatibility packet interpreter later invokes `0x1400446F0` for the
SHW `0x58` render path. That helper uses `descriptor+0x18` as its outer
iteration count.

Therefore the count chain is now closed:

```text
SHW record +0x02
 -> signed 16-bit triangle_count
 -> runtimeHull+0x08
 -> runtime render descriptor+0x18
 -> 0x1400446F0 outer triangle loop
```

Status:
**EXE_CONFIRMED_SHW_TRIANGLE_COUNT_TO_RENDER_LOOP**.

### Expansion semantics

For each active SHW triangle, `0x1400446F0` emits one mandatory
`0x48`-byte / six-vertex group and up to three additional groups depending
on the three per-triangle selector lanes. Thus:

```text
groupsPerActiveTriangle = 1 + zero(selector0) + zero(selector1) + zero(selector2)
range = 1..4

generatedVertices = 6 * groups
generatedBytes = 72 * groups
```

Dynamic rejection/culling may remove a triangle entirely, so the all-visible
source-selector projection is a deterministic static upper bound for a given
retail hull.

### Bounded SHW corpus

The exact historical archive
`DMC 3 RENGINE (6).zip`
(SHA-256 `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`)
contains:

- 16 SHW paths;
- 16 unique SHW payloads;
- 158 hull records;
- 1,882 serialized vertices;
- 3,104 serialized triangles;
- maximum observed hull vertex count: **60**;
- maximum observed hull triangle count: **116**.

All observed counts are positive and far below the signed-16 runtime boundary.

Largest all-visible per-hull render expansion:

`em035_023.shw`, hull 0
(SHA-256 `b51e70ce704205f0a4b00fdf5418db718ac800e5c508b366eb69c5ae6e93c5d6`):

- 60 vertices;
- 116 triangles;
- all 60 transform selectors are zero;
- 464 generated groups;
- **2,784 generated vertices**;
- **33,408 generated/ring bytes**.

Largest aggregate all-visible projection for one SHW file in the bounded
archive:

`em035_022.shw`
(SHA-256 `dc7884346b35c76a83c246b07bb5e616b8ea48f60dc591e2b4b549ef7e7da598`):

- 5 hulls;
- 162 triangles;
- 648 generated groups;
- **3,888 generated vertices**;
- **46,656 generated/ring bytes**.

The per-file figure assumes each hull is submitted once and all source-visible
triangles survive dynamic rejection; it is a corpus-side upper projection, not
a measured frame occupancy.

### 128 KiB scratch candidate demotion

The earlier `0x20000` gap after `0x140BEB3F0` is not sufficient to prove
a dedicated 128 KiB scratch object. The base lies in the large virtual/BSS
portion of the executable's `.data` section and is referenced by multiple
legacy compatibility paths. The exact SHW count provenance now shows that a
true hard limit must come from an explicit bounds check, initializer, or
upstream allocation contract, not merely the next referenced global address.

Therefore:

- exact SHW count/expansion path: **EXE_CONFIRMED**;
- retail SHW expansion maxima above: **CORPUS_CONFIRMED**;
- dedicated 128 KiB scratch extent: **PRESERVED AS STRUCTURAL_CANDIDATE ONLY**;
- 10,920 vertices / 1,820 groups: **REJECTED as an engine-limit claim**.
