# Canonical EXE: control-flow decoding and switch-table separation

Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Evidence: **EXE_CONFIRMED** for the six bounded table consumers and extracted
slots; **STRUCTURAL_CONFIRMED** for decoding, edge arithmetic and metadata.
Subsystem semantics and complete runtime reachability remain unconfirmed.

## Correcting the earlier census

The original counter only recognized `(bad)` as the first mnemonic token.
GNU objdump also emits prefixed forms, including `rex.XB (bad)`. The corrected
linear totals are **3,651**, including **3,292** inside exception-table ranges,
instead of 3,488 / 3,133. The census script, summary and report are corrected;
the existing address tables are byte-identical after regeneration.

A `.pdata` entry is not proof that every covered byte, or even every range
start, is executable code. Five range starts are confirmed switch-table data.
This corrects the assumption that seeding every range start always seeds code.

## Bounded results

| Measurement | Result |
|---|---:|
| Runtime ranges in canonical PE | 12,235 |
| Confirmed data starts excluded from initial seeds | 5 |
| Remaining distinct initial seeds, including PE entry | 12,230 |
| Visited instruction starts | 611,752 |
| Sum of visited instruction lengths | 2,723,341 bytes |
| Decoder failures on visited traversal | 0 |
| Overlapping visited instructions | 0 |
| Direct call candidates | 38,370 |
| Direct conditional branch candidates | 46,399 |
| Direct unconditional jump candidates | 14,246 |
| Recovered switch edges, distinct targets per site | 49 |
| Unresolved indirect calls, including import calls | 8,637 |
| Unresolved indirect jumps | 1,383 |

The indirect-call count includes imports; it must not be compared directly to
the earlier census category that separately resolved IAT operands.

All **99,015** direct edge destinations were independently recomputed from
machine-byte displacements, including short/near conditional branches. All
66 switch slots and the 241-byte selector remap match the EXE. Source/target
instruction membership, assumed call/conditional fallthrough, lack of
overlapping instructions and exclusion of these tables were verified.

GNU objdump and Capstone 5.0.7 agree on instruction lengths at 611,666 common
linear starts. Another 86 starts require re-anchoring objdump at the address
reached by traversal; all 86 then agree. This verifies boundaries and lengths,
not equivalence of all decoder operand semantics.

Of 3,651 linear `(bad)` sites, 3,646 are not visited, and five lie inside valid
instructions reached from a different boundary. No `(bad)` site is a visited
instruction start. **Unvisited does not mean proven data or dead code.**

## Six confirmed switch consumers

All table entries below are uint32 RVAs added to image base `0x140000000`.
The script carries these as a canonical-hash-gated profile, not a heuristic
that silently labels arbitrary bytes as tables.

| Indirect dispatch VA | Table VA | Slots | Index bound |
|---|---|---:|---|
| `0x14002C48D` | `0x14002C818` | 17 | selector 0..240 remapped through 241 bytes at `0x14002C85C` |
| `0x1400B3FBD` | `0x1400B4FE8` | 8 | unsigned index <=7 |
| `0x1402417D1` | `0x14024190C` | 9 | unsigned index <=8 |
| `0x14027CAD1` | `0x14027D3C0` | 11 | unsigned index <=10 |
| `0x140295615` | `0x140295A28` | 13 | unsigned index <=12, after subtracting 23 |
| `0x1402B0E19` | `0x1402B1104` | 8 | unsigned index <=7 |

`switches.json` records bounds/load sites and each slot target. The five
excluded initial seeds are the last five table starts in this table. The first
table instead lies inside a range beginning at `0x14002C812`; its epilogue ends
in `ret` at `0x14002C817`, immediately before the table.

For the first dispatch: `cmp rax,0xF0` at `0x14002C466` and unsigned `ja`
select the default for higher selectors. The byte remap at `0x14002C85C`
maps 241 selectors to 17 slots; a uint32 load at `0x14002C482`, image-base
addition at `0x14002C48A`, and `jmp rcx` at `0x14002C48D` establish the target
calculation. Slot 16 points to `0x14002C78E`, also the out-of-range default.
No gameplay meaning is assigned to these selector values by this pass.

Before excluding confirmed table seeds, a baseline traversal had 611,085
instruction starts, five decode failures and no instruction overlaps. Their
addresses were `0x1400B5020`, `0x140241910`, `0x14027D3C2`, `0x140295A28`
and `0x1402B1114`. Each path originated in one of the five table seeds.
Resolving the six switches also exposes additional case code, so simply
subtracting five errors would not reproduce the corrected traversal.

## Reproduction

Dependencies: Python 3, `capstone==5.0.7`, GNU objdump 2.42.

```sh
python scripts/reverse/trace_canonical_exe.py /path/to/dmc3.exe /tmp/dmc3-cfg
python scripts/reverse/verify_canonical_cfg.py /path/to/dmc3.exe /tmp/dmc3-cfg
```

Address-only results live in `data/reverse/exe-cfg-20260914/`. `visited.tsv.gz`
and `edges.tsv.gz` use deterministic gzip mtime=0. `verification.json` is the
stdout receipt from the second command. No game binary or assembly dump is
committed. The trace tool refuses a noncanonical source before writing output.

## Exact remaining frontier

The zero-error result applies only to this traversal. Calls assume a return
path even when a callee may not return; runtime metadata seeds are not proof
of observed execution. Other indirect jumps terminate traversal. Additional
switches, vtable methods, callbacks, TLS callbacks, exception handlers, leaf
functions and dynamically generated paths are not comprehensively covered.

Next work should join visited indirect sites with the IAT and existing vtable
anchors, recover further bounded tables, and add proven roots with explicit
provenance. Only then extend subsystem contracts and C++ reconstruction.
Neither instruction coverage nor zero decoder errors establishes full reverse
or a working recompiled game.
