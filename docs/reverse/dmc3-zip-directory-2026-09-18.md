# What the file layer does with a handle

2026-09-18. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-zip-directory`.

The file-access note of 2026-09-18 ended on an admission: the orchestrators in
the file layer combine size, seek and read, but nothing there said *into what*.
This note reads that. It is one archive format, read out of the instructions
that read it, and it closes the open item.

## Two kinds of claim, kept apart

Everything in the section "What the code does" is decoded from bytes in the
image, and every offset and size named there is an immediate or a displacement
in an encoding. Nothing in it depends on knowing what the format is called.

Everything in the section "What the format is called" is public interface
knowledge about a published container format, and is labelled as such for the
same reason `WM_DESTROY` was labelled in the window note: it is a different
kind of claim, resting on a public specification rather than on this image, and
a reader who distrusts it can delete the section without losing any of the
other one.

## Prior art in this project

`docs/reverse/gdspaces-blocked-window-acquisition.md` already lists, under
"Already strong architecture — do not restart broadly":

```text
0x140328540 ZIP/inflater lazy realization;
0x140328820 InflateRead;
0x140328F50 ZipEntryRead;
0x140328FE0 compressed seek/reset/reinflate architecture;
```

So the project had already located this neighbourhood and named its
decompression half. None of the functions read below appears in that list, and
no record layout or object layout was recorded anywhere in the repository. This
note adds the directory half to an area whose compressed-stream half was
already claimed. It sharpens an existing claim; it does not originate one.

## What the code does

### Finding the trailer: `0x328C30`

It calls the size wrapper `0x3277C0`, returns zero on a negative result, and
returns zero if the size is below 22 (`cmp esi,0x16` at `0x328C66`). The
candidate offset starts at size − 22 (`lea ebx,[rax-0x16]`).

Then it loops:

- read length is `min(size - candidate, 256)`, built as `mov edi,0x100` /
  `sub eax,ebx` / `cmp eax,edi` / `cmovl edi,eax`;
- seek to the candidate through `0x327B60` with `r8d = 0`;
- read through `0x327910` into a 256-byte stack buffer, and fail unless the
  full length arrives;
- walk that buffer comparing four bytes at a time — `[rax-1] == 'P'`,
  `[rax] == 'K'`, `[rax+1] == 5`, `[rax+2] == 6`, at `0x328CE0` through
  `0x328CF9`;
- on no match, step the candidate back by 234 (`lea eax,[rbx-0xEA]`) and clamp
  at zero with `xor ebx,ebx` / `cmovns`.

234 is 256 − 22. Consecutive windows overlap by exactly the length of the thing
being looked for, so a record straddling a window boundary is still seen whole.
The scan is not capped at one window: it walks back to offset zero.

On a match it converts the buffer position to an absolute file offset, writes
that through its second argument (`mov [r14],r8d`), assembles a 16-bit
little-endian value from record bytes 20 and 21, and accepts only when
`offset + 22 + value == size`. Then it calls `0x328D80` with the handle and the
same out-struct.

### The two record decoders: `0x327E40` and `0x328020`

These are one function written twice, for a 46-byte record and a 30-byte one.
Each takes a raw buffer in `rcx` and an output struct in `rdx`, and for every
field emits the same shape: `movzx` each byte, `shl` 8, `or`, low byte last, so
each field is assembled little-endian from single-byte loads. Each ends with

```text
cmp r9d, <constant>
sete al
ret
```

where `r9d` holds the first assembled dword — `0x02014B50` at `0x32800E` for
the 46-byte decoder, `0x04034B50` at `0x328151` for the 30-byte one. The
callers test that return value and abandon the record on zero.

Neither has a `.pdata` entry. Both sit in the 802-byte hole between
`0x327E3E` and `0x328160` that no runtime-function record covers, which is
legitimate for a function with no prolog and matters for method reasons set out
at the end.

Because every source offset and every destination offset is an immediate, the
layouts read straight off:

The extractor also reports what each decoder consumes and writes: 46 record
bytes into 48 struct bytes, and 30 record bytes into 32. The record figures are
derived purely from the decoders' displacements and match the read sizes at the
call sites, which are unrelated immediates in different functions — 46 from
`lea r8d,[r9+0x2D]` at `0x328A2D` and 30 from `lea r8d,[r9+0x1D]` at
`0x3283A3`. Two independent measurements agreeing is worth more here than
either alone.

46-byte record, destination struct:

| record | width | struct |
| --- | --- | --- |
| 0x00 | 4 | 0x00 |
| 0x04 | 2 | 0x04 |
| 0x06 | 2 | 0x06 |
| 0x08 | 2 | 0x08 |
| 0x0A | 2 | 0x0A |
| 0x0C | 2 | 0x0C |
| 0x0E | 2 | 0x0E |
| 0x10 | 4 | 0x10 |
| 0x14 | 4 | 0x14 |
| 0x18 | 4 | 0x18 |
| 0x1C | 2 | 0x1C |
| 0x1E | 2 | 0x1E |
| 0x20 | 2 | 0x20 |
| 0x22 | 2 | 0x22 |
| 0x24 | 2 | 0x24 |
| 0x26 | 4 | 0x28 |
| 0x2A | 4 | 0x2C |

30-byte record, destination struct:

| record | width | struct |
| --- | --- | --- |
| 0x00 | 4 | 0x00 |
| 0x04 | 2 | 0x04 |
| 0x06 | 2 | 0x06 |
| 0x08 | 2 | 0x08 |
| 0x0A | 2 | 0x0A |
| 0x0C | 2 | 0x0C |
| 0x0E | 4 | 0x10 |
| 0x12 | 4 | 0x14 |
| 0x16 | 4 | 0x18 |
| 0x1A | 2 | 0x1C |
| 0x1C | 2 | 0x1E |

Neither table was typed. `research/exe/extract_record_decoders.py` recovers
both from the image by walking the four instruction forms these functions use,
accumulating source displacements and closing a field at each store, and it is
fail-closed: an unrecognised byte inside the range aborts naming the offset
rather than being skipped, because a skipped instruction would move every field
after it. Run it against a local copy of the target and it prints the two tables
above. Its own tests synthesise the shapes and need no executable, and every
guard in it is mutation-killed, including the one that distinguishes a store
through `rdx` from the same ModRM byte with REX.B set — which is the exact
confusion that produced a decoder bug earlier in this reverse.

Both tables mirror the record exactly until the first 32-bit field that would
land on an odd multiple of two, at which point the destination shifts forward
by two and stays shifted. That is a C struct with natural alignment and no
packing pragma, and the shift is where the compiler inserted the padding. It is
worth saying plainly because it is a portability hazard in the other direction:
a port that declares these structs and memcpys a record into one gets different
answers from this code.

### The 22-byte reader and the object: `0x328D80`

Seeks to the offset the locator stored at `[rdx]`, reads 22 bytes
(`lea r8d,[r9+0x15]` with `r9d = 1`, then `cmp rax,0x16`), and assembles the
fields inline rather than through a decoder:

| record | width | object |
| --- | --- | --- |
| 0x00 | 4 | 0x04 |
| 0x04 | 2 | 0x08 |
| 0x06 | 2 | 0x0A |
| 0x08 | 2 | 0x0C |
| 0x0A | 2 | 0x0E |
| 0x0C | 4 | 0x10 |
| 0x10 | 4 | 0x14 |
| 0x14 | 2 | 0x18 |

Object offset `0x00` already holds the absolute offset the locator wrote, which
is why the record's own fields start at `0x04`. It then checks the assembled
first dword against `0x06054B50` and abandons on mismatch.

If the 16-bit value at object `0x18` is non-zero it `malloc`s exactly that many
bytes (import `api-ms-win-crt-heap-l1-1-0.dll!malloc` at `0x34F450`), stores the
pointer at object `0x20`, and reads exactly that many bytes into it — with no
terminator, unlike the per-entry strings below. Zero length stores a null
pointer.

Then it seeks to `[rbx] - [rbx+0x10]` — the located offset minus the size field,
not the offset field at `0x14` — and calls `0x3289F0`. Using the back-computed
position rather than the stored one is what makes the reader tolerate an
archive with anything prepended to it. On failure it frees the blob at `0x20`.

### The directory walk and the entry node: `0x3289F0`

Per iteration:

- read 46 bytes (`lea r8d,[r9+0x2D]` with `r9d = 1`);
- if at least 4 arrived and the first dword assembles to `0x06054B50`, stop and
  return success — the trailer is the terminator for this loop;
- otherwise require exactly 46 and decode with `0x327E40`, abandoning if the
  decoder's signature test returns zero;
- read three variable-length parts whose lengths are the 16-bit fields the
  decoder wrote at struct `0x1C`, `0x1E` and `0x20`, each through `0x327650`;
- `calloc(1, 0x50)` (import `calloc` at `0x34F458`) at `0x328AF5`;
- copy struct bytes `0x00`–`0x2F` into it with three 16-byte moves, then the
  three pointers to `0x30`, `0x38`, `0x40`;
- `mov rax,[rsi+0x28]` / `mov [rcx+0x48],rax` / `mov [rsi+0x28],rcx` — the old
  head becomes the node's next, and the node becomes the head.

So the entry node is 0x50 bytes: 0x30 of decoded scalars, three heap pointers,
and a next pointer at `0x48`, which accounts for the allocation exactly with
nothing left over. The list is singly linked at object `0x28`, built in reverse
directory order.

The teardown at `0x328B60` confirms the layout from the other side: walk from
`[rsi+0x28]`, save `[rdi+0x48]`, free `[rdi+0x30]`, `[rdi+0x38]`, `[rdi+0x40]`
and the node, then null the head. Four frees per node, three pointers and the
node itself, and nothing else — which is the strongest available statement that
0x30, 0x38 and 0x40 are the only owned pointers in it.

### The counted-string reader: `0x327650`

`(handle, char **out, size_t n)`. Nulls `*out`; returns success immediately for
`n == 0`; otherwise `malloc(n + 1)` with an explicit overflow clamp
(`add rcx,1` / `cmovb rcx,-1`), reads exactly `n`, frees and re-nulls on a short
read, and on success writes `0` at `[n]`. Every variable-length part of an entry
is therefore a NUL-terminated C string in memory regardless of what the archive
contains, while the trailer blob read directly at `0x328D80` is not.

### The local-record reader: `0x328360`

Seeks to a 32-bit value at `[rcx+0x34]` of its first argument, verifies the seek
landed there, reads exactly 30 bytes, and decodes them with `0x328020`. On any
failure it calls `0x3276F0`. This is the only caller of the 30-byte decoder.

### The chain has no branches

Scanning every `E8` and `E9` displacement in `.text` for these targets gives one
direct caller apiece: `0x328C30` from `0x328345`, `0x328D80` from `0x328D48`
inside the locator, `0x3289F0` from `0x328F1D` at the tail of the 22-byte
reader, `0x327E40` from `0x328A85` in the walk, `0x328360` from `0x3285B4`, and
`0x328020` from `0x3283CB` inside it. Only `0x327650` has more than one, and its
three are the three variable-length parts of a single entry. The directory side
of this reader is a straight line with a single entry point at `0x328345`, which
is why reading it end to end was possible at all and which bounds what a port
has to replace.

### Signature census

A raw scan of all 6,356,432 bytes for the seven four-byte constants the format
defines finds four occurrences of three constants and nothing else:

| constant | occurrences | RVA |
| --- | --- | --- |
| `0x02014B50` | 1 | `0x328011` |
| `0x04034B50` | 1 | `0x328154` |
| `0x06054B50` | 2 | `0x328A69`, `0x328EAF` |
| `0x06064B50` | 0 | — |
| `0x07064B50` | 0 | — |
| `0x08074B50` | 0 | — |
| `0x05054B50` | 0 | — |

All four occurrences are compare immediates in code, and all four are inside the
file layer of the 2026-09-18 note.

## What the format is called

This section is public interface knowledge, not a reading of this image.

The constants, sizes and field order above are those of the ZIP container as
published in the PKWARE application note. `0x06054B50` is the end-of-central-
directory signature and its fixed part is 22 bytes; `0x02014B50` is the central
directory file header and its fixed part is 46 bytes; `0x04034B50` is the local
file header and its fixed part is 30 bytes. Against that, the three record
layouts above name themselves field for field: the 46-byte record's three
16-bit lengths at 0x1C, 0x1E and 0x20 are the file name, extra field and file
comment lengths, its 0x2A is the relative offset of the local header, and the
trailer's 0x14 is the archive comment length. The check
`offset + 22 + comment_length == size` at `0x328D1C` is the standard
end-of-central-directory validation, and stepping back by 256 − 22 is the
standard way to scan for it.

Two consequences follow that are worth stating for a port, and both are read
from the image rather than from the specification:

- The absent constants are absent. There is no ZIP64 end-of-central-directory
  record, no ZIP64 locator and no data descriptor signature anywhere in the
  image, so this reader cannot address an archive that needs them. The
  directory offset it uses is a 32-bit field and the size fields it decodes are
  32-bit fields.
- The entry node carries the decoded central directory and nothing about
  position beyond it. A port that wants to open entries in a different order
  than the archive stores them has everything it needs; a port that assumes the
  list is in directory order has it backwards, because the walk prepends.

## A correction, and what it was worth

Earlier in this pass I scanned compare immediates function by function within
the file layer, found `0x06054B50` twice and nothing else, and said that it was
the only such constant present in the image. That was wrong, and the way it was
wrong is the useful part: the scan enumerated functions from `.pdata`, and the
two decoders holding the other two constants have no `.pdata` entry, so they
were never candidates. The scan was not mistaken about what it saw. It was
mistaken about what it was able to see, and it reported a census without
declaring its own population.

The raw byte scan that corrects it has no such blind spot and costs less to run.
It is the check that should have come first, and it is now what the packet
records. The lesson generalises past this note: any census in this project that
enumerates functions is a census of `.pdata`-covered functions, and the 802-byte
hole at `0x327E3E` is proof that this is not the same set as the functions.

What the miscount cost was one sentence. What correcting it bought is the
better claim — that the 64-bit extension constants are absent — which is a real
constraint on a port and which the earlier, narrower scan could not have
established at all.

## What is still unread

- `0x328540`, `0x328820`, `0x328F50` and `0x328FE0` are named in the earlier
  architecture list as the compressed-stream side. This note did not read them,
  and does not corroborate or contest their names.
- The compression method field is decoded but nothing here reads what dispatches
  on it.
- `0x328360` seeks to `[arg+0x34]`, which is not an offset in the entry node
  laid out above. Whatever object it receives is a different one, and this note
  does not identify it.
- The function at `0x328320` that calls the locator, and the one at `0x32858A`
  that calls the local-record reader, are the two ends this note stops at. What
  hands them a handle, and what consumes the entry list afterwards, is unread.
