# DMC3 family-mask `object + 0xE0` consumer census — 2026-08-31

**Status:** EXE CONFIRMED / BOUNDED NEGATIVE  
**Target SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

Machine-readable receipt:

`data/reverse/dmc3-family-mask-object-e0-consumer-census-20260831.json`

## Result

The four-byte family classifier at `0x1402FD650` is called at `0x1402F9628`,
and its result is written to runtime `object + 0xE0` at `0x1402F9631`.
Low-bit runtime flags are then ORed into the same field, so family consumers
isolate the high nibble with `0xF0000000`.

A whole-`.text` direct-consumer census found:

| Measure | Result |
|---|---:|
| direct `object + 0xE0` family-discriminator loads | 10 |
| containing functions | 9 |
| `MOD` branches | present |
| `EFM` branches | present |
| `SCM` branches | present |
| `MRP` branches | **not found** |
| `MCV` branches | **not found** |
| `SHW` branches | **not found** |

The ten direct load sites are:

```text
0x1402F9E47  MOD / EFM
0x14030238C  SCM
0x14030255C  MOD / EFM
0x140302572  EFM
0x140302E21  MOD / EFM
0x140303A60  MOD / EFM
0x140303B89  MOD / EFM
0x14030BF58  MOD / EFM / SCM
0x14030D9CA  EFM
0x14030DA9A  SCM
```

Exact `0x30`-byte window offsets and hashes are sealed in the machine-readable
receipt. The full `.text` section is independently pinned as:

```text
VA start    0x140001000
file offset 0x400
size        0x34DD4E
SHA-256     9348fcf8e3ec2c9189eb930cde752f8a7fbb8c4701fbd1197da2b06d692d5522
```

## What this changes

This promotes the earlier model-neighborhood observation to a whole-`.text`
statement for the classifier-backed `object + 0xE0` representation:

> Every direct family discriminator found for this object field specializes
> only `MOD`, `EFM`, or `SCM`.

That reinforces the current architecture:

```text
PrimaryModelDocument
  MOD / EFM / SCM

Recognized families without this model-object specialization
  MRP / MCV / SHW
```

It does **not** prove that `MRP`, `MCV`, or `SHW` have no runtime consumers.
Their masks may be copied into another structure, consumed indirectly, selected
through a data table, or owned by a different runtime object layout. It also does
not establish any of their on-disk fields.

## MRP boundary after this pass

`MRP ` remains EXE-confirmed identity through the four-byte family classifier,
and `MRP` remains recognized by the three-byte registry probe. However, it now
has a stronger negative boundary:

- no normal registrar handler;
- no PAC/PNST child-dispatch handler;
- no primary model factory branch;
- no primary model memory-specialization branch;
- no direct `object + 0xE0` family consumer in the entire canonical `.text`.

Therefore MRP must remain:

> **runtime-recognized model/render-side companion; standalone mesh ownership
> and exact downstream owner are not proven.**

The next valid promotion requires either a real retail MRP payload or evidence
of a copied/indirect mask path into a different owner. No field schema should be
invented before that gate.
