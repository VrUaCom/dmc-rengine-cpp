# L1 embedded name-table semantic correction — 2026-08-28

## Status

This pass corrects the canonical display suffix of a structurally validated PAC slot-0 embedded name table.

## Evidence boundary

The retained `st001.index` external manifest labels physical PAC slot 0 as `st001_000.ukn`. That raw external label remains preserved as name evidence and is not rewritten.

The slot-0 payload itself is independently validated by the recovered retained `GDContainerNameHints.ts` contract as an embedded name-list authority. For the canonical `st001` corpus payload it contains:

- `st001.ptx`
- `st001.scm`
- `st001.sch`

and maps those aliases sequentially to physical slots 1, 2 and 3.

The canonical GDSpaces contract treats physical identity, external `.index` identity, embedded identity and semantic identity as separate fields. Therefore the slot-0 resource must not remain semantically `unknown` merely because the external manifest supplied `.ukn`.

## Correction

The C++ semantic format remains `name-list` so this resource is not confused with the separate companion external `.index` resource.

Canonical display suffix for a proven `name-list` changes from `.txt` to `.index`:

```text
physical identity: PAC slot 0
external index label: st001_000.ukn
semantic format: name-list (embedded name index)
canonical display: st001_000.index
```

If no external index label is available, the synthetic display is `<parent>_000.index`.

## Non-claims

- This does not claim that the retained extracted filename was literally `st001_000.index`; the retained stage dump physically used `st001_000.ukn`.
- This does not merge the embedded name table with the companion `st001.index` resource.
- This does not modify `ResourceId`, physical slot lineage, payload bytes, writer authority or external-index provenance.
- The raw `st001_000.ukn` label remains queryable persistent evidence after canonical display reconciliation.

## Regression

The existing real-corpus embedded-name regression now requires:

1. semantic format remains `name-list`;
2. canonical display is `st001_000.index` both before and after applying the external `st001.index`;
3. external-index persistent evidence still reports normalized name `st001_000.ukn`;
4. applying embedded evidence before or after external `.index` produces the same final display and physical identities;
5. all existing SHA-bound replay and fail-closed parser guards remain unchanged.
