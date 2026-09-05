# DMC3 HD canonical EXE — `.index` runtime reverse pass

Date: 2026-09-03

## Target

- file: `dmc3.exe`
- size: 6,356,432 bytes
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- preferred image base: `0x140000000`

## Result

**Bounded conclusion:** external `.index` is not an original-runtime lookup/materialization authority on the recovered canonical DMC3-HD resource path. The executable has a concrete packed-container -> `.lst` fallback and list-synthesis path, but no analogous `.index` selection, suffix construction, parser, or materialization branch was recovered.

This is a strong static negative for the recovered resource path, not a mathematical claim that no generic file API could ever open a caller-supplied filename ending in `.index`.

## Whole-image string / construction census

Fresh whole-image scans found:

- ASCII `.index`: 0
- ASCII lowercase `index`: 0
- ASCII `folder`: 0
- ASCII `PNST` / `pnst`: 0
- UTF-16LE equivalents above: 0
- capitalized `Index` occurrences are shader/RTTI vocabulary such as `matIndex`, not file paths.

To avoid relying only on literal-string absence, the `.text` instruction stream was also inspected for little-endian immediate construction of `index` fragments and byte-by-byte lowercase construction. No internal `index` suffix construction was recovered.

Positive controls:

- `lst` is written as immediate `0x74736c` at `0x1401B7A7B`, `0x1401B865E`, `0x1401B93EC`.
- `pac` is written as immediate `0x636170` at `0x1401B81F9`, `0x1401B8972`, `0x1401B8BB2`.
- runtime `.rdata` contains exact `dummy` at `0x1404DC4D8` and `lst` at `0x1404DC4E0`.

## Recovered representation selector

### `0x1401B79E0` — packed vs `.lst` representation selection

Bounded behavior:

```text
path = TypeInfo->path
if size_or_exists(path) > 0:
    return 1        // packed representation

find final '.' in path
if no extension boundary:
    return 0

copy path
replace bytes after '.' with "lst\0"
if size_or_exists(rewritten_lst_path) > 0:
    return 2        // loose-list representation

return 0
```

Direct xrefs: `0x1401B7BC2`, `0x1401B8CC5`.

No third `.index` branch exists in this selector.

### `0x1401B9390` — dedicated `.lst` extension rewrite

Scans backward for the final dot, copies the source path, writes `lst\0` after the dot, and returns success. No extension boundary returns false.

Direct xref: `0x1401B7BEE`.

## Materialization boundary

### `0x1401B8CA0`

For container-kind resources:

```text
mode = select_representation(...)
mode 0 -> acquisition failure
mode 1 -> 0x1402EF4D0 direct packed whole-file acquisition
mode 2 -> 0x1401B85C0 synthesize container from `.lst`
```

For non-container resources it directly calls `0x1402EF4D0` with the TypeInfo path.

There is no `.index` load/overlay branch in this original materialization dispatcher.

## `.lst` parser/synthesis positive-control chain

Recovered direct functions include:

- `0x1401B7C70` — first-line/magic directive handling; `/` skip/comment state, `#XXXX`, default PAC magic.
- `0x1401B7D10` — list-line enumeration.
- `0x1401B7E60` — indexed child-line resolution.
- `0x1401B7FD0` — required-size planning for synthesized container.
- `0x1401B85C0` — in-memory synthesis.

Nested-list handling compares the exact `lst` token, rewrites a sibling candidate to `pac`, gives the packed sibling precedence, and otherwise recursively synthesizes the list. Exact lowercase `dummy` is handled as the sparse/empty child marker.

This is the control case that a real executable-owned companion metadata representation looks like.

## Generic resource/open path

### `0x14002FCA0` — `OpenGameResource`

Recovered behavior includes basename-oriented archive candidate construction and provider iteration. It does not append or rewrite `.index`.

Direct xrefs found: `0x14003340A`, `0x1403380C7`, `0x1403381F7`.

### `0x140327430` — mount/provider resolution

Archive and physical provider branches operate on the candidate supplied by the caller. No `.index` extension synthesis was recovered.

### `0x140327800` — physical final-open

The physical branch reaches `CreateFileA` with the already-built candidate path. The resource path itself does not synthesize `.index` here.

## File API census

Relevant KERNEL32 imports include:

- `FindFirstFileA`
- `FindClose`
- `SetFilePointer`
- `ReadFile`
- `GetFileSize`
- `CreateFileA`
- `CreateFileW`

The resource-provider `CreateFileA` calls are at `0x14032783C` / `0x140327894`; the other `CreateFileA` site belongs to a separate generic file wrapper. The sole `CreateFileW` site opens an existing file read-only. No `.index`-specific bypass path was recovered.

## Classification

```text
.index as original runtime resolver authority       REJECTED on recovered canonical path
.index as original materialization companion       REJECTED on recovered canonical path
.index as physical slot identity authority         REJECTED
.index as extraction/naming metadata               SUPPORTED / corpus-confirmed elsewhere
extracted ordinal == physical slot                 REJECTED for sparse containers
.index entry N -> N-th populated extracted payload CORPUS-CONFIRMED elsewhere
.lst as original runtime fallback input             EXE-CONFIRMED
```

## Residual boundary

A whole-program static analysis cannot prove the universal proposition that no generic caller-provided filename ending in `.index` can ever reach an OS file API. What is closed here is the DMC3 canonical resource/container selection and materialization path: it has explicit packed/`.lst` handling and no recovered `.index` authority.

A future original-process trace can add dynamic absence/presence receipts, but it should not be required to model `.index` as a resolver unless it produces contradictory direct evidence.
