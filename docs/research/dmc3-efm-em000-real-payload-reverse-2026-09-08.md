# DMC3 HD EFM — em000 real-payload binding (2026-09-08)

**Branch:** `reverse/efm-em000-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Bound payload:** `em000_023.efm`, 11,584 bytes  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Conclusion

`em000_023.efm` closes the major gap between the already recovered EFM executable handler and a real retail payload.

EFM is a **mesh-bearing effect-model format**, not generic effect metadata and not a MOD file with a different extension. It belongs to the related DMC3 model-document family but has a format-specific stream at mesh `+0x38` that binds naturally to the EFM shader's extra `COLOR0` vertex input.

Safe architecture:

```text
model-family common ABI
    +-- MOD adapter
    +-- SCM adapter
    +-- EFM adapter   <-- this branch
```

No EFM field may inherit a MOD semantic merely because offsets overlap.

## Executable authority already present

The canonical executable independently confirms:

```text
registry content tag: EFM
container handler:     0x1402F7A90
family mask:           0x20000000
shared MOD/EFM transform initializer: 0x1402FA080
```

The EFM post-load handler traverses the same related high-level shell:

```text
header 0x40
outer records 0x40
inner/mesh records 0x50
```

and relocates mesh pointers at:

```text
+0x10
+0x18
+0x20
+0x28
+0x30
+0x38
```

while `+0x40` is inner-record-relative runtime/generated workspace and `+0x48` receives generated topology count.

The executable also embeds `DMC3_EFM*.hlsl` input with:

```text
POSITION
NORMAL
TEXCOORD0
BLENDINDICES
PSIZE
COLOR0
```

The extra `COLOR0` relative to MOD is direct semantic evidence for a format-specific per-vertex colour channel.

## Real em000 header

`em000_023.efm` begins with:

```text
+0x00 "EFM "
+0x04 revision bytes compatible with the 1.01 model-family revision representation
+0x10 outer_count              = 2
+0x11 node_domain_count        = 5
+0x12 texture_slot_domain      = 3
+0x13 runtime-carried byte     = 1
+0x14 raw metadata u32         = 100502 decimal
+0x20 node-domain block offset = 0x2690
```

The decimal shape of `100502` resembles the arithmetic resource-code patterns observed in MOD/SCM, but **EFM high-level semantics are not promoted from similarity alone**.

## Outer/object records

Two 0x40 records are present.

Observed bounded fields:

```text
outer 0 @ 0x40
    mesh_count          = 1
    aggregate vertices  = 44
    mesh_table_offset   = 0x00C0

outer 1 @ 0x80
    mesh_count          = 1
    aggregate vertices  = 205
    mesh_table_offset   = 0x07A0
```

The aggregate counts match their single child mesh counts.

## EFM mesh 0

0x50 mesh record at `0x00C0`:

```text
vertex_count  = 44
texture_slot  = 2

+0x10 0x0110  position stream
+0x18 0x0320  normal stream
+0x20 0x0530  fixed-width UV stream
+0x28 0x05E0  BLENDINDICES-correlated stream
+0x30 0x0690  packed topology/control stream
+0x38 0x06F0  EFM extra colour stream
+0x40 0x26B0  generated-workspace relative field candidate
```

Exact physical span relationships for the source streams are consistent with:

```text
position       = count * 12 bytes
normal         = count * 12 bytes
UV             = count *  4 bytes
blend indices  = count *  4 bytes
topology ctrl  = count *  2 bytes (+ alignment as needed)
COLOR0         = count *  4 bytes
```

## EFM mesh 1

0x50 mesh record at `0x07A0`:

```text
vertex_count = 205

+0x10 0x07F0
+0x18 0x1190
+0x20 0x1B30
+0x28 0x1E70
+0x30 0x21B0
+0x38 0x2350
```

The next major structural region is the node-domain block at `0x2690`, and the same stream-width relationships close cleanly before it.

## `+0x38` colour binding

The real payload materially strengthens the existing EXE/HLSL hypothesis:

- mesh `+0x38` is a dedicated per-vertex 4-byte stream;
- its length is exactly `vertex_count * 4` in both bound meshes;
- bytes are colour-shaped rather than pointer/index-shaped;
- observed alpha is `128` in the current sample;
- RGB values vary meaningfully across the first mesh and use repeated palette-like values in the second;
- the EFM shader independently requires `float4 rgba : COLOR0`.

Safe promotion:

> EFM mesh `+0x38` is the serialized per-vertex colour channel feeding the EFM `COLOR0` input for this bound layout.

Status: **EXE_AND_CORPUS_CONFIRMED at the stream-role level for the bound payload**, while exact normalization/conversion rules still require runtime trace or shader-path binding.

## Node-domain binding

At `0x2690`, the observed related model-family node domain exposes relative sub-offsets including:

```text
0x20
0x28
0x30
0x40
```

The first observed arrays include:

```text
parent-like bytes: ff 00 01 02 03
order bytes:       00 01 02 03 04
```

and the transform region beginning at relative `0x40` has five 0x20-byte local-transform records, consistent with the recovered MOD/EFM transform infrastructure.

The parent/order semantic labels remain adapter evidence targets until the EFM-specific consumer is fully rebound; no MOD-only field semantics are copied blindly.

## Required C++20 module boundary

EFM must become its own format adapter:

```text
include/dmc_rengine/formats/efm.hpp
src/formats/efm.cpp
analysis/efm/...
tests/efm_tests.cpp
```

It should reuse only evidence-backed common model-family ABIs such as:

```text
model_document_core
model_object_core
model_mesh_core
model_node_domain_core
```

and then add EFM-specific fields/streams explicitly.

Initial parser requirements:

1. require `EFM ` identity;
2. validate the 0x40 header and 0x40/0x50 table topology;
3. preserve all unknown header/object/mesh bytes;
4. validate all source stream pointers and extents;
5. expose `+0x38` as EFM vertex colour only where the layout validates;
6. reconstruct topology using the EFM-specific post-load contract rather than calling MOD blindly;
7. validate node-domain bounds independently;
8. remain structural/read-only.

## Open reverse gates

- bind the real payload field-by-field against `0x1402F7A90`;
- recover exact EFM vertex-colour normalization and alpha semantics;
- determine whether EFM uses the same skin packing as MOD for every revision;
- bind texture companion ownership and texture-slot-domain behavior;
- recover the meaning of header `+0x13/+0x14` for EFM specifically;
- acquire additional EFM payloads across effects/enemies/stages;
- determine revision variation;
- only after multi-corpus validation consider writer research.

## Evidence status

| Claim | Status |
|---|---|
| EFM runtime identity | `EXE_CONFIRMED` |
| EFM mesh-bearing purpose | `EXE_CONFIRMED` |
| real `em000_023.efm` conforms to related 0x40/0x40/0x50 shell | `CORPUS_CONFIRMED` |
| mesh `+0x38` is count*4 extra vertex stream | `CORPUS_CONFIRMED` |
| extra stream corresponds to EFM `COLOR0` | `EXE_AND_CORPUS_CONFIRMED` for bound layout |
| complete field semantics | `PARTIAL` |
| writer authority | `NOT AUTHORIZED` |
