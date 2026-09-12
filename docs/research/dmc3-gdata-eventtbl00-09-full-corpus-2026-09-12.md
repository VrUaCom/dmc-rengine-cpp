# DMC3 HD GData.afs EventTbl00-09 full-corpus correction

Date: 2026-09-12

## Source boundary

The owner supplied `EventTbl00.bin` through `EventTbl09.bin` and identified these ten files as the complete EventTbl set present in `GData.afs`. This corpus supersedes the earlier assumption that the separately supplied `EventTbl13.bin` through `EventTbl21.bin` represented the GData event-table set. The earlier files remain useful secondary EVT corpus evidence, but their archive origin is not asserted here.

No proprietary payload bytes are committed. Only hashes, sizes and structural observations are retained.

## Hash-bound complete GData.afs corpus

| file | bytes | SHA-256 | streams | terminal | commands |
|---|---:|---|---:|---:|---:|
| `EventTbl00.bin` | 1440 | `282c5297c15e5829a713b5c7404db48c633aaa14aef562b724c44badd240c194` | 1 | `0x590` | 211 |
| `EventTbl01.bin` | 608 | `b7453245decc847dd582bb1c6ef7a1bb0e77eb7c89fc8e0f0892503fd055a3c4` | 1 | `0x25C` | 79 |
| `EventTbl02.bin` | 1152 | `08c82e11d0d6e9e1522be8a31209f019260b2c04dd7a1a800c5c95b40f1f0ce6` | 2 | `0x474` | 118 |
| `EventTbl03.bin` | 4256 | `55464a10925a3a2b7ad7ced2a6a5f9d659c4eae63c1115da003eb0fd87684bd3` | 2 | `0x1098` | 485 |
| `EventTbl04.bin` | 3840 | `6ecb5623b39b5e2121a7d44b4271944258a1b7489a3da94b72029829d6dd3a0e` | 1 | `0xEE4` | 437 |
| `EventTbl05.bin` | 7552 | `93952fdca52a01de5edbdcc73944edf910aaaecb99dc4ba4a1f768367344a521` | 2 | `0x1D78` | 820 |
| `EventTbl06.bin` | 6880 | `2df1c12cb5e8de85752480d98252737b311a904a2b1db3211c3cd3472ebb5453` | 5 | `0x1ACC` | 712 |
| `EventTbl07.bin` | 15584 | `abf2441e37a7fa923bf9ef04a58298331f8c79c7e35544c1f0198a951297753d` | 6 | `0x3CC8` | 1646 |
| `EventTbl08.bin` | 4032 | `edb3c02a80246bd5f7faa5bf5c1c88a349e8ccf14a08c47ecba754688b3c70b4` | 1 | `0xFA8` | 432 |
| `EventTbl09.bin` | 3584 | `e5d646923fc25a9bb601b8e2144971e117315c027c687c76e0ec2816e00918fb` | 1 | `0xDE8` | 411 |

Across the ten files the command grammar walks 5351 commands without desynchronization. There are 103 observed opcode values, maximum opcode `0x89`, and maximum arity 6. Within this corpus every observed opcode has one stable arity.

## Corrected header ABI

The earlier bounded reader treated the full u32 at `+0x04` as a constant version `0x00010001`. The complete GData corpus disproves that interpretation.

The structural split is:

```text
+0x00  char[4]  "EVT\0"
+0x04  u16      revision      = 1 in 10/10
+0x06  u16      stream_count  = 1, 2, 5 or 6
+0x08  u32      terminal command absolute offset
+0x0C..+0x1F    reserved      = zero in 10/10
+0x20           first command / implicit stream 0 start
```

The old 32-bit `version` field remains exposed only as the exact packed raw word for source/API compatibility. It is no longer semantic authority.

## Stream-offset table

For `stream_count = N`, exactly `N-1` little-endian u32 offsets occur immediately after the terminal `0x20/argc0` command. They are not padding. The first stream start is implicit at `0x20`.

Observed explicit starts:

- EventTbl02: `0x280`
- EventTbl03: `0xFE0`
- EventTbl05: `0x1B80`
- EventTbl06: `0x1520, 0x1620, 0x1680, 0x1960`
- EventTbl07: `0x3620, 0x3720, 0x3780, 0x3A60, 0x3BE0`

All 13 explicit offsets are strictly increasing, 4-byte aligned, lie before the terminal, and land exactly on decoded command boundaries. Every target is opcode `0x57` with arity 1 and is immediately preceded by opcode `0x00` with arity 0. These two opcode roles remain structural observations; semantic names are not assigned without executable evidence.

After the offset table, remaining bytes are zero alignment padding. All ten file sizes are 0x20-aligned.

## End-of-stream variants

- every single-stream file ends `0x01/argc0 -> 0x20/argc0`;
- every multi-stream file ends `0x0E/argc0 -> 0x20/argc0`.

The parser validates the terminal `0x20/argc0` as structural authority. The pre-terminal pair and observed `0x57`/`0x00` stream-boundary pattern are currently warnings on variance, not hard semantic gates.

## Evidence level

`STRUCTURAL_CONFIRMED` for the physical EVT grammar above, bound to the complete owner-identified GData.afs `EventTbl00-09` corpus. Opcode meanings remain `PRESERVED_UNDECODED` pending canonical `dmc3.exe` interpreter/dispatcher reverse.
