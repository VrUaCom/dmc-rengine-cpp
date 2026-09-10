# DMC3 HD MOD — transform +0x1C canonical runtime dormancy closure

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Evidence follow-up:** 2026-09-10

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
+0x1C f32 raw_1c / unresolved
```

The bounded corpus contains 285 MOD transforms and 5 bound EFM transforms. `+0x1C` is 0.0f in all 290 records. This remains `CORPUS_CONFIRMED` evidence only: zero does not make the field padding, reserved space, or authorizable zero-fill.

## Canonical manager pointer provenance

`0x1402F1DB0` materializes the model-manager source pointers from the serialized descriptor. The resolved transform-block pointer is written to manager `+0x20` at:

```text
0x1402F1E3C  write resolved transform-block pointer -> manager +0x20
```

The same manager domain carries the transform/node count at `+0xEA` and the runtime transform workspace at `+0x188`. These three fields form the type fingerprint used for the whole-image census below.

## Whole-executable manager-fingerprint census

The complete canonical executable was scanned by function for this joint fingerprint:

```text
object-like qword load from +0x20
AND access to +0xEA
AND access to +0x188
```

Stack-frame `rbp/rsp +0x20` accesses are excluded. Exactly five functions match in the entire image:

| Function | Proven role | `+0x1C` result |
|---|---|---|
| `0x1402F1DB0` | source-pointer materialization into manager | does not consume transform record scalars |
| `0x1402FA080` | MOD/EFM local transform construction | fourth rotation lane is physically copied, but rotation helper consumes XYZ only |
| `0x1402FA360` | SCM-specific local transform control path | fourth rotation scratch lane is explicitly overwritten with 0 before the same rotation helper |
| `0x14030F850` | CMotion materialization path A | copies `+00/+04/+08/+10/+14/+18`, then advances by `0x20`; skips `+1C` |
| `0x14030FAD0` | CMotion materialization path B | same scalar-copy pattern; skips `+1C` |

A weaker `+0x20` + `+0xEA` coincidence in `0x14030DF40` is rejected as the model-manager transform source: it operates on a separate high-level owner/config object with large fields such as `+0x900`, obtains a model core through `0x140089DE0` on a different object, and its `+0x20` pointee is consumed as a different structure. Offset coincidence is not type provenance.

This whole-image fingerprint census materially closes the previous escape concern: every provenance-confirmed canonical consumer of the manager transform-block domain either constructs the pointer or ignores serialized `+0x1C` as runtime transform state.

## MOD/EFM local-matrix path

Canonical MOD/EFM initializer `0x1402FA080` loads the serialized transform in two float4 blocks. The second block contains:

```text
{ rotation.x, rotation.y, rotation.z, raw_1c }
```

It is forwarded to rotation helper `0x140330450`. That helper reads exactly:

```text
0x14033045A  scratch +0x00 -> rotation X
0x14033046A  scratch +0x04 -> rotation Y
0x14033047A  scratch +0x08 -> rotation Z
```

There is no read of scratch `+0x0C`, which corresponds to serialized `+0x1C`. The fourth scalar therefore has no effect on canonical MOD/EFM local rotation-matrix construction.

## CMotion path A — `0x14030F850`

Manager `+0x20` is loaded into the source cursor at `0x14030F8D1`. For each 0x20-byte transform record the function reads:

```text
0x14030F9CC  +0x00
0x14030F9DE  +0x04
0x14030F9F1  +0x08
               +0x0C skipped
0x14030FA04  +0x10
0x14030FA17  +0x14
0x14030FA2A  +0x18
0x14030FA2F  source += 0x20
               +0x1C skipped
```

The last copied rotation scalar is written to runtime node `+0x1C4` at `0x14030FA33`. No runtime-node destination is produced from serialized `+0x1C`.

## CMotion path B — `0x14030FAD0`

The sibling materializer independently repeats the same behavior:

```text
0x14030FB4C  manager +0x20 -> source cursor
0x14030FC1E  +0x00
0x14030FC30  +0x04
0x14030FC43  +0x08
               +0x0C skipped
0x14030FC56  +0x10
0x14030FC69  +0x14
0x14030FC7C  +0x18
0x14030FC81  source += 0x20
               +0x1C skipped
0x14030FC85  source +0x18 -> runtime +0x1C4
```

This is an independent negative control against accidentally treating one CMotion function as the only transfer path.

## SCM cross-family control

SCM uses the separate initializer `0x1402FA360`, not the MOD/EFM initializer. The shared 0x20-byte transform shell is useful as a physical comparison only; SCM semantics are not copied into MOD.

The SCM path loads the second serialized float4 at `0x1402FA422`, then explicitly overwrites scratch lane W with 0:

```text
0x1402FA432  xorps xmm0,xmm0
0x1402FA435  write 0.0f -> rotation scratch +0x0C
0x1402FA45E  call 0x140330450
```

Likewise its translation scratch W is set to 1.0f before translation application. This demonstrates that the fourth serialized lanes are not generic homogeneous-vector inputs. It does **not** assign a MOD semantic to `+0x1C`.

## Canonical closure

The evidence-safe conclusion is now:

```text
serialized ABI                               STRUCTURAL_CONFIRMED
MOD/EFM bounded corpus +0x1C == 0            CORPUS_CONFIRMED
whole-image typed manager consumer census    EXE_CONFIRMED
canonical runtime transform effect           EXE_CONFIRMED: dormant / no effect
high-level serialized semantic               PRESERVED_UNDECODED
writer policy                                preserve exact source bits
```

“Dormant” here is a behavioral conclusion, not a serialized semantic name. The byte remains part of the ABI and must survive no-edit round-trip even when synthetic or future data makes it non-zero.

No further transform-runtime consumer tracing is required before the MOD unknown-field phase can advance. A future discovery from a different executable/version or corpus may reopen the semantic label, but canonical DMC3 HD runtime consumption is closed.

## C++ and regression contract

`include/dmc_rengine/formats/mod/transform_domain.hpp` exposes the field as evidence-safe `raw_1c`; historical `reserved1c` remains only a source-compatibility alias.

`tests/mod_skin_tests.cpp` already contains the required synthetic regression:

- parse a transform with `raw_1c = 123.25f` and preserve the exact value;
- mutate only `raw_1c` to `-99.5f`;
- verify `world_transform::build_local_matrix` remains unchanged.

This test is implementation regression evidence for the C++ contract. The EXE disassembly above remains the semantic authority.

## Rejected hypotheses

- `+0x1C` is padding because current corpus values are zero — `REJECTED`.
- `+0x1C` is reserved merely because canonical runtime transform paths ignore it — `REJECTED` as a serialized semantic promotion.
- SCM offset similarity authorizes importing an SCM meaning into MOD — `REJECTED`.
- the 16-byte SIMD copy in `0x1402FA080` proves the fourth lane is semantically consumed — `REJECTED`.
- a writer may synthesize or normalize `+0x1C` to 0.0f — `REJECTED`.
