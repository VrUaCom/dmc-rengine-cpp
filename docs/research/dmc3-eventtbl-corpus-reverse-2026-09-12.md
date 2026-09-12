# DMC3 EventTbl runtime-family reverse — 2026-09-12

## Scope

Canonical scope is the complete runtime family `EventTbl00.bin` through `EventTbl21.bin` (22 indexed slots). The database/reverse model never drops a runtime slot merely because its current byte payload is unavailable.

- runtime slots modeled: **22/22** (`00..21`);
- byte payloads currently available and re-parsed directly: **19/22** (`00..09`, `13..21`);
- byte payloads currently unavailable: **10, 11, 12**;
- decoded commands from available bytes: **12,012**;
- raw u32 arguments: **16,927**;
- distinct observed opcodes: **121**;
- highest observed opcode: **`0x8C`**;
- maximum observed arity: **6**.

> Correction note: this report is regenerated from the actual supplied binary payloads. It supersedes earlier aggregate/per-file tables that disagreed with the bytes.

## Structural grammar

```text
+0x00  char[4]  "EVT\0"
+0x04  u16 LE   revision
+0x06  u16 LE   stream_count
+0x08  u32 LE   terminal_command_offset
+0x0C..+0x1F preserved/reserved bytes
+0x20          first command / stream 0 root
```

Command descriptor: `bits0..7=opcode`, `bits8..15=u32 argument_count`, `bits16..31=0` in every available payload. Serialized size is `4 + 4*argument_count`. The command at `terminal_command_offset` is `0x20/argc0`.

For `stream_count > 1`, `stream_count-1` absolute stream-root offsets follow the terminal command. In the available multi-stream corpus every explicit root lands on `0x57/argc1` and is immediately preceded by `0x00/argc0`.

Single-stream files end `0x01 -> 0x20`; multi-stream files end `0x0E -> 0x20`.

## Runtime slot coverage

