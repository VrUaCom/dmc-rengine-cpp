# The report was not partial, and regenerating it takes one second

2026-09-20. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-report-refresh`.

Yesterday's layer-B note could not establish whether two functions were
reachable, and said so — but it said so for the wrong reason:

> the function-map report committed in `evidence/` is a partial artefact of 2,889
> functions with its depth fields absent, so it cannot answer the question

Both halves are wrong, and the second is the worse one.

## It walked everything

The committed report's own summary says `functions: 7389` and its top level says
`functions_emitted: 2889`. Those are not the same number because they do not mean
the same thing. `map-functions` takes an `--all` flag; without it, per-function
records are emitted only for the 2,889 functions the tool calls *attributed*,
while the walk and every counter cover all 7,389. The report states both figures
and I read one of them.

Running the tool today against the same image gives `functions: 7389` and
`walks_complete: 7381` — **identical** to the committed report — and every shared
counter matches exactly except six, which differ because the analysis changed in
this session's own earlier work rather than because anything was truncated:

| counter | committed | current |
| --- | --- | --- |
| `computed_index_references` | 16 | 0 |
| `constant_index_references` | 186 | 189 |
| `name_tables_referenced` | 27 | 17 |
| `name_tables_unreferenced` | 218 | 51 |
| `with_name_table` | 45 | 40 |
| `with_string_reference` | 136 | 139 |

## And it takes a second

The real reason the committed report could not answer the question is narrower:
it predates the counters and fields that later work added. `outside_every_closure`
— the source of the 1,641 figure several notes cite — is not in it at all,
because that counter did not exist when it was generated.

That is worth knowing. What is worth more is that I never checked. Regenerating
the report takes **one second**, and I wrote that an instrument could not answer a
question without running it. Two days ago I recorded a habit after nearly
publishing that the CRC machinery was unreachable: before trusting a caller count
of zero, disassemble the bytes before the address. This is the same failure one
level up — an assertion of unavailability with a one-second check not taken — and
this time it was published rather than caught.

## The committed report is now current

Regenerated with the same emission policy as before, no `--all`: 2,889 records of
7,389 walked, 2.08 MB to 2.76 MB. All 102 figures declared across the evidence
packets verify against it, 0 stale and 0 unknown, and the 221-test suite passes.

The figure checker could never have caught this drift, and that is by design: it
regenerates a report and compares published figures against *that*, which is the
right check for "have the numbers moved" and no check at all for "is the
committed artefact current". The two questions needed separate answers and now
have them.

## What the fresh report says about layer B

The question yesterday's note left open, answered:

| function | depth | callers | note |
| --- | --- | --- | --- |
| `0x2FCA0` resource open | 11 | 3 | reachable from an export |
| `0x49120` layer B open | 12 | 1 | reachable from an export |
| `0x338140` a resource-open caller | — | 0 | **outside every closure** |
| `0x2F930` layer B seek caller | — | 0 | inside the loose closure |
| `0x49200` layer B read | — | 1 | inside the loose closure |
| `0x49290` layer B seek | — | 1 | inside the loose closure |

Layer B's read and seek are not outside every closure, so they are reachable
under the loosest sound assumption — that every virtual call reaches whatever sits
at its slot. Their calling functions have **no direct callers at all**, so
whatever reaches them does so through dispatch. That is a bracket, not a proof:
being inside the CHA closure is an over-approximation and does not establish that
any execution reaches them.

One of the resource open's own three callers, `0x338140`, is itself outside every
closure — so at least one of the three ways into `OpenGameResource` is a way
nothing takes.

## A convention that will trip the next comparison

The report's `callers` field counts **distinct calling functions, not call
sites**. `0x49120` shows one caller, and yesterday's byte census found two call
sites — `0x2FEFA` and `0x2FF4C` — which are both inside the same function, the
continuation at `0x2FE2A`. Neither number is wrong and they answer different
questions. Anyone checking a byte scan against this report needs to know which
one they are holding.

## What is still unread

- Where a layer-B id comes from, still. The dispatch route is now the place to
  look rather than the direct call graph.
- The 1,641 functions outside every closure, as a population. The instrument for
  it works and is one second away.
- Section 1F of the 2026-09-05 reconciliation.
