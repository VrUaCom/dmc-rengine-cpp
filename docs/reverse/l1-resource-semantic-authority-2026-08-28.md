# L1 resource semantic authority — 2026-08-28

## Scope

This pass closes the authority gap between a canonical display suffix and the semantic type of an already-materialized PAC/PNST child. It does not change physical ResourceId, slot topology, byte ownership, rebuild selection, or runtime resolver policy.

## Recovered evidence

The retained v6 `GDContainerNameHints.ts` behavior recognizes a small, printable slot-0 payload as an embedded name list only after its retained filename-token gate succeeds. The old `.txt` suffix was assigned later by display/presentation code; it was not recovered from the payload bytes.

The canonical retained corpus contains these relevant `_000.ukn` cases:

- `st001_000.ukn`: 48 bytes, SHA-256 `7efcf182f28135a3d694324ba06715ca6f6b075a028d035c89c69809df23faa5`, accepted aliases `st001.ptx`, `st001.scm`, `st001.sch`;
- `st445_000.ukn`: 32 bytes, SHA-256 `c93360ac57fd602b5d544dffa5a506f1c6769ed8a194c600e95c1e4873a1e687`, accepted aliases `st445.ptx`, `st445.scm`, `st445.sch`;
- `m20_s00_012_000.ukn`: 96-byte binary payload, SHA-256 `77f715e3b1e01386cea9ba03f889d1745981a4ff45ba50c3b05d2553ab3ab114`, rejected by the retained embedded-name parser.

Therefore neither `_000` nor `.ukn` is semantic authority. Only the exact structural observation can promote a slot to embedded `name-list` semantic evidence.

## Authority model

```text
physical ResourceId + slot + byte image
  -> materialization / write authority

external .index manifest entry
  -> external naming evidence

embedded aliases observed in slot 0
  -> embedded naming evidence for mapped children

sealed slot-0 observation + exact SHA-256
  -> semantic evidence: name-list

canonical .index display suffix
  -> presentation/export naming candidate only
```

A display string never becomes write authority.

## Semantic-laundering defect

The raw classifier is intentionally magic-first and then extension-based. This is correct for an unknown loose probe, but after canonical display reconciliation a proven embedded name table can have display name `st001_000.index`. Re-feeding only that display name into the raw classifier would classify a non-magical payload as `index`.

The materialized-resource classifier therefore has a stronger contract:

1. valid sealed semantic evidence must match the exact physical `ResourceId`;
2. its authority SHA-256 must match the current complete byte image;
3. valid evidence wins over presentation suffix;
4. if semantic evidence exists but becomes stale, presentation hints are ignored entirely and classification falls back to physical logical identity plus fresh bytes.

Thus `st001_000.index` cannot manufacture or preserve semantic authority after its bytes change.

## Transaction boundary

`ContainerNamingReconciler` operates on a staged copy and commits only after all supplied naming authorities validate. Before commit it proves unchanged:

- parent ResourceId;
- parser format;
- child count;
- every physical slot index;
- every child ResourceId;
- offset, size and populated state;
- every child byte image.

Failure leaves the caller's expansion unchanged.

Synthetic display is leaf-only. A path-like upstream presentation such as `DMC3/st001.pac` is normalized for synthetic display to `st001_000.index`; namespace/path components are not allowed to leak into a loose filename candidate.

## Writer boundary

The existing DMC3 relative-slot rebuild CLI selects a target by physical slot index/path and constructs `AuthoredChildImage.resource` from the existing child `ResourceId`. The packed reflow writer validates physical provenance/topology. Display names and `.index` labels do not select the bytes that are reintegrated.

This proves naming reconciliation is outside rebuild authority. It does not yet prove the final loose-export filename/publication contract; that remains the next bounded L1 pass.

## Non-claims

- The historical extracted physical filename is still recorded as `st001_000.ukn`; this pass does not claim the retail archive literally stores `st001_000.index`.
- Embedded name-list semantic evidence is not the same object as the companion external `st001.index` manifest.
- No blanket `_000.ukn -> .index` rule exists.
- Loose export/publication and companion-index discovery are not promoted to complete by this pass.
