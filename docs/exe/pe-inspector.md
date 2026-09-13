# Read-Only PE Inspector

The EXE foundation provides a generic, content-clean PE32/PE32+ parser. It is not specific to DMC3 and does not modify files.

## Implemented fields

- DOS `MZ` signature;
- PE signature;
- COFF machine, section count, timestamp, and characteristics;
- PE32 vs PE32+;
- image base;
- entry-point RVA;
- SizeOfImage;
- SizeOfHeaders;
- subsystem and DLL characteristics;
- section/file alignment, checksum, and code/data sizes;
- the data-directory array;
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

Directory and RTTI regressions build synthetic PE32+ images carrying invented imports, exports, forwarders, unwind ranges, an RSDS record, relocations, TLS callbacks, and a two-subobject RTTI hierarchy. They also assert the failure paths: an unmapped directory, a locator whose self-reference does not match, a locator with no type descriptor, and a vtable that runs past the code section.

## Known DMC3 target

Historical target metadata is recorded in Evidence/known-target records. The generic parser and generic byte-window extractor do not hardcode DMC3 addresses or infer semantic truth merely because a caller supplies a matching hash.

A game/profile-specific reverse workflow is responsible for selecting its canonical expected artifact identity and for promoting any later semantic claim through Evidence.

## Directory tables

`PeDirectoryReader::read(bytes, image)` parses the directory tables of an
already-parsed image:

- imports, including delay imports and ordinal-only entries, with per-function
  IAT RVAs and hints;
- exports, distinguishing real addresses from forwarder strings;
- the x64 exception directory as a `RUNTIME_FUNCTION` inventory with size
  statistics;
- the debug directory, including CodeView (RSDS) GUID, age, and PDB path;
- base relocations, summarized by block and type;
- the TLS directory and its callback array;
- the resource tree, summarized by top-level type.

Each table is recovered independently and reports its own failures, so one
malformed directory does not cost the caller the rest. Every walk is bounded by
an explicit safety limit.

## RTTI class graph

`RttiScanner::scan(bytes, image)` recovers the MSVC run-time type information:
type descriptors, complete-object locators, class hierarchy descriptors,
base-class descriptors, and vtables.

A locator is accepted only when its self-reference field equals its own RVA.
That check rejects essentially every coincidental byte pattern, which is what
makes an unanchored scan trustworthy.

Results are grouped by type, not by locator: multiple inheritance gives one
type several locators and several vtables, and each vtable is kept with the
subobject offset its locator records. A class the scan cannot tie to a vtable
is reported with a zero vtable RVA rather than a guessed one.

`reconstruct_type_name` turns an MSVC decorated name into a readable one
(`.?AVFullMotionVideo@DMC3@@` becomes `DMC3::FullMotionVideo`) and reports
whether the reconstruction is complete. Unmodelled constructs are preserved
verbatim rather than dropped.

## Full analysis report

```bash
dmc-rengine analyze-exe <path> [--out <file>] [--ranges] [--no-rtti] [--no-imports]
```

Emits a deterministic JSON report — identical bytes produce byte-identical
output — covering headers, sections, directories, imports, exports, the
function inventory, debug identity, relocations, TLS, resources, and the RTTI
class graph. `--ranges` adds the individual `RUNTIME_FUNCTION` entries, which
are omitted by default because they are large and regenerable.

The report records a SHA-256 and a size for the artifact, never its bytes.

See [the DMC3 structural reverse](../reverse/dmc3-exe-structural-reverse-2026-09-13.md)
for the canonical target's results.

## Planned extensions

- load-config and control-flow-guard tables;
- COM vtable recovery for interfaces the import table cannot see;
- sanitized higher-level disassembly acquisition packets;
- per-function recovery over the unwind inventory;
- executable source-recovery database.
