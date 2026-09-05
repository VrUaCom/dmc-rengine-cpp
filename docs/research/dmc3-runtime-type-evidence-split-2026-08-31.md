# DMC3 runtime type-evidence split — 2026-08-31

**Status:** CANONICAL CORRECTION PASS  
**Target:** canonical unpacked DMC3 HD analysis executable  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Size:** `6,356,432` bytes  
**ImageBase:** `0x140000000`  
**EntryPoint:** `0x14034615C`

## 1. Correction summary

The original runtime does **not** expose one universal `DMC3 type detector`.
Direct reverse of the canonical executable separates at least three distinct
instruction-backed evidence paths:

```text
A. registry / resource registration
   name extension precedence
   -> three-byte content probe @ 0x1402DB1F0

B. PAC/PNST materialized-child traversal
   -> container dispatcher @ 0x1401B9FA0

C. higher-level runtime resource/object systems
   -> four-byte family-mask probe @ 0x1402FD650
```

The old global claim:

> the runtime compares exactly five payload tags

is **SUPERSEDED**.

The narrower statement remains correct:

> registry content probe `0x1402DB1F0` compares exactly three bytes and recognizes five registry content tags.

Likewise the old global statement:

> the fourth byte does not matter

is **REJECTED**.

Correct replacement:

> `0x1402DB1F0` ignores byte 3, while `0x1402FD650` explicitly compares four bytes and requires trailing ASCII space (`0x20`).

## 2. Evidence site A — registry content probe

Exact function window:

```text
VA          0x1402DB1F0
file offset 0x2DA5F0
size        0x72
SHA-256     4e614cc2d0168d6049a449ed4a1c6a78e0ebdd6b5c4b9699fabd98a63c153d19
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

Direct callers recovered from the canonical image include:

```text
0x1402D9184
0x1402DB5A2
```

The second caller is inside registrar/classifier `0x1402DB3C0`.

### 2.1 Registrar precedence

`0x1402DB3C0` checks resource-name extensions before falling back to the content
probe. The matching import slot `0x14034F3D0` resolves to `strstr`.

Observed variants:

```text
.ptx / .PTX / .Ptx
.clt / .CLT / .Clt
.c1d / .C1D / .c1D / .C1d
```

Recovered precedence:

```text
runtime resource name
  -> extension substring checks
  -> if no extension match
     -> registry_content_probe @ 0x1402DB1F0
```

This must not be collapsed into one generic magic detector.

## 3. Evidence site B — container dispatcher

Independent materialized-child dispatcher:

```text
VA 0x1401B9FA0
```

It reaches the recovered normal post-load handlers:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
SHW -> 0x1403204C0
```

This independently corroborates the same four handlers reached from the
registry/resource-manager path.

The dispatcher also compares:

```text
EFW
EFE
```

but no normal handler is established from those branches in this path.
Therefore the current evidence boundary is deliberately narrow:

```text
EFW / EFE runtime-recognized dispatcher prefix = EXE_CONFIRMED
normal handler                                  = NOT CONFIRMED
exact schema                                    = OPEN
semantic/acronym expansion                      = OPEN
```

No canonical extension or format schema may be invented solely from these
sentinel comparisons.

### 3.1 PAC physical slot 0

The PAC walk reaches physical slot 0; it is not skipped as a privileged manifest
slot.

If slot 0 contains text such as:

```text
st001.ptx
st001.scm
st001.sch
```

the type dispatcher simply fails to classify that text as one of its typed
payload prefixes and returns through the non-match path.

This strengthens the authority separation:

```text
embedded slot-0 name list != runtime type manifest
```

## 4. Evidence site C — four-byte family-mask classifier

Exact function window:

```text
VA          0x1402FD650
file offset 0x2FCA50
size        0x273
SHA-256     a31a8c1e225bc62c07dea05921c42eeff85c28b2f4872713594262e579b91961
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

Here the fourth byte is authoritative:

```text
MOD\x20 -> 0x10000000
MODX    -> 0
```

This is active runtime evidence, not a dead helper. Multiple direct callers are
present. One confirmed caller at `0x1402F9628` stores the returned family mask in
its runtime object at `+0xE0`; downstream paths test mask values for type-specific
behavior.

### 4.1 MCV promotion

`MCV` does not participate in the older three-byte registry probe, but the
four-byte family classifier directly recognizes:

```text
MCV<space> -> 0x50000000
```

Therefore:

```text
MCV runtime family recognition = EXE_CONFIRMED
MCV registry three-byte tag    = NOT PRESENT
```

This upgrades MCV from extension/subsystem-only identity to independent byte-backed
runtime family recognition without pretending both classifiers are the same.

## 5. Product evidence provenance

The canonical core now preserves the evidence sites separately:

```text
ResourceTypeContract::registry_type_for_prefix(...)
    -> profile_runtime_content_tag
    -> ResourceClassifier.runtime_content_tag_confirmed

