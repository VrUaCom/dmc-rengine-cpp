# What the 1,641 are: half of them are pointed at, and 332 are funclets

2026-09-20. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-unreached-population`.

> **Corrected** by `docs/reverse/dmc3-closure-gaps-2026-09-22.md`. The 332
> unwind-data figure is superseded by an exact parse (383 no-caller exception
> targets), and the 526 "ordinary `.rdata`" references were mostly exception
> metadata beyond the short extents used here, not a second mechanism. The second
> mechanism is in code: taken addresses passed to the runtime's array iterators.

The reachability note of 2026-09-16 established the bracket and counted the
population outside it. It listed what those functions are *not* — none exported,
none bound to a vtable slot, none an import thunk — and left what they *are*
unexamined. This examines it, and corrects one of that note's figures.

## A published figure is wrong

That note says of the 1,641:

> | nothing in the image refers to them at all | 847 |

847 is the number with **no call edge**. It is not the number nothing refers to.
Scanning the whole file for each function's start address as a 32-bit RVA or a
64-bit VA, excluding `.pdata`'s own begin and end fields, finds **972 of the
1,641 referenced somewhere** — and of the 847 with no caller, **597 are
referenced** and only **250 are not**.

The report's `callers` field counts call edges, which is what it says it counts.
The prose turned that into a statement about references, which is a different
claim about a different instrument, and it made the mystery look three times
larger than it is.

## The scan is measured against a control, not assumed

A raw dword scan finds coincidences: four bytes anywhere that happen to equal an
RVA. So the same scan was run against a control — 1,641 sixteen-byte-aligned
addresses drawn from the same span, none of them a function start:

| | referenced outside `.pdata` |
| --- | ---: |
| the 1,641 real starts | **972 — 59.2%** |
| 1,641 matched non-starts | **133 — 8.1%** |

Seven times the background. The effect is real and its coincidence rate is
measured rather than argued. Subtracting a uniform background would put the
genuine figure near 840, but the background is not uniform, so the honest
statement is the pair of rates above and the direction they establish.

## Where the references are, and the entry mechanism

Per function, counting each section the references land in:

| holding section | functions |
| --- | ---: |
| `.rdata` | 526 |
| unwind data | 332 |
| `.data` | 43 |
| `.rdata` and `.text` | 34 |
| `.text` | 13 |
| everything else combined | 24 |

**332 are referenced from exception-handling data.** Those are catch and unwind
funclets: the runtime enters them from the unwind machinery using addresses in
the handler data, never by a call instruction. That is a route no call graph can
record — not because this one is incomplete, but because the mechanism is not a
call. The 2026-09-16 note's "every route into it is one this file does not
record" is exactly right, and this is one of the routes, named.

That 332 is a **lower bound**. The extent computed for each unwind structure
covers the fixed part and the handler pointer but not the variable handler data
that follows, so references sitting deeper in that data are attributed to
`.rdata` instead.

The 526 in `.rdata` are the next question: function-pointer tables that are
neither vtables nor among the ten runs the earlier correction retained.

## It is not library residue

Taking the library tail as everything at or above `0x33F000` — zlib begins at
`0x340000` and the CRT stubs run to the end of `.text`:

| | functions | bytes |
| --- | ---: | ---: |
| unreached, at or above `0x33F000` | 405 | 39,362 |
| unreached, below it | 1,236 | 548,621 |

**92% of the library tail is unreached** — 405 of the 442 functions there — which
is what statically linked library code looks like when a program uses three
functions of it. But that tail is only **6.7% of the unreached bytes**. Ninety-
three per cent of the unreached mass is game code.

The largest single run of consecutive unreached functions is
`0x346808`–`0x34EC00`: 360 functions in 16,824 bytes, 47 bytes each, the CRT stub
region. The largest by mass is `0x1FED40`–`0x2118F0`, 119 functions over 76,769
bytes, which is not library code.

## And it is not compiler noise

Against the 5,748 functions inside the closure:

| | unreached | reached |
| --- | ---: | ---: |
| calls an import | **11.1%** | 6.1% |
| references a string | **3.7%** | 1.4% |
| contains a dispatch site | 14.3% | 33.6% |
| installs a vtable | 4.7% | 11.0% |
| bound to a vtable slot | **0%** | 32.7% |
| median size | 191 | 227 |

The unreached code calls the operating system nearly twice as often as the
reached code and names things nearly three times as often. Restricted to the
game-code part below `0x33F000` the gap widens: 12.5% and 4.7%. Whatever this is,
it is not stubs and not residue — it is code that talks to the platform and
carries literals.

**77 of them install a vtable**, which is to say 77 are constructors for classes
nothing in the recorded graph ever builds. The 2026-09-16 note found that rapid
type analysis starves here because construction is itself behind dispatch; these
77 are the same wall approached from the other side.

The 0% bound to a vtable slot reproduces exactly, which is a consistency check
rather than a finding: a slot-bound function is inside the closure by
construction.

## What is still unread

- The 526 functions referenced from ordinary `.rdata`. What those tables are is
  the next question and the one most likely to name a second entry mechanism.
- The 250 that nothing refers to at all, which is the residue the 847 was
  standing in for.
- Whether the unwind-data references can be attributed exactly by parsing the
  handler data rather than bounding it.
