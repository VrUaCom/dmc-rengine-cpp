# Seeking a deflated member re-inflates it from zero, and truncation reads as a clean end

2026-09-18. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-inflate-semantics`.

## Why these two functions

`docs/gdspaces/proof-roadmap-2026-09-05.md` marks the NBZ STORE and raw-DEFLATE
reader with a warning and names exactly what would close it:

> only bounded original behavior is recovered | close only if broader
> original-equivalence claim needs `0x140328540`, `0x140328FE0`,
> malformed/partial-read semantics

`docs/roadmap.md` says the same in one line: "full original stream/error
equivalence remains open." `0x328540` was read earlier today, in the mount-list
note. This note reads `0x328FE0` and the partial-read semantics. Whether that
closes the item is the project's call and not mine; what follows is the reading.

The inflate path itself — the stored-versus-inflated branch at `0x328F50` and
raw-DEFLATE streaming at `0x328820` — is recorded prior art in
`docs/gdspaces/decompilation-layer-classification.md` at "the recovered scope".
Nothing below contradicts it.

## The inflater object

`[entry+0x40]`, read off its users rather than its constructor:

| offset | width | holds |
| --- | --- | --- |
| 0x00 | 8 | the stored substream over the compressed bytes |
| 0x08 | 4 | the member's uncompressed length |
| 0x0C | 4 | uncompressed bytes produced so far |
| 0x10 | 0x1000 | the input buffer |
| 0x1010 | 8 | next input |
| 0x1018 | 4 | available input |
| 0x1020 | 8 | next output |
| 0x1028 | 4 | available output |
| 0x1068 | 4 | the exhausted flag |
| 0x1070 | 8 | the input cursor |
| 0x1078 | 8 | the end of valid input |

The substream at `+0x00` is the same 24-byte shape the archive note recorded for
a stored member — reader at `+0x00`, position at `+0x0C`, size at `+0x10` — and
the refill reads `[sub+0x10] - [sub+0x0C]` bytes, so the compressed extent
bounds the input.

The four fields at `0x1010`–`0x1028` are next-in, available-in, next-out,
available-out packed at 0, 8, 0x10 and 0x18 of a sub-struct. That is the shape of
a streaming decompressor's descriptor, not any particular published one: the
common library's layout puts a total-in counter between available-in and
next-out, and this does not.

## Seeking, at `0x328FE0`

```text
origin 1 -> target += [inf+0x0C]     ; current position
origin 2 -> target += [inf+0x08]     ; length
otherwise   target is absolute
```

Then, in order:

- if the target equals the current position, return it and touch nothing;
- clamp a negative target to zero and a target above the length to the length;
- if the clamped target equals the length, set the exhausted flag and return —
  **seeking to the end decompresses nothing**;
- otherwise reset the decompressor at `inf+0x1010`, re-seek the compressed
  substream to the start of the member through `0x327BE0`, and then **read and
  discard exactly `target` bytes** in chunks of at most `0x2000` into a
  `0x2030`-byte stack frame obtained through the stack probe.

So any seek that is not to the current position or to the end **re-inflates the
member from byte zero**, and the cost is linear in the target offset. There is no
forward path: seeking one byte forward from the current position throws the
decompressor away and starts again. A port that adds the obvious optimisation —
inflate forward when the target is ahead of the current position — changes the
timing profile of every seek and, because the discard loop uses a different
driver than the read path, is not guaranteed to reproduce the same byte counts on
a damaged member.

On failure the function returns `-1`: if the re-seek fails it first clears
`0x58` bytes at `inf+0x1010`, and if a discard read comes back negative it
returns `-1` without clearing.

## Reading, at `0x328680` and `0x328820`

Both begin the same way and share every field offset: return **0** immediately if
the requested length is zero **or the exhausted flag is set**, so once a member
has ended every further read is a zero-length success rather than an error. Then
loop: point next-out and available-out at the caller's buffer, refill the input
when the cursor has reached the end, call the inflate step at `0x341900`, and
account for what came out.

The inflate step has exactly **two** call sites in the image — `0x328769` inside
`0x328680` and `0x328929` inside `0x328820` — so there are two drivers over one
object. `0x328F50` uses `0x328820`, passing the caller's flag word through `r9d`;
the seek's discard loop uses `0x328680`, which has no such parameter. Any
difference in their handling is a difference between reading a member and seeking
within it.

### Truncation is reported as a clean end

The refill clamps its request to the compressed bytes remaining. When none
remain, it sets the cursor and the end to the same address — available input
zero — writes a single zero byte at the buffer, and calls the inflate step
anyway. The step returns `-5`, and the code then checks whether the input buffer
is empty:

- empty — treat it as **stream end**, set the exhausted flag, return the bytes
  produced so far;
- not empty — an error: set the exhausted flag and return `-1`.

Any other code above 1 is an error by the same path; 0 and 1 are accepted, 1
being end of stream.

So a member whose compressed bytes run out before the decompressor says it is
done produces a **short read and no error**. The caller cannot distinguish a
truncated member from a member that was that long. That the codes `-5` and `1`
line up with a widely used library's buffer-error and stream-end conventions is
public interface knowledge, noted here and not relied on: the behaviour above is
read from the comparisons.

## Nothing verifies a member against its directory record

The image does contain the machinery. A 256-entry CRC-32 table sits at `.rdata`
`0x508A50` — its second entry and its reflected polynomial both present — and
`0x340010` is a table-driven CRC routine that returns zero for a null buffer and
tails into the body at `0x340020`. It is live: **13 call sites, every one of them
inside `0x341000`–`0x343000`**, the compression library that also holds the
inflate step.

Not one of those call sites is in the archive path, and the CRC the record
decoders store at entry `+0x18` is never read. Grepping the six functions that
handle an entry — `0x328360`, `0x328540`, `0x328680`, `0x328820`, `0x328F50` and
`0x328FE0` — for any access at displacement `0x18` returns only register spills
to `[rsp+0x18]`. So the engine decodes the CRC, keeps it, and never checks it. A
corrupted member inflates to whatever it inflates to.

## A near-miss worth recording

My first pass asked who calls `0x340018` and `0x340020` and got zero and one,
and I was one step from writing that the CRC machinery is unreachable dead code.
It is not: `0x340018` is a five-byte `jmp` and the real entry point is
`0x340010`, eight bytes earlier, with 13 callers. `0x340010` has no `.pdata`
entry, which is why neither the address I guessed nor a function enumeration
would have found it.

That is the fourth time in one day that `.pdata` hid something — after the two
ZIP signature decoders, the third dispatcher, and the 33 getters. The habit that
caught it was cheap: before trusting a caller count of zero, disassemble the
bytes *before* the address.

## What is still unread

- The inflater's constructor. `0x343110`, the reset, has two call sites: the
  seek, and `0x327DD2` inside `0x327DB0`, which is presumably where the object is
  built. Its full layout is therefore inferred from its users, not from its
  initialisation.
- `0x341900` itself, and whether the library is a stock build.
- `0x328210`, the archive teardown.
- Sections 1C, 1E and 1F of the 2026-09-05 reconciliation.
