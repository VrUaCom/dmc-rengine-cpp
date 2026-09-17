# DMC3 HD: the published numbers had drifted (2026-09-17)

The counters now check each other. That is bookkeeping *inside* one report. This
is the other direction: do the numbers I **published** still match what the code
produces?

**Evidence:** four correction records, in
[`class-size`](../../evidence/executable/dmc3-hdc-class-size.evidence.json),
[`global-state`](../../evidence/executable/dmc3-hdc-global-state.evidence.json),
[`receiver-analysis`](../../evidence/executable/dmc3-hdc-receiver-analysis.evidence.json)
and [`class-layout`](../../evidence/executable/dmc3-hdc-class-layout.evidence.json).

## Four records had gone stale

Three fixes in quick succession — the interior-address rule, reading REX.B, and
resolving element-size conflicts — moved figures that records were still
stating as current.

| Record | Claimed | Now |
| --- | --- | --- |
| size-floor counts | 326 / 177 / 124 / 39 | **329 / 192 / 112 / 36** |
| global blocks | 3,582 calls, 1,044 dropped, 26 with a reach | **3,753 / 1,130 / 27** |
| interior address | on `this` 1,855, bound-function 767 | **1,880 / 785** |
| constructor stores | 270 / 265 / 44 entries | **279 / 274 / 47** |

The last one is a different failure from the first three. Its numbers were
corrected — in a *different packet*, about the receiver analysis — and nothing
linked back. A correction nobody can find from the thing it corrects is not much
of a correction.

All four are now correction records with `supersedes`, not edits to history.

## And a fifth, in SQL

Chasing the figures turned up a disagreement between the summary and the view:
`global_blocks_reach_within_the_gap` said **21**, `v_exe_global_block` said
**22**.

The highest block has nothing after it. The view's `CASE` fell through to
`REACH_FITS_THE_GAP` — there is no gap for it to fit. It now reports
`NO_NEXT_BLOCK`, and the two agree at 21.

## How I found them, and why that is not good enough

I scanned every number in every packet against the current counters. It produced
**44 hits, of which 4 were real**. The rest were RVAs, byte offsets, the *old*
side of a stated "was → now", and numbers that simply happen to coincide.

> Matching bare numbers in prose is a lead generator, not a verdict. Every one of
> the 44 had to be read.

## The durable fix

A record can now declare the figures it states, bound to the counter each comes
from:

```json
"figures": {
  "dispatch_sites_on_this": 1880,
  "dispatch_sites_in_a_bound_function": 785,
  "dispatch_sites_resolved": 105
}
```

The evidence schema is strict, so this needed a real field rather than a
convention — it rejected the first attempt outright, which is the schema working.
A figure must be a **non-negative whole number**; a string, a negative, a real
and a nested object are all refused, and a test covers each.

`check_evidence_figures.py` then verifies every declared figure against a fresh
report:

```text
20 declared figure(s) checked, 0 stale, 0 unknown
```

It skips records a correction supersedes — their figures are exactly what the
correction says no longer hold — and it **fails on a figure naming no counter**,
because a check that silently checks nothing is worse than no check. Five tests,
in CI.

## Binding the rest

**102 figures across 33 records** are now bound, up from 20. The method was to
propose a binding wherever a record's text contains the *current* value of a
counter, then read every proposal.

That reading mattered. Roughly half the proposals were coincidences and had to be
rejected:

| Text | Coincides with | Actually |
| --- | --- | --- |
| "200 of 200 comparisons" | `global_state_blocks` = 200 | a ratio |
| "sinf (51)" | `name_tables_unreferenced` = 51 | a call count |
| "376 bytes — 47 slots" | `field_layout_entries` = 47 | a slot count |
| "32 of its 503 bytes" | `dispatch_sites_on_a_member` = 75 | a byte range |
| "recovered arrays from 278 to 281" | `indexed_arrays` = 278 | a *historical* value |

That last row is the subtle one: a number that was true in the past and happens
to equal a counter's value now. Binding it would make the check pass for entirely
the wrong reason.

**Binding 82 more figures found no new drift.** All 102 agree. That is worth
stating plainly rather than dressing up: the four found by hand were the whole of
it, for the counters that exist.

## Where the check runs

The checker needs a report, which needs the executable, so it runs where the
artifact is rather than in CI. What runs in CI is the checker's own guardrails —
five tests that a contradicted figure fails, a superseded record is skipped, and
a figure naming no counter is an error.

## Still only prose

Numbers with no counter behind them — RVAs, byte offsets, ratios, per-class
figures — remain unbound, and always will be: there is nothing to bind them to.
The mechanism covers what the report counts, which is the part that moves when
the analysis changes.