| slot | stock file | bytes | SHA-256 | streams | commands | terminal | roots |
|---:|---|---:|---|---:|---:|---:|---|
| 00 | `EventTbl00.bin` | 1440 | `282c5297c15e5829a713b5c7404db48c633aaa14aef562b724c44badd240c194` | 1 | 211 | `0x590` | `0x20` |
| 01 | `EventTbl01.bin` | 608 | `b7453245decc847dd582bb1c6ef7a1bb0e77eb7c89fc8e0f0892503fd055a3c4` | 1 | 79 | `0x25C` | `0x20` |
| 02 | `EventTbl02.bin` | 1152 | `08c82e11d0d6e9e1522be8a31209f019260b2c04dd7a1a800c5c95b40f1f0ce6` | 2 | 118 | `0x474` | `0x20`, `0x280` |
| 03 | `EventTbl03.bin` | 4256 | `55464a10925a3a2b7ad7ced2a6a5f9d659c4eae63c1115da003eb0fd87684bd3` | 2 | 485 | `0x1098` | `0x20`, `0xFE0` |
| 04 | `EventTbl04.bin` | 3840 | `6ecb5623b39b5e2121a7d44b4271944258a1b7489a3da94b72029829d6dd3a0e` | 1 | 437 | `0xEE4` | `0x20` |
| 05 | `EventTbl05.bin` | 7552 | `93952fdca52a01de5edbdcc73944edf910aaaecb99dc4ba4a1f768367344a521` | 2 | 820 | `0x1D78` | `0x20`, `0x1B80` |
| 06 | `EventTbl06.bin` | 6880 | `2df1c12cb5e8de85752480d98252737b311a904a2b1db3211c3cd3472ebb5453` | 5 | 712 | `0x1ACC` | `0x20`, `0x1520`, `0x1620`, `0x1680`, `0x1960` |
| 07 | `EventTbl07.bin` | 15584 | `abf2441e37a7fa923bf9ef04a58298331f8c79c7e35544c1f0198a951297753d` | 6 | 1646 | `0x3CC8` | `0x20`, `0x3620`, `0x3720`, `0x3780`, `0x3A60`, `0x3BE0` |
| 08 | `EventTbl08.bin` | 4032 | `edb3c02a80246bd5f7faa5bf5c1c88a349e8ccf14a08c47ecba754688b3c70b4` | 1 | 432 | `0xFA8` | `0x20` |
| 09 | `EventTbl09.bin` | 3584 | `e5d646923fc25a9bb601b8e2144971e117315c027c687c76e0ec2816e00918fb` | 1 | 411 | `0xDE8` | `0x20` |
| 10 | `EventTbl10.bin` | **MISSING** | — | — | — | — | — |
| 11 | `EventTbl11.bin` | **MISSING** | — | — | — | — | — |
| 12 | `EventTbl12.bin` | **MISSING** | — | — | — | — | — |
| 13 | `EventTbl13.bin` | 2272 | `8b70cd2feacfe3770a438e5388b556e56f0655a51c6623ef57b45f5a7d853845` | 1 | 246 | `0x8D8` | `0x20` |
| 14 | `EventTbl14.bin` | 6880 | `38e6f035328a33da877c89ba248833c3f37c29e6eeeb877886ac23d2b6ceb991` | 1 | 668 | `0x1ADC` | `0x20` |
| 15 | `EventTbl15.bin` | 5760 | `4b8a99b8d802596e5279a91c2e48bdeed742306ac6fbbe23bbbf1176bbb470f3` | 1 | 597 | `0x167C` | `0x20` |
| 16 | `EventTbl16.bin` | 5152 | `b29310a26cb72c03402eda82fcbdc9b8e7223f120fa1c5fe0d36c2a8b84d3da8` | 1 | 580 | `0x1404` | `0x20` |
| 17 | `EventTbl17.bin` | 4128 | `74986d0584b3889d98d04aa7545496144ea20a164365804bdd7351d128c0d3d0` | 1 | 484 | `0x1010` | `0x20` |
| 18 | `EventTbl18.bin` | 6816 | `8370d6187f031023fcefa76ad104f7e9bcbcd37a8b7990d4317b90e5afb9015b` | 1 | 758 | `0x1A8C` | `0x20` |
| 19 | `EventTbl19.bin` | 4256 | `42f36fd5ad8aeca759ee0ff183864384687c80df539a14de9bba26786415d0ee` | 1 | 462 | `0x1098` | `0x20` |
| 20 | `EventTbl20.bin` | 416 | `1bc56695cb97bc793bde214da6f76d56e9f7434e654a5476b48b74d9bd32f348` | 1 | 51 | `0x190` | `0x20` |
| 21 | `EventTbl21.bin` | 31968 | `6a65f3842f9125273b1c328917b41f3af14e84dfce2e32f4e0fda874fd1c9b1e` | 1 | 2815 | `0x7CD0` | `0x20` |

## Structural scope evidence

- `0x07/argc1` and `0x08/argc0` are balanced across the available corpus.
- `0x09/argc1` and `0x0A/argc0` are balanced across the available corpus.
- `0x0B/argc1` and `0x0C/argc0` are balanced across the available corpus.
- `0x0D/argc2` and `0x0E/argc0` form the main scope pair, but multi-stream files contain exactly `stream_count-1` additional `0x0E` closures. Therefore `0x0E` also participates in stream/file boundary structure and must not be given a narrower meaning yet.

## Semantic registry already justified

The existing executable-backed meanings remain valid only where independently recovered from `dmc3.exe`. Corpus counts do not promote semantics by themselves. Current promoted families include structural control, controller/event state, item conditions, inventory add/subtract, registered/direct item spawn, and last-spawn state change. `0x57` remains only a corpus semantic candidate (monotonic position/timeline-like marker in secondary streams).

## SQL/reverse authority

The SQLite research layer models all 22 runtime slots. Available payloads are expanded into every command and every raw argument. Slots 10–12 stay present with `MISSING_BYTES` rather than disappearing from the model. Unknown opcodes/arguments remain `PRESERVED_UNDECODED`.

Machine-readable source: `docs/research/dmc3-eventtbl-opcode-census-2026-09-12.json`. Full runtime importer: `research/sql/import_eventtbl_runtime_family.py`.

## Reverse frontier

1. Continue handler/dispatcher reverse over the complete runtime opcode space, not per-file subsets.
2. Treat `EventTbl10..12` as active runtime slots now; when their bytes become available, import them without schema changes.
3. Promote a semantic name only with EXE/runtime or explicit corpus evidence.
4. Feed confirmed semantic classes to Native Reader `EventFlowView`; unknown nodes keep raw opcode, arguments, offsets and evidence status.
