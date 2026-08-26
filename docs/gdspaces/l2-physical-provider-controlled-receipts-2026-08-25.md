# GDSpaces Layer 2 — controlled physical-provider receipts — 2026-08-25

**Scope:** Layer 2 / Runtime Resolver only.

**Status:** product controlled receipts implemented; Windows bounded parity receipt pending final exact-head confirmation after review hardening.

This receipt supersedes the older implementation statement that the current physical phase always uses a source-derived `0x0C` `ResourceKeyIndex`.

## Current physical implementation

The resolver keeps archive and physical mechanisms distinct:

- archive provider: source-derived `ResourceKeyIndex` with recovered `0x0E` semantics;
- physical provider with `IDirectPathSource`: source-native direct path lookup;
- physical provider without that capability: explicit `product_physical_index` fallback.

`LocalDirectorySource` implements `IDirectPathSource`. Candidate construction, `0x0C` normalization and provider ordering remain owned by the DMC3 Layer-2 resolver. The source receives only the already-normalized relative provider key and returns the canonical `ResourceRef` belonging to the mounted source.

The direct capability is fail-closed: empty or non-C-string-compatible input, absolute/out-of-root paths, native I/O failure, foreign/invalid identity, or structurally inconsistent status/resource output must not be converted into a successful lookup.

## Evidence classes

### `recovered_archive_index`

Reserved for the archive backend where normalized sorted lookup is directly recovered from the original runtime.

### `product_physical_native_path`

Used when a physical source resolves through its native path backend.

This is deliberately product-classified. It does not claim universal Win32 equivalence because GDSpaces adds containment hardening and non-Windows hosts have different filesystem semantics.

### `product_physical_index`

Compatibility fallback for physical sources without direct-path capability. It remains mechanically different from the recovered original type-0 direct Win32 path.

## Controlled receipts implemented

`physical_provider_receipt_tests.cpp` covers:

1. zero archive volumes + first physical candidate hit through `IDirectPathSource`;
2. complete six-candidate native physical miss;
3. one archive volume missing all six archive candidates, followed by first physical candidate hit;
4. preservation of the actual mounted-source `ResourceRef` identity rather than manufacturing a request-case identity.

`runtime_resource_resolver_tests.cpp` adds a Windows-only bounded parity receipt:

1. create an actual `GDataX360.afs/CaseProbe.PAC` file;
2. normalize/query the case-variant `GDataX360.afs/caseprobe.pac` with the old `0x0C` product index and prove that index misses;
3. call `CreateFileA` with the recovered original flags and prove the case-variant path opens on the Windows test filesystem;
4. resolve the same request through `RuntimeResourceResolver` + `LocalDirectorySource` and prove a native-path hit;
5. verify that the selected identity is the canonical enumerated `GDataX360.afs/CaseProbe.PAC` identity.

This demonstrates a concrete parity defect in the old index-only physical model and the bounded correction provided by direct native lookup.

## What this closes

- physical-provider product model: closed for the implemented direct-capability/fallback boundary;
- controlled physical hit: closed;
- controlled complete miss: closed;
- controlled archive-to-physical fallback: closed;
- Windows case-variant direct-path parity: closed only after the final exact-head Windows job for this reviewed head is green.

## What this does not close

- real DMC3 retail `0x0E` collision census;
- direct-retail resolver receipt;
- protected-distribution runtime address mapping;
- original-process selected-identity receipt;
- exhaustive Win32 error/retry equivalence;
- Layer 1 or Stage Ops.

## External evidence blockers

The known `dmc3-0.nbz` retail artifact is 960,358,951 bytes and cannot be transferred by the connected Drive raw path, whose current ceiling is 268,435,456 bytes. A central-directory/member-list artifact from that exact NBZ would be sufficient for the collision census.

The protected distribution executable is known from prior audit as 6,567,320 bytes, SHA-256 `81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6`. The raw binary is not currently present in Drive or File Library. Its on-disk `.text` is protected/high-entropy, so addresses recovered from the `e454...` analysis build must not be treated as original-process addresses without runtime reacquisition/mapping.
