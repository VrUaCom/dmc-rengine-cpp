# DMC3 runtime type-evidence split — 2026-08-31

**Status:** CANONICAL CORRECTION PASS — reconciled with full EXE format census  
**Target:** canonical unpacked DMC3 HD analysis executable  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Size:** `6,356,432` bytes  
**ImageBase:** `0x140000000`

Current machine-readable authorities:

- [`../../data/reverse/dmc3-runtime-type-identification-20260831.json`](../../data/reverse/dmc3-runtime-type-identification-20260831.json)
- [`../../data/reverse/dmc3-exe-format-census-20260831.json`](../../data/reverse/dmc3-exe-format-census-20260831.json)

## 1. Core correction

The original runtime does **not** expose one universal DMC3 type detector. The canonical executable contains multiple independent evidence paths with different byte widths and meanings:

```text
A. registry/resource registration
   extension precedence
   -> three-byte content probe @ 0x1402DB1F0

B. PAC/PNST materialized-child traversal
   -> container dispatcher @ 0x1401B9FA0

C. higher-level model/resource systems
   -> four-byte family-mask probe @ 0x1402FD650

D. motion/control manager
   -> extension dispatcher @ 0x1402E01A0

E. format-specific direct content checks
   -> VAGp / DDS / TM2

F. constructor/object type tags
   -> LIG2
```

Therefore the global claims “the runtime compares exactly five tags” and “byte 3 never matters” are superseded.

## 2. Registry content probe — `0x1402DB1F0`

Bounded window:

```text
VA          0x1402DB1F0
file offset 0x2DA5F0
size        0x72
SHA-256     4e614cc2d0168d6049a449ed4a1c6a78e0ebdd6b5c4b9699fabd98a63c153d19
```

Mapping:

| bytes 0..2 | type |
|---|---:|
| `MOD` | 0 |
| `EFM` | 1 |
| `SCM` | 2 |
| `MRP` | 3 |
| `SHW` | 7 |
| other | -1 |

Only bytes `0..2` are inspected. At this site:

```text
MOD\x00 -> MOD
MOD\x20 -> MOD
MODX     -> MOD
```

This rule is **site-scoped**. It must not be promoted into a global file-magic rule or automatic mesh-decode permission.

Direct callers include `0x1402D9184` and `0x1402DB5A2`.

## 3. Registrar/classifier — `0x1402DB3C0`

Resource-name extension checks precede the three-byte content probe. The matching import resolves to `strstr`.

Confirmed extension families:

```text
PTX -> class 4
CLT -> class 5
C1D -> class 6
```

Observed case variants are explicitly checked. Extension identity is original-runtime evidence, but it does not imply a complete binary schema.

## 4. Container dispatcher — `0x1401B9FA0`

