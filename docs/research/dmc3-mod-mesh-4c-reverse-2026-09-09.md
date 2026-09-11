# DMC3 HD MOD — mesh +0x4C canonical runtime dormancy closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Evidence follow-up:** 2026-09-11

## Serialized ABI

```text
mesh +0x40 u64 topology-workspace pointer/offset field
mesh +0x48 u32 generated topology count
mesh +0x4C u32 raw_4c / unresolved
mesh +0x50 next record
```

`+0x4C` is `STRUCTURAL_CONFIRMED` as a little-endian `u32`. Its serialized semantic remains unresolved.

`+0x48` is a separate serialized/runtime-generated topology count field. It is **not** the physical workspace capacity `align16(6*(vertex_count-2))`; that span belongs to the workspace associated with `+0x40`.

## Corpus

```text
MOD  180/180 meshes -> +0x4C == 0
EFM    2/2 meshes   -> +0x4C == 0
```

This is bounded `CORPUS_CONFIRMED` evidence only.

## Positive control: serialized +0x48

Canonical MOD post-load and runtime builder establish a live neighboring field:

```text
0x1402FE684  write generated topology count -> serialized mesh +0x48
0x1402FE904  read serialized mesh +0x48
0x1402FE907  write count -> runtime mesh +0x198
```

Later post-builder paths also read the same serialized `+0x48` through the live source-mesh backreference. Therefore the lack of a `+0x4C` consumer is not an artifact of stopping the census before the end of the 0x50-byte record.

## Whole-executable direct serialized-mesh derivation census

The canonical executable contains exactly six functions with the exact serialized mesh-table derivation:

```text
mesh_record = object.mesh_table(+0x08) + mesh_index * 0x50

0x1402F7A90
0x1402F7D60
0x1402FDB40
0x1402FDD10
0x1402FE3B0
0x1402FE6A0
```

No function in this complete direct-derivation set reads or writes serialized `+0x4C`. The MOD post-load path reaches and writes `+0x48`, then advances by the 0x50-byte record stride without touching `+0x4C`. The MOD runtime builder reads and forwards `+0x48` but performs no `+0x4C` companion transfer.

## Runtime-mesh backreference census

`0x1402FE713` stores the serialized mesh pointer into `runtime mesh +0x10`. The backreference remains live after builder completion.

A whole-executable census of the runtime-mesh derivation `mesh_index * 0x1A0 + object.runtime_mesh_array(+0x20)` finds exactly **34 functions**. Typed propagation covers **43 reachable functions** and **20 direct typed calls** with:

```text
serialized-source pointer escapes    0
serialized-source indirect calls     0
```

Live backreference consumers produce 21 provenance-confirmed serialized field reads:

```text
+0x00  6
+0x02  2
+0x04  2
+0x06  1
+0x08  1
+0x0A  1
+0x40  3
+0x48  5
```

There are zero reads or writes of serialized `+0x4C`.

The strongest positive control is `0x140309C60`, which reads `+0x48` repeatedly after following `runtime mesh +0x10`, including comparisons against `0xA2` and `0x16`, while never accessing adjacent `+0x4C`.

## Raw +0x4C displacement control

A model-region raw displacement scan found five non-stack `+0x4C` reads:

```text
0x1402F78DE
0x14030134F
0x140305FAF
0x14030806E
0x1403086A6
```

Pointer provenance rejects all five as serialized mesh accesses. They belong to global statistics/control blocks (`0x1405D9D98` / `0x1405D9D68`) and related aggregate counters. Equal displacement does not establish structure identity.

Likewise, writes such as `0x1402FF35A` target generated runtime/draw structures, not the serialized source mesh.

## Canonical closure

The evidence-safe result is:

```text
serialized offset/width                       STRUCTURAL_CONFIRMED
bounded zero histogram                        CORPUS_CONFIRMED
whole-image direct mesh-table census          EXE_CONFIRMED
whole-image runtime-backreference census      EXE_CONFIRMED
neighbor +0x48 live positive control          EXE_CONFIRMED
canonical runtime effect                      EXE_CONFIRMED: dormant / no effect
serialized high-level semantic                PRESERVED_UNDECODED
writer policy                                 preserve exact source u32
```

Canonical runtime dormancy does not establish `padding`, `reserved`, or a second topology-count word.

## C++ / regression contract

`include/dmc_rengine/analysis/mod/mesh_serialized.hpp` preserves `+0x4C` as raw source data.

The registered synthetic regression requires:

```text
mesh +0x4C = 0xA5A55A5A
parse/preservation projection -> 0xA5A55A5A
```

No writer may synthesize zero for this field solely because canonical retail samples are zero and the canonical runtime does not interpret it.

## Rejected hypotheses

- `+0x4C` is a companion/second half of generated topology count `+0x48` — `REJECTED`;
- `+0x4C` is padding because it terminates the 0x50-byte record — `REJECTED`;
- raw `+0x4C` accesses in unrelated runtime/statistics layouts are MOD serialized mesh evidence — `REJECTED`;
- canonical runtime dormancy authorizes zero-normalization — `REJECTED`.
