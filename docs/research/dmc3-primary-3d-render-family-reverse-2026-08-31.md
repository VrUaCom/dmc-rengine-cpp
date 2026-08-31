# DMC3 primary 3D / render family reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM — reconciled with EXE format census  
**Target:** canonical unpacked DMC3 HD analysis executable  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

Machine-readable authorities:

- [`../../data/reverse/dmc3-primary-3d-family-20260831.json`](../../data/reverse/dmc3-primary-3d-family-20260831.json)
- [`../../data/reverse/dmc3-runtime-type-identification-20260831.json`](../../data/reverse/dmc3-runtime-type-identification-20260831.json)
- [`../../data/reverse/dmc3-exe-format-census-20260831.json`](../../data/reverse/dmc3-exe-format-census-20260831.json)

## 1. Current canonical classification

| Family | Runtime identity | 3D/model conclusion | Current boundary |
|---|---|---|---|
| `MOD` | EXE-confirmed by 3-byte registry, container handler and 4-byte family mask | **mesh-bearing actor/object model** | full variants/writer partial; 3-byte identity is broader than canonical `MOD ` mesh-layout proof |
| `EFM` | EXE-confirmed by 3-byte registry, container handler and 4-byte family mask | **mesh-bearing effect model** | real retail EFM needed for exact stream-to-shader binding |
| `SCM` | EXE-confirmed by 3-byte registry, container handler and 4-byte family mask | **mesh-bearing stage/scene model** | full material/ownership/writer schema partial; 3-byte identity is broader than canonical `SCM ` mesh-layout proof |
| `MRP` | EXE-confirmed by 3-byte registry + 4-byte family mask | **model/render-side companion identity; standalone mesh not proven** | no generic handler/factory specialization established; exact schema open |
| `MCV` | EXE-confirmed by exact `MCV ` family mask + `.mcv` motion/control registration | **runtime motion/model companion identity; mesh ownership not proven** | exact fields and downstream owner open; no MOD/SCM-compatible decoder claim |
| `SHW` | EXE-confirmed by 3-byte registry, container handler and 4-byte family mask | **shadow geometry/topology companion** | triangle/index topology references external spatial pool; not a self-contained textured mesh |

The old shorthand `MOD + SCM = geometry; EFM/MRP/SHW = companions` is superseded. `EFM` is mesh-bearing. `MCV` is now an independently confirmed runtime family identity, but it is **not** promoted to a mesh family.

## 2. Runtime identity systems remain separate

```text
registry_content_probe @ 0x1402DB1F0
    width: 3 bytes
    MOD / EFM / SCM / MRP / SHW

container_dispatch @ 0x1401B9FA0
    MOD / EFM / SCM / SHW -> normal post-load handlers
    EFW / EFE              -> recognized sentinels, no normal handler established
    PNST                   -> exact four-byte recursion identity

family_mask_probe @ 0x1402FD650
    exact four-byte values, trailing ASCII space required
    MOD  -> 0x10000000
    EFM  -> 0x20000000
    SCM  -> 0x30000000
    MRP  -> 0x40000000
    MCV  -> 0x50000000
    SHW  -> 0x60000000

motion/control extension dispatcher @ 0x1402E01A0
    .mot -> 0
    .mcv -> 1
    .cam -> 2
    .hid -> 3
    .clt -> 4
    .tsc -> 5
```

This means `MCV` has two independent EXE-backed identity paths even though it is absent from the three-byte registry probe.

## 3. MOD / EFM / SCM related model-document family

Original normalizers:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
```

Direct reverse supports a related high-level topology:

```text
header
  count-like field near +0x10
  base-relative pointer near +0x20
