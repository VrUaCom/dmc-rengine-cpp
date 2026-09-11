# DMC3 HD MOD — mesh +0x0C canonical runtime dormancy closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Evidence follow-up:** 2026-09-11

## Serialized ABI

```text
mesh +0x00 u16 vertex/element count
mesh +0x02 u16 texture slot
mesh +0x04..+0x0A four u16 GS CLAMP values
mesh +0x0C u32 raw_0c / unresolved
mesh +0x10 first u64 stream pointer
...
mesh +0x40 u64 topology-workspace pointer/offset field
mesh +0x48 u32 generated topology count
mesh +0x4C u32 raw_4c / unresolved
mesh +0x50 next record
```

`+0x0C` is `STRUCTURAL_CONFIRMED` as a little-endian `u32`. Its serialized semantic remains unresolved.

## Corpus

Current bounded evidence:

```text
MOD  180/180 meshes -> +0x0C == 0
EFM    2/2 meshes   -> +0x0C == 0
```

This is `CORPUS_CONFIRMED` only. Zero is not padding/reserved proof.

## Whole-executable direct serialized-mesh derivation census

The canonical executable was scanned by PE `.pdata` function boundaries for the exact mesh-table derivation used by this ABI:

```text
mesh_record = object.mesh_table(+0x08) + mesh_index * 0x50
```

The whole image contains exactly six functions with this exact `0x50` stride plus `object +0x08` table-pointer derivation:

```text
0x1402F7A90
0x1402F7D60
0x1402FDB40
0x1402FDD10
0x1402FE3B0
0x1402FE6A0
```

The two `0x1402F7...` functions are the EFM load/build sibling paths; `0x1402FE3B0` and `0x1402FE6A0` are the canonical MOD post-load/runtime-builder pair. The two middle functions derive the same 0x50-byte records for layout/planning work. None of the six reads or writes serialized `+0x0C`.

Positive neighboring accesses prove that the census is following the real source record rather than an unrelated 0x50-byte structure. The MOD builder reads `+0x00`, `+0x02`, the stream fields, `+0x40`, and `+0x48`; the post-load path relocates the pointer fields and generates `+0x48`.

## Runtime-mesh backreference

Canonical MOD runtime builder `0x1402FE6A0` preserves the source record pointer:

```text
0x1402FE6F4..0x1402FE707  derive serialized mesh
0x1402FE713              serialized mesh -> runtime mesh +0x10
```

This backreference is live. Canonical post-builder consumers dereference it for material, topology and texture information.

A whole-executable structural census for the runtime-mesh array derivation:

```text
runtime_mesh = object.runtime_mesh_array(+0x20) + mesh_index * 0x1A0
```

finds exactly **34 functions**. Conservative typed propagation from all 34 roots covers **43 reachable functions** and **20 typed direct calls**. It finds:

```text
serialized-source pointer escapes    0
serialized-source indirect calls     0
```

Two roots outside the earlier local model range, `0x14030D9B0` and `0x14030DA80`, pass the runtime mesh to `0x14030D8B0`; that helper operates on runtime stream buffers at `+0x160` and never dereferences runtime `+0x10`.

## Live backreference consumer census

The provenance-confirmed canonical consumers of `runtime mesh +0x10` expose the following serialized fields:

```text
+0x00  6 reads
+0x02  2 reads
+0x04  2 reads
+0x06  1 read
+0x08  1 read
+0x0A  1 read
+0x40  3 reads
+0x48  5 reads
----------------
       21 reads total
```

Representative paths:

- `0x1402F9890` reads texture slot `+0x02` and GS CLAMP `+0x04/+0x06/+0x08/+0x0A`;
- `0x140307230` independently reads texture slot `+0x02`;
- `0x1402F8000` and MOD sibling `0x1402FE930` read `+0x00/+0x40/+0x48`;
- `0x140309C60` reads `+0x00/+0x40/+0x48`, including comparisons of `+0x48` against `0xA2` and `0x16`.

Across this live backreference surface:

```text
serialized mesh +0x0C reads   0
serialized mesh +0x0C writes  0
```

The common material helper is a particularly strong positive control: it consumes every preceding material/CLAMP field through `+0x0A` and stops before `+0x0C`.

## Canonical closure

The direct table census and the independent live-backreference census cover both ways the canonical executable retains or reconstructs the serialized mesh pointer. No source pointer escapes from the typed surface, and no indirect call receives the serialized source pointer.

The evidence-safe conclusion is therefore:

```text
serialized offset/width                       STRUCTURAL_CONFIRMED
bounded zero histogram                        CORPUS_CONFIRMED
whole-image direct mesh-table census          EXE_CONFIRMED
whole-image runtime-backreference census      EXE_CONFIRMED
canonical runtime effect                      EXE_CONFIRMED: dormant / no effect
serialized high-level semantic                PRESERVED_UNDECODED
writer policy                                 preserve exact source u32
```

“Dormant” is a canonical-runtime behavioral conclusion only. It is not a serialized name such as `padding` or `reserved`.

## C++ / regression contract

`include/dmc_rengine/analysis/mod/mesh_serialized.hpp` keeps `+0x0C` in the raw preservation projection.

`tests/mod_skin_tests.cpp` contains a synthetic non-zero regression:

```text
mesh +0x0C = 0xDEADBEEF
parse/preservation projection -> 0xDEADBEEF
```

That regression remains mandatory even after canonical runtime non-consumption is closed.

## Rejected hypotheses

- `+0x0C` is alignment padding because it precedes an aligned u64 — `REJECTED`;
- `+0x0C` is reserved because every bounded corpus value is zero — `REJECTED`;
- canonical runtime dormancy authorizes writer zero-normalization — `REJECTED`;
- unrelated object/packet accesses at displacement `+0x0C` are serialized MOD mesh consumers — `REJECTED` without source-mesh provenance.
