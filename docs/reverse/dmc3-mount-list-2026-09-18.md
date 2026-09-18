# Nothing hands it a handle — it walks a mount list

2026-09-18. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-mount-list`.

> **Corrected the same day** by
> `docs/reverse/dmc3-bootstrap-confirmation-2026-09-18.md`: three functions
> share the dispatch shape below, not two. The third, at `0x3272A0`, dispatches
> size and its archive branch leaves the file layer.

Two notes today ended on the same question from opposite sides: the archive note
asked what hands the directory reader a handle, and the buffered-reader note
asked what the open-entry object is. They have one answer. Nothing hands
anything a handle; a name is resolved by walking a list of mounts, and the
result is a sixteen-byte object that is either an archive entry or a file and
never both.

## The stream is sixteen bytes and has no vtable

`0x327430` begins with `calloc(1, 0x10)` and everything after it fills one of
two fields:

| offset | width | holds |
| --- | --- | --- |
| 0x00 | 8 | an archive entry, or null |
| 0x08 | 8 | a buffered reader, or null |

Exactly one is ever set. Two functions in the `.pdata` hole at `0x3275BD` do the
dispatching, and both are three instructions and a tail jump:

```text
0x3275C0  mov rax,[rcx]      ; read
          test rax,rax
          je  0x3275D0
          mov rcx,rax
          jmp 0x328F50       ; the archive entry read
0x3275D0  mov rcx,[rcx+8]
          jmp 0x327910       ; the buffered reader read

0x3275E0  ... jmp 0x329160   ; the archive entry seek
          ... jmp 0x327B60   ; the buffered reader seek
```

A hand-rolled tagged union discriminated by a null test, with the tail jump
making each dispatch free. Whatever a port does here, it replaces two functions
of nine instructions, not a class hierarchy.

## Resolution walks a list of mounts, first hit wins

A global at `0xCF3180` heads a singly linked list, linked at `+0x50`. Each node
carries a kind at `+0x00` and `0x327473` switches on it, taking the first mount
that produces something and freeing the stream and failing if the list runs out.

The caller's flags, in `bpl`, can exclude either kind: bit 1 skips archives
(`test bpl,2` before the archive branch) and bit 0 skips directories. So the
same resolver serves "look anywhere", "packed only" and "loose only" without a
second code path.

**Kind 1, an archive.** `0x328160` is called with the mount's index object at
`+0x10` and the requested name; on a hit, `0x328290` builds an entry from the
result and the mount's `+0x08`, and the entry goes into stream `+0x00`.

**Kind 0, a directory.** The name is copied into a stack buffer, normalised by
`0x327160` with the mount's own flags from `+0x04`, joined against the mount's
base path from `+0x08` into a 1,024-byte buffer by `0x3272C0`, and opened by
`0x327800`. The reader goes into stream `+0x08`.

So a directory mount and an archive mount are interchangeable at the call site,
and the ordering of the list is the whole of the override policy.

## The lookup is a binary search, which closes two fields left open

`0x328160` copies the name into a `0x400` buffer with a bounded copy that fails
on truncation, normalises it with flag word `0x0E`, and then calls `bsearch`:

```text
key     = { pointer to the normalised name, 0 }
base    = [index + 0x30]
count   = [index + 0x38]
width   = 0x10
compare = 0x3291D0
```

The index is a sorted array of sixteen-byte pairs, and the matched pair's second
field at `+0x08` is the central-directory node.

That closes something the archive note left open. The archive object is zeroed
across 0x40 bytes at `0x328320`, and of those the directory walk writes only up
to `+0x28`; `+0x30` and `+0x38` were zeroed and never written by anything in
that note. They are the sorted array and its count. What sorts the linked list
into that array is not read here, but the two ends now meet: the walk at
`0x3289F0` builds the list, and `0x328160` searches the array.

A consequence worth stating plainly for a port: lookup is `O(log n)` and
requires the array to be sorted by the same normalisation the query goes
through. Get the normalisation wrong and the search does not degrade, it misses.

## The entry is eighty bytes and carries the directory record at +8

`0x328290` is `calloc(1, 0x50)` followed by three sixteen-byte moves copying
0x30 bytes from the central-directory node into `[entry+0x08]`, then a `_strdup`
of the mount's `+0x08` into `[entry+0x00]`, freeing the entry and failing if the
duplicate fails.

The eight-byte shift is confirmed four times over by uses in three other
functions, each landing exactly on the archive note's field map shifted by
eight:

| used at | entry | record | is |
| --- | --- | --- | --- |
| `0x328482` | 0x12 | 0x0A | the compression method |
| `0x32843B` | 0x1C | 0x14 | the compressed size |
| `0x3285DC` | 0x20 | 0x18 | the uncompressed size |
| `0x328434` | 0x34 | 0x2C | the local record's offset |

None of those functions decodes anything; they read displacements that were
fixed by a decoder in a different function. Four independent agreements is a
stronger check on the field map than the decoder alone could give.

## Realisation is lazy, and it costs a handle and 16 KiB per entry

`[entry+0x00]` is a private copy of the **archive's path**, which is why
`0x328540` can do this on the first read and nothing before it:

- open the archive again with `0x327800`, passing that path;
- `free` the copy and null `[entry+0x00]`;
- read the local record through `0x328360`.

The handle is the archive's, opened afresh — a second `CreateFileA` on the same
file, a second `calloc(1, 0x28)` and a second `malloc(0x4001)`. Every entry a
caller holds open costs its own descriptor and its own sixteen-kilobyte buffer,
independent of how many entries come from the same archive. That is a real
resource fact and it is read from the calls, not inferred from behaviour.

Before realisation, `0x328540` reports the entry as open if either `+0x38` or
`+0x40` is set and closed otherwise, so the three states — pending, stored,
inflating — are readable from the object without a tag.

## What `0x328360` does after the local record

It reads the 30-byte record, decodes it, then **skips the name and extra field
rather than parsing them**: a loop reading `min(remaining, 0x200)` bytes into a
scratch buffer until `name_length + extra_length` bytes are gone. The local
record's name is never compared against the central directory's. The data offset
is then computed as

```text
[entry+0x34] + 0x1E + name_length + extra_length
```

taking the lengths from the *local* record and the base from the *central* one.

Then the compression method at `[entry+0x12]` decides:

- **zero** — `calloc(1, 0x18)`, a stored substream at `[entry+0x38]` holding the
  reader at `+0x00`, the data offset at `+0x08`, the position at `+0x0C` and the
  size at `+0x10`, twenty bytes used of twenty-four, and `[entry+0x40]` is
  nulled;
- **non-zero** — an inflater at `[entry+0x40]`, which this note does not read.

`0x328F50` then reads from whichever is set: the inflater through `0x328820`,
or the stored substream by clamping the request to `size - position`, calling
the buffered read and advancing the position. A request that clamps to zero
returns zero rather than failing.

## What is still unread

- What sorts the entry list into the array at index `+0x30`, and what builds
  the mount list at `0xCF3180` in the first place.
- The comparison function at `0x3291D0` and the remaining bits of the
  normalisation flag word `0x0E`; only bit 2, which strips leading separators,
  is read here.
- The inflater at `[entry+0x40]` and everything behind `0x328820`.
- Whether anything ever reopens or shares an archive handle between entries. On
  this path it does not.
