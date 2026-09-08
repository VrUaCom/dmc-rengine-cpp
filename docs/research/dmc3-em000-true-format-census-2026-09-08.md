# DMC3 HD em000 — true-format census and branch ownership (2026-09-08)

**Branch:** `reverse/em000-format-census-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

This checkpoint separates **extraction suffixes** from **semantic resource formats** for the full recursive em000 corpus and assigns one reverse branch per real format/research domain.

The archive contains **302 extracted leaf files**. Every leaf is accounted for below. No remaining `.bin` or `.txt` leaf is treated as a format merely because of its suffix.

## Raw extraction suffix census

```text
.bin  238
.mod   35
.txt   10
.mot   10
.dds    8
.efm    1
------------
      302
```

These suffix counts are extraction representation only.

## Semantic accounting of all 302 leaves

### `.bin` leaves: 238 / 238 accounted for

```text
1   PTX texture bundle              em000_000.bin
72  MOT-family binary resources     children of em000_035/036/037.pac
3   SO working-family resources     em000_038/039/040.bin
152 effect-pack G/V/E/P/A records   inner em000_041.pnst
10  M optional 16-byte companions   inner em000_041.pnst
---------------------------------
238
```

Therefore there is no evidence for a generic DMC3 `.bin` format in this corpus.

### `.txt` leaves: 10 / 10 accounted for

```text
8  CLT cloth/deformation resources
1  TSC motion/control resource
1  effect-pack ASCII manifest
-------------------------------
10
```

Therefore there is no reason to route these ten resources through one generic semantic TXT identity. Their **physical encoding is text**, but their semantic grammars differ.

### `.mot` leaves: 10 / 10

All ten are MOT-family resources. Together with the 72 MOT payloads extracted as `.bin`, the em000 motion-container corpus contains **82 MOT resources**.

### `.dds` leaves: 8 / 8

All eight occur as effect-pack `T` resources and physically begin with a DMC **0x70-byte texture descriptor**, followed by embedded `DDS ` at offset `+0x70`.

They are therefore **wrapped DDS texture slots**, not plain DDS resources beginning with `DDS ` at file byte zero and not full PTX bundles.

### `.mod` leaves: 35 / 35

```text
22 top-level MOD resources
13 effect-pack M primary MOD resources
--------------------------------------
35
```

These are already covered by the canonical MOD module and completed recursive em000 MOD sweep.

### `.efm` leaves: 1 / 1

`em000_023.efm` is a real EFM mesh-bearing effect-model resource and is now assigned to a dedicated EFM reverse branch.

## Top-level em000 resource map

```text
000  PTX bundle               source suffix .bin
001  MOD
002  CLT                      source suffix .txt
003  MOD
004  MOD
005  MOD
006  CLT                      source suffix .txt
007  MOD
008  MOD
009  CLT                      source suffix .txt
010  MOD
011  CLT                      source suffix .txt
012  MOD
013  MOD
014  CLT                      source suffix .txt
015  MOD
016  CLT                      source suffix .txt
017  MOD
018  MOD
019  MOD
020  CLT                      source suffix .txt
021  MOD
022  CLT                      source suffix .txt
023  EFM
024  TSC                      source suffix .txt
026-034 MOD family resources
035  PAC containing MOT family
036  PAC containing MOT family
037  PAC containing MOT family
038  SO graph/control         working identity, source suffix .bin
039  SO compact link table    working identity, source suffix .bin
040  SO spatial volume table  working identity, source suffix .bin
041  PNST effect pack
```

Slots not listed here are absent/empty in the extracted top-level representation and must not be compacted into a different physical identity.

## Effect-pack semantic inventory

`em000_041.pnst` contains an ASCII manifest plus an inner PNST.

Manifest kinds:

```text
G 12
V 50
E 45
P 34
T  8
A 11
M 13
---
 173 logical entries
```

The inner PNST has 186 physical slots, 183 populated. `M` is grouped: 13 primary MOD slots plus optional second companion slots, 10 populated and 3 empty.

The letters are currently **record-kind identifiers**, not proven standalone filename extensions.

## Branch ownership

### New branches created from current main

```text
reverse/clt-em000-20260908
    owner: CLT textual grammar, cloth field consumer reverse, future CLT module

reverse/tsc-em000-20260908
    owner: TSC textual grammar and motion/control consumer reverse

reverse/mot-em000-20260908
    owner: MOT binary envelope, variable records, CMotion binding

reverse/efm-em000-20260908
    owner: EFM real-payload structural adapter and EFM-specific vertex colour

reverse/effect-pack-em000-20260908
    owner: grouped manifest entries G/V/E/P/T/A/M and physical member mapping

reverse/so-em000-identity-20260908
    owner: true identity/consumer reverse for em000_038/039/040

reverse/ptx-em000-binding-20260908
    owner: bind em000_000.bin to existing canonical PTX reader
```

### Existing dedicated format history retained

```text
MOD   reverse/mod-completion-20260907 and canonical main promotion
SO    research/so-cpp20 predecessor; current identity continuation above
PTX   existing pass87/pass88/pass90 branches + canonical main reader
PNST  existing PNST structural/reflow branches + canonical main parser
DDS   canonical DDS reader; wrapped-DDS framing is reused, not duplicated
PAC   canonical relative-slot container architecture
```

No duplicate branch is created merely to rename an already canonical format.

## Architecture rule

The classifier must follow evidence, not filename suffix:

```text
physical bytes + container context + executable evidence
    -> semantic format
    -> dedicated module
    -> dedicated analysis domain
    -> presentation extension / label
```

Never:

```text
suffix .bin -> BIN parser
suffix .txt -> TXT parser
```

## Required modular targets

```text
formats/clt
formats/tsc
formats/mot
formats/efm
formats/effect_pack          # grouped semantic profile over PNST
formats/so/*                 # already exists; identity remains working
formats/ptx                  # already canonical
formats/dds                  # already canonical
formats/mod                  # already canonical structural module
formats/pnst                 # physical container, not effect semantics
```

Shared low-level utilities are allowed. Semantic ownership must remain separate.

## `PST` boundary

No independent `PST` marker or `.pst` resource is present in the complete em000 extraction inspected for this checkpoint.

The corpus contains **PNST** containers and **PTX** textures. Until separate byte/runtime evidence for `PST` exists, no `PST` format or branch should be invented.

## Promotion order

1. land this census/evidence checkpoint only after review;
2. validate each format branch independently;
3. promote structural/read-only modules one at a time;
4. update semantic classifier so misleading `.bin/.txt/.dds` extraction labels route by proven structure;
5. bind cross-resource relations only after each leaf format has stable identity;
6. writer/editing work begins only after byte-exact round-trip and original-game acceptance evidence.

## Current status

The em000 corpus is now **fully classified at the leaf-accounting level (302/302)**, but several semantic schemas remain intentionally incomplete. Classification completeness is not reverse completeness and does not grant writer authority.
