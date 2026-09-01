# DMC3 Dante costume body + cloak paired evidence — 2026-09-01

**Status:** CORPUS-CONFIRMED RESEARCH ADDENDUM  
**Branch:** `research/dmc3-primary-3d-format-abi-20260831`

## Scope

This note records a user-supplied paired Dante costume sample consisting of a base/body MOD+SHW and a separate cloak/cloth MOD+SHW plus cloth TXT configuration. The raw game assets are not committed; only hashes and recovered structure are recorded.

## Files and hashes

```text
slot_0001 (2).mod
  size   216544
  SHA256 e219e89285604cb6d800b0afdd3bec6684a6b00cd1862d464a669d2861ff3c89

slot_0008.shw
  size   9488
  SHA256 cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e

slot_0012.mod
  size   35696
  SHA256 7a2be875b3702f59a607655f7a0a412801a6aea639dcb6e3b23d9b0a09c7e740

slot_0014.shw
  size   12560
  SHA256 1df8eda6ac737af7a64d38ca1dc021763093bcb06f5783cbc3b2658c49a22a18
```

## Corrected assembly interpretation

The two MOD+SHW pairs are **not two different Dante costumes**. They are components of the same costume/appearance:

```text
Dante costume
├─ base/body MOD
│  └─ body SHW
└─ cloak/cloth MOD
   ├─ cloak SHW
   └─ cloth configuration TXT
```

## Header correlation

Recovered MOD header fields:

```text
base/body MOD
  objectCount = 17
  bone/matrix count = 24
  texture count = 3

cloak MOD
  objectCount = 6
  bone/matrix count = 33
  texture count = 3
```

Recovered SHW headers:

```text
body SHW
  recordCount = 17
  matrixPaletteCount = 24
  vertices = 152
  triangles = 236

cloak SHW
  recordCount = 17
  matrixPaletteCount = 33
  vertices = 200
  triangles = 332
```

The exact match between cloak MOD matrix/bone count (33) and cloak SHW matrix palette count (33) is strong direct paired-resource evidence that the cloak shadow uses the cloak component's own transform palette.

## Cloth TXT evidence

The supplied TXT identifies itself as `pl000_02.clt` and contains:

```text
ClothNum    1
ClothNo     0
ClothId     0
Gravity     0.000000 -0.010000 0.000000
SpringForce 0.020000
MaxSpeed    50.000000
Stiffness   0.250000
Wind        0.000000 0.000000 0.000000
WindLocal   1
WindParent  0
WindType    1
```

It enables cloth participation for 26 bone indices ranging from 2 through 32. The maximum referenced index 32 is consistent with a 33-entry zero-based transform/bone palette.

## Consequence

This costume demonstrates a componentized character assembly in which cloth is not merely a material flag on the body mesh. The cloak has:

- its own MOD geometry;
- its own 33-entry transform/bone system;
- its own SHW shadow proxy using the same 33-entry palette;
- a separate cloth-physics configuration controlling gravity, spring force, maximum speed, stiffness and wind behavior.

This materially changes authoring requirements for DMC Rengine: a complete costume importer/editor must support multiple model components and multiple SHW components per appearance, with optional cloth simulation per component.

## Safe editor model

```text
Character Appearance
├─ Body component
│  ├─ MOD
│  └─ SHW
├─ Cloth/Cape component
│  ├─ MOD
│  ├─ SHW
│  └─ Cloth Physics
│      ├─ Gravity
│      ├─ Spring Force
│      ├─ Max Speed
│      ├─ Stiffness
│      ├─ Wind vector
│      ├─ WindLocal / WindParent / WindType
│      └─ participating bones
└─ textures/material bindings
```

## Status

- one-costume multi-component interpretation: **USER-SUPPLIED IDENTITY + DATA-CONFIRMED**
- cloak MOD 33-palette ↔ cloak SHW 33-palette correlation: **DATA_CONFIRMED**
- TXT is cloth configuration and references bones 2..32: **DATA_CONFIRMED**
- exact runtime ownership/link path between this specific `pl000_02.clt` and cloak MOD: **EXE TRACE / GAME TEST STILL REQUIRED**
