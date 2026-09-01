# DMC3 Dante cloak cloth physics reverse — 2026-09-01

**Status:** CORPUS + EXE CONFIRMED RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## User-supplied cloth config

Recovered ASCII content identifies the file as `pl000_02.clt` and contains one cloth block:

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

The block enables 26 bone entries, all with axis token `Y`: bones `2,3,4,5,7,8,9,10,12,13,14,15,17,18,19,20,22,23,24,25,26,28,29,30,31,32`.

## Parser/runtime evidence

The canonical EXE contains the cloth vocabulary around `0x1405067F8..0x1405068D0`, including `ClothSim1D`, `Gravity`, `SpringForce`, `Damping`, `MaxSpeed`, `FloorLevel`, `Cut`, `ClothNo`, `Wind`, `WindLocal`, `WindParent`, `Stiffness`, `WindType`, `LimitLength`, `Bone`, `NX/NY/NZ/X/Y/Z`, and `ClothNum`.

Main cloth parser: `0x1402CA310`.

Recovered field mapping:

```text
Gravity      -> config +0xB0/+0xB4/+0xB8 (float3)
Wind         -> config +0xA0/+0xA4/+0xA8 (float3)
WindLocal    -> config +0x80 (boolean; true only for integer 1)
WindParent   -> config +0x7C (integer)
Stiffness    -> config +0x6C (float)
MaxSpeed     -> config +0x74 (float)
SpringForce  -> config +0x78 (float)
FloorLevel   -> config +0x84 (float)
WindType     -> config +0x8C (integer)
LimitLength  -> config +0xE4 (integer)
```

`Bone <index> <axis>` assigns a per-bone ClothSim1D mode at bone runtime `+0x240`:

```text
X  -> 0
Y  -> 1
Z  -> 2
NX -> 3
NY -> 4
NZ -> 5
```

Therefore this Dante cloak uses 26 physically simulated cloth bones, all configured with the `Y` axis mode. This is a bone-based 1D cloth system rather than modern per-vertex cloth simulation.

`ClothNum` is directly parsed as the number of cloth blocks. `ClothNo` selects the requested block by number. No `ClothId` literal/parser branch was found in the canonical executable string-driven parser; for this build it should be treated as legacy/export metadata or otherwise non-authoritative until another consumer is found.

## Runtime behavior recovered

- Gravity is read as a vector and added into the cloth dynamics each update.
- `MaxSpeed` is used as a magnitude clamp on cloth velocity/motion.
- `SpringForce` multiplies the corrective/spring response.
- `Stiffness` enters the bone constraint/orientation solve; the solver uses it as the configured rigidity factor.
- `WindLocal == 1` uses the authored Wind vector directly. When not 1, the engine obtains `WindParent`, resolves that model/bone transform and transforms the wind vector through it before cloth evaluation.
- In this file the Wind vector is exactly zero, so wind contributes no constant force regardless of the routing flags.

Constructor defaults recovered at `0x1402CA000` include:

```text
Stiffness   = 0.30
MaxSpeed    = 50.0
SpringForce = 0.05
Gravity     = (0.0, -0.20, 0.0)
FloorLevel  = -999999.0
LimitLength = 1
```

Thus the Dante cloak overrides the defaults toward a much gentler/floppier setup:

```text
Gravity Y   -0.20 default -> -0.01 authored (20x weaker magnitude)
SpringForce  0.05 default ->  0.02 authored (40% of default)
Stiffness    0.30 default ->  0.25 authored (~16.7% softer)
MaxSpeed    50.00 default -> 50.00 authored (unchanged)
```

## Corpus comparison

A bounded legacy corpus contains 20 other cloth configs with the same grammar. Observed ranges/patterns:

```text
Gravity Y:   0, -0.01, -0.02, -0.03, -0.05
SpringForce: 0.002, 0.01, 0.02, 0.05
MaxSpeed:    mostly 50; one 70
Stiffness:   0.2, 0.25, 0.3, 0.4, 0.6
Wind:        (0,0,0) in all 20 bounded samples
WindLocal:   1 in all 20
WindParent:  0 in all 20
WindType:    1 in all 20
```

`Y` is overwhelmingly the dominant bone-axis token in that corpus (320 of 340 observed bone assignments), so the Dante cloak follows the stock/common ClothSim1D axis pattern.

## Interpretation for Dante cloak

Evidence-backed practical reading:

- **very weak downward gravity**: the cloak is intentionally prevented from dropping heavily;
- **moderate/soft spring return**: it follows body motion without snapping back aggressively;
- **slightly below-default stiffness**: permits visible lag and folding rather than rigid plate-like motion;
- **normal stock velocity cap**: fast animation cannot make the cloth solver accelerate without bound;
- **no constant authored wind**: movement is driven primarily by character motion/inertia/gravity/spring constraints in this config;
- **26 simulated bones**: the visible cape geometry is deformed through a dense bone network rather than through direct vertex physics;
- **all Y-axis modes**: the stock cape uses the same 1D simulation axis mode across all enabled cloth bones.

## Modding consequence

A DMC Rengine Cloth Editor can expose these values directly and safely as high-level controls:

```text
Gravity
Spring / Return Force
Stiffness
Max Cloth Speed
Wind Vector
Wind Space / Parent
Wind Type (raw until fully classified)
Floor Level
Length Limit
Simulated Bone list + Axis mode
```

Because the solver is bone-based, importing a new cape requires not only geometry but an appropriate cloth-bone lattice/chain, per-bone axis assignments, and matching skin weights. The separate cloak SHW can then follow the same deformed matrix palette to cast an animated cloth shadow.