ResourceTypeContract::family_mask_for_prefix(...)
    -> profile_runtime_family_mask_tag
    -> ResourceClassifier.runtime_family_mask_confirmed

ResourceTypeContract::container_dispatch_*(...)
    -> runtime processing/corroboration evidence
    -> EFW/EFE sentinel recognition does not manufacture a semantic format
```

Both registry and family-mask byte evidence may support a **presentation-only**
derived semantic suffix because they directly classify the payload bytes and are
stored with distinct provenance. Neither becomes historical extraction authority,
`ResourceId`, write authority or proof that every runtime subsystem shares the same
classifier.

## 6. Handler ABI correction

Recovered normalizer functions:

| family | function VA | recovered function/window SHA-256 |
|---|---:|---|
| MOD | `0x1402FE3B0` | `2319717d2b827fddf1821832ca8bf12a665317d954d116400151f0e95c60c565` |
| EFM | `0x1402F7A90` | `0b5ccd9aaa1701fab677ea35bd44924f4d2ad1ab9cbfac754dcf7e246ca1052b` |
| SCM | `0x1403051B0` | `5f3923913db171026470d8d15537d58b823f19f9a6770b6508cee778d1fbd321` |
| SHW | `0x1403204C0` | `14dc368e054ef8a7ed686e55de23b0ac1e8d20be66a9909576bee01f34ca008d` |

The old common-shell description was too broad.

### 6.1 MOD / EFM / SCM related family

Direct reverse supports a related runtime document family with a broad pattern:

```text
header
  -> count-like field near +0x10
  -> relocated base-relative pointer near +0x20
  -> groups beginning near +0x40
  -> group stride 0x40
  -> format-specific inner records / relocation
```

The inner layouts are related but not identical.

`MOD` specifically reads header byte `+0x11` and compares it with `1`; that field
must not be projected onto EFM or SCM without independent evidence.

### 6.2 SHW correction

`SHW` does not prove the same generic document shell. Its normalizer is much
smaller and operates on its own record arrangement, including four qword pointer
relocations.

Therefore:

```text
MOD/EFM/SCM related runtime document family = EXE_CONFIRMED, PARTIAL ABI
SHW identical shared shell                  = REJECTED / OVERGENERALIZED
```

## 7. MRP boundary

`MRP` is independently recognized by two byte-backed systems:

```text
registry probe:    MRP -> type 3
family-mask probe: MRP<space> -> 0x40000000
```

However neither the registrar nor the container-dispatch path establishes a
normal immediate post-load handler equivalent to MOD/EFM/SCM/SHW.

Current safe boundary:

```text
MRP runtime family identity = EXE_CONFIRMED
MRP normal post-load handler = NOT CONFIRMED
MRP exact fields/schema      = OPEN
```

## 8. Negative and separate evidence domains

### 8.1 HITS

No canonical EXE evidence currently promotes `HITS` to one of these runtime type
tags. The structural/corpus format remains a different evidence domain.

```text
HITS structural/corpus identity = separate data/authoring evidence
HITS runtime type tag           = REJECTED for this type system
```

A format can be structurally real without participating in these runtime byte
classifiers.

### 8.2 LIG2

`LIG2` occurs in executable code as a constructed/stored immediate, including the
store near `0x14023ECC9`:

```text
mov [rcx+8], 0x3247494C
```

This proves runtime use/construction of the identifier, not generic type-dispatch
comparison.

Keep the distinction:

```text
runtime writes/constructs identifier
!=
runtime dispatches resource by identifier
```

## 9. Canonical evidence matrix

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
| HITS runtime tag | REJECTED for these classifiers |
| HITS structural/corpus format | SEPARATE EVIDENCE DOMAIN |

## 10. Supersession rule

Any older project text, test description or downstream port saying one of the
following without an evidence-site qualifier is superseded:

```text
"the runtime compares exactly five payload tags"
"the runtime only checks the first three bytes"
"the fourth byte does not matter"
"MOD/EFM/SCM/SHW all use one identical document shell"
```

Correct replacements must name the evidence site explicitly:

```text
registry_content_probe @ 0x1402DB1F0
container_dispatch     @ 0x1401B9FA0
family_mask_probe      @ 0x1402FD650
```

Pocket/mobile parity and any later web implementation must preserve those
separate authorities rather than reintroducing one universal detector.
