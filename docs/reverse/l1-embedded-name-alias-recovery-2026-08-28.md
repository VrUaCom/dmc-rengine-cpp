# L1 embedded-name alias recovery — 2026-08-28

## Status

Evidence-backed recovery of the retained GDSpaces v6 embedded-name convention. This pass is naming/provenance work only; it does not promote Layer 1 to complete.

## Authoritative retained baseline

The exact historical project archive was reacquired from the retained project Library:

- artifact: `DMC 3 RENGINE (6).zip`
- size: `237658858` bytes
- SHA-256: `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`

The digest matches the canonical v6 GDSpaces audit, so the recovered TypeScript and stage-drop files below are the same audited baseline rather than a reconstructed approximation.

## Exact retained parser contract

Source recovered from:

`lib/gdspaces/display/GDContainerNameHints.ts`

The retained implementation establishes the following rules:

1. only populated physical slot `0` is inspected as the possible embedded-name list;
2. accepted payload size is `1..4096` bytes;
3. bytes are decoded as UTF-8 with replacement for invalid input;
4. NUL characters are replaced by newline before the printable-ratio gate;
5. printable ratio must be at least `0.75`;
6. text is split on CR/LF/TAB/SPACE and each token is trimmed;
7. only tokens matching the retained filename-extension vocabulary are accepted;
8. accepted names map sequentially: name 0 -> physical slot 1, name 1 -> physical slot 2, and so on;
9. slot 0 is classified as an embedded name-list entry only when at least one accepted name is recovered.

The retained extension vocabulary is:

`ptx, clt, c1d, scm, sch, txt, pac, mot, motN, cam, hid, tsc, hits, dds, tm2, mod, lig, eve, pos, sef, dca, est`.

This parser is deliberately not generalized to extensions that were absent from the retained contract.

## Real `st001` corpus receipt

Recovered file:

`analysis_inputs/stage_drops/st001 - copia/st001 - copia/st001_000.ukn`

Receipt:

- size: `48` bytes
- SHA-256: `7efcf182f28135a3d694324ba06715ca6f6b075a028d035c89c69809df23faa5`
- accepted aliases and byte offsets:
  - offset `0`: `st001.ptx` -> physical slot 1
  - offset `11`: `st001.scm` -> physical slot 2
  - offset `22`: `st001.sch` -> physical slot 3
- the textual sequence is followed by 15 NUL bytes.

The `st001.sch` alias is retained as raw embedded evidence even when the child bytes classify semantically as HITS; semantic bytes control the canonical display suffix, while the historical alias remains queryable metadata.

## Exact DDS companion index receipt

Recovered file:

`analysis_inputs/stage_drops/st001 - copia/st001 - copia/st001_001/st001_001.index`

- size: `323` bytes
- SHA-256: `e2475eb401ecf9017ba48cd012369ae21b6a92a2f3ef58e2544dfcb0c8bea609`
- CRLF-separated labels:
  - `st001_001_000.dds`
  - `st001_001_001.dds`
  - `st001_001_002.dds`
  - `st001_001_003.dds`
  - `st001_001_004.dds`
  - `st001_001_005.dds`
  - `st001_001_006.dds`
  - `st001_001_007.dds`
  - `st001_001_008.dds`
  - `st001_001_009.dds`
  - `st001_001_010.dds`
  - `st001_001_011.dds`
  - `st001_001_012.dds`
  - `st001_001_013.dds`
  - `st001_001_014.dds`
  - `st001_001_015.dds`
  - `st001_001_016.dds`

This removes the previous blocker where only the count of 17 DDS names was retained in the audit.

## C++ authority model

Pass 5 ports the recovered behavior into a sealed observation model:

`physical slot 0 bytes -> EmbeddedNameListObservation -> embedded_alias ResourceNameEvidence -> physical slots 1..N`

The observation is bound to:

- parent physical `ResourceId`;
- slot-0 authority `ResourceId`;
- exact slot-0 SHA-256;
- raw alias;
- alias source byte offset;
- target physical slot index.

Changing slot-0 bytes after observation invalidates replay. Embedded aliases never become ResourceId, lookup or writer authority.

## Authority priority

The canonical identity/display hierarchy remains:

`physical identity` != `external .index name` != `embedded alias` != `semantic format`.

For display stem:

`external .index > embedded alias > physical/synthetic`.

For canonical suffix:

`validated bytes/structure > source/index/embedded extension`.

Applying embedded aliases before or after an external `.index` must therefore yield identical physical identities and identical final display semantics.

## `.post` corpus status

A full filename scan of the exact 29,969-entry v6 ZIP found zero files with extension `.post`. This is strong negative evidence for this retained corpus, but it is not promoted to a universal claim that no DMC3 build can contain such a format.

Current classification: `.post` is not observed in the canonical v6 project/stage corpus; do not alias it to PNST without separate evidence.

## Remaining integration boundary

Before any Layer 1 completion claim:

1. Pass 5 must pass whole-head Windows + Ubuntu CI;
2. stacked name/identity passes must be reconciled with the unpublished local NBZ member-identity commit `1a76cb4` without guessing its API;
3. a final nested retail round-trip must preserve NBZ member identity, PAC/PNST physical slots, external `.index` evidence, embedded aliases, DDS child identity, bytes and rebuild/reopen targeting.
