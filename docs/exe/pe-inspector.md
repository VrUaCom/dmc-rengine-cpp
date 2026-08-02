# Read-Only PE Inspector

The EXE foundation provides a generic, content-clean PE32/PE32+ parser. It is not specific to DMC3 and does not modify files.

## Implemented fields

- DOS `MZ` signature;
- PE signature;
- COFF machine and section count;
- PE32 vs PE32+;
- image base;
- entry-point RVA;
- SizeOfImage;
- SizeOfHeaders;
- subsystem;
- section names, virtual ranges, raw ranges, and characteristics.

## Safety behavior

- all reads are bounds checked;
- section count has a safety limit;
- truncated headers and section tables fail with errors;
- raw ranges outside the file fail;
- virtual ranges beyond SizeOfImage warn;
- overlapping raw sections warn;
- unsupported machine types remain inspectable with warnings;
- the parser exposes no write methods.

## Address conversion

`PeImage` supports checked:

- RVA → VA;
- RVA → file offset;
- file offset → RVA.

Header RVAs map directly within SizeOfHeaders. Section mappings use raw file ranges; virtual tails without raw data do not produce file offsets.

## CLI

```bash
dmc-rengine inspect-exe <path>
```

The CLI mounts the file's parent directory as a read-only GDSpaces source, reads a `ResourcePayload`, then invokes the PE reader. This preserves the no-second-resolver rule.

## Tests

The repository generates an original synthetic PE32+ byte vector in tests. No original game bytes are included.

## Known DMC3 target

Historical target metadata is recorded in the migration ledger and should later become a hash-keyed Evidence Packet. The generic parser does not hardcode DMC3 addresses or assume the file is trustworthy because its hash matches.

## Planned extensions

- data directory metadata;
- imports/exports;
- exception/unwind ranges;
- relocations;
- debug directory identity;
- section overlap policy refinement;
- sanitized JSON report;
- EXE evidence annotations;
- executable source-recovery database.
