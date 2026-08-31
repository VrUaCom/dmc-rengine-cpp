# DMC3 runtime type-evidence split — 2026-08-31

**Status:** CANONICAL CORRECTION PASS ON PR #268 BRANCH  
**Target:** canonical unpacked DMC3 HD analysis executable  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Purpose:** correct the previous overgeneralization that one three-byte content census represented the complete DMC3 runtime type system.

## 1. Correction summary

The original runtime does **not** expose one universal `DMC3 type detector`.

At least three distinct instruction-backed evidence paths are now separated:

```text
A. registry / resource registration
   name extension precedence
   -> three-byte content probe @ 0x1402DB1F0

B. PAC/PNST materialized-child traversal
   -> container dispatcher @ 0x1401B9FA0

C. higher-level runtime resource/object systems
   -> four-byte family-mask probe @ 0x1402FD650
```

The old statement:

> the runtime compares exactly five payload tags, globally

is **SUPERSEDED**.

The narrower statement remains correct:

> registry content probe `0x1402DB1F0` compares exactly three bytes and recognizes five registry content tags.

Likewise, the old global statement:

> the fourth byte does not matter

is **REJECTED**.

The corrected statement is:

> `0x1402DB1F0` ignores byte 3, while `0x1402FD650` explicitly compares four bytes and requires trailing ASCII space (`0x20`).

## 2. Evidence site A — registry content probe

Function:

```text
VA          0x1402DB1F0
file offset 0x2DA5F0
size        0x72
window SHA  4e614cc2d0168d6049a449ed4a1c6a78e0ebdd6b5c4b9699fabd98a63c153d19
```

Recovered mapping:

| bytes 0..2 | registry type |
|---|---:|
| `MOD` | 0 |
| `EFM` | 1 |
| `SCM` | 2 |
| `MRP` | 3 |
| `SHW` | 7 |
| other | -1 |

This function does not inspect the fourth byte. Therefore **at this site only**:

```text
MOD\x00 -> MOD
MOD\x20 -> MOD
MOD\x7F -> MOD
```

Direct callers include `0x1402D9184` and `0x1402DB5A2`; the latter is inside registrar/classifier `0x1402DB3C0`.

### Registrar precedence

`0x1402DB3C0` first checks name extensions through the import slot at `0x14034F3D0`, resolved as `strstr`:

```text
.ptx / .PTX / .Ptx
.clt / .CLT / .Clt
.c1d / .C1D / .c1D / .C1d
```

Only when those name checks do not determine the type does it call the three-byte content probe.

Therefore the recovered registry rule is:

```text
runtime resource name
  -> extension substring checks
  -> if no extension match: registry content probe
```

This precedence must not be collapsed into a single generic magic detector.

## 3. Evidence site B — container dispatcher

Independent materialized-child dispatcher:

```text
VA 0x1401B9FA0
```

It reaches the same normal post-load handlers for:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
SHW -> 0x1403204C0
```

It also compares the three-byte prefixes:

```text
EFW
EFE
```

Those two cases are currently evidence-bounded as:

```text
runtime-recognized dispatcher sentinel/prefix
!= confirmed normal format handler
!= decoded schema
!= proven acronym/semantic expansion
```

Do not promote `EFE` or `EFW` beyond that boundary until their consumer/parser path is recovered.

The same traversal independently corroborates the MOD/EFM/SCM/SHW handler mapping and recursively processes nested PNST/container payloads.

### Slot 0

PAC traversal reaches physical slot 0. Slot 0 is not skipped as a special manifest slot.

If slot 0 contains text such as:

```text
st001.ptx
st001.scm
st001.sch
```

the type dispatcher simply does not classify that text as MOD/EFM/SCM/SHW/etc.

This supports the existing authority separation:

```text
embedded slot-0 name list != runtime type manifest
```

## 4. Evidence site C — four-byte family-mask classifier

Newly separated runtime classifier:

```text
VA          0x1402FD650
file offset 0x2FCA50
size        0x273
window SHA  a31a8c1e225bc62c07dea05921c42eeff85c28b2f4872713594262e579b91961
```

Recovered mapping:

| exact bytes 0..3 | returned mask |
|---|---:|
| `MOD ` | `0x10000000` |
| `EFM ` | `0x20000000` |
| `SCM ` | `0x30000000` |
| `MRP ` | `0x40000000` |
| `MCV ` | `0x50000000` |
| `SHW ` | `0x60000000` |
| other | `0` |

The trailing space is part of the comparison:

```text
MOD\x20 -> 0x10000000
MODX    -> 0
```

This function is not dead evidence. It has multiple direct runtime call sites; an observed caller at `0x1402F9628` stores the result in the runtime object at `+0xE0`, and downstream callers test family-mask values for type-specific behavior.

### MCV promotion

`MCV` is therefore promoted beyond extension/name-only evidence:

```text
MCV four-byte runtime family recognition = EXE_CONFIRMED
```

This does **not** imply that MCV participates in the older three-byte registry probe; it does not.

## 5. Handler ABI correction

Recovered normalizer functions:

| family | function VA | recovered function/window SHA |
|---|---:|---|
| MOD | `0x1402FE3B0` | `2319717d2b827fddf1821832ca8bf12a665317d954d116400151f0e95c60c565` |
| EFM | `0x1402F7A90` | `0b5ccd9aaa1701fab677ea35bd44924f4d2ad1ab9cbfac754dcf7e246ca1052b` |
| SCM | `0x1403051B0` | `5f3923913db171026470d8d15537d58b823f19f9a6770b6508cee778d1fbd321` |
| SHW | `0x1403204C0` | `14dc368e054ef8a7ed686e55de23b0ac1e8d20be66a9909576bee01f34ca008d` |

The old shared-shell description was too broad.

### Confirmed related document family

`MOD`, `EFM`, and `SCM` are strongly related runtime documents:

```text
header
  -> count-like field near +0x10
  -> relocated base-relative pointer near +0x20
  -> groups beginning near +0x40
  -> group stride 0x40
  -> inner records / format-specific relocation
