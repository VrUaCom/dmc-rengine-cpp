# DMC3 nested PAC/PNST reintegration

Status: bounded Layer-1 product authoring seam. This does not reconstruct a Capcom offline packer and does not convert nested materialized offsets into archive storage offsets.

## Purpose

The read path already expands PAC/PNST children into distinct `ResourcePayload` identities. The write path now needs the inverse operation without losing byte-domain or alias semantics:

```text
child immutable payload
 -> child WorkingCopy
 -> child bounded writer/validator
 -> AuthoredChildImage
 -> exact parent materialized span
 -> alias arbitration
 -> parent WorkingCopy
 -> RelativeSlotLayoutWriter
 -> validated parent authored image
```

For multi-level trees, repeat bottom-up until the root resource is ready for the canonical STORE-overlay NBZ writer.

## Why raw WorkingCopy bytes are insufficient

A parent relative-slot parser validates its own header/table/topology but treats nested child bytes as an opaque span. A same-size corrupted nested PNST could therefore leave its outer PAC structurally valid.

The reintegrator consequently accepts only an `AuthoredChildImage` with explicit source/output SHA and writer-mode identity. The caller must first run the child's own bounded writer/validator. Current PAC/PNST children use `RelativeSlotLayoutWriter` from #140.

The reintegrator independently recomputes both hashes before applying a replacement; copied receipt strings are not trusted by themselves.

## Exact expansion binding

`NestedRelativeSlotReintegrator` requires:

- readable immutable parent bytes whose size agrees with parent `ResourceId`;
- a usable `ContainerExpansion` whose parent `ResourceRef` exactly equals the supplied parent;
- PAC or PNST parser-format identity;
- every populated expansion child still equals the exact bounded parent span at `ContainerEntry.offset/size`;
- expanded child source ID/identity coordinates remain bound to the supplied parent;
- every authored child matches exactly one populated expansion child.

A stale expansion fails closed before any edit is applied.

## Materialized coordinate rule

All parent writes use `ContainerEntry.offset` inside the already-materialized parent image.

For an NBZ DEFLATE parent, child provenance may be `materialized_parent_span` even though the parent source provenance names compressed archive storage. Reintegration never performs:

`compressed NBZ data offset + child offset`.

That operation is invalid because compressed and materialized byte domains are distinct.

## Same-span requirement

This seam is layout-preserving. An authored child output must contain exactly the same byte count as the expanded bounded child span.

The generic authored-image envelope does not imply this globally; a future runtime-synth writer may produce a different-sized image. This particular reintegrator rejects that image and requires the separate size-changing parent tier.

## Duplicate-offset aliases

All populated children are grouped by exact materialized parent `(offset,size)`.

- one changed authored alias -> patch shared span once;
- multiple changed aliases with byte-identical authored images -> patch once;
- divergent changed aliases for one span -> fail closed `alias_conflict`;
- unedited alias identities remain distinct and are recorded as affected because their physical bytes are shared.

The parent topology is then validated by #140, so duplicate slot identities remain duplicate slot identities over one span.

## Guarded parent edit

Each changed physical span becomes one guarded `WorkingCopy` edit on the immutable parent:

- base revision = current parent WorkingCopy revision;
- offset = materialized `ContainerEntry.offset`;
- expected bytes = exact immutable parent span;
- replacement = arbitrated authored child bytes.

After all changed spans are applied, `RelativeSlotLayoutWriter` reparses source/output and proves unchanged parent topology. If that writer rejects the result, reintegration fails.

## Receipt

The reintegration receipt records:

- exact parent ResourceId;
- parent source/output SHA;
- final parent WorkingCopy revision;
- parent #140 rebuild receipt;
- every changed physical span offset/size/source SHA/output SHA;
- all alias child identities affected by that span;
- the subset of alias identities that supplied changed authored images.

No new source `ByteProvenance` is created. This remains authoring lineage until the final root bytes are persisted and reopened through a source.

## Bottom-up recursion

Nested authoring is deliberately compositional:

```text
deep child writer
 -> immediate-parent reintegration / #140
 -> resulting parent authored image
 -> next-parent reintegration / #140
 -> ...
 -> root authored image
 -> #141 STORE-overlay writer
 -> NbzZipSource reopen
```

Every structural layer validates itself before bytes propagate upward.

## Validation boundary

Synthetic regression covers:

- transformed parent provenance with materialized child coordinates;
- two distinct PAC slot identities sharing one PNST span;
- one-alias edit patches the shared span once;
- identical dual-alias edit succeeds;
- divergent dual-alias edit fails closed;
- stale source/output SHA rejection;
- clean/no-change authored image;
- duplicate/unknown child input;
- stale parent/expansion rejection;
- parent canonical reparse with alias topology preserved.

Still open after this seam: real nested legal-corpus authoring receipt, runtime-synth size-changing reintegration, capability-aware workspace editing gate and controlled game-backed validation.
