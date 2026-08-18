# DMC3 Numbered NBZ Volume Bootstrap

**Status:** implementation prepared; current-main whole-head promotion pending.

## Recovered runtime policy

The DMC3 bootstrap probes numbered archives as `DMC3-N.nbz` beginning at index `0` and stops at the first missing index. Later files after that first gap are not runtime-equivalent mounts.

The physical data root is registered before numbered archives. Each archive mount is prepended to the runtime mount list, so registration order and effective archive resolution order differ:

```text
registration: 0, 1, 2, ... N
resolution:   N, ... 2, 1, 0
```

`VolumeBootstrapPolicy` receives product-side discovered numeric indices and produces this deterministic runtime-equivalent plan. It performs no filesystem I/O.

## Product integrity

The plan canonicalizes unsorted/duplicate discovery input. `valid()` verifies:

- physical-root-before-archives and prepend semantics;
- contiguous registered indices `0..first_missing-1`;
- canonical `DMC3-N.nbz` filename for every registered index;
- exact ascending registration metadata;
- exact reverse archive resolution order;
- post-gap diagnostic indices are strictly increasing, unique, and strictly greater than the first missing index.

Post-gap entries may be displayed diagnostically but are not silently mounted into the runtime-equivalent set.

## Boundary

This policy does not discover files, create `NbzZipSource` objects, bind source IDs, normalize resource keys, choose duplicate-key winners, resolve resources, synthesize `.lst`, or reproduce original FileSlot/cache/lifecycle ownership.
