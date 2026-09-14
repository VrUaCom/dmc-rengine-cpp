# DMC3 HD function-level map (2026-09-14)

Second layer of the reverse: instruction-accurate walks of every function, the
call and data-reference graph, and mechanical attribution of code to classes,
imports and literals.

Builds on [the structural reverse](dmc3-exe-structural-reverse-2026-09-13.md).

**Evidence:** [`dmc3-hdc-function-map.evidence.json`](../../evidence/executable/dmc3-hdc-function-map.evidence.json)
**Report:** [`dmc3-hdc-function-map.report.json`](../../evidence/executable/dmc3-hdc-function-map.report.json)

```bash
dmc-rengine map-functions /path/to/dmc3.exe --out map.json
```

Deterministic, and it runs in about a tenth of a second.

## Correction: 7,389 functions, not 12,235

The structural pass reported 12,235 `RUNTIME_FUNCTION` entries and said the
count bounds the function count from below. The exact figure is now known:

| | |
| --- | --- |
| Exception-directory entries | 12,235 |
| **Functions** | **7,389** |
| Continuation ranges | 4,846 |

An entry whose `UNWIND_INFO` sets `UNW_FLAG_CHAININFO` is a continuation of
another function, and its chained parent entry names the function it belongs to.
The linker splits functions across ranges freely.

The exported `dmc3_main` is a clean demonstration: its primary range is
`0x2C5DF0`–`0x2C5E10`, 32 bytes, but three chained ranges carry it to
`0x2C5FE7` — 503 bytes in total. An analysis that reads only the primary range
sees six percent of the function and concludes, wrongly, that `dmc3_main` barely
does anything.

This correction supersedes `ev-dmc3-function-inventory` in the evidence
registry. The original record is preserved.

## Instruction decoding

The walk needs exact instruction boundaries, so the Binary Inspector gained an
x86-64 length decoder. It is a length decoder, not a disassembler: it answers
where an instruction ends, whether it transfers control and where to, and
whether it addresses memory relative to RIP. It models no mnemonics, because
nothing here needs them and a smaller decoder is one that can be trusted.

It fails closed. VEX and EVEX encodings, opcodes invalid in 64-bit mode and
truncated instructions return nothing rather than a guessed length. The target
turns out to contain no real AVX at all — it is an SSE-era build — so the
unmodelled encodings only ever appear in data.

### Cross-validation

The decoder was compared against GNU objdump 2.42 at every position objdump
decoded inside a function range:

| | |
| --- | --- |
| Positions compared | 875,067 |
| Agreement | 99.93% |

Every disagreement examined was in embedded data, not in an instruction stream.
At RVA `0x6E200` the bytes hold the values `0x6DF9B`, `0x6E009` and `0x6E06F` —
all block addresses inside the function at `0x6DF70`–`0x6E12F`. That region is a
switch jump table living inside a neighbouring function's extent. objdump itself
loses synchronisation there and starts printing `.byte` directives.

Instruction length is undefined over such bytes. That is precisely why the
production walk is a **recursive descent** from each function entry rather than a
linear sweep: a descent follows fall-through and direct branches and so never
enters a jump table.

## The graph

| | |
| --- | --- |
| Functions walked | 7,389 |
| Walks complete | 7,381 |
| Instructions decoded | 680,902 |
| Bytes decoded | 3,042,423 — **98.6%** of the unwind extent |
| Direct call edges | 28,866 |
| RIP-relative data references | 27,821 |
| Switch tables recovered | 637 |
| Block addresses from switch tables | 6,867 |
| Indirect jumps still unresolved | 1,177 |
| Literals recovered | 13,767 |

The residual 1.4 percent is alignment padding and blocks reached only through
indirect dispatch — not decoder failure.

### Switch dispatch

Reaching 98.6 percent required resolving compiled switches, and the dispatch
form in this build is not the textbook one. It does not load the table with
`lea reg, [rip+table]`:

```text
cmp  eax, 6
ja   default
mov  ecx, [r12 + rax*4 + 0x29B14]   ; r12 holds the image base
add  rcx, r12
jmp  rcx
```

The table base arrives as a plain 32-bit displacement, because the compiler
keeps the image base in a register and indexes `base + displacement`. Entries
are therefore image-base-relative RVAs, not offsets from the table.

That is why the first implementation found zero tables: it only collected
RIP-relative `lea` targets. Any disp32 in a memory operand is now a candidate,
and **validation does the filtering** — a candidate is accepted only when
consecutive entries land inside the same function's own ranges. A large
structure offset produces zero valid entries and is rejected. Both readings
(offset-from-table and image-base-relative) are tried, and the one validating
further wins.

Recovering the tables cut the functions with no structural referrer at all from
2,101 to **848**, which is the clearest measure of what those missing edges were
costing.

## Attribution

2,426 of 7,389 functions now carry at least one structural fact beyond their
extent.

