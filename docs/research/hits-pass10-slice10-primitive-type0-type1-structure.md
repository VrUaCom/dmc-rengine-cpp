# HITS Pass 10 — Slice 10: Runtime Primitive Type 0 / Type 1 Structural Semantics

Date: 2026-08-15  
Canonical target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE CONFIRMED / IMPLEMENTED / VALIDATED**

## Promotion boundary

This slice closes structural geometry semantics for runtime primitive types `0` and `1` without inventing source-text names.

- runtime type `0` = **one-point runtime representation**;
- runtime type `1` = **two-endpoint segment representation**.

`source_token_confirmed` remains false for both.

## Runtime type 0 constructor — `0x1402CCA30`

Canonical body:
- `0x1402CCA30..0x1402CCA64`;
- 52 bytes;
- SHA-256 `8033e51cb704386397c4ee988304b55dbffd538e0710514eaedc785da26ba132`.

Behavior:
- reads primitive descriptor pointer from runtime object `+0x118`;
- reads one position at descriptor `+0x10`;
- transforms it through runtime matrix `+0x30`;
- stores the transformed point at runtime `+0x130`;
- writes runtime primitive type `0` at `+0x120`.

No second endpoint, radius or face-normal payload is consumed by this constructor.

## Runtime type 1 constructor — `0x1402CCA70`

Canonical body:
- `0x1402CCA70..0x1402CCB46`;
- 214 bytes;
- SHA-256 `c06cf1554f9b98b3d012ab0f83b3d2ae85d1a0b0c7a49ea69c573cd4820218fd`.

Behavior:
- descriptor `+0x10` -> transformed runtime endpoint `+0x130`;
- descriptor `+0x20` -> transformed runtime endpoint `+0x140`;
- writes runtime primitive type `1` at `+0x120`;
- computes `(endpoint0 + endpoint1) * 0.5` and stores XYZ midpoint at runtime `+0xE0/+0xE4/+0xE8`;
- computes exact XYZ endpoint distance and stores it at runtime `+0x18`.

The distance helper `0x14032E5F0` computes `(a-b)` XYZ squared length and tail-calls the square-root helper.

Distance-helper body:
- `0x14032E5F0..0x14032E62D`;
- 61 bytes;
- SHA-256 `7ca8c6b01c910665e3f70490b9339010ab6a17a5193779f29f0863278520f7d1`.

## Downstream contact update

Dynamic pair/contact paths at `0x14005A668` and `0x14005B65A/0x14005B66E` special-case runtime type `1`: when a contact resolution succeeds, the produced contact position may replace runtime endpoint `+0x130`.

This is consistent with an active two-endpoint segment representation rather than an arbitrary pair of unrelated points.

## Source vocabulary boundary

This slice does **not** claim:
- a parser token for type `0`;
- a parser token for type `1`;
- that the game's source authors named these structures `point` or `segment`.

The promoted names are structural reconstruction terminology only.

## Implementation

- `include/dmc_rengine/profiles/dmc3/hits_primitive_type01_evidence.hpp`
- `tests/hits_primitive_type01_evidence_tests.cpp`
- CTest `hits_primitive_type01_evidence`

All public evidence remains exact-canonical-SHA gated. Packed SHA `81c7...c7d6` receives no canonical type0/type1 descriptors.

## Validation receipt

Exact code head `9ec763dd2016c9d52b9aa7ad5467d2df1b3ec89c` passed GitHub Actions run `31854177561` on both Ubuntu and Windows, including `hits_primitive_type01_evidence`.

Later documentation/observation-plan commits do not alter the validated C++ code.

## Related type-5 boundary

The same Pass-10 reverse cycle found no direct primitive type-5 write among post-C260 immediate writes or nine register-derived descriptor-type writes. Type 5 therefore remains a separate producer/source problem. Resource-backed C260 paths resolve descriptor tables from serialized blob-relative offset fields, so runtime tracing should observe resolved `manager+0x110` descriptors rather than assume one fixed resource-header offset.
