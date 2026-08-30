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
    -> DMC3 profile semantic observation
       -> structural-parser evidence OR runtime-content-tag evidence
    -> ResourceNamingIdentity
```

The profile semantic pass is sealed by `ContainerNamingReconciler`, and only the
`Dmc3NamingPipeline` is allowed to invoke that sealing path. A UI suffix,
filename or arbitrary caller therefore cannot forge `ResourceSemanticEvidence`.

Crucially, the two profile provenance families are not collapsed into one enum:

- `profile_structural_format` means a structural parser proved the interpretation;
- `profile_runtime_content_tag` means the interpretation comes from the exact
  instruction-backed runtime content probe.

This distinction is retained both with and without an external `.index`, so an
index overlay cannot launder a runtime tag into a structural-parser claim.

Two DMC3 evidence families currently feed that pass:

1. **Texture structure** — `TextureSlotFramingParser` proves a texture bundle or
   wrapped DDS and yields canonical `ptx` / `dds` presentation semantics with
   `profile_structural_format` provenance.
2. **Recovered runtime content tags** — `ResourceTypeContract` records the
   original `dmc3.exe` three-byte content probe at `0x1402DB1F0` and the second
   dispatcher at `0x1401B9FA0`. Nameless payload prefixes `MOD`, `EFM`, `SCM`,
   `MRP`, and `SHW` therefore yield `mod`, `efm`, `scm`, `mrp`, and `shw`
   semantics with `profile_runtime_content_tag` provenance when generic magic
   evidence has not already supplied a stronger sealed interpretation.

The three-byte boundary is deliberate. `ResourceTypeContract::type_for_prefix`
compares exactly bytes 0..2, with no fourth-byte test and no case folding.
Requiring a fourth byte would be stricter than the recovered game code and would
convert instruction-backed evidence into a product invention.

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
    -> semantic evidence = profile_structural_format
    -> canonical_display_name = em000_000.ptx
```

`ResourceTypeContract` compile-time assertions independently pin the exact
three-byte `MOD` and `SCM` prefix behavior, including the fact that an arbitrary
fourth byte does not change a `MOD` result and lowercase `mod` is not accepted.
The DMC3 resolver maps that runtime result into the separate
`profile_runtime_content_tag` evidence path.

The tests additionally verify that external-index evidence remains absent where
no exact `.index` exists and that physical `ResourceId` and payload bytes do not
change. Whole-head CI must remain green on both Ubuntu and Windows.
