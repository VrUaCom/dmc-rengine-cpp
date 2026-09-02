# GDSpaces L1 derived display fallback — 2026-08-30

Status: **IMPLEMENTED IN MAIN — NOT HISTORICAL NAME AUTHORITY**

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
       -> structural-parser evidence
       -> registry-content-probe evidence
       -> family-mask-probe evidence
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

## Evidence-site separation

The original runtime does not expose one universal DMC3 type detector. L1 keeps
three different evidence reasons separate:

- `profile_structural_format` means a structural parser proved the interpretation;
- `profile_runtime_content_tag` means the interpretation comes from the exact
  three-byte registry content probe at `0x1402DB1F0`;
- `profile_runtime_family_mask_tag` means the interpretation comes from the
  independent four-byte family-mask probe at `0x1402FD650`.

This distinction is retained both with and without an external `.index`, so an
index overlay cannot launder one evidence family into another.

The private DMC3 semantic pass currently uses:

1. **Texture structure** — `TextureSlotFramingParser` proves a texture bundle or
   wrapped DDS and yields canonical `ptx` / `dds` presentation semantics with
   `profile_structural_format` provenance.
2. **Registry content tags** — `ResourceTypeContract::registry_type_for_prefix`
   reproduces the original three-byte probe and recognizes `MOD`, `EFM`, `SCM`,
   `MRP`, and `SHW`. The fourth byte is deliberately ignored **at this site only**.
3. **Runtime family masks** — `ResourceTypeContract::family_mask_for_prefix`
   reproduces the separate four-byte classifier and requires exact tags with a
   trailing ASCII space: `MOD `, `EFM `, `SCM `, `MRP `, `MCV `, `SHW `.
   `MCV` therefore receives byte-backed semantic presentation only through the
   family-mask provenance family, not by widening the registry rule.

A third runtime path, `container_dispatch @ 0x1401B9FA0`, is recorded in the
contract but is not blindly converted into naming semantics. It independently
reaches the normal handlers for `MOD/EFM/SCM/SHW` and also recognizes `EFW/EFE`
as sentinel prefixes. `EFW/EFE` currently receive **no invented extension,
handler or decoded schema** from that fact alone.

Correct evidence topology:

```text
registry_content_probe @ 0x1402DB1F0
    3-byte recognition
    MOD / EFM / SCM / MRP / SHW

container_dispatch @ 0x1401B9FA0
    MOD / EFM / SCM / SHW -> recovered normal handlers
    EFW / EFE -> recognized sentinel only

family_mask_probe @ 0x1402FD650
    exact 4-byte recognition with trailing 0x20
    MOD / EFM / SCM / MRP / MCV / SHW
```

The obsolete global statements “the runtime recognizes exactly five tags” and
“the fourth byte does not matter” are therefore superseded.

### Compatible generic magic refinement

The generic classifier historically recognizes `SCM` as `magic_confirmed`.
That result and the DMC3 registry probe agree on the semantic type but do not have
the same provenance. The profile pass therefore applies this rule:

```text
generic magic says scm
AND registry content probe says scm
    -> replace the lower-precision magic record
       with profile_runtime_content_tag

conflicting generic magic
    -> block profile refinement
```

The result is one active semantic reason for the physical byte image, not two
parallel records that happen to spell the same format differently.

### Downstream classification provenance

Sealed evidence remains distinct after naming reconciliation. Materialized
`ResourceClassifier::classify(payload)` preserves the actual reason:

```text
embedded_name_list / profile_structural_format
    -> structural_confirmed

magic_confirmed_format
    -> magic_confirmed

profile_runtime_content_tag
    -> runtime_content_tag_confirmed

profile_runtime_family_mask_tag
    -> runtime_family_mask_confirmed
```

Thus instruction-backed runtime evidence cannot be laundered into generic magic
or structural-parser proof merely by crossing the naming/classifier API boundary.

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

GData.afs/obj/em000.pac + ordinal 1 + registry tag MOD
    -> em000_001.mod

synthetic PAC child + ordinal 2 + family tag MCV<space>
    -> <container-stem>_002.mcv
```

These examples describe the deterministic derived-display rule. They are not
claims that those names were retained historical extractor filenames unless
separate `.index` or direct stored-name evidence proves them.

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

`dmc3_runtime_content_tag_provenance_tests` pins the registry-probe path:

```text
SCM bytes
    -> full Dmc3NamingPipeline
    -> exactly one semantic evidence record
    -> profile_runtime_content_tag
    -> ResourceClassifier.runtime_content_tag_confirmed = true
    -> magic_confirmed = false
    -> structural_confirmed = false
```

`ResourceTypeContract` compile-time assertions independently pin the evidence
sites:

- registry `MOD`: arbitrary fourth byte is ignored only by the registry probe;
- family mask: `MOD ` succeeds while `MODX` fails;
- `MCV ` is family-mask recognized but registry-probe unknown;
- `EFW`/`EFE` are container-dispatch recognized sentinels with no handler;
- `SCM` is a normal container-dispatch handler path.

The tests additionally verify that external-index evidence remains absent where
no exact `.index` exists and that physical `ResourceId` and payload bytes do not
change. Whole-head validation remains required on Ubuntu and Windows after each
semantic-evidence change.