outer records from +0x40, stride 0x40
inner records, stride 0x50
multiple relocated data/topology pointers
format-specific topology/fixup work
```

This is a related runtime document family, not one byte-identical schema.

### MOD

`0x1402FE3B0` relocates mesh-side pointers and rebuilds topology. Observed inner offsets include `+0x10`, `+0x18`, `+0x20`, `+0x28`, `+0x30`, generated output near `+0x40` and count near `+0x48`. The topology source is a `u16` stream with control bit `0x8000`.

MOD also has format-specific header behavior (`+0x11` compared with `1`) that must not be projected to EFM/SCM.

### EFM

`0x1402F7A90` traverses the related `0x40` outer / `0x50` inner topology and relocates streams at `+0x10`, `+0x18`, `+0x20`, `+0x28`, `+0x30`, `+0x38`. It rebuilds topology from the `+0x30` stream and writes generated count at `+0x48`.

Embedded effect-model shaders independently confirm mesh semantics:

```text
DMC3_EFM.hlsl
DMC3_EFM_SP.hlsl
DMC3_EFM_STX.hlsl
DMC3_EFM_VA.hlsl
DMC3_EFM_VA_SP.hlsl
```

The recovered vertex contract includes:

```text
POSITION NORMAL TEXCOORD0 BLENDINDICES PSIZE COLOR0
```

and the embedded engine comment explicitly describes “EFM models” with extra vertex RGB. Therefore EFM is safely classified as **mesh-bearing effect model**, while exact stream-to-semantic field binding still waits for a real retail payload.

### SCM

SCM remains a stage/scene mesh family with its own related but separate normalizer and factory branch. Existing corpus tooling independently corroborates positions, normals, UVs, faces and vertex colours.

## 4. Construction/factory split

Bounded factory path:

```text
VA          0x140248140
file offset 0x247540
size        0xA0
SHA-256     8ae56885624cbcbc89ece904fe8dc38ba3ea89291ac6575e0f0cf1c16a6f8079
```

Recovered behavior:

```text
MOD / EFM -> shared branch, size 0x780
SCM       -> separate branch, size 0x580
MRP       -> no branch here
MCV       -> no branch here
SHW       -> no branch here
```

A separate runtime memory specialization around `0x1402FD8D0` likewise has MOD/EFM and SCM-specific contributions, but no MRP/MCV/SHW-specific branch in that bounded function.

This negative evidence is scoped: it does not prove those families lack consumers elsewhere.

## 5. SHW boundary

`SHW` normalizer `0x1403204C0` operates on a distinct record arrangement and relocates four qword pointers per `0x40` record. Downstream code around `0x1403204F0` consumes triplets of 32-bit indices into an external `0x10`-stride spatial/vector pool. Helper `0x140320BB0` forms two triangle edges and computes their cross product.

Embedded `DMC3_SHW.hlsl` uses `POSITION` only.

Safe conclusion:

> SHW owns/organizes shadow triangle/topology information over an external spatial vertex pool; it is geometry-related but not proven to be a MOD/SCM-style self-contained textured mesh.

## 6. MRP boundary

MRP is a real runtime identity twice over:

```text
MRP -> type 3                  @ 0x1402DB1F0
MRP<space> -> 0x40000000       @ 0x1402FD650
```

But no immediate generic container handler, MOD/EFM/SCM factory branch or model-memory specialization is established in the bounded paths above.

Therefore:

```text
MRP identity                 = EXE_CONFIRMED
MRP standalone mesh          = NOT PROVEN
MRP normal generic handler   = NOT ESTABLISHED
MRP exact fields/schema      = OPEN
```

Next gate: find a downstream owner/consumer of the `0x40000000` family mask or bind a real retail MRP payload.

## 7. MCV boundary — newly promoted identity

`MCV` is absent from the 3-byte registry probe, but two independent runtime systems now establish it:

```text
MCV<space> -> 0x50000000      @ 0x1402FD650
.mcv       -> class 1         @ 0x1402E01A0
```

Therefore:

```text
MCV runtime family identity  = EXE_CONFIRMED
MCV 3-byte registry tag      = NOT PRESENT
MCV mesh ownership           = NOT PROVEN
MCV MOD/SCM decoder parity   = NOT PROVEN
MCV exact fields/consumer    = OPEN
```

MCV belongs in the primary model/motion identity census because the family-mask system recognizes it beside MOD/EFM/SCM/MRP/SHW, but that fact must not be turned into a guessed mesh schema.

## 8. Retail corpus gap

Current retained evidence has standalone MOD and SCM samples, but no independently named standalone EFM, MRP, MCV or SHW retail payloads in the bounded corpus used by this pass.

Keep these layers separate:

```text
EXE-confirmed identity/layout/shader evidence
!=
real-payload field-perfect schema binding
```

## 9. Viewer/product implication

A safe viewer architecture is layered:

```text
PrimaryModelDocument
  MOD
  EFM
  SCM

ShadowTopologyDocument
  SHW
  external spatial pool

RenderCompanion
  MRP
  exact schema open

MotionModelCompanionIdentity
  MCV
  exact schema/owner open
```

Do not force SHW, MRP or MCV into the MOD/SCM mesh ABI merely to make a uniform API. Recognition may be EXE-confirmed while decoding remains fail-closed.
