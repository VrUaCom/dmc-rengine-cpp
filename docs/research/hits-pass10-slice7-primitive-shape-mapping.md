# HITS Pass 10 — Slice 7 — Parsed Shape → Descriptor → Runtime Primitive Mapping

Date: 2026-08-14  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE CONFIRMED / IMPLEMENTED — CI VALIDATION**

## Purpose

Close the semantic gap between the text/parser shape vocabulary and runtime primitive type values used by `0x14005BCF0`.

The important distinction is three-layered:

1. parsed source shape enum (`sphere/box/cylinder/capsule`);
2. internal collision descriptor type;
3. runtime primitive type produced by `0x1402CC530` and its constructors.

These values are not numerically identical and must not be collapsed into one enum.

## Canonical byte provenance

All functions in this Slice lie inside the Phase-17 verifier-preserved original raw PE range `0x400..0x60E600`, cloned from canonical `dmc3.exe` SHA `e454...d082`. As with Slices 4–6, only VAs, hashes and reconstructed semantics are committed; no proprietary instruction bytes are stored in the repository.

## Shape parser vocabulary

The parser cluster beginning at `0x140247AD0` recognizes four literal shape names and writes an enum byte into the parsed shape structure:

| Token | parser enum |
|---|---:|
| `sphere` | `0` |
| `box` | `1` |
| `cylinder` | `2` |
| `capsule` | `3` |

Canonical parser-cluster body:

- `0x140247AD0..0x1402481E0`;
- 1808 bytes;
- SHA-256 `1bd8a8e369105512166e5f2557d274c86e3801b56631f0d4f7022983adfda6e2`.

## Direct parser-to-converter data-flow proof

The mapping below is not inferred from similar field counts.

In parser caller `0x14024A540`, the parsed shape structure lives at `[rbp-0x38]`. Before builder `0x140249710` is called, the caller passes `R9 = rsp+0x30`. With that function's frame relation `rbp = rsp+0x100`:

`R9 + 0x98 = rsp + 0xC8 = rbp - 0x38`.

Builder `0x140249710` obtains the shape member at `R14+0x98` and forwards that exact address to converter `0x1402481E0`.

Therefore the converter consumes the same parsed shape storage populated by the token parser.

## Parsed shape → internal descriptor mapping

Converter `0x1402481E0` performs the exact remap:

| Parsed token | parser enum | internal descriptor type |
|---|---:|---:|
| `sphere` | `0` | `2` |
| `box` | `1` | `3` |
| `cylinder` | `2` | `6` |
| `capsule` | `3` | `4` |

Converter body:

- `0x1402481E0..0x1402482C5`;
- 229 bytes;
- SHA-256 `ed50d780aa2bd2868b783c4924edcac6c2af9b4269225ce92b7c778535d7c58a`.

This corrects the unsafe shorthand `parser type == runtime type`.

## Descriptor → runtime constructor dispatcher

`0x1402CC530` reads the internal descriptor byte and dispatches through a jump table:

| descriptor | constructor | primary runtime type / role |
|---:|---:|---|
| `0` | `0x1402CCA30` | runtime `0`, source semantics not closed by this Slice |
| `1` | `0x1402CCA70` | runtime `1`, source semantics not closed by this Slice |
| `2` | `0x1402CCB50` | runtime `2` normally; may promote to runtime `4` for a swept moving sphere |
| `3` | `0x1402CC610` | runtime `3` |
| `4` | `0x1402CC890` | runtime `4` |
| `5` | `0x1402CCCB0` | runtime `5`, three-vertex-face structural representation |
| `6` | `0x1402CC9A0` | runtime `6` |

Dispatcher body:

- `0x1402CC530..0x1402CC5EF`;
- 191 bytes;
- SHA-256 `e8e0065e0aeb76b6839d0506e09623b48091d853bd9ccee3127bd0bec7e19364`.

## Closed source-shape → primary runtime mapping

Combining the proven parser data flow, converter mapping and runtime dispatcher gives:

| Source shape | internal descriptor | primary runtime type |
|---|---:|---:|
| `sphere` | `2` | `2` |
| `box` | `3` | `3` |
| `cylinder` | `6` | `6` |
| `capsule` | `4` | `4` |

This supersedes any previous prose that directly associated runtime type `2` with cylinder or runtime type `3` with capsule.

## Runtime type 4 has two proven origins

Runtime type `4` must not be named simply `capsule`.

### Parsed capsule path

Descriptor `4`, produced from parsed `capsule`, is handled by `0x1402CC890` and produces runtime type `4` with:

- first endpoint at runtime `+0x130`;
- second endpoint at `+0x140`;
- radius/scalar at `+0x150`.

Constructor body:

