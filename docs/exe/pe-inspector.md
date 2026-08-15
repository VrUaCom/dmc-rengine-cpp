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

## CLI inspection

```bash
dmc-rengine inspect-exe <path>
```

The CLI mounts the file's parent directory as a read-only GDSpaces source, reads a `ResourcePayload`, then invokes the PE reader. This preserves the no-second-resolver rule.

## Reverse byte-window acquisition

For exact reverse-evidence reacquisition, use the separate hash-gated command:

```text
dmc-rengine extract-exe-window <exe> <expected-sha256> <va> <size> [--hex]
```

See [Executable Byte-Window Acquisition](byte-window-acquisition.md).

The byte-window extractor is intentionally stricter than generic address conversion. A section-backed request must stay inside `SizeOfImage` and inside the intersection of the section virtual extent and raw file extent, and the entire requested RVA interval must have one unambiguous file-backed VA mapping authority. Virtual-only tails, raw-only padding beyond `VirtualSize`, ambiguous overlaps and mapping-boundary crossings are rejected.

Raw bytes are omitted by default. `--hex` is an explicit local reverse mode and must not be used to commit proprietary executable bytes to the public repository.

## Tests

The repository generates original synthetic PE32+ byte vectors in tests. No original game bytes are included.

PE and acquisition regressions cover normal mappings, malformed/truncated ranges, `SizeOfImage`, virtual/raw intersection boundaries, virtual-only tails, raw-only padding, ambiguous section mappings, deterministic acquisition receipts and optional raw-byte SHA binding.

## Known DMC3 target

Historical target metadata is recorded in Evidence/known-target records. The generic parser and generic byte-window extractor do not hardcode DMC3 addresses or infer semantic truth merely because a caller supplies a matching hash.

A game/profile-specific reverse workflow is responsible for selecting its canonical expected artifact identity and for promoting any later semantic claim through Evidence.

## Planned extensions

- data directory metadata;
- imports/exports;
- exception/unwind ranges;
- relocations;
- debug directory identity;
- sanitized higher-level disassembly acquisition packets;
- EXE evidence annotations;
- executable source-recovery database.
