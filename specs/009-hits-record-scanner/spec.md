# Specification 009 — HITS$ Confirmed Record Scanner

## Status

Scanner implemented; cross-platform validation in progress. Binary Inspector adapter source is drafted and must be integrated/validated separately.

## Problem

Historical Stage Ops work recognized HITS$ collision/hit resources, but the clean repository must not import an opaque viewer or invent a complete header schema. Only the confirmed record layer is currently safe to migrate.

## Confirmed input facts

- resource magic: `HITS$`;
- record marker: `0x18060001`;
- record size: 56 bytes;
- record structure: one little-endian `u32` marker and thirteen little-endian `float32` values.

## Goals

- recognize HITS$ magic;
- scan for complete confirmed record markers after the magic;
- permit unknown padding/header bytes before records;
- decode thirteen raw floats without semantic naming;
- preserve byte offsets;
- report wrong magic, no record, and truncation diagnostics;
- use the shared bounds-checked Binary Reader;
- use synthetic public fixtures only;
- map confirmed records into the shared Binary Inspector domain.

## Non-goals

- claiming a complete file-level HITS schema;
- assuming a record-count field;
- assigning semantic names to the thirteen floats;
- writing or repacking HITS resources;
- adding a HITS-specific source resolver;
- committing original game samples.

## Scanner algorithm

1. Require `HITS$` at offset zero.
2. Start scanning after the five-byte magic.
3. Search byte-by-byte for little-endian marker `0x18060001`.
4. Require all 56 record bytes.
5. Decode marker plus thirteen floats.
6. Preserve the record offset.
7. Advance by 56 bytes after a confirmed record.

## Diagnostics

- `hits.unrecognized` — HITS$ magic absent;
- `hits.no_records` — magic recognized but no complete confirmed record found;
- `hits.truncated_record` — marker found without the full record range;
- `hits.invalid_record` — a complete range could not be decoded.

## Binary Inspector mapping

The adapter should create:

- magic region and string field;
- one record region per confirmed record;
- one parent structure field;
- marker field;
- thirteen generically named raw float fields;
- `formats.hits` ownership claims;
- unknown gaps for bytes not covered by confirmed structures.

No field receives a semantic gameplay name until a new Evidence Packet supports it.

## Acceptance criteria

- synthetic two-record fixture parses deterministically;
- arbitrary unknown padding before records is tolerated;
- wrong magic is unrecognized;
- magic-only input produces a warning, not fabricated records;
- a truncated marker produces an error;
- raw float values and offsets match the fixture;
- Binary Inspector mapping produces stable region/field IDs;
- Windows and Ubuntu CI pass;
- no proprietary bytes are present.

## Evidence migration requirement

Before semantic field naming or write support:

- identify local legal sample hashes;
- create a public sanitized Evidence Packet;
- correlate raw fields with runtime/visual behavior;
- preserve corrected/rejected interpretations.
