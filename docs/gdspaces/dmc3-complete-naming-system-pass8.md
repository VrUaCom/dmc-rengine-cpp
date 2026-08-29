# DMC3 complete naming system — L1 Pass 8

Status: **IMPLEMENTED / validation pending**  
Scope: Layer 1 resource materialization naming, extraction provenance, and presentation identity.  
Stack base: Pass 7 extracted-ordinal recovery (`#261`).

## Purpose

Pass 8 replaces the remaining collection of naming-specific rules with one evidence-separated naming model. The implementation must be able to describe one physical payload without conflating:

1. physical container identity;
2. physical slot index;
3. extracted ordinal;
4. exact external `.index` label;
5. embedded semantic alias;
6. byte/structure-derived semantic format;
7. canonical GDSpaces display name;
8. historically observed legacy extraction representation.

No presentation field is allowed to become write authority.

## Canonical identity chain

```text
physical ResourceId
  -> physical_slot_index
  -> extracted_ordinal
  -> external .index evidence
  -> embedded alias evidence
  -> semantic-format evidence
  -> canonical display identity
  -> optional legacy extraction replay
```

The coordinates remain intentionally separate.

### Physical identity

`ResourceId` remains parent/container/physical-slot based. Renaming, format classification, embedded aliases, `.index` labels, or folder materialization do not modify the physical target.

### Extracted ordinal

The extracted ordinal is the zero-based position in the populated payload sequence. It is derivable from physical container topology even when an external `.index` is absent.

```text
slot 0 = payload  -> ordinal 0
slot 1 = empty    -> no ordinal
slot 2 = payload  -> ordinal 1
```

When an external `.index` exists, its sealed ordinal evidence must agree with the topology-derived ordinal. Mismatch is fail-closed.

### External `.index`

External `.index` evidence retains both the exact raw label and the normalized name. For example:

```text
raw label:       st001_001 folder
normalized name: st001_001
folder marker:   true
ordinal:         1
physical slot:   1
```

`folder` is extraction representation metadata and is not appended as a filename suffix.

### Embedded aliases

Embedded aliases such as `st001.ptx`, `st001.scm`, and `st001.sch` remain a separate evidence domain. They can guide semantic presentation but are not promoted to original physical filenames without stronger evidence.

### Semantic format

Byte/structure evidence may establish a semantic format independently from both external and embedded names. Semantic evidence is bound to:

- exact physical `ResourceId`;
- exact physical slot;
- SHA-256 of the current payload bytes.

Stale or conflicting semantic evidence is rejected.

## Unified `ResourceNamingIdentity`

Each child naming snapshot now retains:

```text
resource_id
physical_slot_index
populated
extracted_ordinal
external_index_raw_label
external_index_name
external_index_folder
embedded_alias
semantic_format
canonical_extension
canonical_display_name
```

A `ContainerNamingIdentitySnapshot` additionally retains the parent-level external `.index` context:

```text
manifest_resource
manifest_sha256
directive
entry_count
```

The only currently accepted directive values are empty and `PNST`. The directive is representation metadata; it does not control slot mapping.

## DMC3 legacy extraction representation

Pass 8 introduces a DMC3-specific legacy extraction naming planner rather than placing historical extraction conventions in generic GDSpaces.

The model distinguishes a semantic display name from the historical expanded representation.

Example:

```text
physical payload:     PAC slot 1
extracted ordinal:    1
external label:       st001_001 folder
embedded alias:       st001.ptx
semantic type:        texture-bundle / PTX
canonical display:    st001_001.ptx
legacy representation:
  st001_001/
    st001_001.index
    ...
```

The directory is therefore `st001_001/`, not `st001_001.ptx/`.

## Legacy `.index` replay

`LegacyIndexReplayPlanner` reconstructs a logical legacy `.index` only when the reconciled snapshot retains exact external manifest evidence.

Replay preserves:

- manifest `ResourceId`;
- manifest SHA-256;
- optional `PNST` directive;
- exact raw labels;
- exact extracted-ordinal order.

The rendered text uses canonical CRLF. This is a logical replay representation and is **not** a claim of byte-for-byte preservation of the producer's original newline encoding.

If no external `.index` was observed, the planner fails closed rather than fabricating a historical manifest.

