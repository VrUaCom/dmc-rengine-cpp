# DMC3 runtime type identification — direct canonical EXE proof (2026-08-31)

Status: **EXE CONFIRMED / IMPLEMENTED IN MAIN**

Canonical artifact:

- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- size: `6,356,432` bytes
- PE32+ x86-64
- ImageBase: `0x140000000`

Machine-readable receipt:

`data/reverse/dmc3-runtime-type-identification-20260831.json`

## Correction to the old model

Two older global statements are superseded:

1. “the runtime compares exactly five payload tags”;
2. “the fourth byte never matters for DMC3 resource identification”.

Both were over-generalizations from one real function. Direct disassembly of the
canonical executable proves at least three distinct identification paths.

## 1. Registry content probe — 3 bytes

`0x1402DB1F0`

Exact window:

- file offset `0x2DA5F0`
- size `0x72`
- SHA-256 `4e614cc2d0168d6049a449ed4a1c6a78e0ebdd6b5c4b9699fabd98a63c153d19`

Returns:

| prefix | code |
|---|---:|
| `MOD` | 0 |
| `EFM` | 1 |
| `SCM` | 2 |
| `MRP` | 3 |
| `SHW` | 7 |
| other | -1 |

This function reads exactly bytes `0..2`; byte 3 is not read. Therefore `MODX`
is still `MOD` **for this function**.

Direct call sites: `0x1402D9184`, `0x1402DB5A2`.

## 2. Register-and-classify — filename precedence, then 3-byte probe

`0x1402DB3C0`

Exact window:

- file offset `0x2DA3C0`
- size `0x2CC`
- SHA-256 `86c670d76a5a8618006ce76f64cc845b9c750081c8e425dd050604213bb7dbd3`

The registrar checks filename substrings through the import slot at
`0x14034F3D0` (`strstr`) before calling `0x1402DB1F0`.

Recovered handler mapping:

- type 0 / `MOD` -> `0x1402FE3B0`
- type 1 / `EFM` -> `0x1402F7A90`
- type 2 / `SCM` -> `0x1403051B0`
- type 3 / `MRP` -> no normal handler call at this site
- type 7 / `SHW` -> `0x1403204C0`

The type-code namespace belongs to this registry and must not be reused as a
global format enum for unrelated registries.

## 3. Container dispatcher — handlers, sentinels, PNST recursion

`0x1401B9FA0`

Exact window:

- file offset `0x1B93A0`
- size `0xF0`
- SHA-256 `7639949ae6c572589a975fb930b496c473d7fffd0caf3e5bdfafe5d1fb4e5671`

Normal three-byte handler dispatch:

- `MOD` -> `0x1402FE3B0`
- `EFM` -> `0x1402F7A90`
- `SCM` -> `0x1403051B0`
- `SHW` -> `0x1403204C0`

The same function also compares `EFW` and `EFE` and exits through a no-handler
path. Therefore they are runtime-recognized sentinel prefixes, but their semantic
format/layout is **not** established by this evidence and must not be invented.

The dispatcher separately recognizes four-byte `PNST` and recursively dispatches
its populated descendants; recursive call site: `0x1401BA073`.

`MRP` is not dispatched here despite being returned by the registry probe.

## 4. Family-mask classifier — 4 bytes, trailing space required

`0x1402FD650`

Exact window:

- file offset `0x2FCA50`
- size `0x273`
- SHA-256 `a31a8c1e225bc62c07dea05921c42eeff85c28b2f4872713594262e579b91961`

This is an independent runtime classifier. It checks **four** bytes and requires
byte 3 to be ASCII space (`0x20`):

| exact 4-byte prefix | mask |
|---|---:|
| `MOD ` | `0x10000000` |
| `EFM ` | `0x20000000` |
| `SCM ` | `0x30000000` |
| `MRP ` | `0x40000000` |
| `MCV ` | `0x50000000` |
| `SHW ` | `0x60000000` |
| other | `0x00000000` |

Consequences:

- `MOD ` matches;
- `MODX` does **not** match this classifier;
- `MCV ` is runtime-recognized here even though MCV is absent from the
  three-byte registry probe.

A direct-call sweep of the canonical `.text` found 14 calls to this function and
2 calls to the three-byte registry probe.

## GDSpaces provenance model

These sites are now represented separately:

- `profile_runtime_content_tag` — exact 3-byte registry/content probe evidence;
- `profile_runtime_family_mask_tag` — exact 4-byte family-mask evidence;
- `profile_structural_format` — structural parser evidence;
- `magic_confirmed_format` — generic magic evidence.

The DMC3 naming pipeline prefers the three-byte PAC/PNST-relevant content probe
for overlapping tags, then uses the four-byte family classifier as a separate
fallback. This means `MCV ` can become byte-backed `mcv` semantic presentation
without pretending it came from the three-byte registry.

Neither runtime evidence kind is external `.index` authority, historical
filename authority, `ResourceId`, or write/repack authority.

## Negative/boundary facts

- `HITS` occurs zero times as ASCII in this canonical executable. This direct
  pass does not promote HITS to runtime-recognized content-tag evidence.
- `EFW` / `EFE` are not assigned invented file extensions or layouts merely
  because the dispatcher compares them.
- `SHW` must not be generalized as structurally identical to MOD/EFM/SCM solely
  because it shares a runtime identification family.
- One probe's byte width and result namespace must never be generalized to
  another probe.

## Regression requirements

`dmc3_runtime_content_tag_provenance_tests` must pin:

- `MODX` succeeds in the 3-byte probe but fails the 4-byte family probe;
- `MOD ` succeeds in both;
- `MCV ` fails the 3-byte probe and succeeds as family mask `0x50000000`;
- SCM remains sealed as `profile_runtime_content_tag`;
- MCV is sealed as `profile_runtime_family_mask_tag`;
- downstream classification preserves those two provenance kinds separately.

Windows + Ubuntu whole-head CI remains the acceptance gate after this correction.
