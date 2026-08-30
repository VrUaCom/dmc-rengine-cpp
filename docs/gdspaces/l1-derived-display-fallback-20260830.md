# GDSpaces L1 derived display fallback — 2026-08-30

Status: **IMPLEMENTED IN VALIDATION BRANCH — NOT HISTORICAL NAME AUTHORITY**

Branch: `ada/l1-naming-display-fallback-20260830`

## Problem

The canonical naming pipeline correctly kept external `.index`, embedded aliases,
enclosing-container stored names, semantic format evidence and physical identity
separate. However, when a PAC/PNST had no retained external `.index`, a synthetic
parser label such as `slot_0000.bin` could survive as the primary display name.

That made a correctly materialized resource look as if the naming pipeline had
not run at all. It also made profile-specific structures such as DMC3 texture
bundles depend accidentally on `.index` processing before they could receive
`ptx`/`dds` semantic identity.

## Canonical correction

DMC3 profile structural semantics are now reconciled independently of external
`.index` availability:

```text
materialized payload bytes
    -> generic magic semantic evidence
    -> DMC3 profile structural semantic evidence
    -> ResourceNamingIdentity
```

The profile semantic pass is sealed by `ContainerNamingReconciler`, so a UI
suffix or filename cannot forge `ResourceSemanticEvidence`.

For a populated resource that is still synthetically named after all exact
naming authorities have been considered, the DMC3 naming snapshot may derive a
presentation-only name:

```text
physical container stem
    + extracted ordinal among populated payloads
    + byte/structure-backed canonical extension
```

Examples:

```text
GData.afs/obj/em000.pac + ordinal 0 + texture-bundle
    -> em000_000.ptx

GData.afs/scr/st001.pac + ordinal 0 + HITS
    -> st001_000.hits
```

Nested synthetic containers retain physical topology in the derived stem so the
presentation remains deterministic rather than collapsing unrelated descendants
onto one name.

## Authority boundary

A derived display name is **not**:

- an external `.index` label;
- evidence that the historical extractor used that filename;
- an embedded alias;
- an enclosing-container stored name;
- a `ResourceId` component;
- a write/repack target;
- legacy extraction/export authority.

`external_index_*` remains absent when no exact `.index` exists. The underlying
`ResourceRef.synthetic_name` also remains true. Exact external or direct stored
names always outrank the derived display.

Unknown semantic bytes use a `.bin` presentation suffix only as an explicit
product fallback; this does not manufacture format evidence.

## Regression boundary

`dmc3_naming_pipeline_tests` now pins the no-index case, including a structurally
valid DMC3 texture bundle:

```text
slot_0000.bin (physical parser placeholder)
    -> semantic_format = texture-bundle
    -> canonical_extension = ptx
    -> canonical_display_name = em000_000.ptx
```

The test also verifies that external-index evidence remains absent and that
physical `ResourceId` and payload bytes do not change.
