# Evidence Packets

Evidence Packets are versioned, deterministic, public metadata documents that connect reverse-engineering claims to exact artifacts without embedding the artifacts.

## Current schema model

`EvidencePacket` contains:

- schema version;
- packet ID;
- title;
- project name;
- artifact identities;
- evidence records.

`ArtifactIdentity` contains:

- stable public artifact ID;
- role;
- SHA-256;
- byte size.

`EvidenceRecord` contains:

- record ID;
- claim ID;
- title;
- summary;
- confidence;
- artifact locations;
- tags;
- superseded record IDs.

Locations may reference file offset, size, RVA, VA, symbol, and a sanitized note.

## Validation rules

- packet IDs and titles are required;
- schema version must be non-zero;
- artifact and record IDs are unique;
- artifact SHA-256 is 64 hexadecimal characters;
- every evidence location references an artifact declared in the packet;
- evidence records satisfy required-field validation;
- invalid packets do not serialize.

## Deterministic JSON

The exporter writes fields and arrays in a stable order so pull requests can review evidence changes as ordinary Git diffs.

The current milestone exports JSON but does not yet import it. Import requires strict diagnostics, schema migration, size limits, and untrusted-input tests.

## Public/private separation

A packet identifies local game files by hash and role. It does not contain the executable, archive, resource, or large binary excerpt.

## Correction model

A correction creates a new record or updates an explicitly versioned packet while preserving `supersedes` links. Old conclusions are not silently erased from project history.

## Example workflow

1. User supplies a legally obtained executable locally.
2. GDSpaces reads it and SHA-256 identifies it.
3. PE/EXE tooling records locations and observations.
4. A reviewer assigns or adjusts confidence.
5. A sanitized packet is exported.
6. Tests or later evidence may confirm, correct, or reject the claim.

## Planned additions

- JSON parser and schema diagnostics;
- packet signatures/checksums;
- tool/version provenance;
- timestamps with reproducible-build policy;
- claim registry separated from evidence records;
- attachments limited to original diagrams or synthetic fixtures;
- automatic Reverse Canon indexes.