## Strict companion `.index` discovery

Pass 8 adds DMC3-specific companion discovery derived only from the source-native physical/logical container path.

For:

```text
GData.afs/scr/st001.pac
```

only these candidates are formed:

```text
GData.afs/scr/st001.index
GData.afs/scr/st001/st001.index
```

The following are explicitly excluded as authority sources:

- display-name-derived candidates;
- embedded-alias-derived candidates;
- semantic-suffix-derived candidates;
- arbitrary "first `.index` in directory" selection;
- descendant `::PAC/slot-*` identities.

If both exact physical-path candidates exist, discovery returns an ambiguity error and selects neither.

## Fail-closed invariants

The reconciled naming snapshot is invalid when any of these occur:

- duplicate physical slot indices;
- an empty slot consumes an extracted ordinal;
- populated ordinals are non-contiguous;
- external `.index` ordinal disagrees with topology;
- external manifest entry count differs from populated payload count;
- stale semantic evidence no longer matches exact bytes;
- multiple conflicting semantic authorities exist;
- multiple external `.index` authorities are attached to one physical child;
- external evidence uses the superseded positional sparse mapping model;
- companion discovery finds multiple exact candidates.

## Representative sparse evidence

The Pass 7 corpus result remains the authority for extracted-order semantics. Representative retained PAC:

```text
em035_037.pac
physical slot count: 95
populated slots: 5, 6, 7, 21, 50, 80, 89, 94
extracted ordinals: 0..7
```

Mapping:

```text
ordinal 0 -> physical slot 5
ordinal 1 -> physical slot 6
ordinal 2 -> physical slot 7
ordinal 3 -> physical slot 21
ordinal 4 -> physical slot 50
ordinal 5 -> physical slot 80
ordinal 6 -> physical slot 89
ordinal 7 -> physical slot 94
```

Pass 8 does not reinterpret this evidence; it makes the two-coordinate identity explicit across the whole naming system.

## Regression coverage

Pass 8 adds three dedicated regression surfaces:

### `resource_naming_identity_tests`

Covers:

- topology-derived extracted ordinals;
- external `.index` plus embedded aliases;
- DMC3 structural texture-bundle semantic evidence;
- canonical PTX/HITS display identities;
- legacy folder representation;
- sparse `em035_037`-shape mapping;
- duplicate physical-slot rejection;
- physical identity invariance.

### `companion_index_locator_tests`

Covers:

- sibling candidate derivation;
- expanded-directory candidate derivation;
- display name not participating in lookup;
- exact candidate resolution;
- no arbitrary near-match fallback;
- dual exact candidate ambiguity fail-closed;
- rejection of descendant `::` identities;
- source mismatch rejection.

### `legacy_index_replay_tests`

Covers:

- exact raw-label replay;
- `PNST` directive retention;
- sparse physical topology with dense extracted order;
- refusal to fabricate a manifest when external evidence is absent;
- invalid empty-slot naming rejection;
- ordinal-gap rejection;
- invalid directive rejection.

All three are registered in the normal CTest matrix.

## Proven / not proven boundary

### Proven or directly implemented

- physical slot and extracted ordinal are separate coordinates;
- extracted ordinal follows populated physical topology;
- external `.index` labels bind through extracted ordinal;
- empty slots consume no ordinal;
- parent-level manifest context survives reconciliation;
- embedded alias, external label, semantic format, canonical display, and physical target are separate domains;
- exact external labels can be logically replayed when sealed evidence exists;
- historical folder representation is kept separate from semantic `.ptx` presentation;
- companion lookup can be performed without presentation-derived fallbacks.

### Still not proven

- exact HDC-adapted extractor source revision that produced the retained corpus;
- original producer byte-for-byte newline/whitespace serialization for every `.index`;
- that embedded aliases are original standalone filenames on retail media;
- that `.index` participates in DMC3 game runtime resolution;
- every possible semantic format hidden behind `.ukn` across the complete retail corpus.

Those boundaries must remain explicit after promotion.

## Promotion requirement

Pass 8 is not promotion-ready until the stacked whole-head CI is green on both Windows and Ubuntu. A green result proves the implementation and regression suite are coherent with the current stack; it does not by itself promote the remaining provenance candidates above their documented evidence level.
