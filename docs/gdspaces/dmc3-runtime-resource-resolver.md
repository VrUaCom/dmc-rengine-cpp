# DMC3 Runtime Resource Resolver — successful mount topology correction

**Reverse authority:** canonical `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, 6,356,432 bytes.  
**Status:** corrected implementation slice under issue #237; Layer 2 remains open pending real retail/runtime evidence and final audit.

The resolver composes the recovered DMC3 lookup order while preserving three distinct facts:

```text
filename discovery
 != registration attempt
 != successful linked mount
```

Only the final category may become a runtime provider node.

## Recovered resolver surface

Relevant canonical path:

```text
bootstrap 0x14002E930
 -> physical registration 0x140326D20
 -> archive registration 0x140326DA0
 -> successful nodes prepend to global head 0x140CF3180
 -> OpenGameResource 0x14002FCA0
 -> ResourceMountResolve 0x140327430
```

`ResourceMountResolve` walks the linked list from the actual head through node `+0x50`. A registration that failed before prepend has no node and is therefore absent from resolver traversal.

This is why the product resolver now accepts `RuntimeMountTopology`, not `VolumeBootstrapPlan`.

## Resolution pipeline

```text
raw request
 -> ResourceLookupPolicy: basename + recovered 12 ordered attempts
 -> ResourcePathPolicy: provider-specific normalization
 -> RuntimeMountTopology: only explicitly successful linked nodes
 -> archive: successful type-1 nodes in prepend-derived order
 -> physical: type-0 node only if physical registration succeeded
 -> ResourceRef / ambiguity / miss
 -> SourceRegistry read by downstream caller