| Attribution | Functions |
| --- | --- |
| Bound to a class vtable slot | 1,879 |
| Calls an imported symbol | 535 |
| References a literal | 160 |
| Exported by name | 2 |

### Code bound to classes

Reading each recovered vtable resolves **3,588 of 13,894 slots** to inventoried
functions. That binds 1,879 distinct functions to a class, a vtable index and a
slot number — the compiler's own binding of code to type.

| Class | Vtable slots | Bound | Distinct functions |
| --- | --- | --- | --- |
| `CComEm006` | 271 | 221 | 202 |
| `CComEm008` | 271 | 221 | 205 |
| `CComEm000` | 265 | 215 | 200 |
| `CEm013` | 148 | 57 | 53 |
| `CEm017` | 148 | 57 | 53 |
| `CEm021` | 148 | 57 | 53 |

The `CComEm###` AI-command classes carry by far the widest virtual surface in
the image: over 200 distinct functions each. Slots that resolve to no
inventoried function are mostly shared or unwind-less stubs.

### Import usage

535 functions call imports, across 218 distinct imported symbols.

| Calling functions | Import |
| --- | --- |
| 110 | `VCRUNTIME140.dll!memset` |
| 66 | `sqrtf` |
| 53 | `cosf` |
| 51 | `sinf` |
| 39 | `atan2f` |
| 38 | `VCRUNTIME140.dll!memcpy` |
| 37 | `strcmp` |

A trigonometric surface spanning more than 150 functions is what a 3D action
game's math load looks like from the outside.

Two forms are attributed. The direct one is `call [rip+slot]`. The indirect one
is a thunk: `jmp [rip+slot]`. Thunks carry no unwind data, so they never enter
the function inventory — an independent objdump pass finds 771 IAT reference
sites, 651 calls and 120 thunk jumps, of which 110 thunks lie outside every
unwind range. The map resolves 90 of them by decoding the thunk's single jump,
which is why `import_thunks` reads zero while import attribution still works.

## Reachability, and what it does not mean

| Root | Functions reached |
| --- | --- |
| CRT entry point, direct and switch edges | 370 |
| `dmc3_main`, direct and switch edges | 357 |
| Union with vtable-bound functions | 2,249 |
| No structural referrer at all | 848 |

**The remaining 5,140 functions are not dead code.** Virtual dispatch, function
pointers in data, jump tables and callbacks are all indirect, and a direct-call
graph cannot follow any of them. In a codebase where `CWork` is the base of two
thirds of all polymorphic types, most calls are virtual by construction. The
number measures the method's reach, not the program's.

## Stack frames from unwind codes

The unwind codes describe exactly what each prologue did, so every function now
carries frame structure recovered without interpreting a single instruction.

| | |
| --- | --- |
| Total stack reserved across all prologues | 694,008 bytes |
| Average frame | 94 bytes |
| Largest single frame | 13,544 bytes |
| Functions establishing a frame pointer | 308 |
| Functions declaring an exception or termination handler | 2,210 |

The handler count matches an independent flag census of the same directory
exactly, which is a useful cross-check on the code walker.

`dmc3_main` reads out as: 19-byte prologue, 88 bytes reserved, no pushed
registers, exception handler present, 103 instructions, 13 callees.

One metric deserves a caution rather than a headline. Exactly **2** of 7,389
functions have an unwind record describing no prologue work. That is a property
of the inventory's membership rule, not of the program: a function earns an
exception-directory entry *because* it has a prologue worth unwinding. The 110
import thunks sitting outside every unwind range are the same effect seen from
the other side.

## Method

Three components were added to the Binary Inspector, each tested against
synthetic fixtures built inside the test files:

- `X86LengthDecoder` — bounded, fail-closed instruction lengths;
- `CodeGraphBuilder` — recursive-descent walks folding chained ranges into the
  function that owns them, and recovering switch tables behind
  register-indirect jumps;
- `FunctionMapBuilder` — joins the graph against the import table, the export
  table and the recovered class graph.

The code-graph fixture deliberately places a refusable VEX encoding in the
unreachable tail of a function: if the walk ever degraded into a linear sweep,
that test would fail.

## Open work

Per-function semantics remain unrecovered, and that is the honest headline. A
function known to be slot 7 of `CEm010`'s third vtable and to call `sqrtf` still
has no established behavior.

Next layers, in order of leverage:

- **resolve virtual call sites.** 1,177 indirect jumps and every virtual call
  remain unresolved. Where a call site's receiver type can be established, the
  edge becomes real and reachability stops being a floor. This is the single
  largest remaining gap in the graph;
- **cross-reference literals** against the documented resource families under
  [`docs/formats/`](../formats/README.md). A function referencing a known path
  pattern gains a first semantic anchor, and 13,767 literals are already
  recovered and attributed;
- **argument and return shape** from the frame facts plus register reads before
  first write, which would give each function a candidate signature;
- **COM vtable recovery** for the D3D11 path, invisible to the import table.
