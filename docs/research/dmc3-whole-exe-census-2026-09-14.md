# Whole-EXE structural census and reverse frontier

Target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Status: **STRUCTURAL_CONFIRMED** for PE metadata and address arithmetic;
**SEMANTIC_CANDIDATE** for the disassembly-derived transfer inventory.
Full semantic reverse and recompilation are **not complete**.

Correction from the CFG follow-up: the original bad-decode counter missed
prefixed forms such as `rex.XB (bad)`. Counts below include them. See
[dmc3-cfg-decoding-2026-09-14.md](dmc3-cfg-decoding-2026-09-14.md)
for the subsequent control-flow traversal and explicit limits.

## Results

| Measurement | Observed count |
|---|---:|
| PE exception-directory runtime ranges | 12,235 |
| Normal import address table slots | 229 |
| Linear-disassembly instruction starts | 880,446 |
| Direct call candidates | 44,407 |
| Direct jump candidates | 21,297 |
| Calls referencing known import slots | 651 |
| Jumps referencing known import slots | 96 |
| Unresolved indirect call candidates | 10,308 |
| Unresolved indirect jump candidates | 699 |
| Bad decodes, including those outside runtime ranges | 3,651 |
| Bad decodes inside runtime ranges | 3,292 |

All 12,235 begin/end/unwind triples match the existing independent C++
runtime-tree extractor's `runtime_ranges.tsv`. All 65,704 direct call/jump
targets were independently recomputed from the file's E8/E9/EB displacement
bytes and matched the disassembler output. This validates address extraction,
not code reachability or semantic meaning.

The bad decodes inside ranges are an explicit unresolved decoding frontier.
Do not label the whole linear listing valid code. A follow-up must re-anchor
decoding at trusted range/branch boundaries, inspect embedded data and
overlapping paths, and reconcile results with a second decoder.

## Specific resolved names

Normal import descriptors confirm `0x14034F1D8` is
`KERNEL32.dll!QueryPerformanceCounter`, and `0x14034F1A0` is
`KERNEL32.dll!Sleep`. These replace the tentative QPC-like/wait labels in
the local startup investigation; this does not establish elapsed-time units
or the intended frame scheduling contract.

Entry `0x14034615C` contains a direct call at `0x140346160` to
`0x14034673C` and a direct jump at `0x140346169` to `0x140345FF0`.
Address labels do not assert original function names.

## Reproduction and files

Run with Python 3 and GNU objdump (observed version 2.42):

```sh
python scripts/reverse/census_canonical_exe.py /path/to/dmc3.exe /tmp/exe-census
```

The tool refuses any noncanonical hash before creating output. It never
executes the game. `data/reverse/exe-census-20260914/` contains the summary,
import mapping, verification receipt, and gzip-compressed address-only TSVs.
The generator emits uncompressed TSVs; the committed copies use gzip with
mtime=0. No executable or raw assembly listing is included.

`range_begin_va` and `target_range_begin_va` denote the containing exception
table interval, not a recovered function identity. Unwind fragments may share
a logical function, and leaf functions may have no exception-table entry.
Empty range fields mean no containing interval. Direct intra-function jumps
are included; they are not all call-graph edges. `import_slot` identifies a
RIP-relative memory operand referencing a parsed IAT slot. All transfer rows
remain linear-disassembly candidates until control flow is validated.

## Remaining completion gates

1. Recover validated instruction boundaries and CFGs, including conditional
   edges, switch tables, unwind chains, exceptions, callbacks and indirect
   dispatch. Resolve the 3,292 in-range bad decodes before coverage claims.
2. Join function identities and vtable targets to existing RTTI/type evidence;
   recover calling contracts, layouts, ownership and side effects.
3. Complete subsystem semantics: startup/shutdown, platform, resource loading,
   rendering, animation, gameplay, collision, audio, input, UI and persistence.
4. Translate verified contracts to typed C++ with preserved unknown domains,
   and validate behavior against the canonical game and resource corpus.
5. Demonstrate integrated rebuild/runtime acceptance. Neither a listing nor
   an address census satisfies this gate.

This is a repository-wide structural foundation and an explicit frontier,
not a percentage estimate for a fully reversed game.
