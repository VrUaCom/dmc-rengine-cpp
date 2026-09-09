# DMC3 HD MOD — transform +0x1C reverse closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Serialized ABI

The recovered 0x20-byte local transform record is:

```text
+0x00 f32 translation.x
+0x04 f32 translation.y
+0x08 f32 translation.z
+0x0C f32 translation_magnitude
+0x10 f32 rotation.x
+0x14 f32 rotation.y
+0x18 f32 rotation.z
+0x1C f32 unresolved
```

The current bounded evidence contains 285 MOD transforms and 5 bound EFM transforms. `+0x1C` is 0.0f in all of them. This is `CORPUS_CONFIRMED` bounded evidence only.

## Initializer provenance

Canonical MOD/EFM initializer `0x1402FA080` copies the serialized rotation block `+0x10..+0x1F` into a 16-byte scratch vector. That copy proves physical transfer of `+0x1C`, but a block copy by itself is not semantic evidence.

## Downstream rotation helper

Canonical helper `0x140330450` reads exactly:

```text
0x14033045A  scratch +0x00 -> rotation X
0x14033046A  scratch +0x04 -> rotation Y
0x14033047A  scratch +0x08 -> rotation Z
```

There is no read of scratch `+0x0C`, which is the copied serialized `+0x1C` value. Therefore `+0x1C` has no effect on the audited canonical MOD/EFM local rotation-matrix construction path. This conclusion is `EXE_CONFIRMED`.

The CMotion binding path at `0x14030F850` independently skips the fourth scalar while advancing the source record by 0x20, reinforcing the same bounded conclusion.

## What is not proven

This pass does not promote a universal semantic name. In particular:

- zero in MOD/EFM does not make the field padding;
- a homologous SCM offset does not transfer SCM semantics into MOD;
- absence from the local-matrix helper does not prove absence from every subsystem in the executable.

The global field status therefore remains `PRESERVED_UNDECODED`.

## C++ contract correction

`include/dmc_rengine/formats/mod/transform_domain.hpp` continues to retain the serialized float as `reserved1c` for compatibility, but its documentation no longer claims a reserved/alignment semantic. The contract now states only the evidence that exists: `EXE_CONFIRMED` non-consumption in the audited matrix path, `CORPUS_CONFIRMED` bounded zeros, and `PRESERVED_UNDECODED` global semantics.

The field must remain source-preserved by any future writer. A writer may not synthesize 0.0f solely from the current corpus.

## Status

```text
serialized ABI                         STRUCTURAL_CONFIRMED
MOD/EFM bounded corpus +0x1C == 0      CORPUS_CONFIRMED
local rotation-matrix non-consumption  EXE_CONFIRMED
global semantic                        PRESERVED_UNDECODED
writer policy                          preserve
```

## Rejected hypotheses

- `+0x1C` is padding because every current sample is zero — `REJECTED`.
- `+0x1C` is reserved/alignment solely because XYZ is consumed and W is not — `REJECTED` as a semantic promotion.
- SCM meaning can be copied into MOD by offset similarity — `REJECTED`.
- A writer may zero-normalize this field — `REJECTED`.

## Next direct-EXE action

Perform a provenance-aware whole-EXE census for serialized transform records and any runtime node fields derived from the 0x20-byte source record. The target is to determine whether any subsystem consumes the fourth rotation scalar outside `0x1402FA080 -> 0x140330450` and `0x14030F850`. Without that closure, `PRESERVED_UNDECODED` remains the correct global status.
