# DMC3 HD MOD document core — typed header promotion

Date: 2026-09-07

## Authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- shared model-manager initializer: `0x1402F9570`
- common header ABI: `model_family::DocumentCoreAbi`

This pass promotes the remaining EXE-confirmed shared document-shell fields into the typed MOD header without assigning semantics that the executable evidence does not yet support.

## Serialized header fields

The common `0x40` model document header exposes:

- `f32 +0x04` — version;
- `u8 +0x10` — outer/object count;
- `u8 +0x11` — node/transform-domain count;
- `u8 +0x12` — serialized texture-slot-domain count/mirror;
- `u8 +0x13` — runtime-carried byte, semantic name unresolved for MOD;
- `u32 +0x14` — runtime-carried metadata, semantic name unresolved for MOD;
- `u64 +0x20` — node/transform-domain block offset in serialized form.

The MOD parser previously exposed `+0x04`, `+0x10`, `+0x11`, and `+0x20` but left `+0x12/+0x13/+0x14` inaccessible through typed IR.

## Texture-slot count authority

Header `+0x12` is intentionally named `texture_slot_count`, but it is treated as a serialized domain mirror rather than live runtime authority.

Canonical manager initialization at `0x1402F9570` takes live texture-table count/authority from the external texture companion path. The companion chain is independently recovered as:

`source companion -> runtime descriptor table -> mesh texture_slot selection`.

Therefore:

- header `+0x12` is useful for structural comparison and consistency diagnostics;
- it must not override the external companion's runtime texture count;
- future binding validation should treat companion count as authoritative and report a header-mirror mismatch separately.

## Raw runtime-carried fields

`+0x13` and `+0x14` are executable-confirmed members of the common manager path, but their precise semantic roles remain unresolved for MOD. They are promoted as:

- `runtime_mode_byte`;
- `runtime_metadata_u32`.

These names are intentionally descriptive of storage/flow only, not gameplay or rendering semantics.

## Canonical C++ changes

`formats::mod::Header` now exposes:

- `texture_slot_count`;
- `runtime_mode_byte`;
- `runtime_metadata_u32`.

`header_size` now derives from `model_family::DocumentCoreAbi::header_size`.

The parser uses `DocumentCoreAbi` offsets for all common header fields instead of duplicating literals.

## Validation fixture

The canonical MOD regression fixture now carries:

- texture-slot mirror `8` at `+0x12`;
- raw mode byte `0x5A` at `+0x13`;
- raw metadata `0x12345678` at `+0x14`;
- mesh texture slot `7`.

The test verifies exact typed recovery while retaining all existing object-core, mesh texture/CLAMP, skin, transform-domain, and runtime-postload regressions.

The `8` versus mesh slot `7` fixture is structural only. It does not claim the header count is the live companion count.

## Evidence status

- common header size `0x40`: `EXE_CONFIRMED`;
- texture-slot mirror `+0x12`: `EXE_CONFIRMED`;
- runtime-carried byte `+0x13`: field/flow `EXE_CONFIRMED`, detailed semantics `PRESERVED_UNDECODED`;
- runtime metadata `+0x14`: field/flow `EXE_CONFIRMED`, detailed semantics `PRESERVED_UNDECODED`;
- external companion as live texture-table authority: `EXE_CONFIRMED`.

## Explicit non-claims

This pass does not establish:

- MOD writer authority;
- that header `+0x12` is the live runtime texture count;
- semantic decoding of `+0x13`;
- semantic decoding of `+0x14`;
- a header/companion equality invariant across every retail asset;
- texture replacement or authoring authority.
