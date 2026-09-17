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

## What is still only prose

20 figures across 4 records are bound. Every other number in every other packet
is still prose, and will still age silently. Binding them is mechanical work, not
a discovery, and the mechanism is there when it is wanted.
