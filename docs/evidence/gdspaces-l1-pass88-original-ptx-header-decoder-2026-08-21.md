# GDSpaces L1 Pass 88 — Corroborated original PTX header decoder — 2026-08-21

## Scope

Layer 1 only. Pass 88 extends the Pass 87 original-PTX geometry model with **raw header decoding** recovered from instruction-level analysis of two preserved derivative DMC3 executables.

This is a **HIGH / corroborated** evidence slice, not yet a canonical-original promotion receipt. The canonical original executable is currently not materialized in this session.

## Canonical target

- expected original: `dmc3.exe`
- expected size: `6,356,432` bytes
- expected SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Corroborating derivative executables

Both preserved derivative files are 6,415,872 bytes and share the same original `.text` layout plus a DMC Rengine extension section.

### Phase 17

- file: `dmc3_phase17_reng_probe.exe`
- SHA-256: `9a3513db0f7cfeabed38f62836a5a6d55e42741b0965bfa5947d3c7b33532735`

### Phase 18

- file: `dmc3_phase18_red_orb_x2_hook.exe`
- SHA-256: `88febb349b4deff0b907de76f98359307f7484f7cb82b0018aa236a60591c5b0`

A whole-file comparison finds only **79 differing bytes in 7 runs**. None overlap the PTX/TIM2 evidence windows below.

## Byte-identical evidence windows

The following file/VA windows are byte-identical in both derivatives:

| Purpose | VA range | File-offset range | SHA-256 |
| --- | --- | --- | --- |
| `TM2\0` entry parser | `0x1403365B0..0x14033669F` | `0x3359B0..0x335A9F` | `67362ff3937d96bb3057080ef9672f12b7fc693a0a5c530a4d9d9906a90e6c55` |
| PTX envelope loop A | `0x140336A70..0x140336B94` | `0x335E70..0x335F94` | `a6c00bdc669c4df5dc47ebf7bc5e91df56b0565a5c0d1d266e6ea2f8b4822c45` |
| PTX envelope loop B | `0x140336BB0..0x140336CD4` | `0x335FB0..0x3360D4` | `706f2bbc88bf7456358449c7d6b96c1f72bfc58bb4d2c9c94b5d48cfffa257af` |

## Recovered raw PTX header reads

PTX loop A begins:

```text
0x140336A93  mov ebp, dword ptr [rcx]
0x140336A95  lea r14, [rcx+0x04]
0x140336A9B  lea r15, [rcx+0x800]
0x140336AAA  test ebp, ebp
...
0x140336AC5  mov rcx, r15
0x140336AC8  call 0x1403365B0
```

This establishes the raw decoder candidate:

```text
textureCount  = u32le(base + 0x00)
blockCount[i] = u32le(base + 0x04 + i*4)
first TIM2    = base + 0x800
```

The loop advance is directly visible:

```text
0x140336B4C  mov ecx, dword ptr [r14]
0x140336B4F  inc edi
0x140336B51  shl ecx, 0x0B
0x140336B54  add r14, 0x04
0x140336B58  shr rcx, 0x02
0x140336B5C  lea r15, [r15+rcx*4]
```

The shift/right-shift/LEA sequence has the net effect:

```text
nextEntry = currentEntry + blockCount[i] * 0x800
```

PTX loop B independently repeats the same raw-field reads and advancement sequence at `0x140336BD3..0x140336CA2`.

## Important correction to Pass 87 semantics

The runtime loop does **not** reject a zero `blockCount` before advancing. Therefore `blockCount` is modeled as a **runtime cursor advance in 0x800-byte sectors**, not automatically as an intrinsic physical byte size.

Pass 87 was corrected accordingly:

- zero advances are preserved;
- a later entry may resolve to the same physical `TM2\0` location;
- the final advance is observed but is not promoted as a proven physical EOF.

## `TM2\0` parser evidence

At `0x1403365BA` the entry parser performs:

```text
cmp dword ptr [rcx], 0x00324D54
```

which corresponds to bytes:

```text
54 4D 32 00 = "TM2\0"
```

The parser then reads another entry-relative field at `+0x08` and follows it as an offset/pointer candidate. Exact semantics of that field remain a later pass target and are not named by Pass 88.

## Implementation

Pass 88 adds `OriginalPtxEnvelopeParser`:

1. decode `textureCount` from `+0x00`;
2. enforce the product safety ceiling of 64 entries;
3. decode `blockCount[]` from `+0x04`;
4. require the decoded table to remain within the first `0x800` bytes;
5. delegate physical entry validation to Pass 87 `OriginalPtxEnvelopeGeometryValidator`;
6. retain all unparsed header bytes as opaque source bytes.

## Promotion boundary

This pass is **not canonical-original EXE confirmed yet** because both instruction sources are derivative executables.

Promotion requires one exact control:

> extract the canonical original byte windows for `0x1403365B0`, `0x140336A70`, and `0x140336BB0` from SHA-256 `e454...d082` and require byte equality (or reconcile any difference instruction-by-instruction).

Until that control exists:

- code may remain in draft/research status;
- do not label `+0x00/+0x04` as canonical-original confirmed in the public authority registry;
- do not implement a PTX/TIM2 writer from this header layout alone.

## Next reverse target

The direct `TM2\0` parser at `0x1403365B0` already exposes a deeper boundary:

- `u32` at entry `+0x08` is added to entry base and passed to `0x140046C40`;
- word at entry `+0x04` is copied into the parsed result;
- helper `0x140046E20` consumes a region beginning at entry `+0x38` when a downstream pointer is absent.

Next pass: recover these entry-relative fields and the called helper contract without importing standard TIM2 assumptions that are not proven by DMC3 code.

## Layer-1 status

Layer 1 remains **NOT COMPLETE**. Pass 88 materially reduces original-PTX uncertainty but does not close TIM2 payload semantics, conversion, serialization, retail corpus, or original-game consumption.
