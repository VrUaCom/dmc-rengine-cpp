# PNST Read-Only Structural Parser

**Reconciled:** 2026-08-17  
**Status:** IMPLEMENTED BOUNDED STRUCTURAL SLICE / SYNTHETIC REGRESSION; CURRENT-PARSER HASH-BOUND REAL-CORPUS EXECUTION RECEIPT OPEN

## Purpose

PNST is promoted as a distinct container identity that shares the executable-backed relative-slot envelope with PAC without sharing PAC semantic slot schemas.

The clean generation uses one structural core:

```text
RelativeSlotContainer
  -> PacParser  (magic PAC\0)
  -> PnstParser (magic PNST)
```

This supersedes the duplicated parser direction in the old stacked #59 branch while preserving its evidence-backed sparse-slot findings.

## Structural envelope

```text
+0x00  char[4]  magic
+0x04  u32 LE   declared slot count
+0x08  u32 LE[] container-base-relative slot offsets
```

For PNST the exact magic is `PNST`.

Rules shared with PAC:

- zero offset = explicit empty physical slot;
- non-zero offset = populated member at container-base-relative offset;
- full declared slot topology is preserved;
- populated offsets must remain outside the header/table and inside the supplied byte span;
- bounded extraction extent uses next greater distinct populated offset or container end;
- duplicate offsets, if encountered, preserve separate slot identities sharing one inferred span;
- parser capacity limit is product safety, not original ABI.

## PNST-specific identity boundary

Sharing the slot-access ABI does **not** mean PAC and PNST have interchangeable semantic schemas.

A resource whose extension is `.pac` but whose first bytes are `PNST` is classified as PNST. Real extracted DMC3 corpus contains this condition, so magic-first classification is required.

The parser does not infer what any PNST slot means. Physical slot number remains positional identity inside the concrete container/schema context.

## Sparse topology

Sparse PNST is ordinary observed behavior, including very large declared topologies with comparatively few populated members. The clean synthetic regression locks an 11-slot topology with only slots 0 and 10 populated.

This specifically prevents the unsafe transformation:

```text
physical slots 0 and 10
        -> compacted children 0 and 1
```

Higher layers and `.index` metadata must retain the original declared slot identity.

## `.index` is not PNST runtime authority

Text `.index` manifests may themselves begin with a `PNST` directive. That textual directive is metadata and must not be confused with binary PNST container magic.

Latest evidence keeps `.index` as extraction/naming metadata with no promoted original DMC3 runtime lookup role. Sparse positional linkage remains blocked.

## Current validation

Synthetic regression covers:

- exact `PNST` magic;
- 11-slot sparse topology with slots 0 and 10 preserved;
- deterministic synthetic names required by `ContainerEntry`;
- magic-first classification despite misleading `.pac` extension;
- wrong-magic rejection;
- offset-into-table rejection.

Existing PAC regression runs through the same shared structural core, guarding the refactor from changing PAC behavior.

## Real-corpus gate

Passes 14/17 already reacquired hash-bound binary PNST corpus, including sparse and large-topology samples. Therefore corpus discovery is not the blocker.

The remaining bounded gate is:

```text
reacquired PNST corpus
  -> this exact clean PnstParser
  -> sanitized deterministic validation receipt
```

Until that execution exists, the parser is evidence-aligned/implemented but not called real-corpus validated in the clean generation.

## Not part of this slice

- recursive PAC<->PNST tree expansion;
- `.index` promotion;
- `.lst` synthesis;
- NBZ provider path;
- semantic PNST slot schemas;
- original typed-postload/runtime lifecycle equivalence.
