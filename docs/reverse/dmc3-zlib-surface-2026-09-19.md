# Three functions of zlib, a dead deflate half, and the archive teardown

2026-09-19. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-zlib-surface`.

Two items from the open list: what uses zlib's deflate half, and `0x328210`.
Both close.

## The surface is three functions

Every direct `E8` or `E9` edge in `.text` whose target lies in
`0x340000`–`0x344000` and whose source lies outside it:

| target | is | called from |
| --- | --- | --- |
| `0x341900` | `inflate` | `0x328680`, `0x328820` |
| `0x343110` | `inflateEnd` | `0x327DB0`, `0x328FE0` |
| `0x343170` | `inflateInit2_` | `0x327BE0` |

Forty-one further targets in that range are called only from within it. Nothing
else crosses in, and nothing crosses in indirectly: the only qwords anywhere in
the image pointing into the range are ten entries of a table discussed below.

So the engine's entire use of the library is three functions, all on the inflate
side. That is the replacement surface for a port, stated exactly.

## The deflate half is linked and dead

`deflate.c` is compiled in. Its configuration table sits at `.rdata 0x50AAA8`:
ten entries of sixteen bytes, four sixteen-bit tuning values then a function
pointer, and the pointers fall into three groups — `0x340320` once, `0x3405E0`
three times, `0x340A50` six times. One compressor for level zero, one for the
next three levels and one for the top six is that library's exact shape, and the
copyright string `"deflate 1.2.5 ..."` at `0x50AA51` confirms which file it came
from.

Nothing calls it. The table is reachable only from the compressor entry point,
that entry point is not among the three edges above, and there is no indirect
path in. A port needs to reproduce no compression at all on this path.

## What that does to yesterday's CRC statement

Yesterday's note recorded that `crc32` at `0x340010` is "live, with 13 call
sites, every one of them inside `0x341000`–`0x343000`". That stands, and it can
now be said more precisely. The stream is armed with window bits `-15`, which
selects raw deflate with no wrapper, and the library computes a check value only
when it is parsing a wrapper. So those thirteen sites are on wrapper handling
this mode never enters: the function is live in the module and unreachable in
this engine's use of it.

That last step is an inference from the mode constant and the library's
structure, not a trace of each of the thirteen sites. What is read, and what the
port-relevant claim rests on, is unchanged: no call site is in the archive path,
and the CRC the record decoders store at entry `+0x18` is never loaded.

## `0x328210`, the archive teardown

It closes the archive object's lifecycle, which is now complete end to end:

- `0x328320` zeroes the 0x40 bytes and locates the trailer;
- `0x328D80` fills `+0x00` through `+0x18` and allocates the trailing comment
  into `+0x20`;
- `0x3289F0` builds the entry list at `+0x28`;
- `0x327CC0` builds the sorted array at `+0x30` with its count at `+0x38`;
- `0x328210` walks the list from `+0x28` through `+0x48`, freeing each node's
  three strings at `+0x30`, `+0x38` and `+0x40` and then the node; frees the
  sorted array at `+0x30` and nulls it; and tail-jumps to `free` for the comment
  at `+0x20`.

Four frees per node and then three, which matches the inline cleanup inside
`0x3289F0` that the first archive note read at `0x328B60` — the same shape,
written twice.

One detail worth having: it nulls only `+0x30`. The list head at `+0x28`, the
count at `+0x38` and the comment pointer at `+0x20` are left as they were, so
**the function is not idempotent** — a second call would walk a freed list. Its
one caller is the failure path in `0x326DFC`, which frees the containing mount
immediately afterwards, so nothing in the image calls it twice.

## A false edge, and the habit that catches it

The first run of the census showed a fourth inbound edge, `0x24479A` reaching
`0x3430FF`. Neither address is code. `0x3430F0` through `0x343107` is a switch
jump table — little-endian dwords `0x342E0C`, `0x342E39`, `0x342F18`,
`0x342F9F`, `0x342FAA`, `0x342178`, all targets inside `inflate` — and the
scanner had disassembled the table as instructions. The source end is data too.

Yesterday's habit was: before trusting a caller count of zero, disassemble the
bytes *before* the address. Its counterpart, which earned itself today: before
trusting a surprising edge, disassemble the bytes *at* both ends. A brute-force
byte scanner reports edges that do not exist, and one of them would have put a
call from the middle of the image into the middle of `inflate`.

## What is still unread

- Sections 1C, 1E and 1F of the 2026-09-05 reconciliation — the request
  construction, the two selection gates and the collision receipt.
- Whether the zlib build is otherwise unmodified against a reference build.
- The 1,641 functions no path reaches, still untouched as a population.
