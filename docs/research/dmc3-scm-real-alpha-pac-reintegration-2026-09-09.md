# DMC3 SCM — hash-bound real alpha edit and parent PAC reintegration

Date: 2026-09-09  
Project: DMC Rengine  
Branch: `reverse/mod-completion-20260907`  
Evidence class: `REAL_CORPUS_BOUND_SCM_EDIT_AND_PARENT_PAC_REINTEGRATION`

## Purpose

Advance the SCM authoring frontier beyond the already-closed 68-unique no-edit corpus with a new bounded evidence class:

```text
hash-bound real SCM payload
 -> canonical typed semantic edit
 -> preserve-layout writer
 -> exact-byte guard
 -> race-safe no-replace publication
 -> parent PAC reintegration
 -> canonical PAC reopen/expansion
 -> physical child extraction
 -> canonical SCM reparse
 -> inverse edit back to exact source bytes
```

This is not a no-op corpus pass and it is not a synthetic writer fixture.

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

The final authoring/reintegration run used the Ubuntu `dmc-rengine` executable built by GitHub Actions for PR #376 from the race-safe publication head:

```text
branch head        9e772709aebe3c6217f664f0514c9159515a1038
workflow run       34404600151
artifact id        10124892898
artifact digest    sha256:4a6a79a0deff530c5f370dcc4046b18052c2ad3af5fb868cad7c358fff03300e
binary SHA-256     664d576bda67b0515a1013c7f8bd7d67c3e56762748983050b9363745d816bad
Ubuntu Build/CTest PASS
Windows Build/CTest PASS
```

The artifact filename uses the pull-request workflow `${{ github.sha }}` label (`abeff3ec...`) rather than the branch-head SHA. Both identities are recorded separately in the machine receipt.

## Controlled semantic edit

The bounded command is a thin harness over the existing canonical path:

```text
Parser
 -> set_alpha_control
 -> Writer(WriteMode::preserve_layout)
 -> writer-owned reparse
 -> independent exact-byte guard
 -> second canonical reparse
 -> staged-file validation
 -> atomic no-replace publication
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

The source has 44 objects; 43 use `0x80` and one uses `0xC5`. Object 0 is therefore a representative common alpha-control case rather than the exceptional `0xC5` value.

Final command result:

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
publication=NO_REPLACE_PASS
```

Hashes:

```text
source SCM     3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
authored 0x40  6bda31119b6f33520f56132779c7892ed7526255c3c565399e50fb573380612e
authored 0x20  8e732e0ecc61c69eb753d6f6213d97227b8d79171d11df136cae0d0a17afd207
```

Both authored variants independently differ from source at exactly one byte, SCM offset `0x41`.

## No-replace publication gate

The first implementation used an `exists() -> ofstream(trunc)` sequence. Final review rejected that as TOCTOU-prone even though the writer bytes were correct. The harness now uses the canonical `publish_bytes_no_replace()` path: exclusive staging directory, complete staged validation, then no-replace hard-link publication.

Two real-payload publication checks were executed with the final CI-built binary:

1. Existing-destination replay: attempting a different edit into an already existing authored output returned exit 12 / `destination-exists`; the existing SHA remained unchanged.
2. Concurrent race: 12 trials launched `0x80 -> 0x40` and `0x80 -> 0x20` simultaneously into the same destination path.

Race result:

```text
trials                               12/12
exactly one successful publisher     12/12
loser                                fail-closed staging-conflict
final output complete authored image 12/12
mixed/corrupt outputs                0
stale staging directories            0
```

Both possible winning SHAs were observed, proving the result was determined by publication ownership rather than a hard-coded ordering.

## Parent PAC reintegration

The `0x40` authored SCM was inserted with the existing canonical `rebuild-relative-slot` path.

```text
format            PAC
target slot       2
source size       4,632,960
output size       4,632,960
source SHA-256    43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9
output SHA-256    f8afd23659035b2d4b81926b667f2e374d0550748a74d7a7ec521f792775a41b
```

The complete parent-image diff contains exactly one byte:

```text
0x3530A1: 0x80 -> 0x40
```

This is exactly `slot 2 offset 0x353060 + local SCM offset 0x41`.

The PAC header and physical slot table are byte-identical. All seven non-target physical slots retain their exact source SHA-256 values.

## Canonical reopen and extraction

The rebuilt parent reopens through the canonical container path and reports fully expanded topology:

```text
root PAC containers expanded  3
root slot count                8
slot 2 format                  scm
slot 2 size                    887,760
slot 2 SHA-256                 6bda31119b6f33520f56132779c7892ed7526255c3c565399e50fb573380612e
```

Canonical `extract-slot parent-authored.pac 2 extracted.scm` returns bytes exactly equal to the authored SCM. Canonical SCM reparse returns object 0 `alpha_control = 0x40`.

## Reversibility gate

The extracted child was passed back through the same final typed authoring path:

```text
object[0].alpha_control: 0x40 -> 0x80
```

It again passes reparse, exact-byte guard and no-replace publication, producing:

```text
sha256  3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5
```

The restored SCM is byte-for-byte equal to the original source payload.

## Relative-output publication regression

The earlier evidence run also exposed a core publication defect: basename-only outputs were rejected because an empty `parent_path()` was treated as invalid instead of meaning the current directory.

PR #376 fixes that behavior in `publish_bytes_no_replace()` and adds a regression to `relative_slot_cli_tests`. The final SCM and PAC gates above were deliberately executed with basename-only outputs, so the fixed path form is now exercised by both CI/core regression and the real-payload authoring/reintegration run.

## Capability boundary

Supported by this receipt:

- hash-bound real SCM same-layout semantic alpha edit;
- canonical preserve-layout writer acceptance for this bounded edit class/sample;
- exact one-byte authoring boundary for the selected edit;
- race-safe no-replace output publication for the bounded SCM command;
- parent PAC physical-slot reintegration of the authored SCM;
- exact preservation of every parent byte outside the authorized child byte;
- canonical parent reopen, full container expansion and physical child extraction;
- bounded inverse edit restoring the exact source SCM.

Not supported by this receipt:

- protected-install provenance for the complete parent PAC;
- generic SCM working-copy authority in Native Reader/ModViz;
- arbitrary SCM semantic mutation authority;
- size-changing real SCM canonical-rebuild acceptance;
- real texture-companion rewriting;
- NBZ overlay acceptance;
- original `dmc3.exe` acceptance;
- Capcom-builder equivalence or full production SCM authoring.

The generic format registry therefore remains `read_only` for SCM; this bounded CLI evidence does not silently promote the whole product surface.

## Machine receipt

`data/reverse/dmc3-scm-real-alpha-pac-reintegration-attestation-20260909.json`

No proprietary SCM or PAC payload bytes are committed.
