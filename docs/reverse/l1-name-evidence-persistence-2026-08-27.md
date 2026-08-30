# L1 persistent resource-name evidence — 2026-08-27

## Purpose

The `.index` passes established correct physical slot identity and display reconciliation, but an applied display overlay must not discard the evidence that produced the visible name.

This pass keeps name provenance on the materialized `ResourcePayload` independently from both `ResourceId` and `ByteProvenance`.

## Model

`ResourceNameEvidence` is read-only naming evidence. It is deliberately **not**:

- part of stable resource identity;
- byte provenance;
- lookup/runtime resolver authority;
- writer/rebuild authority.

For an external `.index` label it retains:

- evidence kind `external_index`;
- mapping mode (`physical_position` or `populated_slot_sequence`);
- exact companion `.index` `ResourceId`;
- SHA-256 of the exact observed `.index` bytes;
- exact raw manifest label;
- normalized name after structural marker removal;
- physical slot index;
- source manifest line.

Applying a newer `.index` overlay replaces the active external-index evidence for that child rather than accumulating stale active labels.

## Authority hardening

`ResourceNameEvidence` is non-forgeable by type:

- private constructor;
- external-index records may be produced only by the sealed `IndexNameOverlayBuilder` path;
- a future embedded-name path must use a dedicated evidence builder rather than public aggregate construction.

This prevents a caller from inventing a label/hash pair and laundering it into evidence-looking resource metadata.

## Embedded aliases

The retained v6 architecture proves that an embedded-name layer existed through `GDContainerNameHints.ts` with `parseEmbeddedNameList()`, `embeddedNameHintsForEntries()`, and related validation helpers.

However, the exact old parser rules are not present in the retained C++ checkout and have not yet been recovered from a source artifact. Therefore this pass adds the generic evidence kind/mapping vocabulary needed for future embedded aliases but **does not implement an embedded-name parser or fabricate aliases heuristically**.

The required priority remains:

`external .index -> embedded alias -> physical name -> synthetic name`.

## Acceptance boundary

Regression proves that:

1. applying a sealed `.index` preserves exact raw and normalized label provenance on the child payload;
2. manifest ResourceId, exact SHA-256, line number, mapping mode and physical slot remain queryable downstream;
3. display reconciliation still leaves child `ResourceId` and bytes unchanged;
4. applying a different `.index` observation replaces stale external-index evidence rather than accumulating it;
5. evidence snapshots cannot be publicly aggregate-constructed.

No Layer 1 completion claim is promoted by this pass. Embedded alias parsing, exact historical `st001_001.index` DDS strings and `.post` remain separate evidence boundaries.
