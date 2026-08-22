# GDSpaces L1 Pass 87 — Runtime PTX/TM2-DDS envelope structural recovery — 2026-08-21

## Scope

Layer 1 only. This pass recovers the **read-side runtime envelope ABI** used by the DMC3 HDC PTX/TM2 texture path.

The recovered structure is a live runtime representation. This pass does **not** prove that the same `TM2\0` wrapper exists byte-for-byte in retail PAC/NBZ storage. The on-disk producer/materializer boundary remains open.

No writer API is promoted.

## Correction to earlier texture interpretation

Earlier Phase16/17 wording treated the recovered `TM2\0` path and DDS-from-memory loading as separate representations. Pass 87 supersedes that narrow interpretation.

Direct executable evidence now proves this live chain:

```text
runtime PTX envelope
  -> TM2\0 runtime entry
  -> entry-relative DDS pointer (+0x08)
  -> DDS byte size (+0x3C)
  -> DDS validator/loader
  -> gfxTexture / D3D11 texture
```

This correction does **not** invalidate Pass 86's provenance boundary. The preserved raw PAC census still contains DDS signatures with no `TM2\0` signatures in the known samples, and no direct receipt yet proves how retail/storage bytes become the live runtime wrapper.

Known preserved raw-PAC census from Phase16 evidence:

```text
st001.pac   17 DDS / 0 TM2
id100.pac   21 DDS / 0 TM2
pl000.pac    4 DDS / 0 TM2
em000.pac   19 DDS / 0 TM2
m09_b01.pac 30 DDS / 0 TM2
TOTAL       91 DDS / 0 TM2
```

Therefore the correct current distinction is:

```text
on-disk/source texture representation        -> NOT YET FULLY PROVEN
runtime PTX/TM2-DDS envelope representation -> EXE-CORROBORATED READ ABI
```

## Executable evidence source

A preserved generated executable was materialized from the ChatGPT Library:

- file: `dmc3_phase17_reng_probe.exe`
- size: `6,415,872` bytes
- SHA-256: `9a3513db0f7cfeabed38f62836a5a6d55e42741b0965bfa5947d3c7b33532735`
- PE entry RVA: `0x34615C`, matching the canonical DMC3 target entry point;
- first eight section names/layouts match the canonical target family;
- one additional executable section exists: `.reng` at RVA `0xDAC000`, raw offset `0x60E600`;
- `.reng` contains marker `DMC_RENG_PHASE17_PROBE`.

The recovered PTX/TM2/DDS instructions are in the original `.text` section, not in `.reng`.

The probe is **corroborating executable evidence**, not a replacement for the canonical unpacked EXE SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`. Promotion to canonical exact-byte authority still requires parity of the cited byte windows against that executable.

Probe `.text` SHA-256:

`6b16d64ea8da9e3a1a0afe415f628328a209bba023364ab6f769db538d84c5ce`

### Independent derivative control

A second preserved executable created for an unrelated later patch was independently materialized:

- file: `dmc3_phase18_red_orb_x2_hook.exe`
- size: `6,415,872` bytes
- SHA-256: `88febb349b4deff0b907de76f98359307f7484f7cb82b0018aa236a60591c5b0`
- binary `TM2\0` occurrences: exactly `1`, at the same `.text` location as Phase17.

The two `.text` sections are not globally identical: 23 bytes differ in the unrelated patch region around file offsets `0x1A444B..0x1A4462` (RVA approximately `0x1A504B..0x1A5062`). The PTX/TM2 regions are nevertheless byte-identical across both derivatives:

| Region | Length | SHA-256 in both executables |
| --- | ---: | --- |
| RVA `0x336340` PTX envelope fragment | `0x250` | `5c97f997f5e64f6a0d1a2651f359b8708ad6a3cad44174ba434dfee18cde7847` |
| RVA `0x3365B0` TM2 parse fragment | `0x130` | `175e55222adfe42bf93177cb15bc5361552be686d4f2926af8b749fea423f691` |
| RVA `0x2FDA66` caller window | `0x100` | `73cc021c2324b60e42da429e3c39a1eef1e49c3f6ae424c5cb9e951673c882d8` |
| RVA `0x30DF90` caller window | `0x80` | `d0ea5478091a7c2a9c43d39348e4bb2dcd2c5936a55edb391db81b083989c946` |

This materially raises confidence that the recovered runtime texture logic is inherited from the common DMC3 executable base and was not introduced by the Phase17 `.reng` probe.

## Runtime PTX envelope — VA 0x140336340

Direct instruction consequences:

```text
mov eax, dword ptr [rcx]       -> u32 textureCount @ envelope +0x00
lea rbx, [rcx+0x04]            -> u32 blockCount[] @ envelope +0x04
lea rsi, [rcx+0x800]           -> first TM2 runtime entry @ envelope +0x800
```

In the entry loop:

```text
call 0x1403365B0               -> parse current TM2 runtime entry
mov ecx, dword ptr [rbx]       -> read blockCount[i]
add rbx, 4                     -> next count cell
shl ecx, 0x0B                  -> blockCount[i] * 0x800
advance entry pointer by that span
```

Recovered runtime envelope ABI:

```text
B + 0x000: u32 textureCount
B + 0x004: u32 blockCount[textureCount]
...
B + 0x800: TM2 runtime entry 0
entry(i).offset = 0x800 + sum(blockCount[0..i-1]) * 0x800
entry(i).span   = blockCount[i] * 0x800 when blockCount[i] != 0
```

The runtime parses the current entry before reading that entry's `blockCount`. Product policy therefore remains conservative:

```text
non-final blockCount == 0 -> fail closed
final blockCount == 0     -> terminal entry bounded to supplied resource EOF
```

This is a bounded parser policy, not a universal format claim.

The bytes from the end of the count table to `+0x800` remain opaque.

## TM2 runtime entry — VA 0x1403365B0

Direct instructions:

```text
cmp dword ptr [rcx], 0x00324D54  -> entry +0x00 = "TM2\0"
mov ebp, dword ptr [rcx+0x08]    -> u32 relative interior offset
add rbp, rcx                      -> interior pointer = entry + offset
```

The deeper consumer chain now identifies the interior pointer as the DDS buffer pointer when the entry carries a non-zero DDS size.

## TM2 -> DDS bridge

`0x1403365B0` passes the entry and interior pointer to `0x140046E20`.

At `0x140046E20`:

```text
mov eax, [rdx+0x04]     where rdx = entry + 0x38
                         -> u32 entry +0x3C
