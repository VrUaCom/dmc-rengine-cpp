# GDSpaces Layer 2 — DMC3 type-0 physical-provider reverse — 2026-08-25

**Scope:** Layer 2 / Runtime Resolver only. This receipt does not promote Layer 1 or Stage Ops claims.

**Status:** instruction-backed static reverse recovered; controlled runtime/parity receipt still open.

## Authority

Canonical executable reacquired from the project file library:

- file: `dmc3.exe`
- size: `6,356,432` bytes
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

The findings below were re-derived from that artifact rather than copied from the earlier Pass 45/49 summaries.

## Recovered chain

### Physical mount registration — `0x140326D20`

The physical mount node is created with:

- provider type `0`;
- path-normalization flags `0x0C`;
- a duplicated root-path string;
- prepend-style insertion into the runtime mount list.

### Runtime mount resolve — `0x140327430`

For a type-0 mount that is eligible under the current provider mask, the resolver:

1. copies the candidate into a bounded `0x400` path buffer;
2. applies `ResourcePathNormalize(..., 0x0C)`;
3. combines the physical root and normalized candidate through `0x1403272C0`;
4. calls the final physical open helper at `0x140327800`;
5. treats a normal open miss as a miss for that mount and continues provider traversal;
6. returns the physical stream wrapper on successful open.

The root join does not insert an extra `\\` when the existing root ends in `/`, `:` or `\\`; otherwise it inserts `\\` before appending the candidate. The destination capacity is `0x400`.

The provider-level bounded copy/join has a fail-closed overflow path. This observation does **not** by itself close the separate `OpenGameResource` `0x400` candidate-construction question tracked by the Layer-2 roadmap.

### Final file open — `0x140327800`

The original physical backend calls Win32 exactly as:

```text
CreateFileA(
    path,
    GENERIC_READ,      // 0x80000000
    FILE_SHARE_READ,   // 0x00000001
    NULL,
    OPEN_EXISTING,     // 3
    0,
    NULL)
```

Failure mapping:

- `ERROR_FILE_NOT_FOUND (2)` -> ordinary miss / false;
- `ERROR_PATH_NOT_FOUND (3)` -> ordinary miss / false;
- other `CreateFileA` errors -> the helper retries the open rather than converting them to a fallback filename or alternate provider identity.

On success the helper allocates the physical stream wrapper and a `0x4001` read buffer and returns success to `ResourceMountResolve`.

### Existence helper — `0x140327720`

`ResourcePathExists` calls `FindFirstFileA(path, ...)` directly.

- success -> `FindClose(handle)` and true;
- `ERROR_NO_MORE_FILES (18)` -> false;
- `ERROR_FILE_NOT_FOUND (2)` -> false;
- `ERROR_PATH_NOT_FOUND (3)` -> false;
- other errors -> retry.

## Case-semantics boundary

After the recovered `0x0C` normalization there is no additional game-side lowercasing or archive-style qsort/bsearch comparison in the recovered type-0 open edge. The normalized path is passed to `FindFirstFileA` / `CreateFileA`, so the final physical filename-resolution behavior is delegated to the Win32 path APIs and the underlying Windows filesystem configuration.

This is intentionally narrower than claiming that every Windows filesystem is universally case-insensitive.

## Product-model consequence

The current GDSpaces physical pass is still implemented as a source-derived `ResourceKeyIndex` over `ISource::enumerate()`. That is a deliberate portable/product-safe representation and is **not** the same mechanism as the recovered direct Win32 path open.

Therefore:

- the original physical-provider contract is now recovered at instruction level;
- `RuntimeLookupEvidenceClass::product_physical_index` must remain product-classified for the current implementation;
- renaming that product lookup to recovered/original-equivalent without a controlled provider parity receipt would be authority laundering;
- the next roadmap gate is an explicit physical-provider model/receipt that compares the portable GDSpaces result to the recovered original contract.

## Layer-2 status impact

- **L2-R1 exact type-0 physical provider after `0x0C`: CLOSED for static reverse.**
- **L2-V3 controlled physical-provider receipt: OPEN.**
- physical-vs-archive ordering already recovered remains unchanged.
- retail `0x0E` collision census remains the next corpus gate after the physical-provider model/receipt.
- Layer 2 as a whole is **not complete**.
