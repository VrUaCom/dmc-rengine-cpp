# Both halves of the name invariant

2026-09-18. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-mount-registration`.

> **Corrected the same day** by
> `docs/reverse/dmc3-bootstrap-confirmation-2026-09-18.md`. The normaliser table
> below is incomplete — it omits an unconditional repeated-separator collapse —
> and the bit map and the `0x0E`/`0x0C` asymmetry were already recorded in
> `docs/reverse/dmc3-resource-vertical-proof-reconciliation-2026-09-05.md` §1D,
> which this note failed to cite.

The mount-list note ended with a hazard stated and not verified: the archive
lookup is a `bsearch`, so it depends on the array being sorted under exactly the
normalisation the query goes through, and a mismatch misses rather than
degrades. Both halves are now read. They agree, and the agreement is not a
coincidence — it is the same function called with the same flag word from both
sides.

## One mount struct, two kinds, the archive embedded

`0x326D20` mounts a directory and `0x326DFC` mounts an archive, and they are the
same shape: `calloc(1, 0x58)`, fill, then push onto the head at `0xCF3180`
through `+0x50`.

| offset | width | holds |
| --- | --- | --- |
| 0x00 | 4 | the kind: 0 a directory, 1 an archive |
| 0x04 | 4 | the normalisation flags this mount applies |
| 0x08 | 8 | a `_strdup` of the base path or the archive path |
| 0x10 | 0x40 | the archive object, embedded |
| 0x50 | 8 | the next mount |

88 bytes with nothing left over, and the embedded object is exactly the 0x40
bytes the archive note saw zeroed at `0x328320` without knowing what contained
them. A directory mount leaves that region zero and pays 64 bytes for the
uniformity.

The archive path is duplicated twice over: once into the mount here, and again
into each entry by `0x328290`, which is what lets an entry outlive nothing and
still reopen the file.

Registration pushes onto the head, so **the most recently mounted source is
searched first**. The resolver takes the first hit, so the override policy is
"last mounted wins", and it is one `mov` pair rather than a priority field.

Mounting an archive does five things in order: open the file, initialise the
embedded object with `0x328320`, close the reader with `0x3276F0`, build the
index with `0x327CC0`, and only then set the kind to 1 and publish the mount. If
any step fails, `0x328210` tears the embedded object down and the mount is
freed. So a mount is never visible in a half-built state, and the archive file
is **not** held open between mounting and use — every entry reopens it, as the
mount-list note recorded.

## The index builder, and the guard beside the unbounded loops

`0x327CC0` takes the archive object and does exactly this:

1. null `+0x30` and `+0x38`;
2. walk the entry list from `+0x28`, linked at `+0x48`, calling
   `0x327160(node->name, 0x0E)` on each — **normalising the stored names in
   place** — and incrementing `+0x38`;
3. `malloc(count * 0x10)` with an overflow guard: `mul rcx` followed by
   `cmovo rax,-1`, so an overflowing product asks for `SIZE_MAX` and fails the
   allocation instead of under-allocating;
4. fill the array, `{name pointer, node}` per sixteen bytes;
5. `qsort` with count `+0x38`, width `0x10` and comparison `0x3291D0`.

`0x328160` then normalises the query with `0x327160(buffer, 0x0E)` and calls
`bsearch` with the same width and the same `0x3291D0`. Same normaliser, same
flag word, same comparator, both sides. The invariant holds by construction
rather than by discipline, which is the version a port should copy.

The overflow guard is worth pausing on. This is the same layer that spins
forever on an I/O error in three of its wrappers, and it is also the layer that
clamps `n + 1` in `0x327650` and `count * 16` here. Carefulness about integer
arithmetic and carelessness about failure are not the same axis, and this code
sits at opposite ends of them.

## The comparator is `strcmp`, and that is not the whole answer

`0x3291D0` is an inlined byte-wise compare on the first field of each pair,
returning ±1 via `sbb eax,eax` / `or eax,1` rather than a difference. Taken
alone that reads as a case-sensitive lookup. It is not, because of what runs
first.

`0x327160` is a flag-driven normaliser, and all of its bits are now read:

| bit | effect |
| --- | --- |
| 0x01 | upper-case ASCII `a`–`z` |
| 0x02 | lower-case ASCII `A`–`Z`, **only when 0x01 is clear** |
| 0x04 | strip leading `/` and `\`, shifting the rest down |
| 0x08 | strip trailing `/` and `\` |
| — | unconditionally, replace every `/` with `\` |

The flag word both sides use is `0x0E`: lower-case, strip leading, strip
trailing, plus the unconditional separator conversion.

So archive lookup is **case-insensitive for ASCII and case-sensitive above
`0x7F`**. The folding is a range test on `A`–`Z` and touches no byte with the
high bit set, so two names differing only in the case of a non-ASCII byte are
two distinct entries in the array and only one of them is findable by a query
spelled the other way. A port that reaches for a locale-aware or UTF-8-aware
case fold here changes which files resolve.

The unconditional `/` to `\` conversion matters in the same way: names are
canonicalised to backslashes on both sides, so a port on a filesystem that
treats `\` as an ordinary character must do the conversion in its index too,
not only at the boundary.

## Three separators, in a bitmask

`0x326EE0` builds a base path by finding the first directory mount and copying
its path, then appends `\` unless the last character already ends the path. The
test is a 64-bit bitmask:

```text
movabs r14, 0x200000000801
...
sub cl, 0x2F          ; cl = last character - '/'
cmp cl, 0x2D
ja   append
bt   r14, rcx
jb   do_not_append
```

The set bits are 0, 11 and 45, which against the `0x2F` base are `/`, `:` and
`\`. So a base path ending in a drive or stream separator is left alone, which
is why `C:` does not become `C:\` here and a path that already ends in either
slash is not doubled.

## What is still unread

- Who calls the two mount registrations, and in what order at startup — the
  override policy is now legible but the actual layering is not.
- `0x328210`, the archive teardown, beyond the fact that failure uses it.
- The inflater at `0x328820`, unchanged from the previous notes.
- Whether any code path re-runs `0x327CC0` after a mount is published; if not,
  the index is fixed for the mount's lifetime.