- `0x1402CC890..0x1402CC994`;
- 260 bytes;
- SHA-256 `61b2966fe7ff3766ec821f3e5704e0e1d11568e796b22513c27b990c79981bd3`.

### Swept moving sphere path

Descriptor `2`, produced from parsed `sphere`, is handled by `0x1402CCB50`.

Normally it produces runtime type `2`. Under the observed motion-state gate, when displacement between the relevant transformed sphere centers exceeds `2 × radius`, the constructor instead builds the same endpoint+radius runtime representation and writes runtime type `4`.

Constructor body:

- `0x1402CCB50..0x1402CCCB0`;
- 352 bytes;
- SHA-256 `6c8735293b9040d4b44405b45a64dabebbcbe3be0bf873a2159f79747493c189`.

Canonical semantic wording for runtime type `4` is therefore **capsule / swept-sphere representation**. Its origin must remain observable separately from its runtime representation.

## Exact constructor boundaries corrected

Second review removed padding/jump-table bytes that had been included in earlier working ranges.

- box / descriptor3: `0x1402CC610..0x1402CC884`, 628 B, SHA `d5d6e4d714626cafeaee21697d856887c4d6505c092c3498f14e82a838eea821`;
- capsule / descriptor4: `0x1402CC890..0x1402CC994`, 260 B, SHA `61b2966fe7ff3766ec821f3e5704e0e1d11568e796b22513c27b990c79981bd3`;
- cylinder / descriptor6: `0x1402CC9A0..0x1402CCA26`, 134 B, SHA `d53e4ecb5e91dfc0fedcf8c77648cb6c9074646e8853ee8f26e9a2b0add056a6`;
- sphere / swept-sphere descriptor2: `0x1402CCB50..0x1402CCCB0`, 352 B, SHA `6c8735293b9040d4b44405b45a64dabebbcbe3be0bf873a2159f79747493c189`;
- descriptor5: `0x1402CCCB0..0x1402CCD5B`, 171 B, SHA `766bf79c39056c7baaa6bb1cc097fab863651f7b5350edd2b07ff0509f432a2e`.

## Runtime type 5 — structural semantics only

Descriptor `5` is dispatched to `0x1402CCCB0`, which:

- transforms descriptor vector/point `+0x10` into runtime `+0x130`;
- transforms `+0x20` into runtime `+0x140`;
- transforms `+0x30` into runtime `+0x150`;
- transforms auxiliary vector `+0x40` into runtime `+0x160`;
- normalizes the auxiliary vector;
- writes runtime type `5` at runtime `+0x120`.

The runtime type-5 geometry path uses a three-element/three-vertex contract. Therefore **three-vertex face representation** is structurally confirmed.

However, no direct source-text token or canonical user-facing shape name for descriptor/runtime type `5` has been proven. The code records this explicitly as `source_text_shape_name_confirmed = false`.

A tempting unrelated `mov ... = 5` in another state-machine function was reviewed and rejected as evidence; it is not a collision descriptor creator. No textual name is promoted from that false lead.

## Implementation

New DMC3-profile module:

- `include/dmc_rengine/profiles/dmc3/hits_primitive_shape_evidence.hpp`;
- `tests/hits_primitive_shape_evidence_tests.cpp`;
- CTest stem `hits_primitive_shape_evidence`.

The module preserves:

- canonical body ranges/hashes;
- parsed token/enum → descriptor → runtime mapping;
- parser-to-converter storage identity proof;
- runtime type-4 dual-origin behavior;
- runtime type-5 structural evidence without a fabricated text name.

All lookups require canonical SHA. Packed build `81c7...c7d6` receives no canonical descriptors.

Implementation commits:

- `0f7599d77c698dd6b430fdf97d2850f6c0d7f303` — profile evidence module;
- `72cd6caa0c330687717479ca0f8c74fa0d492435` — regression coverage;
- `cccb5fc45c0c4cb1746cd5630d474533fadb6f76` — CTest registration.

## Validation handoff

Slices 4–6 exact implementation head `db7a557de428d4e379ecd51b9670e97dc8c435f7` passed GitHub Actions run `31813329645` on Ubuntu and Windows.

Slice 7 has its own exact-head CI run and must receive a separate green receipt before its implementation status is promoted from validation-active to validated.

## Remaining semantic boundary

Next work after Slice 7:

- identify upstream producer/source vocabulary for descriptor/runtime type `5` if direct evidence exists;
- resolve internal descriptor/runtime types `0` and `1` source semantics rather than assuming they duplicate sphere/box;
- close the exact semantic identity of the common HITS metadata vector `+0x28/+0x2C/+0x30` and related fourth-component data;
- descend into geometry helpers for source-equivalent implementation and build controlled runtime behavior comparisons.