```

Discovery remains relevant to bootstrap diagnostics and next-volume authoring, but it is not resolver authority.

## Archive topology

Successful archive registrations are attempted in ascending discovered-index order and prepend their nodes. Therefore resolver order is the reverse of the **successful registration set**.

Clean case:

```text
0 success, 1 success, 2 success -> 2,1,0
```

Sparse reverse-valid case:

```text
0 success, 1 mount failure, 2 success -> 2,0
```

Volume `1` in the sparse case is not queried and is not recorded as a lookup miss. It never became a linked archive provider node.

For a complete miss with three successful archives and a successful physical node, probe count remains:

```text
6 archive candidates * 3 mounted archives + 6 physical candidates = 24
```

For two successful archives and no physical node:

```text
6 * 2 = 12 archive probes
0 physical probes
```

## Physical topology

Bootstrap attempts physical registration before numbered archives, but it does not consume the return value of `0x140326D20`.

If physical registration fails, the type-0 node does not exist in the mount list. The product resolver therefore skips physical provider operations entirely instead of manufacturing six `not_found` probes for a provider that was never linked.

If the physical node exists, its provider behavior remains separated from archive behavior.

### Recovered physical path

The type-0 edge applies `0x0C` normalization, joins the registered root and candidate into the bounded path, and ultimately reaches the shared low-level open through the specific resolver caller edge:

```text
0x14032755C -> 0x140327800
```

The recovered final open is:

```text
CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)
```

`ERROR_FILE_NOT_FOUND` and `ERROR_PATH_NOT_FOUND` are ordinary open misses. This is distinct from archive `0x0E` qsort/bsearch lookup.

### Product physical evidence classes

When the mounted source exposes direct path lookup, the resolver uses source-native path behavior and marks probes:

`RuntimeLookupEvidenceClass::product_physical_native_path`.

Otherwise it derives a current-source `0x0C` `ResourceKeyIndex` fallback and marks probes:

`RuntimeLookupEvidenceClass::product_physical_index`.

Neither label is promoted as exact original Win32 equivalence merely because the static chain is known.

## Archive evidence class and collision gate

Archive lookup remains:

- entry pathname normalization with `0x0E`;
- CRT `qsort`;
- CRT `bsearch`;
- comparator `0x1403291D0` over normalized C-string only.

No recovered secondary equal-key tie-break exists. Therefore the real-retail `0x0E` normalized-key collision census remains mandatory.

Product `ResourceKeyIndex` preserves comparator-equal identities and reports ambiguity rather than inventing a semantic duplicate winner.

Archive probes remain:

`RuntimeLookupEvidenceClass::recovered_archive_index`.

This label refers to the recovered lookup mechanism/normalization class, not to a claim that a synthetic source is original runtime evidence.

## Archive lookup-hit failure boundary

Archive lookup success is not automatically selected-resource success:

```text
0x140328160 normalized lookup -> central entry
0x140328290 wrapper/open -> usable stream wrapper
```

If lookup hits but wrapper/open fails, `0x140327430` exits through cleanup/null. It does not continue to a lower archive as an ordinary miss.

The product/evidence model must therefore continue to fail closed for provider/backend failure rather than rewriting it as `miss`.

## Candidate order

For the observed canonical direct-call surface, all three direct callers of `OpenGameResource 0x14002FCA0` pass `flags = 1`.

Recovered outer candidate order:

1. `GDataX360.afs/<basename>`
2. `GData.afs/<basename>`
3. `Video/<basename>`
4. `afs/sound/<basename>`
5. `SAVEDATA/<basename>`
6. `<basename>`

All six archive attempts are processed before all six physical attempts.

Candidate order remains the outer loop; successful archive precedence is the inner loop. Therefore an earlier-prefix hit in a lower-precedence mounted archive still beats a later-prefix hit in a higher-precedence mounted archive.

## 0x400 request boundary

Candidate construction uses the recovered bounded helper `0x1403272C0` with capacity `0x400`. If the active candidate does not fit including NUL, the canonical direct-call path aborts the request rather than continuing to a shorter prefix.

The product whole-plan length guard remains fail-closed at this boundary.

## Source ownership

`ISource` remains exact identity/enumeration/read authority and `SourceRegistry` remains product mount/routing authority.

`RuntimeSourceBindings::valid_for(topology)` now requires:

- a physical source ID iff the successful topology contains a physical type-0 node;
- exactly one source binding for every successfully mounted archive;
- no archive binding for a discovered-but-failed archive;
- unique archive volume indices and source IDs;
- no archive source aliasing the physical source ID.

Every resolve call derives archive/product indexes from the exact currently mounted source enumeration. Caller-owned stale index pointers are not accepted.

## Fail-closed boundaries

- embedded-NUL/invalid request fails before source probes;
- discovery evidence cannot be passed directly as resolver topology;
- duplicate/undiscovered successful mount claims are invalid;
- a binding for an archive absent from successful topology is invalid;
- a physical binding without a successful physical node is invalid;
- a successful topology node without its exact source binding is invalid;
- mounted source enumeration with foreign/invalid resources is invalid;
- archive duplicate normalized identities are ambiguity, not an invented winner;
- archive lookup-hit + wrapper failure must not become a lower-volume miss;
- oversized first candidate aborts the request;
- no external index pointer/profile may establish resolver authority.

## Reverse regression receipts

Tests lock at least these topology cases:

```text
clean:  0 success,1 success,2 success -> probes 2,1,0
sparse: 0 success,1 failure,2 success -> probes 2,0
no physical node -> no physical probes
no linked nodes -> zero provider probes
```

The next-volume overlay integration test also separates concerns: filename discovery chooses the authored next volume number, while successful synthetic mounting is explicitly recorded before resolver winner assertions.

## Evidence references

- `l2-exe-reverse-pass-2026-08-26-pass2.md`
- `l2-mount-topology-lifetime-reverse-2026-08-26.md`
- `l2-archive-index-duplicate-key-reverse-2026-08-26.md`
- `data/reverse/dmc3-gdspaces-l2-resolver-static-census-2026-08-26.v1.json`
- merged PR #235
- issue #237

## Remaining Layer 2 gates

This product correction does not close Layer 2. Still required:

1. real-retail `0x0E` collision census;
2. real protected-process R2B v2 mapping receipt;
3. trusted process-bound R3 publisher/origin mechanism;
4. trusted original-process selected-provider identity receipt;
5. representative contradiction/reconciliation audit;
6. exact-head Windows + Ubuntu validation and explicit final L2 promotion.

No synthetic topology test is a substitute for those receipts.
