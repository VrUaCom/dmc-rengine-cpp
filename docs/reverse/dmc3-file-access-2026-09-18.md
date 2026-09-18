# DMC3 HD: how the engine reads files (2026-09-18)

Back to the subject matter after several rounds on verification. The biggest
uncovered thing a port needs: where the bytes come from.

**Evidence:** [`dmc3-hdc-file-access.evidence.json`](../../evidence/executable/dmc3-hdc-file-access.evidence.json)

## The surface is twelve functions

A game shipping gigabytes of assets reaches the operating system through almost
nothing:

| Import | Functions calling it |
| --- | ---: |
| `ReadFile` | 4 |
| `CreateFileA` | 2 |
| `GetFileSize` | 2 |
| `SetFilePointer` | 2 |
| `CreateFileW` | 1 |
| `FindFirstFileA` / `FindClose` | 1 |
| `GetFileInformationByHandleEx` | 1 |

**Twelve functions own the entire file surface**, the largest 486 bytes and the
smallest 60. That is listable only because the engine wrapped the platform once
instead of calling it from everywhere.

## One of them is a library

The functions owning most of that surface are not scattered. **57** sit in one
contiguous range:

```text
RVA 0x326000 – 0x32A000    57 functions    12,895 bytes
       774 functions outside call into it
        17 functions outside it calls
```

**Forty-six users for every dependency.** That ratio is what makes the range a
boundary rather than an address coincidence.

Inside, three tiers:

```text
KERNEL32 imports
  ↑
thin wrappers        0x3276F0 (43 B)  0x327720 (150 B)  0x3277C0 (60 B)
                     0x327800 (261 B) 0x327910 (353 B)  0x327A80 (218 B)
                     0x327B60 (115 B)
  ↑
orchestrators        0x328C30 — size, seek and read together
                     0x326DA0 — open and close
                     0x328360 — seek, read and close
```

It allocates heavily — `free` 13 times, `calloc` 7, `malloc` 4 — and **it names
nothing**: one string reference across 57 functions, and no resource-family
literal at all. Callers hand it names; it does not know what it is reading.

## There are two, and they are opposites

| | Range A | Range B |
| --- | --- | --- |
| address | `0x326000–0x32A000` | `0x048000–0x04B000` |
| functions | 57 | 33 |
| bytes | 12,895 | 11,047 |
| callers in | **774** | 16 |
| callees out | 17 | 10 |
| ratio | **46 : 1** | 2 : 1 |

Both call `CreateFileA`, `GetFileSize`, `ReadFile` and `SetFilePointer`,
independently. What separates them is readable:

- only **B** touches `CreateFileW` and the file-time conversions;
- only **A** touches `FindFirstFileA` and `FindClose`.

Two layers over one platform, of which one is a library and the other is not.
**Nothing here says why the engine has both** — that is an observation about
shape, and the shapes differ by more than twenty-fold.

A third function elsewhere, `0x36570`, calls `SHGetFolderPathW` beside
`CreateDirectoryW` and is the only place in the image that asks the system where
a per-user directory is.

## Querying it

Two views carry this. `v_exe_import_owner` lists the functions calling each
import with their size, callers and depth. `v_exe_range_coupling` measures any
range the same way — the range is chosen by whoever asks, so it measures a
boundary rather than discovering one.

Both figures were computed twice, once from the report and once through SQL, and
agree.

## Open work

- what the layer *does* with a handle is unread: the orchestrators combine size,
  seek and read, but nothing here says into what structure;
- Range B's purpose is unstated. Wide-char paths and file-time conversions are
  suggestive, and suggestive is not read;
- the archive formats themselves are untouched by this note. Knowing the bytes
  arrive through `0x327910` says nothing about their layout.
