# The tool now follows funclets and taken addresses, and two implementations agree set for set

2026-09-23. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-closure-corrected`.

The closure-gaps note measured, in Python, that the function map's
`outside_every_closure` flag misses two edge kinds the file records, and left the
fix for its own increment because it moves a published counter. This is that
increment.

## What changed in the tool

- **`read_unwind`** now records, for a frame with a handler, the handler's RVA
  and the RVA of the language-specific data after it. The data is kept raw; its
  shape depends on the handler.
- **The code graph** records every RIP-relative `lea` target as a *taken
  address*, separately from data references.
- **The function map** runs one closure twice. The first follows calls, tail
  jumps and dispatch, exactly as before, and sets `reachable_through_dispatch`.
  The second adds two edge kinds and sets `reachable_through_recorded_edges`:
  - a **funclet** edge from a function whose handler data is a C++ `FuncInfo` —
    recognised by its magic number, with bounds no compiler-emitted structure
    approaches — to each unwind action, catch handler and IP-map entry it names;
    or an SEH scope table, accepted only when **every** entry's range lies inside
    the function that owns the handler;
  - a **taken-address** edge from a function to any function whose exact start
    it loads with `lea`.

  Both runs let a newly reached function contribute its own dispatch slots.
- **`outside_every_closure`** now counts what the second closure does not reach.
  A new counter, `reached_only_through_funclets_or_taken_addresses`, holds the
  difference, so the dispatch-only bound stays measurable and the three form a
  partition.

The SEH path identifies scope tables by structure alone, without the handler's
import name, and on this image it accepts exactly the five that the Python
measurement found by name.

## The numbers

| counter | value |
| --- | ---: |
| `functions` | 7,389 |
| `reachable_through_dispatch` | 5,748 |
| `reached_only_through_funclets_or_taken_addresses` | **362** |
| `outside_every_closure` | **1,279** (was 1,641) |
| `funcinfo_structures` | 792 |
| `scope_tables` | 5 |
| `funclet_edges` | 693 |
| `taken_address_edges` | 472 |

`reachable_through_dispatch` does not move, so every figure bound to it still
holds. `outside_every_closure` moves from 1,641 to 1,279, and the two live
records that bound 1,641 are superseded here.

## Two implementations, one set

The Python script decides a taken address by a byte pattern over all of `.text`;
the tool decides it from instructions its decoder actually walked. The script
parses handler data independently of the C++ parser and identifies SEH tables by
import name where the tool uses structure. Starting from the tool's own
dispatch-only complement, the script re-derives the split and reports
`agrees_with_the_tool: true` — **the same 1,279 functions, not just the same
count**. Neither implementation was checked against the other's code; they were
checked against each other's answer.

## The population, restated for 1,279

| | outside | only via funclets or taken addresses | via calls and dispatch |
| --- | ---: | ---: | ---: |
| functions | 1,279 | 362 | 5,748 |
| calls an import | **13.1%** | 4.1% | 6.1% |
| references a string | **4.1%** | 2.2% | 1.4% |
| contains a dispatch site | 18.0% | 1.4% | 33.6% |
| installs a vtable | 5.6% | 1.7% | 11.0% |
| has no callers | 40.1% | 92.3% | 32.5% |
| median size, bytes | 268 | 49 | 227 |

The 362 are what funclets look like — median 49 bytes, 92% with no caller, almost
no imports or dispatch. Removing them leaves a population that is *more*
distinctive than before: the 1,279 call the platform more than twice as often as
dispatch-reached code and reference strings three times as often. 71 of them
install a vtable.

The library-tail split changes most. At or above `0x33F000` only 92 functions
over 24,816 bytes remain outside, against 405 before — most of the runtime
stubs were exception-handling support now reached. Below it, 1,187 functions over
532,188 bytes: **95.5% of the unreached mass is game code**.

## A bug in the checker, found by this change

Moving the counter made `check_evidence_figures.py` report four stale records,
two of which had already been superseded — by records in *other* packets. The
checker honoured `supersedes` only within the packet that declared it, so a
correction filed in a later packet did not silence the record it corrected; it
had only looked correct while the old values happened to still match. It now
decides supersession across all packets, a test pins that, and the pre-fix
checker fails that test.

## What is still unread

- The 1,279, and the 257 among the earlier population that nothing refers to.
- Whether any further edge kind the file records — registration tables written
  at run time, for instance — explains more of them. The two edges followed here
  are the ones this pass could prove.
