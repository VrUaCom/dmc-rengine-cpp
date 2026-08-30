# GDSpaces L1 derived display fallback — 2026-08-30

Status: **IMPLEMENTED IN VALIDATION BRANCH — NOT HISTORICAL NAME AUTHORITY**

Branch: `ada/l1-naming-display-fallback-20260830`

## Problem

The canonical naming pipeline correctly kept external `.index`, embedded aliases,
enclosing-container stored names, semantic format evidence and physical identity
separate. However, when a PAC/PNST had no retained external `.index`, a synthetic
parser label such as `slot_0000.bin` could survive as the primary display name.

That made a correctly materialized resource look as if the naming pipeline had
not run at all. It also made profile-specific byte/structural semantics depend
accidentally on `.index` processing before they could influence presentation.

## Canonical correction

DMC3 profile byte/structural semantics are now reconciled independently of
external `.index` availability:

```text
materialized payload bytes
    -> generic magic semantic evidence
    -> DMC3 profile byte/structural semantic evidence
    -> ResourceNamingIdentity
```

The profile semantic pass is sealed by `ContainerNamingReconciler`, and only the
`Dmc3NamingPipeline` is allowed to invoke that sealing path. A UI suffix,
filename or arbitrary caller therefore cannot forge `ResourceSemanticEvidence`.

Two DMC3 evidence families currently feed that pass:

1. **Texture structure** — `TextureSlotFramingParser` proves a texture bundle or
   wrapped DDS and yields canonical `ptx` / `dds` presentation semantics.
2. **Recovered runtime content tags** — `ResourceTypeContract` records the
   original `dmc3.exe` three-byte content probe at `0x1402DB1F0` and the second
   dispatcher at `0x1401B9FA0`. Nameless payload prefixes `MOD`, `EFM`, `SCM`,
   `MRP`, and `SHW` therefore yield `mod`, `efm`, `scm`, `mrp`, and `shw`
   semantics from the same comparisons used by the original runtime.

The three-byte boundary is deliberate. Requiring a fourth byte would be stricter
than the recovered game code and would convert instruction-backed evidence into
a product invention.

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

GData.afs/obj/em000.pac + ordinal 1 + runtime tag MOD
    -> em000_001.mod

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

`dmc3_naming_pipeline_tests` pins the no-index texture case:

```text
slot_0000.bin (physical parser placeholder)
    -> semantic_format = texture-bundle
    -> canonical_extension = ptx
    -> canonical_display_name = em000_000.ptx
```

The existing pipeline corpus also exercises an `SCM` runtime content tag through
the same profile-semantic phase, while `ResourceTypeContract` compile-time
assertions pin `MOD -> mod` and the other recovered tag mappings. Whole-head CI
must remain green on both Ubuntu and Windows.

The tests additionally verify that external-index evidence remains absent where
no exact `.index` exists and that physical `ResourceId` and payload bytes do not
change.
