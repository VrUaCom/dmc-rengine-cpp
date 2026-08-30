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

DMC3 profile byte/structural semantics are reconciled independently of external
`.index` availability:

```text
materialized payload bytes
    -> generic magic semantic evidence
    -> DMC3 profile semantic observation
       -> structural-parser evidence OR runtime-content-tag evidence
    -> ResourceNamingIdentity
```

The profile semantic pass is sealed by `ContainerNamingReconciler`. Profile
callbacks are not themselves evidence and are no longer a public sealing input:

- public `ContainerNamingReconciler::reconcile` accepts only the materialized
  expansion plus an optional exact external `.index`;
- the callback-bearing `reconcile_profiled` path is private;
- `apply_profile_semantics` is private;
- only the friended `Dmc3NamingPipeline` may invoke those profile transitions.

Therefore a UI suffix, filename, arbitrary resolver callback or product caller
cannot ask the trusted reconciler to seal an invented profile semantic string.
The registered regression compile-time checks that neither a three-argument
public `reconcile(..., resolver)` nor `reconcile_profiled` is accessible outside
that authority boundary.

Crucially, the profile provenance families are not collapsed into one enum:

- `profile_structural_format` means a structural parser proved the interpretation;
- `profile_runtime_content_tag` means the interpretation comes from the exact
  instruction-backed runtime content probe.

This distinction is retained both with and without an external `.index`, so an
index overlay cannot launder a runtime tag into a structural-parser claim.

Two DMC3 evidence families currently feed the private profile pass:

1. **Texture structure** — `TextureSlotFramingParser` proves a texture bundle or
   wrapped DDS and yields canonical `ptx` / `dds` presentation semantics with
   `profile_structural_format` provenance.
2. **Recovered runtime content tags** — `ResourceTypeContract` records the
   original `dmc3.exe` three-byte content probe at `0x1402DB1F0` and the second
   dispatcher at `0x1401B9FA0`. Nameless payload prefixes `MOD`, `EFM`, `SCM`,
   `MRP`, and `SHW` therefore yield `mod`, `efm`, `scm`, `mrp`, and `shw`
   semantics with `profile_runtime_content_tag` provenance.

The three-byte boundary is deliberate. `ResourceTypeContract::type_for_prefix`
compares exactly bytes 0..2, with no fourth-byte test and no case folding.
Requiring a fourth byte would be stricter than the recovered game code and would
convert instruction-backed evidence into a product invention.

### Compatible generic magic refinement

The generic classifier historically recognizes `SCM` as `magic_confirmed`.
That result and the DMC3 runtime probe agree on the semantic type but do not have
the same provenance. The profile pass therefore applies this rule:

```text
generic magic says scm
AND recovered DMC3 runtime probe says scm
    -> replace the lower-precision magic record
       with profile_runtime_content_tag

conflicting generic magic
    -> block profile refinement
```

The result is one active semantic reason for the physical byte image, not two
parallel records that happen to spell the same format differently. A dedicated
full-pipeline `SCM ` regression requires exactly one final semantic record and
requires its kind to be `profile_runtime_content_tag`.

### Downstream classification provenance

Sealed evidence remains distinct after naming reconciliation. Materialized
`ResourceClassifier::classify(payload)` no longer turns every sealed semantic
record into `structural_confirmed = true`. Its confirmation flags preserve the
actual reason:

```text
embedded_name_list / profile_structural_format
    -> structural_confirmed

magic_confirmed_format
    -> magic_confirmed

profile_runtime_content_tag
    -> runtime_content_tag_confirmed
```

Thus an instruction-backed `MOD` or `SCM` claim cannot be laundered into a
structural-parser claim merely by crossing the naming/classifier API boundary.

## Derived display name

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

The second example describes the deterministic derived-display rule; it is not a
claim that `em000_001.mod` was a retained historical extractor filename unless
separate `.index` evidence proves that name.

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

`dmc3_runtime_content_tag_provenance_tests` pins the recovered-runtime path:

```text
SCM bytes
    -> full Dmc3NamingPipeline
    -> exactly one semantic evidence record
    -> profile_runtime_content_tag
    -> ResourceClassifier.runtime_content_tag_confirmed = true
    -> magic_confirmed = false
    -> structural_confirmed = false
```

The same test also pins the non-forgeable reconciler surface at compile time.
Tests that require profile-aware `.index` semantics now enter through
`Dmc3NamingPipeline`; they do not retain a privileged test-only route through a
public resolver callback.

`ResourceTypeContract` compile-time assertions independently pin the exact
three-byte `MOD` and `SCM` prefix behavior, including the fact that an arbitrary
fourth byte does not change a `MOD` result and lowercase `mod` is not accepted.

The tests additionally verify that external-index evidence remains absent where
no exact `.index` exists and that physical `ResourceId` and payload bytes do not
change. Whole-head CI must remain green on both Ubuntu and Windows for the exact
validation head before promotion.