lea r8, [rdx+0x18]      -> entry +0x50 metadata block
r9 = entry + *(u32*)(entry+0x08)
call 0x140047000
```

`0x140047000` stores the `+0x08` pointer and `+0x3C` dword in a small owned wrapper. Its virtual follow-up reaches `0x140046AF0`, which forwards that pointer and size to `0x1400499C0`.

`0x1400499C0` validates the forwarded buffer as DDS:

```text
input size >= 0x80
u32 +0x00 == 0x20534444      -> "DDS "
u32 +0x04 == 0x7C            -> DDS_HEADER size
u32 +0x4C == 0x20            -> DDS_PIXELFORMAT size
```

Therefore the runtime entry fields are promoted as:

```text
entry +0x08: u32 DDS-relative offset
entry +0x3C: u32 DDS buffer byte size
```

When `+0x3C != 0`, the bounded DDS blob is:

```text
DDS start = entry + *(u32*)(entry+0x08)
DDS span  = *(u32*)(entry+0x3C)
```

and the blob must remain within the entry span and begin with `DDS `.

When `+0x3C == 0`, no DDS ownership wrapper is allocated; Pass 87 therefore records `dds_present=false` and does not invent payload semantics for the `+0x08` value in that zero-size case.

## Product parser policy

`Dmc3PtxEnvelopeParser` is read-only and now enforces:

- minimum runtime envelope header `0x800`;
- `textureCount @ +0x00`;
- `blockCount[] @ +0x04`;
- count table bounded inside the first `0x800` bytes;
- entry placement from EXE-derived `blockCount * 0x800` geometry;
- `TM2\0 @ entry+0x00`;
- `DDS-relative offset @ entry+0x08` bounded inside the entry;
- `DDS byte size @ entry+0x3C`;
- if DDS size is nonzero: minimum `0x80`, full DDS span inside the entry, and `DDS ` magic at the computed offset;
- opaque header/trailing bytes reported rather than normalized;
- non-final zero block counts fail closed;
- final zero block count represented explicitly as `terminal_span_to_eof`;
- **no writer API**.

## What this pass proves

- exact runtime PTX envelope count offset/width;
- exact runtime block-count table offset/width;
- exact first-entry offset;
- exact sector multiplier;
- exact `TM2\0` runtime entry magic check;
- exact DDS-relative offset field at `+0x08`;
- exact DDS byte-size field at `+0x3C`;
- direct runtime `TM2\0 -> DDS -> gfxTexture/D3D11` loading chain;
- bounded read-side runtime materialization contract suitable for GDSpaces diagnostics and provenance tracking.

## What this pass does not prove

- that the `TM2\0` wrapper is stored byte-for-byte in retail PAC/NBZ files;
- the producer/materializer that creates the runtime wrapper from storage bytes;
- canonical-EXE byte parity for the derivative instruction windows;
- real retail PTX/TM2 runtime-buffer receipt;
- semantics of the remaining runtime entry metadata;
- an original texture writer/converter;
- original-game consumption of a rebuilt retail texture resource.

Layer 1 remains **NOT COMPLETE**.