Normal post-load handler mapping:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
SHW -> 0x1403204C0
```

Additional identities:

```text
EFE -> explicitly compared sentinel, no normal handler established
EFW -> explicitly compared sentinel, no normal handler established
PNST -> exact four-byte recursive identity
MRP -> no generic handler branch established here
```

Therefore:

```text
EFE/EFW identity on this dispatcher = EXE_CONFIRMED
EFE/EFW exact semantics/schema       = RESEARCH_REQUIRED
PNST recursion identity              = EXE_CONFIRMED
```

The dispatcher reaches physical slot 0; an embedded text name-list in slot 0 is not a runtime type manifest.

## 5. Four-byte family-mask probe — `0x1402FD650`

Bounded window:

```text
VA          0x1402FD650
file offset 0x2FCA50
size        0x273
SHA-256     a31a8c1e225bc62c07dea05921c42eeff85c28b2f4872713594262e579b91961
```

Exact mapping:

| exact bytes 0..3 | mask |
|---|---:|
| `MOD ` | `0x10000000` |
| `EFM ` | `0x20000000` |
| `SCM ` | `0x30000000` |
| `MRP ` | `0x40000000` |
| `MCV ` | `0x50000000` |
| `SHW ` | `0x60000000` |
| other | `0` |

Here the trailing ASCII space is authoritative:

```text
MOD<space> -> 0x10000000
MODX       -> 0
```

The probe has 14 recovered direct call sites. At least one caller stores the resulting family mask into runtime object state for downstream type-specific behavior.

### MCV promotion

`MCV` is absent from the three-byte registry, but `MCV ` is directly recognized here. It is independently corroborated by the motion/control extension dispatcher below.

```text
MCV runtime family identity = EXE_CONFIRMED
MCV three-byte registry tag = NOT PRESENT
MCV mesh schema             = NOT PROVEN
```

## 6. Motion/control extension dispatcher — `0x1402E01A0`

```text
.mot -> 0
.mcv -> 1
.cam -> 2
.hid -> 3
.clt -> 4
.tsc -> 5
```

This confirms runtime family/subsystem registration for `MOT`, `MCV`, `CAM`, `HID`, `CLT`, `TSC`. It does not authorize invented field schemas.

## 7. Direct canonical content checks

The expanded census found direct first-DWORD comparisons that are stronger than filename-only evidence.

### `VAGp`

```text
0x140032970
cmp DWORD PTR [rdi], 0x70474156
56 41 47 70 = VAGp
```

Status: `EXE_CONFIRMED_MAGIC`.

### `DDS `

```text
0x140049A8E
0x14004AD9D
44 44 53 20 = DDS<space>
```

Status: `EXE_CONFIRMED_MAGIC`.

DDS compression/layout FourCCs such as DXT/ATI/BC/RGBG/GRGB/YUY2 remain subformats, not independent DMC resource families.

### `TM2\0`

```text
0x1403365BA
cmp DWORD PTR [rcx], 0x00324D54
54 4D 32 00 = TM2\0
```

Status: `EXE_CONFIRMED_MAGIC`.

Canonical correction:

> Do not use a `TIM2`-only probe as canonical EXE content authority. `TIM2` may remain an alias/historical tooling label, but the direct content check on this path is exactly `TM2\0`.

## 8. LIG2 owned object tag

The constructor around `0x14023ECB0` writes:

```text
0x14023ECC9
mov DWORD PTR [rcx+0x08], 0x3247494C
4C 49 47 32 = LIG2
```

This is stronger than a lone printable-string occurrence: it proves executable-side ownership/construction of a `LIG2` FourCC/type tag.

Boundary:

```text
EXE owns/constructs LIG2 type tag = CONFIRMED
exact tag-to-file-header relation = OPEN
complete on-disk LIG2 schema      = OPEN/PARTIAL from corpus
```

Runtime type construction is not the same thing as generic file dispatch.

## 9. Media capability extension checks

A bounded comparison region around `0x14002A5D1..0x14002A779` recognizes:

```text
PSS THP PAM XMV WMV PMF AVI MPG BIK MP4
```

These are `EXE_CONFIRMED_CAPABILITY_ONLY`. The media layer recognizing a generic extension does not make that format a native DMC resource family or give DMC Rengine ownership over the system format.

## 10. Runtime/path reference evidence

The canonical executable contains owned runtime references for:

```text
NBZ: %sDMC3-%d.nbz @ 0x14036E930
AFS: GData.afs/ @ 0x140363188, plus GDataX360.afs/
PAC: large resource-name corpus
ADX / OGG / SFD / TM2 / TXT: filename corpora
PTZ: basic.ptz
DDS: LOADERICON.dds plus direct content checks
FON / ICO / icon.sys / SAV: runtime/save/UI names
PHD / TSB / BD: snd_sys.phd / snd_sys.tsb / snd_sys.bd
BIN: SpuMap.bin and EventTblNN.bin
SPUMAPDT: explicit string @ 0x140508988
```

These prove references/ownership context, not automatically complete file grammars.

## 11. HITS correction

`HITS` is a real four-byte collision identity in retained corpus/parser evidence. However the bounded canonical EXE type-identification sweep reports **zero ASCII `HITS` occurrences**.

The correct status is:

```text
HITS corpus/parser identity            = DATA_CONFIRMED
HITS participation in these EXE tags   = NOT ESTABLISHED
HITS$ historical five-byte magic       = REJECTED
```

Do **not** say “HITS runtime tag is rejected” when the evidence only establishes absence from this bounded type system. Absence from these classifiers is not proof that no other runtime consumer exists elsewhere.

## 12. Explicit exclusions

The following are not standalone DMC resource families merely because they appear in the executable:

```text
DDS subformat/layout tags:
DX10 DXT1 DXT2 DXT3 DXT4 DXT5 ATI1 ATI2 BC4U BC4S BC5U BC5S RGBG GRGB YUY2

shader compiler/container metadata:
RDEF SPDB D3DSHDR SHEX

source/debug/dependency artifacts:
HLSL FXH PDB DLL
```

Printable machine-code fragments and PE section names require a bounded owner/classifier/handler/content check before they count as format evidence.

## 13. Product provenance rule

Clean C++ must preserve independent evidence dimensions:

```text
three-byte runtime content tag
four-byte runtime family-mask tag
direct file/content magic
constructor/object type tag
extension registration
runtime path/reference
structural corpus grammar
```

They may converge on the same family, but they are not interchangeable.

Recognition is allowed to advance ahead of decoding. Unknown schemas remain fail-closed. In particular, three-byte `MOD`/`SCM` runtime recognition must not automatically authorize mesh decode unless the validated canonical layout contract is also satisfied.

Pocket/mobile/web consumers must preserve these same boundaries rather than reconstructing one universal detector.
