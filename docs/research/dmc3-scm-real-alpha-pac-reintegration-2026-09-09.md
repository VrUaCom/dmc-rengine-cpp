# DMC3 SCM — hash-bound real alpha edit and parent PAC reintegration

Date: 2026-09-09  
Project: DMC Rengine  
Branch: `reverse/mod-completion-20260907`  
Evidence class: `REAL_CORPUS_BOUND_SCM_EDIT_AND_PARENT_PAC_REINTEGRATION`

## Purpose

Advance the SCM authoring frontier beyond the already-closed 68-unique no-edit corpus with a genuinely new evidence class:

```text
hash-bound real SCM payload
 -> canonical typed semantic edit
 -> preserve-layout writer
 -> independent exact-byte diff
 -> parent PAC reintegration
 -> canonical PAC reopen/expansion
 -> physical child extraction
 -> canonical SCM reparse
 -> inverse edit back to exact source bytes
```

This is not another no-op corpus pass and it is not a synthetic writer fixture.

## Source authority

The SCM child is the independently validated real `st001.scm` payload already recorded by the canonical deep-reverse corpus:

```text
size    887,760
sha256  3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
```

The supplied parent `st001.pac` resolves that exact payload at physical slot 2:

```text
parent size        4,632,960
parent SHA-256     43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9
slot               2
slot offset        0x353060
slot size          887,760
slot SHA-256       3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
```

The SCM payload is therefore hash-bound to existing real-corpus authority. The complete parent PAC SHA was not previously bound to a protected installation, so this note deliberately does **not** promote the parent to protected-install provenance.

## Exact runner authority

The authoring run used the Ubuntu `dmc-rengine` executable built by GitHub Actions for PR #376.

```text
branch head        64b400a5f13830a95a8ca9b8ff1666746c9a573a
workflow run       34369178488
artifact id        10111260325
artifact digest    sha256:9702ab289203f15a0c7b7762b8120de0706ae6b203a5ac26fa70f30a7652352e
binary SHA-256     9b789b44cb325be2c484354719baf05561625ff104d7f436a8ecffb3f8cb698a
Ubuntu Build/CTest PASS
Windows Build/CTest PASS
```

The artifact filename contains the pull-request workflow `${{ github.sha }}` label rather than the branch-head SHA. Both identities are recorded separately in the machine receipt to avoid conflating them.

## Controlled semantic edit

The bounded command is only a thin harness over the existing canonical path:

```text
Parser
 -> set_alpha_control
 -> Writer(WriteMode::preserve_layout)
 -> writer-owned reparse
 -> independent exact-byte guard
 -> second canonical reparse
```

Selected edit:

```text
object             0
object record      0x40
field              alpha_control (+0x01)
old                0x80
new                0x40
SCM changed offset 0x41
```

The source corpus has 44 objects; 43 use `0x80` and one uses `0xC5`. Object 0 is therefore a representative common alpha-control case rather than the exceptional `0xC5` control value.

Canonical command result:

```text
SCM_ALPHA_EDIT_OK
object=0
old=128
new=64
objectOffset=64
changedOffset=65
sourceSize=887760
outputSize=887760
reparse=PASS
exactByteGuard=PASS
```

Hashes:

```text
source SCM   3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
authored SCM 6bda31119b6f33520f56132779c7892ed7526255c3c565399e50fb573380612e
```

Independent raw comparison found exactly one modified byte:

```text
0x41: 0x80 -> 0x40
```

No layout, count, offset, mesh stream, hierarchy, unknown byte or index-workspace byte changed.

## Parent PAC reintegration

The authored SCM was inserted with the existing canonical `rebuild-relative-slot` path; no SCM-specific PAC writer was introduced.

Result:

```text
format            PAC
target slot       2
source size       4,632,960
output size       4,632,960
source SHA-256    43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9
output SHA-256    f8afd23659035b2d4b81926b667f2e374d0550748a74d7a7ec521f792775a41b
```

The complete parent-image raw diff contains exactly one byte:

```text
0x3530A1: 0x80 -> 0x40
```

This is exactly:

```text
slot 2 offset 0x353060 + local SCM offset 0x41
```

The PAC header and physical slot table are byte-identical. All seven non-target physical slots retain exact source SHA-256 values.

## Canonical reopen and extraction

The rebuilt parent successfully reopens through the canonical container path and fully expands. Physical slot 2 re-extracts as SCM with:

```text
size    887,760
sha256  6bda31119b6f33520f56132779c7892ed7526255c3c565399e50fb573380612e
```

The extracted bytes are exactly equal to the authored child bytes, and canonical SCM reparse returns object 0 `alpha_control = 0x40`.

## Reversibility gate

The extracted child was passed back through the same canonical typed edit path:

```text
object[0].alpha_control: 0x40 -> 0x80
```

The inverse edit again passes the exact-byte guard and produces:

```text
sha256  3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
```

The complete restored SCM is byte-for-byte equal to the original source payload.

## Incidental publication defect found during the run

A basename-only output path initially exposed an unrelated CLI/core publication bug: `publish_bytes_no_replace()` rejected a valid destination whose `parent_path()` was empty instead of interpreting it as the current directory.

The absolute-path execution succeeded and is the authority for the receipt. PR #376 also fixes the relative-path defect and adds a `relative_slot_cli_tests` regression. This bug did not alter the writer or reintegration result; it only affected final path publication for one CLI spelling.

## Promoted claims

Supported by this receipt:

- hash-bound real SCM same-layout semantic alpha edit;
- canonical preserve-layout writer acceptance for this bounded edit class/sample;
- exact one-byte authoring boundary for the selected edit;
- parent PAC physical-slot reintegration of the authored SCM;
- exact preservation of every byte outside the authorized child byte;
- canonical parent reopen, expansion and physical child extraction;
- bounded inverse edit restoring the exact source SCM.

Not supported by this receipt:

- protected-install provenance for the complete parent PAC;
- arbitrary SCM semantic mutation authority;
- size-changing real SCM canonical-rebuild acceptance;
- real texture-companion rewriting;
- NBZ overlay acceptance;
- original `dmc3.exe` acceptance;
- production-ready or 100% SCM authoring.

## Machine receipt

`data/reverse/dmc3-scm-real-alpha-pac-reintegration-attestation-20260909.json`

No proprietary SCM or PAC payload bytes are committed.
