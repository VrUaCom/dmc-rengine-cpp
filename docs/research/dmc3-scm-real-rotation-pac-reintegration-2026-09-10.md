# DMC3 SCM — hash-bound real node-rotation edit and parent PAC reintegration

Date: 2026-09-10  
Project: DMC Rengine  
Branch: `reverse/mod-completion-20260907`  
Evidence class: `REAL_CORPUS_BOUND_SCM_ROTATION_AND_PARENT_PAC_REINTEGRATION`

## Purpose

Prove a third independent bounded SCM semantic edit class after `alpha_control` and node translation:

```text
hash-bound real SCM payload
 -> typed node rotation edit
 -> preserve-layout writer
 -> exact rotation-span byte guard
 -> staged no-replace publication
 -> parent PAC reintegration
 -> canonical reopen/extract
 -> inverse edit restoring exact child and parent bytes
```

This is not a synthetic fixture and does not promote generic SCM writer authority.

## Source authority

`st001.scm`, 887,760 bytes, SHA-256 `3ed787cc8a41f4c21b972664cf4174739382230ce0010f166c32c2a9e3626ba5`. The parent `st001.pac` is 4,632,960 bytes, SHA-256 `43034a2e0596f744ea2d50fd785b2783d319fe5f0c7e15061e7c68b2786a57d9`; the SCM is physical slot 2 at `0x353060`. The child is hash-bound real-corpus authority; the complete parent PAC is not promoted to protected-install provenance.

## Selected transform and edit

Node 0 transform record is `0xB6AF0`:

```text
translation           (0, -27.5, 0)
translation magnitude 27.5
rotation XYZ          (0, 0, 0)
reserved +0x1C        0
```

Bounded edit: rotation X `0 -> 0.125` radians, Y/Z unchanged. `0.125F` is exactly representable. The authorized serialized rotation span is `0xB6B00..0xB6B0B` (`transform +0x10..+0x1B`).

## Exact CI-built runner

PR #380, branch head `4a9e587c30623d5a58f68d092da7b3118f275cfd`, workflow run `34445596904`, Ubuntu artifact `10139581586`, artifact digest `sha256:8d566bbfba887b8c03ac884435c9c5609c3332dddc4a1fe1d48f3f4cc109b618`, executable SHA-256 `b624c6c022ae663a5ec894ea2c4cb1965baf0e531556a25bab75244fbb3f1eae`.

Exact-head CI result: Ubuntu Build + full CTest PASS; Windows Build + full CTest PASS.

## Canonical SCM result

The CI-built executable returned `SCM_ROTATION_EDIT_OK` with reparse, exact-span guard and no-replace publication PASS. Authored SCM SHA-256 is `e29e298260127d5154a783e9d32b215c816d62926d9ee0164f25562820deaa0c`. Independent full-file comparison found exactly one changed byte:

```text
0xB6B03: 00 -> 3E
```

That is the high byte of the little-endian float `0.125F`. Translation, translation magnitude, rotation Y/Z and unresolved transform `+0x1C` are byte-identical to source. A replay into the already-existing output failed closed with exit 12 / `destination-exists`, and the existing SHA stayed unchanged. Additional exact-binary failure guards also passed: no-op edit exit 6, out-of-range node exit 5, non-finite rotation exit 2, and source-as-output exit 12 while preserving the exact source SHA.

## Parent PAC reintegration

Canonical `rebuild-relative-slot` replaced physical slot 2. Output parent SHA-256 is `46e4ffc77bc36af62e8b8701278a95c2f0ba3f59e95446642136deb829263996`, size remains 4,632,960. Independent whole-parent comparison found exactly one changed byte:

```text
0x409B63: 00 -> 3E
```

This is exactly `slot offset 0x353060 + SCM offset 0xB6B03`. All seven non-target physical slots remain byte-identical.

## Canonical reopen and reversibility

The rebuilt PAC fully expands through the canonical container path. Re-extracted slot 2 equals the authored SCM byte-for-byte and reparses with rotation `(0.125,0,0)`. The inverse rotation `(0.125,0,0) -> (0,0,0)` restores the exact source SCM SHA. Reinserting the restored SCM into the authored PAC restores the exact original parent PAC SHA and bytes.

## Evidence boundary

Supported: bounded real SCM node-rotation editing for this sample, exact rotation-span confinement, translation/magnitude/+0x1C preservation, PAC reintegration, reopen/extract and exact inverse restoration.

Not supported: generic SCM transform authoring, generic Native Reader/ModViz write authority, protected-install parent provenance, NBZ acceptance, original `dmc3.exe` acceptance, or Capcom-builder/full-writer equivalence.

Machine receipt: `data/reverse/dmc3-scm-real-rotation-pac-reintegration-attestation-20260910.json`. No proprietary payload bytes are committed.
