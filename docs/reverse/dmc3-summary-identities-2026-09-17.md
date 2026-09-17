# DMC3 HD: making the counters check each other (2026-09-17)

The `68` vs `99` mistake was the third of its kind. A one-off catch is worth a
fix; three is worth a mechanism.

**Evidence:** [`dmc3-hdc-summary-identities.evidence.json`](../../evidence/executable/dmc3-hdc-summary-identities.evidence.json)

## The fault, three times

| Where | Name promised | What it held |
| --- | --- | --- |
| `startup_path_dispatch_sites` | dispatch sites | call-position ones only — 68 of 99 |
| `indirect_call_sites` vs `dispatch_sites` | both "sites" | 10,958 and 11,434 — different populations |
| `exe_dispatch_site` (SQL) | one row per site | one row per **(function, offset)** — 5,395, not 11,434 |

None of these is a wrong number. Each is a right number of the wrong thing,
printed next to a number of something else.

The SQL table is now `exe_dispatch_offset`, with the difference stated in the
schema rather than left to be discovered.

## Nineteen identities, all holding

Six are **partitions** — the parts must equal the whole:

```text
functions            = reachable_through_dispatch + outside_every_closure
vtable slots         = pure + empty + implemented
indexed accesses     = image-base + held-base
held-base accesses   = consistent + string-scan + outside-element + conflicted-base
spanning groups      = several-sections + undecided
dispatch sites       = on this + on an argument + unnamed + fixed-or-taken
```

That last one needed two new counters. I had **published** "9,535 (83.4%) nothing
known" in the receiver census while the library emitted no such figure — the
headline number of that census was not reproducible from the report. It is now,
and the partition closes exactly:

```text
1,880 + 12 + 9,535 + 7 = 11,434
```

Eleven more are subset relations (`constructors_identified ≤ stores_of_a_vtable`,
and so on), and one is the same quantity computed twice by different code: the
depth closure and the reachability flag both reach **370**.

All nineteen hold on the real image, and the importer refuses a report that
breaks any of them.

## The part that matters: identities do not catch this fault

I checked the original `68` against every identity. **Nothing fires.** 68 is a
perfectly good number of a different population and sits happily inside 99,
inside 11,434, and inside every total above it.

> An identity set is blind to a counter that is internally consistent and simply
> measures the wrong thing.

What catches it is measuring the same quantity **twice** and comparing. That
needs the report to carry the per-function counts the census is made of, so it
now does — and the check names the original fault outright:

```text
summary says startup_path_dispatch_sites=68
but the functions on the path add up to 99
```

A test pins exactly that shape: a summary whose census total is correct
(`6 + 3 = 9`) but whose subset figure counts a different population.

## Where the checks live

- **the library** — the four receiver categories partition the census, and the
  per-function count is the same population as the census. Two tests, one
  deliberately built around the fact that a call clobbers the argument registers,
  which is why each category needs its own function;
- **the importer** — all nineteen identities plus the two summary-against-detail
  sums, run against the real image's numbers on every load, not against a
  fixture. Four tests, each breaking one on purpose.

## What this does not do

It does not make a measurement correct. Every identity here is about
*bookkeeping* — that the numbers describe one consistent population. A counter
can satisfy all nineteen and still be measuring something nobody wants; that is
what reading the anomalies is for, and this is the other half.