```

Their inner layouts are related but not identical.

`MOD` specifically reads header byte `+0x11` and compares it with `1`; that behavior must not be projected onto EFM/SCM without evidence.

### SHW correction

`SHW` does **not** prove the same generic document shell.

Its handler is much smaller and operates on its own record arrangement, including a set of four qword pointer relocations. Therefore:

```text
MOD/EFM/SCM related runtime document family = EXE_CONFIRMED
SHW identical shared shell                 = REJECTED / OVERGENERALIZED
```

## 6. MRP boundary

`MRP` is independently recognized by:

- registry three-byte probe (`MRP -> type 3`);
- four-byte family-mask probe (`MRP  -> 0x40000000`).

However the normal registrar/container path does not establish an immediate normalizer equivalent to MOD/EFM/SCM/SHW.

Current safe boundary:

```text
MRP runtime family identity = EXE_CONFIRMED
MRP normal post-load handler = NOT CONFIRMED
MRP exact fields/schema      = OPEN
```

## 7. Negative and separate evidence domains

### HITS

The canonical executable contains no recovered `HITS` runtime type-dispatch comparison in this evidence family.

Therefore:

```text
HITS structural/corpus identity = separate authoring/data evidence
HITS runtime type tag           = REJECTED
```

A structural format can be real without being a runtime-recognized content tag.

### LIG2

`LIG2` exists in executable code as a store of immediate `0x3247494C` (for example near `0x14023ECC9`), but that alone is not evidence of a generic content-tag dispatcher compare.

Keep the distinction between:

```text
runtime writes/constructs an identifier
vs
runtime uses identifier as type-dispatch probe
```

## 8. Canonical evidence matrix after correction

| Claim | Status |
|---|---|
| MOD registry recognition | EXE_CONFIRMED |
| EFM registry recognition | EXE_CONFIRMED |
| SCM registry recognition | EXE_CONFIRMED |
| MRP registry recognition | EXE_CONFIRMED |
| SHW registry recognition | EXE_CONFIRMED |
| registry probe uses exactly 3 bytes | EXE_CONFIRMED, SITE-SCOPED |
| fourth byte globally ignored | REJECTED |
| four-byte family-mask classifier | EXE_CONFIRMED |
| MCV four-byte family recognition | EXE_CONFIRMED |
| EFW/EFE container-dispatch comparison | EXE_CONFIRMED |
| EFW/EFE exact semantics | RESEARCH_REQUIRED |
| MOD/EFM/SCM related runtime layouts | EXE_CONFIRMED, PARTIAL ABI |
| SHW identical MOD/EFM/SCM shell | REJECTED / OVERGENERALIZED |
| MRP normal handler | NOT CONFIRMED |
| HITS runtime tag | REJECTED |
| HITS structural/corpus format | SEPARATE EVIDENCE DOMAIN |

## 9. Product/core consequence

`ResourceTypeContract` must expose evidence-site scope rather than one global tag census:

```text
registry_content_probe @ 0x1402DB1F0
container_dispatch     @ 0x1401B9FA0
family_mask_probe      @ 0x1402FD650
```

The existing L1 naming operation remains valid when it explicitly uses the **registry three-byte probe** as `profile_runtime_content_tag` evidence. That is a site-specific, instruction-backed reason for a semantic presentation suffix; it is not a claim that every DMC3 runtime subsystem uses the same probe.

Pocket/mobile parity must reproduce the same scoped authority rather than inventing one universal detector.

## 10. Supersession rule

Any older project text or test fixture saying one of the following without a site qualifier must be treated as superseded:

```text
"the runtime compares exactly five payload tags"
"the runtime only checks the first three bytes"
"the fourth byte does not matter"
"MOD/EFM/SCM/SHW all use one identical document shell"
```

Correct replacements are the evidence-site-specific statements recorded above.
