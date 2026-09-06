# DMC3 HD MOD object core — typed IR promotion

Date: 2026-09-07

## Authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- MOD/EFM object initializer VA: `0x1403029E0`
- shared material consumer VA: `0x1402F9890`

This promotion uses the canonical executable behavior already captured by `model_family::ObjectCoreAbi`. The MOD parser previously consumed only object mesh count, aggregate element count and child mesh table, leaving the rest of the EXE-confirmed object core inaccessible through `OuterModel`.

## EXE-confirmed serialized object core

The serialized MOD outer/object record has stride `0x40`.

Fields promoted by this pass:

- `u8  +0x01` — raw alpha/control byte;
- `u32 +0x10` — serialized source flags;
- `f32 +0x30` — bounding center X;
- `f32 +0x34` — bounding center Y;
- `f32 +0x38` — bounding center Z;
- `f32 +0x3C` — bounding radius.

The existing canonical object initializer `0x1403029E0` copies the source flag state into homologous runtime object state and carries the serialized bounding sphere into the runtime object. The alpha/control byte participates in the common runtime alpha-control path.

## Typed IR boundary

`formats::mod::OuterModel` now exposes:

- `alpha_control`;
- `source_flags`;
- `bounding_center`;
- `bounding_radius`.

`outer_record_size` is now derived from `model_family::ObjectCoreAbi::record_size` instead of duplicating the literal `0x40`.

The parser reads all promoted fields through `ObjectCoreAbi` offsets. Raw values are retained as serialized. This pass deliberately does not import SCM-only compatibility rewrites into MOD.

## Flag semantics

This promotion makes the raw `source_flags` observable in the MOD typed IR. It does not claim that every bit is decoded.

One already-confirmed shared consumer is `0x1402F9890`: source/effective flag mask `0x00004000` selects nearest texture filtering (`TEX1 = 0`) instead of the default linear state (`TEX1 = 0x60`). Other bits remain separately evidence-gated.

## Validation

The MOD regression fixture now carries non-default object-core values and verifies exact typed recovery:

- alpha/control `0x90`;
- source flags `0x00004000`;
- bounding center `(10, -20, 30)`;
- bounding radius `42.5`.

Existing MOD texture/CLAMP, skin, transform-domain and runtime-postload regressions remain in the same test target.

## Evidence status

- object record stride `0x40`: `EXE_CONFIRMED`;
- alpha/control field `+0x01`: `EXE_CONFIRMED`;
- source flags field `+0x10`: `EXE_CONFIRMED`;
- bounding sphere `+0x30..+0x3F`: `EXE_CONFIRMED`;
- raw typed parser promotion: project implementation backed by the above executable contract.

## Explicit non-claims

This pass does not establish:

- MOD writer/serialization authority;
- semantics for every `source_flags` bit;
- SCM-specific alpha compatibility corrections as MOD behavior;
- a claim that parsed bounding values are always finite in every possible asset;
- texture companion ownership or naming rules beyond separately recovered evidence.
