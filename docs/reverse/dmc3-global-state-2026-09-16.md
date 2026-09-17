# DMC3 HD: the engine's global state (2026-09-16)

396 polymorphic classes, 915 vtables — and none of them is a global instance.
This is where the other half of the engine lives.

**Evidence:** [`dmc3-hdc-global-state.evidence.json`](../../evidence/executable/dmc3-hdc-global-state.evidence.json)

## The shape of a receiver

The Microsoft x64 convention puts the first argument in `rcx`, which is also
where `this` goes. An address the code takes with a RIP-relative `lea` and
leaves there is something the callee **operates on**.

| | |
| --- | ---: |
| such calls | **3,753** |
| distinct addresses they name | **200** |
| of those calls, landing in `.data` | 3,562+ |
| landing in `.rdata` | 15 |

> Call sites rose from 3,582 after the REX.B fix in
> [the receiver note](dmc3-receiver-analysis-2026-09-16.md); the 200 addresses
> and the section split are unchanged.

That split is the reason this is a measurement of state and not of arguments:
passing string literals would put almost everything in `.rdata`, and almost
nothing is there.

The head is steep. **17** addresses are reached from twenty or more distinct
functions; **112** are passed exactly once.

| Address | Callers | Entry points | Calls |
| --- | ---: | ---: | ---: |
| `0xD6DC90` | **347** | 34 | 600 |
| `0xCF2520` | 302 | 14 | 340 |
| `0x5D0890` | 301 | **1** | 519 |
| `0xC99D30` | 115 | 25 | 323 |

`0x5D0890` is the odd one: 301 functions reach it, all through a *single* entry
point. `0xD6DC90` is the opposite — 34 different ways in.

Nothing here says any of these is a C++ object. A free function's first argument
is just an argument. What is measured is the *shape*: one address, many callees,
many callers.

## Two measurements that did not have to agree

How far into a block the code reaches, and how far away the next block is, come
from entirely different places — instruction displacements inside a callee, and
the sorted set of addresses the code takes.

A callee only lends its reach to a block it has **to itself**: every image
address it is given is that block, and it has no callers beyond those sites.
Without that restriction a function serving several blocks would lend its
deepest offset to all of them.

| Dropped | Sites |
| --- | ---: |
| callee serves several blocks | 575 |
| callee is reached some other way | 1,130 |

27 blocks survive with a reach:

- **22** — the reach fits inside the gap to the next block;
- **5** — it runs past it.

The 5 are not a disagreement. They are a reading: *the next address the code
takes is a field inside this block*. The clearest is `0xC99D30`, reaching 26,465
bytes against a gap of 26,432 — thirty-three bytes past an address that is
therefore part of it.

## The engine splits cleanly in two

Of the 200 blocks, **exactly one** is reached by a function the constructor
identification names: `DMC3::FullMotionVideoManager` at `0xBEAFA0`. The other
199 have no class, because nothing writes a vtable into them.

```text
allocated      396 polymorphic classes, 915 vtables, full RTTI
fixed          200 blocks, 1 with a class, reached through free functions
```

A port can read the second half only through the code that touches it, which is
what makes the reach measurement the only handle on it.

## Correction: the constructor half of the size measurement never ran

The [class size floors](dmc3-class-size-2026-09-16.md) were published as coming
from two kinds of function: vtable-bound methods, and constructors identified by
the vtable they store at offset zero. **Only the first ever contributed.**

The measurement read each function's identified class from a field that the
constructor identification fills in *further down the same build*. At the point
it ran, that field was empty for every function, and the constructor branch was
dead. The same ordering mistake made this global-block census find no class at
all where it should find one.

Nothing published was unsound — a floor from bindings alone is still a floor —
and the largest floors are unchanged. The support counts move:

| | Published | Corrected |
| --- | ---: | ---: |
| floor above their own vtable pointer | 320 | **326** |
| corroborated | 171 | **177** |
| lone outlier | 121 | **124** |
| single source | 32 | **39** |
| type information only | 72 | **56** |
| base placement gives the deeper floor | 4 | **14** |

`CEm035` is still 61,305 bytes and `CPlDante` still 46,625; the median is still
1,307.

## Querying it

```sql
SELECT * FROM v_exe_global_block LIMIT 20;
SELECT * FROM v_exe_global_block WHERE agreement = 'NEXT_ADDRESS_IS_INSIDE';
```

## Open work

- 174 of the 200 blocks have no dedicated callee, so nothing bounds their
  extent. Widening the receiver analysis past the first argument would supply
  some of them;
- the 112 addresses passed exactly once are as likely to be one-off arguments as
  state, and nothing here separates the two;
- a block's *contents* remain unread, and this measurement is built so that they
  stay that way.
