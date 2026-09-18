# The handle is not a handle

2026-09-18. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-buffered-reader`.

The archive note read seven functions that pass something called a handle to
three wrappers. It is not a handle. It is a 40-byte object holding one, and
reading how that object is built changes what a port has to reproduce.

## What `0x327800` builds

`CreateFileA(path, 0x80000000, 1, NULL, 3, 0, NULL)` — read access, shared for
reading, open existing. Then, with `edx` set to 0x28 and `ecx` to
`0x28 - 0x27`, `calloc(1, 40)`, and the handle is stored at offset 0.

Then `malloc(0x4001)` — 16,385 bytes, not 16,384 — and

```text
mov BYTE PTR [rax+0x4000],0x4D
```

a guard byte of `0x4D` written into the one extra byte, one past the usable
window. Nothing in the file layer reads it back, so what checks it is not here;
what is here is that it is written deliberately and that the allocation is
sized for it.

The remaining fields follow immediately, and account for the 40 bytes exactly:

| offset | width | holds |
| --- | --- | --- |
| 0x00 | 8 | the Win32 handle |
| 0x08 | 8 | the buffer, 16,385 bytes |
| 0x10 | 8 | buffer + 0x4000, the capacity end |
| 0x18 | 8 | the current position inside the buffer |
| 0x20 | 8 | the end of valid data inside the buffer |

Both positions start at the buffer base, which is the empty state, and the three
users agree on that reading:

- `0x327910` refills when `[0x18] == [0x20]`, and only for requests below
  0x4000 — a larger request bypasses the buffer;
- `0x327A80` moves the unconsumed bytes to the front with `memmove`, reads
  `[0x10] - [0x20]` bytes, and advances `[0x20]` by what arrived;
- `0x327B60` resets both to the buffer base, and when the origin is
  `FILE_CURRENT` it first adds `[0x18] - [0x20]` to the requested displacement
   — a negative number equal to minus the unconsumed bytes, which is exactly
  the correction needed because the operating system's file pointer sits at the
  end of the buffered data rather than at the logical position.

Neither the `calloc` nor the `malloc` result is tested before use. The guard
byte is written into whatever `malloc` returned.

## Five functions, one error idiom, three with no way out

Every call into Win32 in this layer is wrapped in the same shape: call, test for
the failure value, call `GetLastError`, compare it against the errors worth
giving up on, and otherwise do the whole thing again. There is no counter, no
delay and no backoff anywhere in it.

| function | import | gives up on | otherwise |
| --- | --- | --- | --- |
| `0x327800` | `CreateFileA` | 2, 3 | retries forever |
| `0x327720` | `FindFirstFileA` | 0x12, 2, 3 | retries forever |
| `0x3277C0` | `GetFileSize` | nothing | retries forever |
| `0x327B60` | `SetFilePointer` | nothing | retries forever |
| `0x327A80` | `ReadFile` | nothing | retries forever |

The bottom three have no exit from the loop at all. The only way out is the API
succeeding. A closed handle, a revoked share, a disk that has gone away — each
spins the calling thread between two imports with nothing bounding it.

Two details make the idiom recognisable as one piece of source rather than five
coincidences. Each of the five calls `GetLastError` once per comparison **plus
once more whose result is never read** — three calls and two comparisons in
`0x327800`, four and three in `0x327720`, one and none in each of the other
three. And each retry rebuilds its arguments from scratch rather than reusing
the first attempt's, which is why the loop bodies are as long as they are.

A zero-byte read is not treated as failure, correctly: `ReadFile` returning true
with zero bytes is end of file, `0x327A80` stores the zero and returns success,
and `0x327910` checks for it at `0x327985` and stops. The unbounded case is a
genuine failure, not exhaustion.

One case is readable but not reachable in practice. `GetFileSize` returns
`0xFFFFFFFF` both on failure and for a file of exactly 4,294,967,295 bytes,
distinguished only by `GetLastError` returning `NO_ERROR`. `0x3277C0` calls
`GetLastError` and discards the result, so on such a file it would loop forever.
The image cannot address a file that large anyway, for the reason below.

## The layer is 32-bit throughout

`SetFilePointer` is called with `lpDistanceToMoveHigh` as `NULL` (`r8d` zeroed
at `0x327B90`), and the `FILE_CURRENT` correction is computed in 32-bit
registers. The size wrapper compares a 32-bit result. Nothing in the layer
carries a 64-bit file offset.

That converges with the archive note from the other direction: the record fields
for sizes and offsets are 32-bit, and none of the format's 64-bit extension
constants appears anywhere in the image. Two independent readings, one of the
platform calls and one of the record decoders, agree that four gigabytes is the
ceiling.

## A second axis between the two layers

The file-access note separated the two file layers by coupling — forty-six users
per dependency against two. They separate on error handling as well, and more
sharply.

In layer A, `0x326000`–`0x32A000`, each of `CreateFileA`, `GetFileSize` and
`SetFilePointer` is called from exactly one function and **twice** from it: the
attempt and the retry. `GetLastError` appears 15 times across 7 of its
functions.

In layer B, `0x048000`–`0x04B000`, `CreateFileA` and `GetFileSize` are called
once each from `0x049120` and `SetFilePointer` once from `0x049290`. Neither
function calls `GetLastError` at all; the layer's single call site is elsewhere,
at `0x04ACAC`.

So one layer retries every transient failure indefinitely and the other never
asks what went wrong. Nothing here says which is older or why the engine has
both — that question is still open from the earlier note — but it is now
separable on two independent axes rather than one, which makes an accident less
likely as the explanation.

## A correction to the file-access note

That note called the contents of layer A "thin wrappers of 43 to 353 bytes
around each" import. The bottom of that range is right and the top is not.
`0x327910` at 353 bytes is a buffered reader with a refill path, a bypass for
large requests and an end-of-file case; `0x327A80` at 218 bytes does the
`memmove` and the refill; `0x327800` at 261 bytes opens a file *and* constructs
the reader object. Calling them thin understated what a port replaces. The
census of imports and callers in that note stands; the description of what sits
between them did not.

## What is still unread

- What reads the guard byte at buffer + 0x4000. Nothing in this layer does.
- The open-entry object that `0x328540` operates on: `[+0x00]` is a pending
  descriptor freed once realised, `[+0x34]` is the local record's offset,
  `[+0x38]` and `[+0x40]` are two alternative realisations of which `[+0x40]`
  goes to the compressed-stream side at `0x328FE0`, and `[+0x48]` is a count
  clamped against `[+0x20]`. The object's size is not established here.
- Why layer B exists. Still open, now with a second difference to explain.
