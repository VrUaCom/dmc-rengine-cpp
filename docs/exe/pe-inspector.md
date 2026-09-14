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

## Instruction decoding and the code graph

`X86LengthDecoder::decode(bytes, offset)` is a bounded x86-64 instruction
*length* decoder. It reports where an instruction ends, its control-flow role,
its RIP-relative displacement and its direct branch displacement. It models no
mnemonics: the analyses built on it do not need them, and a smaller decoder is
one that can be trusted.

It fails closed. VEX and EVEX encodings, opcodes invalid in 64-bit mode and
truncated instructions return nothing rather than a guessed length.

`CodeGraphBuilder::build(bytes, image, functions)` walks each function by
**recursive descent** from its entry, following fall-through and direct
branches. It is deliberately not a linear sweep: MSVC embeds switch jump tables
inside function ranges, and a sweep decodes those tables as instructions.

The builder also folds chained ranges. An exception-directory entry whose
`UNWIND_INFO` sets `UNW_FLAG_CHAININFO` is a continuation of another function,
so entry count overstates function count — on the canonical DMC3 target, 12,235
entries are 7,389 functions.

Behind a register-indirect jump it attempts switch-table recovery. Candidate
table bases are every RIP-relative `lea` target and every 32-bit displacement in
a memory operand, because a compiler that keeps the image base in a register
reaches its tables through `base + disp32` rather than through a RIP-relative
load. A candidate is accepted only when consecutive entries land inside the same
function's own ranges; both the offset-from-table and the image-base-relative
reading are tried, and the one validating further wins. Jumps with no valid
table are counted as unresolved rather than assumed empty.

`PeDirectoryReader` also decodes the unwind code array, so each range reports its
prologue size, stack reservation, pushed and saved register counts,
frame-pointer register and handler flags. An unrecognised unwind operation stops
the walk instead of desynchronising it, and the range is marked not fully
decoded.

## Function attribution

`FunctionMapBuilder::build(bytes, inputs)` joins the code graph against the
import table, the export table and the recovered RTTI class graph. Per function
it reports callers and callees, reachability from the entry point and from each
export, vtable slots that bind it to a class, imported symbols it calls, and
literals it references.

Per function it also surfaces the prologue facts from the primary unwind range —
a continuation's own record is never mistaken for the function's prologue.

Import calls are attributed in both forms: `call [rip+slot]` directly, and
`jmp [rip+slot]` thunks. Thunks usually carry no unwind data and so never enter
the inventory; the builder decodes a called-but-uninventoried target's single
jump to recover the import behind it.

```bash
dmc-rengine map-functions <path> [--out <file>] [--all] [--limit <n>] [--no-strings]
```

By default the report emits the attributed subset plus the aggregate tables;
`--all` emits every function.

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
- virtual call-site resolution, which would lift reachability off its floor;
- argument and return shape from frame facts plus first-use register analysis;
- COM vtable recovery for interfaces the import table cannot see;
- per-function semantic recovery over the function inventory;
- executable source-recovery database.
