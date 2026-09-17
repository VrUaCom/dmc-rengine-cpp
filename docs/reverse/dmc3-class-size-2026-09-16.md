# DMC3 HD: how big is an object? (2026-09-16)

The reverse has known 396 classes, their hierarchies and their vtables for a
while. It has known how large a single object is for none of them.

**Evidence:** [`dmc3-hdc-class-size.evidence.json`](../../evidence/executable/dmc3-hdc-class-size.evidence.json)

## Two sources, neither reading a field

**From the type information.** The class hierarchy descriptor records where each
base subobject sits. A base carrying a vtable occupies at least the 8 bytes of
that pointer, so *deepest such base + 8* is a floor. 200 classes get one above 8
this way.

**From the code.** A method the compiler bound into a class's vtable is handed
the object as its first argument. A memory operand at offset K inside it means
the object reaches at least K+1 bytes. Where the binding sits at a non-zero
subobject offset, `this` points at the subobject, so the reach is that much
further into the complete object.

| | |
| --- | ---: |
| classes | 396 |
| with a floor above their own vtable pointer | **326** |
| median floor | **1,307 bytes** |

> **Corrected 2026-09-16.** As first published, the constructor half of this
> measurement never ran — it read a field the constructor identification fills
> in further down the same build. Nothing was unsound and the largest floors are
> unchanged; the counts below are the corrected ones. See
> [the global-state note](dmc3-global-state-2026-09-16.md#correction-the-constructor-half-of-the-size-measurement-never-ran).

Neither source reads what is *at* any offset.

## Support is a column, not a filter

A deep access could be one mistaken reading, so each floor carries how many
functions independently reach at least half of it.

| Support | Classes | Floors |
| --- | ---: | --- |
| two or more functions agree | **192** | 9 – 61,305 |
| a lone outlier among several | 112 | 8 – 48,405 |
| one function speaks at all | 36 | 8 – 41,411 |
| only the type information | 56 | 8 |

> Raised twice by fixes in
> [the receiver note](dmc3-receiver-analysis-2026-09-16.md) — the interior-address
> rule and then the REX.B fix: 177 → 189 → **192** corroborated, 124 → 114 →
> **112** lone outliers.

The largest floors are the well-supported ones, not the fragile ones:

| Class | Floor | Deepest base | Agreeing |
| --- | ---: | --- | ---: |
| `CEm035` | 61,305 | `IComAction` | 11 of 27 |
| `CEm025` | 59,865 | `IComAction` | 5 of 22 |
| `CUIDStatusFileSec` | 53,542 | — | 2 of 2 |
| `CPlNewVergil` | 47,273 | `IPlayer` | 6 of 14 |
| `CPlDante` | 46,625 | `IPlayer` | 5 of 13 |

## A check nobody asked for

The four playable-character classes all derive from `IPlayer`:

```text
CPlNewVergil  >= 47,273
CPlDante      >= 46,625
CPlVergil     >= 45,865
CPlLady       >= 32,721
```

Four separately compiled classes of one family landing in one size band, with
nothing in the measurement making them agree.

## Three reasons it is a floor

1. Only offsets some method actually touches are counted. A class with fields
   nothing reads is understated.
2. A function whose code sits in several ranges is analysed with **each range's
   start knowing nothing**, because a range can be entered by a branch the
   analysis cannot see. A field touched only in such a range does not count.
3. `lea` is excluded — computing an address is not touching what is at it, and a
   one-past-the-end pointer is an ordinary thing to compute.

## The thing that looked wrong and was not

The code floor exceeds the base floor in **200 of 200** classes where both
speak, and is never lower. That looked like a systematic error and is the
opposite: MSVC lays base subobjects *before* members, so the deepest member sits
past the deepest base whenever a class has any member at all. The ordering is
what the layout rule requires, and the four exceptions are classes whose methods
only ever read the vtable pointer at offset 0 — flooring them at 1 against their
own 8. There are 14 such classes once constructors are counted, 4 before.

Chasing it was still worth it. It sent me through the register analysis looking
for a missing invalidation, and the invalidation is there and correct: every
unmodelled register write clears the fact, a call clears the volatile set, and
each range's start begins empty. Reason 2 above came out of that read.

## Querying it

```sql
SELECT class_name, floor_bytes, deepest_base, functions_reaching_half, support
FROM v_exe_class_size WHERE support = 'CORROBORATED' LIMIT 20;
```

## Open work

- a floor is not a size. An **upper** bound needs either an allocation whose
  constant size reaches a constructor, or two objects adjacent in data — neither
  measured yet;
- the 121 lone-outlier floors are the ones to attack first: one more agreeing
  function moves each into the corroborated set, and the receiver analysis
  reaching beyond `this` would supply them;
- nothing here says what is *at* any offset, and nothing here should be read as
  saying so.
