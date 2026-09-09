# DMC3 HD MOD — source flag 0x00100000 reverse

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Serialized domain

The target is bit `0x00100000` in serialized object source flags at object `+0x10`.

Known bounded corpus presence: 36 em000 objects, 8 pl000 slot-1 objects and 1 pl000 slot-12 object. Corpus appearance is not used to assign an artistic name.

## Baseline/effective runtime state

Canonical object initialization copies serialized object `+0x10` into:

```text
runtime object +0x10  baseline/source flags
runtime object +0x14  mutable/effective flags
```

`0x140305911..0x14030593B` later applies:

```text
effective = (effective & 0xFFEFFFF0) | (baseline & 0x0010000F)
```

Thus the low mode nibble and bit `0x00100000` are explicitly restored from baseline.

## MOD helper projection

`0x140302640` builds legacy renderer state at runtime object `+0x80`.

For an active low-nibble mode other than mode 4:

```text
bit clear: packet +0x08 = 0x5000D
bit set:   packet +0x08 = 0x5010D
```

For the same active-mode path, bit clear additionally sets `1ULL << 32` in packet `+0x00`; bit set leaves that bit clear. Mode 4 is a special fixed path (`packet+0x08 = 0x50007`).

## Packet identity: legacy PS2 GS A+D

`0x1403027E0` initializes each per-mesh descriptor at:

```text
runtime object +0xB0 + mesh_index * 0x50
```

with:

```text
GIF tag            0x4000000000008001
register descriptor 0x10EEEE
```

The value/register pairs are:

```text
descriptor +0x10 value ; +0x18 register 0x4E = GS ZBUF_1
descriptor +0x20 value ; +0x28 register 0x47 = GS TEST_1
descriptor +0x30 value ; +0x38 register 0x42 = GS ALPHA_1
descriptor +0x40 value ; +0x48 register 0x00 = GS PRIM
```

`0x1403028F0` copies packet `+0x00/+0x08/+0x10/+0x18` into those four value slots. Therefore the bit-20 differences are now register-identified, not merely opaque packet differences.

The GS register IDs and packing layout match the established PS2 GS interface: TEST_1 is register `0x47`, ZBUF_1 is `0x4E`, ALPHA_1 is `0x42`; GIF A+D descriptor is `0x0E`.

## Exact TEST_1 semantic

GS TEST packing is:

```text
ATE       bit 0
ATST      bits 1..3
AREF      bits 4..11
AFAIL     bits 12..13
DATE      bit 14
DATM      bit 15
ZTE       bit 16
ZTST      bits 17..18
```

Decoding the two canonical values gives:

```text
                     bit clear       bit set
TEST_1               0x5000D         0x5010D
ATE                   1               1
ATST                  6 (GREATER)     6 (GREATER)
AREF                  0               16
AFAIL                 0               0
DATE / DATM           0 / 0           0 / 0
ZTE                   1               1
ZTST                  2 (GEQUAL)      2 (GEQUAL)
```

So bit `0x00100000` raises the alpha-test reference from `0` to `16`; the alpha-test method remains `GREATER` and Z test remains `GEQUAL`.

## Exact ZBUF_1 semantic

GS ZBUF packs `ZMSK` at bit 32. The extra mask written by the clear-bit branch is exactly:

```text
0x0000000100000000 == ZBUF_1.ZMSK
```

Therefore, in the active low-mode path:

```text
source bit clear -> ZMSK = 1 -> depth-buffer writes masked
source bit set   -> ZMSK = 0 -> depth-buffer writes enabled by this selector
```

The lower ZBUF fields (buffer address / pixel format) are built independently and are not renamed by this result.

## Command/backend closure

Render-command construction references the same descriptor through command tag `0x30000005`.

Generic interpreter `0x140029880` decodes the high dword as an offset from arena base `0x1405D9EA8`. Type `3` dispatches to `0x140029958`, which passes the reconstructed descriptor to generic backend helper `0x140032CD0`.

This closes the chain:

```text
serialized object flag 0x00100000
 -> runtime baseline/effective flags
 -> 0x140302640
 -> GS TEST_1 / ZBUF_1 values
 -> per-mesh A+D GIF descriptor
 -> generic render-command decoder/backend
```

## Status

```text
serialized bit presence                  CORPUS_CONFIRMED
baseline/effective state-machine         EXE_CONFIRMED
legacy GS packet/register identity       EXE_CONFIRMED
TEST_1 AREF 0 <-> 16 selector            EXE_CONFIRMED
ZBUF_1 ZMSK 1 <-> 0 selector             EXE_CONFIRMED
technical renderer semantic              EXE_CONFIRMED
artistic/material category               PRESERVED_UNDECODED
writer policy                             preserve source bit exactly
```

The technical semantic is now closed. The bit should be represented as a legacy GS alpha-test-reference/depth-write-state selector, not as an invented category such as `cloth`, `transparent`, or `alpha material`.

## C++ contract

`include/dmc_rengine/analysis/mod/object_flags.hpp` now contains:

- baseline/effective restoration;
- exact source-bit packet projection;
- GS TEST decoder;
- GS ZBUF `ZMSK` decoder;
- compile-time guards for AREF `0` vs `16`, ATST `GREATER`, ZTST `GEQUAL`, and ZMSK behavior.

## Remaining flag gate

The major unresolved object-flag gate is now source bit `0x00200000`. It is confirmed as baseline/effective carried state, but its distinct terminal consumer remains unresolved and must not inherit the semantics of manager `+0xE0` bit 21 or runtime object `+0x304` bit 21.
