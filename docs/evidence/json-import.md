# Strict Evidence Packet JSON Import

DMC Rengine imports public Evidence Packets as untrusted input.

## Parsing layers

### Generic bounded JSON parser

The core JSON parser provides:

- null, boolean, signed/unsigned 64-bit integer, finite floating-point, string, array, and object values;
- configured maximum input bytes;
- maximum nesting depth;
- maximum total value count;
- array and object member limits;
- maximum decoded string bytes;
- duplicate object-key rejection;
- trailing-content rejection;
- integer overflow rejection;
- JSON number grammar validation;
- Unicode `\uXXXX` escapes and surrogate pairs.

The parser reports a byte offset and message. It does not perform network or filesystem access.

## Evidence schema importer

`evidence_packet_from_json()` maps the generic JSON DOM into `EvidencePacket`.

### Strict field whitelist

Schema version 1 allows only:

Root:

- `schema_version`;
- `id`;
- `title`;
- `project`;
- `artifacts`;
- `records`.

Artifact:

- `id`;
- `role`;
- `sha256`;
- `size`.

Record:

- `id`;
- `claim_id`;
- `title`;
- `summary`;
- `confidence`;
- `tags`;
- `supersedes`;
- `locations`.

Location:

- `artifact_id`;
- `file_offset`;
- `size`;
- `rva`;
- `va`;
- `symbol`;
- `note`.

Unknown fields are rejected. This prevents silent schema drift and accidental acceptance of misspelled security/evidence fields.

## Validation

The importer validates:

- schema version 1;
- required fields and exact JSON types;
- ID/title/summary length limits;
- artifact/record/location/tag/supersession count limits;
- unique artifact and record IDs;
- recognized confidence values;
- 64-character hexadecimal SHA-256 values;
- artifact-reference integrity for every location;
- final `EvidencePacket::valid()` invariants.

SHA-256 strings are normalized to lowercase.

## Diagnostics

Schema diagnostics use JSON-style paths:

```text
$.records[2].locations[0].artifact_id
```

Syntax errors use the parser byte offset:

```text
$@137
```

## Round trip

The test suite requires:

```text
EvidencePacket
  → deterministic JSON export
  → strict JSON import
  → deterministic JSON export
```

The final JSON must be identical.

## CLI

```bash
dmc-rengine validate-evidence evidence/known-targets/dmc3-hdc-phase12.evidence.json
```

The command reads the file through GDSpaces and reports schema, packet ID, title, project, artifact count, and record count. Invalid input prints every importer diagnostic and exits non-zero.

## Current compatibility policy

- only schema version 1 is accepted;
- unknown fields are rejected;
- extensions require a future explicit schema decision;
- no automatic confidence promotion occurs;
- successful import validates structure, not the factual truth of claims;
- referenced proprietary artifacts remain local and are never fetched automatically.
