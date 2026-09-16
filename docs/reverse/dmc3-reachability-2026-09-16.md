# DMC3 HD: how much of the image can the entry point reach? (2026-09-16)

Every reachability figure in this reverse has been published as a lower bound
with nothing beside it. This puts a number on the other end.

**Evidence:** [`dmc3-hdc-reachability.evidence.json`](../../evidence/executable/dmc3-hdc-reachability.evidence.json)

## The bracket

| | Functions | Share |
| --- | ---: | ---: |
| direct calls from the entry point | **370** | 5.0% |
| + every virtual call reaching any vtable's slot | **5,748** | 77.8% |
| outside both | **1,641** | 22.2% |
| **inventory** | **7,389** | |

The upper bound rests on an assumption that is *false as an answer and sound as
a bound*: nothing in the file says what a receiver's type is, so the set of
things it could be is the set of vtables that carry the slot. Whatever this does
not reach, nothing in the file says the entry point can.

The truth is between 370 and 5,748. **The width of that gap is the result** —
five per cent of this image's control flow is fixed at compile time, and the
rest is decided at run time.

## The usual tightening does nothing

A virtual call can only reach a class something actually builds, so restricting
candidates to vtables that reachable code installs normally narrows a bound like
this sharply.

Here it collapses. Of the **915** located vtables, code reachable by direct calls
installs **4**.

Construction is itself behind dispatch, so the analysis starves before it
starts. The bound is reported unrestricted for that reason — not out of caution.
The `4` is the evidence that nothing tighter is available from structure alone.

## What sits outside

1,641 functions, and the interesting part is what they are *not*:

| | |
| --- | ---: |
| exported | **0** |
| bound to any vtable slot | **0** |
| import thunks | **0** |
| called by something (itself outside the bound) | 794 |
| nothing in the image refers to them at all | 847 |

587,983 bytes of the 3,085,665 the inventory covers — **19% of the code by
size**.

This is not a claim that the code is dead. It is the precise statement that
every route into it is one this file does not record. It replaces the older,
vaguer figure of 848 "structurally unreferenced" functions, which counted what
nothing *pointed at* rather than what nothing could *reach*.

## Correction: 210 dispatch tables in data were 10

Runs of consecutive function addresses in read-only data are a dispatch
mechanism beside the vtables. Scanning for them first reported **210 runs
holding 1,617 entries**. That was wrong.

A vtable *is* such a run, and the scan excluded vtables **by base address
only**. A slot holding something the function inventory does not cover splits a
vtable into fragments whose bases are not the vtable's — so each fragment counted
as a table of its own. The three largest supposed tables each sat exactly **376
bytes (47 slots)** inside a located `CComEm` vtable, which is what gave it away.

Excluding each vtable's **whole extent** leaves:

| | Before | After |
| --- | ---: | ---: |
| runs | 210 | **10** |
| entries | 1,617 | **45** |

**97% of the entries were an artefact of a gap in the function inventory.**

What survives is uniform in a way the inflated figure hid: **all 45 entries name
functions the reachability bound does not reach**, and only 1 of the 10 runs has
its address taken by any inventoried function. A test pins this — the same bytes
inside a vtable's declared extent are not a table, and outside it they are.

## A second thing the tests caught

`INSERT OR IGNORE` suppresses **CHECK** violations, not just uniqueness
conflicts. A malformed row would have vanished silently rather than been
refused, in all three tables that carry CHECKs. Every one now uses a conflict
clause scoped to its own unique key, so idempotent re-import still works and a
contradiction still raises.

## Querying it

```sql
SELECT * FROM v_exe_reachability_bracket;
SELECT * FROM v_exe_unreachable_function LIMIT 20;
SELECT * FROM exe_function_pointer_run ORDER BY entries DESC;
```

Build the database with `map-functions --all`; without it the report carries a
subset of functions and the bracket's percentages describe that subset rather
than the image.

## Open work

- the 1,641 are reached *somehow* — a startup path outside the unwind inventory
  is the obvious candidate, and the 10 runs are 45 entries of evidence for it,
  but nothing in the file names the mechanism;
- the bound counts a slot as reachable image-wide once any reachable function
  dispatches through it. Pairing a dispatch site with its receiver's *class
  family* would tighten it considerably, and the receiver analysis resolves only
  47 of 11,434 sites today;
- COM receivers are not image classes, so D3D11 dispatch is outside the bound
  entirely — it adds no image functions, but it is not covered by it either.
