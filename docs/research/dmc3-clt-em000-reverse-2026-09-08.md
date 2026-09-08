# DMC3 HD CLT — em000 reverse checkpoint (2026-09-08)

**Branch:** `reverse/clt-em000-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Conclusion

The eight `em000_*.txt` members listed below are **not generic stage TXT resources**. They are serialized **CLT resources whose physical encoding is textual ASCII/CRLF with trailing NUL alignment padding**.

This distinction is architectural:

```text
physical encoding: text
semantic format:   CLT
runtime family:    cloth/deformation companion
```

The extractor-provided `.txt` suffix is therefore a representation/fallback label, not semantic format authority.

## Corpus members

| Extracted member | Embedded source-name evidence |
|---|---|
| `em000_002.txt` | `;em000_01.clt` |
| `em000_006.txt` | `;em001_01.clt` |
| `em000_009.txt` | `;em002_01.clt` |
| `em000_011.txt` | `;em002_02.clt` |
| `em000_014.txt` | `;em003_01.clt` |
| `em000_016.txt` | `;em003_02.clt` |
| `em000_020.txt` | `;em005_01.clt` |
| `em000_022.txt` | `;em005_02.clt` |

All eight samples are printable ASCII/CRLF up to the first NUL byte and use only zero-valued bytes after the textual terminator. The complete physical resources are 16-byte aligned; observed NUL padding is retained as physical data rather than discarded.

## Executable identity evidence

The canonical DMC3 executable contains the motion/control extension dispatcher at `0x1402E01A0`:

```text
.mot -> 0
.mcv -> 1
.cam -> 2
.hid -> 3
.clt -> 4
.tsc -> 5
```

The existing format-purpose census also binds CLT to cloth/deformation simulation through manager placement and original cloth-token families. This is independent executable evidence that `.clt` is a real resource family, not a UI extension invented from these text samples.

Status:

- CLT identity: **EXE_AND_CORPUS_CONFIRMED**;
- textual physical encoding: **CORPUS_CONFIRMED**;
- cloth/deformation purpose: **HIGH_CONFIDENCE / EXE-backed subsystem evidence**;
- exact semantics of every field: **PARTIAL / OPEN**.

## Observed textual grammar

Across the eight samples, observed tokens include:

```text
ClothNum
ClothNo
ClothId
Gravity <x> <y> <z>
SpringForce <value>
MaxSpeed <value>
Stiffness <value>
Wind <x> <y> <z>
WindLocal <value>
WindParent <value>
WindType <value>
Bone <index> <axis-token>
LimitLength <value>      # optional in current corpus
End
$
```

Observed `Bone` axis tokens in this corpus:

```text
Y
NY
Z
```

Examples that prove the grammar is not one fixed template:

- `em000_002.txt` uses bones `2/3/4` with axis `Y`;
- `em000_009.txt` uses bones `2..5` with axis `NY` and `WindParent 9`;
- `em000_011.txt` uses four `NY` bone rows with `WindParent 13`;
- `em000_022.txt` uses `Z` bone rows and includes `LimitLength 0`.

No meaning is assigned to `NY`, `WindType`, `WindParent`, or numeric fields beyond their literal token/value structure unless separately bound to executable behavior.

## Required C++20 module boundary

CLT must be a dedicated module rather than routed through the generic `formats.stage-txt-lexer` product identity.

Target structure:

```text
include/dmc_rengine/formats/clt.hpp
src/formats/clt.cpp
analysis/clt/...              # only when semantic consumers are recovered
tests/clt_tests.cpp
```

Initial reader requirements:

1. accept a byte span, never a host text file abstraction;
2. identify the first NUL as the textual/physical boundary;
3. require every byte after that boundary to be zero for the currently evidenced variant, but report this as a variant constraint rather than a global Capcom law;
4. preserve the exact source bytes, CRLF/LF form, comments, unknown lines and alignment padding;
5. recognize embedded `;*.clt` name evidence independently from external `.index` naming;
6. tokenize known keywords without deleting or rewriting unknown keywords;
7. expose repeated `Bone` lines in source order;
8. remain read-only until round-trip and executable acceptance evidence exists.

## Separation from TXT

The generic TXT lexer is still valid for actual DMC3 script/config TXT resources. CLT is different:

```text
TXT  = generic stage/config script family
CLT  = dedicated motion/control cloth resource, text-serialized
```

Sharing a low-level text tokenizer is acceptable. Sharing the semantic format identity is not.

## Open reverse gates

- bind the CLT loader/parser downstream of extension-dispatch code `4`;
- recover numeric parse helpers and field destinations;
- establish whether `ClothNum` opens multiple logical records in wider corpora;
- recover enum meaning for bone axis tokens and wind fields;
- bind bone indices to MOD/EFM node domains in machine code;
- acquire non-em000 CLT corpora and test grammar variation;
- only then consider canonical writer/editing authority.

## Promotion rule

Do not promote CLT to a writable format merely because all eight samples parse. The first canonical product promotion should be **structural/read-only**, with exact byte preservation and evidence-backed semantic classification as `clt`.
