# It is zlib 1.2.5, and the layout I called non-standard is standard

2026-09-19. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-zlib-identification`.

Yesterday's note left the inflater's layout inferred from its users rather than
its initialisation, and guessed that `0x327DD2` inside `0x327DB0` was where the
object is built. Both are now settled. `0x327DB0` is the destructor, the
constructor is a branch of `0x328360`, and the object is exact. Along the way two
sentences of mine turn out to be wrong, one of them for an instructive reason.

## The library names itself

`0x327BE0` clears `0x58` bytes at `inf+0x1010`, zeroes three pointers inside that
region, loads `-15` into `edx`, the address `0x5086F8` into `r8` and `0x58` into
`r9d`, and calls `0x343170`. The bytes at `0x5086F8` are `"1.2.5"`.

That is `inflateInit2_(strm, -15, "1.2.5", 0x58)` — the version-checked
initialiser, with **negative window bits, which select raw deflate with no
wrapper**. The project's existing description of this path as raw DEFLATE, in
`docs/gdspaces/decompilation-layer-classification.md`, is now backed by the
constant that selects it rather than by the shape of the stream.

The rest of the library is present and says the same thing. `.rdata` holds
`"deflate 1.2.5 ..."` at `0x50AA51` and `"inflate 1.2.5 ..."` at `0x50BEA1`,
alongside the standard message set — `"incorrect header check"`,
`"invalid distance too far back"`, `"need dictionary"`. Both halves are compiled
in, although only inflate is reached from the archive path; what uses deflate, if
anything, is not read here.

With the version fixed, the four functions the archive path calls identify
themselves by their call shapes:

| address | is |
| --- | --- |
| `0x343170` | `inflateInit2_` |
| `0x341900` | `inflate` |
| `0x343110` | `inflateEnd` |
| `0x340010` | `crc32` |

`0x343110` is worth showing, because it settles a question yesterday's note could
not:

```text
343110  test rcx,rcx          ; strm
34311E  mov  rax,[rcx+0x28]   ; state
343127  mov  r8,[rcx+0x38]    ; zfree
343130  mov  rdx,[rax+0x38]   ; state->window
343139  mov  rcx,[rcx+0x40]   ; opaque
34313D  call r8               ; zfree(opaque, window)
343148  call [rbx+0x38]       ; zfree(opaque, state)
34314D  mov  [rbx+0x28],0     ; strm->state = NULL
34315B  mov  eax,-2           ; Z_STREAM_ERROR on a null argument
```

Free the window, free the state, null the state pointer. That is `inflateEnd`.

## The layout I called non-standard

Yesterday I wrote that the four fields at `inf+0x1010` through `inf+0x1028` are
"the shape of a streaming decompressor's descriptor, not any particular published
one: the common library's layout puts a total-in counter between available-in and
next-out, and this does not."

That is wrong, and the reason is worth keeping. zlib's `z_stream` does put
`total_in` between `avail_in` and `next_out` — as a `uLong`. On LP64 that is
eight bytes and `next_out` lands at `0x18`. On **Windows x64 `unsigned long` is
four bytes**, so `total_in` occupies `0x0C`, inside what would otherwise be
padding, and `next_out` lands at `0x10`. Every offset then follows:

| offset | field |
| --- | --- |
| 0x00 | `next_in` |
| 0x08 | `avail_in` |
| 0x0C | `total_in` |
| 0x10 | `next_out` |
| 0x18 | `avail_out` |
| 0x1C | `total_out` |
| 0x20 | `msg` |
| 0x28 | `state` |
| 0x30 | `zalloc` |
| 0x38 | `zfree` |
| 0x40 | `opaque` |
| 0x48 | `data_type` |
| 0x4C | `adler` |
| 0x50 | `reserved` |
| 0x58 | size |

`0x58` is the size the code passes to `inflateInit2_`, and `state` at `0x28`,
`zalloc` at `0x30`, `zfree` at `0x38` and `opaque` at `0x40` are exactly what
`0x343110` and `0x327BE0` read and write. It is stock zlib. I had computed the
offsets for the wrong data model and then reported the mismatch as a property of
the image.

## The inflater object, exactly

The constructor is the `method != 0` branch of `0x328360`:
`calloc(1, 0x1080)`, `[inf+0x00] = ` the compressed substream, `[inf+0x08] = `
the uncompressed size from `[entry+0x20]`, then `0x327BE0` to arm it.

| offset | width | holds |
| --- | --- | --- |
| 0x00 | 8 | the stored substream over the compressed bytes |
| 0x08 | 4 | the member's uncompressed length |
| 0x0C | 4 | uncompressed bytes produced so far |
| 0x10 | 0x1000 | the input buffer |
| 0x1010 | 0x58 | the `z_stream` |
| 0x1068 | 4 | the exhausted flag |
| 0x1070 | 8 | the input cursor |
| 0x1078 | 8 | the end of valid input |

`0x1078 + 8` is `0x1080`. Nothing is left over, and the `0x58` that `0x327BE0`
clears and that `inflateInit2_` is told is exactly the `z_stream` region, with
the exhausted flag beginning where it ends.

`0x327BE0` also rewinds: it sets the substream's position to zero, seeks the
reader to the member's data offset and requires the seek to land exactly, clears
the `z_stream`, initialises raw inflate, points the cursor and the end at the
empty input buffer, zeroes the produced count — and sets the exhausted flag from
`[inf+0x08] == 0`, so **a member of zero uncompressed length is born exhausted**.

## Ownership moves, and nothing leaks

The pairing of `+0x38` and `+0x40` on the entry is enforced, not assumed. When
the inflater is built successfully, `[entry+0x38]` is set to null and the
substream belongs to the inflater; when the allocation or the arming fails, the
substream and its reader are closed and freed and `[entry+0x38]` is nulled too.
So exactly one of the two is ever non-null, which is what makes the destructor's
two independent frees safe, and what `0x328540`'s open test and `0x328F50`'s
branch both rely on.

I suspected a leak here and there is none. `0x343110` is `inflateEnd`, and it is
called in both places that need it: the seek calls it and then `0x327BE0`
re-initialises, and the destructor `0x327DB0` calls it before freeing the object.
The window and the state are zlib's own allocations and both paths release them.

`0x327DB0`, in order: if the inflater exists, `inflateEnd`, close and free the
compressed substream's reader and the substream, free the inflater, null
`[entry+0x40]`; if the stored substream exists, close and free it and null
`[entry+0x38]`; then free `[entry+0x00]`, which by then is normally already null
because realisation freed the path, and null it. Two callers: a 29-byte wrapper
at `0x327D90` and the failure path inside `0x328540`.

## Corrections

- `docs/reverse/dmc3-inflate-semantics-2026-09-18.md` says `0x327DD2` inside
  `0x327DB0` is "presumably where the object is built". It is the destructor; the
  constructor is a branch of `0x328360` that the same note had already read for
  its stored-member case without noticing it also holds the other.
- The same note's claim that the stream descriptor is not any published layout is
  wrong, for the LLP64 reason above.

Neither changes any behavioural claim in that note — the seek, the truncation
semantics and the absence of a CRC check stand — but the second is the kind of
error that produces a confident negative, which is worse than an undercount.

## What is still unread

- What uses zlib's deflate half.
- `0x328210`, the archive teardown, still.
- Sections 1C, 1E and 1F of the 2026-09-05 reconciliation.
- Whether the zlib build is otherwise unmodified; the version string and the
  message set say 1.2.5, and nothing here checks its code against a reference
  build.
