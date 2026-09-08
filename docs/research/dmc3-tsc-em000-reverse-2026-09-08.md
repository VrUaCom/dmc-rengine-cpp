# DMC3 HD TSC — em000 reverse checkpoint (2026-09-08)

**Branch:** `reverse/tsc-em000-20260908`  
**Base:** `main@da852451d9729d15e3106d87859a83d694d2813b`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Canonical EXE:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Conclusion

`em000_024.txt` is not a generic TXT resource. It is a **TSC resource serialized as text**.

The first semantic line is literally:

```text
.TSC
```

and the canonical executable independently recognizes `.tsc` in the original motion/control resource-manager extension dispatcher.

Correct architectural model:

```text
physical encoding: text + NUL alignment padding
semantic format:   TSC
manager family:    motion/control
```

The extractor's `.txt` suffix must not override this evidence.

## Bound corpus sample

`em000_024.txt`:

- physical size: `240` bytes;
- printable textual extent before NUL padding: `226` bytes;
- trailing zero padding: `14` bytes;
- total resource size is 16-byte aligned.

Observed text:

```text
.TSC
# RELATIVE
<Start
ScrlNo 0
ScrlType 3
TexNo 2
DirUV stay, down
TimeUV 450, 90
MinimumUV 0.004, 0.003
End>
<Finish>
$
# ABSOLUTE
```

Line-ending and final padding bytes are physical evidence and must be preserved by the initial reader.

## Executable identity evidence

The canonical DMC3 motion/control extension dispatcher at `0x1402E01A0` contains:

```text
.mot -> 0
.mcv -> 1
.cam -> 2
.hid -> 3
.clt -> 4
.tsc -> 5
```

Therefore the `.TSC` marker inside the corpus is independently corroborated by executable behavior.

Status:

- TSC resource identity: **EXE_AND_CORPUS_CONFIRMED**;
- text serialization of the observed em000 sample: **CORPUS_CONFIRMED**;
- motion/control-manager ownership: **EXE_CONFIRMED**;
- exact runtime meaning of all TSC fields: **OPEN / RESEARCH_REQUIRED**.

## Observed structural grammar

The current sample shows a line-oriented DSL with:

```text
.TSC                    # format marker
# <comment/section text>
<Start                  # block start
<Key> <value...>        # properties
End>                    # block end
<Finish>                # marker/block
$                       # terminator
```

Known keys in the bound sample:

```text
ScrlNo
ScrlType
TexNo
DirUV
TimeUV
MinimumUV
```

Known value shapes in this sample:

```text
ScrlNo     -> integer
ScrlType   -> integer
TexNo      -> integer
DirUV      -> two comma-separated tokens
TimeUV     -> two comma-separated numeric values
MinimumUV  -> two comma-separated floating-point values
```

These are structural observations only. `ScrlType 3`, `stay`, `down`, `450`, `90`, and the UV minima are not assigned stronger runtime semantics here.

## Why TSC must not be routed as generic TXT

Generic text transport and semantic format are separate layers:

```text
byte reader / text tokenizer
        |
        +--> TXT stage/config grammar
        +--> CLT cloth grammar
        +--> TSC motion/control grammar
```

TSC can reuse a common lexical utility, but it must own a distinct parser ID, evidence identity, diagnostics and future semantic model.

## Required C++20 module boundary

Target structure:

```text
include/dmc_rengine/formats/tsc.hpp
src/formats/tsc.cpp
analysis/tsc/...              # after runtime consumer recovery
tests/tsc_tests.cpp
```

Initial module requirements:

1. consume raw bytes, not normalized host text;
2. recognize `.TSC` from content independently of filename suffix;
3. retain comments and unknown sections verbatim;
4. expose block boundaries and key/value lines without inventing enums;
5. retain exact line endings and NUL padding;
6. reject malformed bounds safely while keeping unknown syntax reportable;
7. remain read-only.

## Open reverse gates

- trace extension-dispatch index `5` to the concrete TSC parser;
- identify parser helpers for `ScrlNo`, `ScrlType`, `TexNo`, `DirUV`, `TimeUV`, `MinimumUV`;
- recover the runtime object receiving each value;
- determine the meaning and valid domain of `ScrlType`;
- recover `RELATIVE` versus `ABSOLUTE` behavior;
- establish whether `<Finish>` is data-bearing in other corpora;
- acquire additional TSC samples before defining a strict full grammar;
- bind TSC texture references to PTX/DDS and, if applicable, MOD/EFM material state.

## Promotion rule

The first canonical TSC promotion must be **structural/read-only**. A writer requires broader corpus coverage, byte-exact replay and original-game acceptance; a single em000 sample is not sufficient writer authority.
