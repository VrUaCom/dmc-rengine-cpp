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
| Instructions decoded | 593,458 |
| Bytes decoded | 2,647,277 |
| Direct call edges | 26,322 |
| RIP-relative data references | 25,105 |
| Literals recovered | 13,767 |

Decoded bytes fall short of the unwind extent for three reasons, none of them
decoder failure: embedded jump tables, alignment padding, and blocks reached
only through indirect dispatch.

## Attribution

2,412 of 7,389 functions now carry at least one structural fact beyond their
extent.

| Attribution | Functions |
| --- | --- |
| Bound to a class vtable slot | 1,879 |
| Calls an imported symbol | 528 |
| References a literal | 150 |
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

528 functions call imports, across 218 distinct imported symbols.

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
unwind range. The map resolves 89 of them by decoding the thunk's single jump,
which is why `import_thunks` reads zero while import attribution still works.

## Reachability, and what it does not mean

| Root | Functions reached |
| --- | --- |
| CRT entry point, direct calls | 319 |
| `dmc3_main`, direct calls | 306 |
| Union with vtable-bound functions | 2,198 |

**The remaining 5,191 functions are not dead code.** Virtual dispatch, function
pointers in data, jump tables and callbacks are all indirect, and a direct-call
graph cannot follow any of them. In a codebase where `CWork` is the base of two
thirds of all polymorphic types, most calls are virtual by construction. The
number measures the method's reach, not the program's.

## Method

Three components were added to the Binary Inspector, each tested against
synthetic fixtures built inside the test files:

- `X86LengthDecoder` — bounded, fail-closed instruction lengths;
- `CodeGraphBuilder` — recursive-descent walks folding chained ranges into the
  function that owns them;
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

- **resolve virtual call sites.** Where a call site's receiver type can be
  established, a virtual call becomes a real edge and reachability stops being
  a floor;
- **switch table recovery.** Decoding the embedded tables the descent currently
  steps around turns indirect jumps into edges;
- **COM vtable recovery** for the D3D11 path, invisible to the import table;
- **prologue and frame analysis** from the unwind codes already parsed, which
  yields stack frame sizes and saved-register sets per function;
- **cross-referencing literals** against the documented resource families under
  [`docs/formats/`](../formats/README.md), where a function referencing a
  known path pattern gains a first semantic anchor.
