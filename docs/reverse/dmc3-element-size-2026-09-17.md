# DMC3 HD: reading the anomalies I had published as an error bar (2026-09-17)

Two rounds running, an impossible category in a census turned out to be a real
bug. So I went back to the anomalies I had shipped *unread*, labelled "the error
bar on this inference".

**Evidence:** [`dmc3-hdc-indexed-access.evidence.json`](../../evidence/executable/dmc3-hdc-indexed-access.evidence.json)

## What I had published

> Bases read at more than one element size, which is a contradiction: at least
> one of the readings is wrong. **Reported rather than resolved, since nothing
> here says which.**

That was honest and it was wrong. The structure does say which.

## The signature

An index multiplier is recorded only off a definite `lea a+a*k`, `imul` or
`shl`. It can therefore be **missed but never invented**. An element size equal
to the raw SIB scale is one read straight off the encoding with no multiplier
seen.

So if a base shows sizes 8 and 24, and the 8 *is* that base's own SIB scale, the
two readings are one array whose multiplier one read missed — and 24 is the
element size.

Across every such base in the image:

| | |
| --- | ---: |
| bases read at more than one element size | **11** |
| the smaller divides the larger | **11** |
| the smaller equals its own SIB scale | **11** |
| both, so it is a missed multiplier | **11** |

Eleven of eleven, no exceptions. Every one: `{8,48}`, `{4,16}`, `{8,40}`,
`{8,16}`, `{8,24}`, `{8,16}`, `{8,80}`, `{8,48}`, `{1,8}`, `{8,16}`, `{4,8}`.

## What resolving them bought

| | Before | After |
| --- | ---: | ---: |
| accesses whose displacement falls outside its element | **28** | **14** |
| consistent array accesses | 541 | **555** |

Exactly 14 moved from one column to the other. **Half the error bar was this one
cause.**

`0x580D20` is the clearest case. It now reads as a 24-byte record:

```text
element 24, fields at 0, 4, 8, 12, 16, 20
```

Before, it was an 8-byte reading plus a handful of accesses reported as landing
outside their element — because a displacement of 16 or 20 is outside an 8-byte
element and inside a 24-byte one.

## What is *not* resolved

A base whose sizes do **not** divide, or whose smaller reading carries a
multiplier of its own, is a real conflict. Nothing says which reading is right,
so such a base is now left out of the layout entirely rather than given a size it
may not have.

```text
arrays_with_a_real_size_conflict   0
accesses_on_a_conflicted_base      0
```

There are none in this image. A test covers the case anyway — `{24, 12}`, where
neither divides the other and both carry a multiplier — and removing the
raw-scale requirement from the rule makes it fail.

## Two entries that remain

`0x573310` and `0x573328` still appear twice in the finished table, at 8 and 48.
Both are **image-base derived**: their base is only an upper bound, and they are
grouped per function and index register, so one base can legitimately appear
twice. The same signature is visible there, but the held-base reasoning does not
transfer, because merging two such groups would mean merging two bases that are
each only a bound. Left alone and said so.

## Method note

Three findings in a row have come from the same move: take a number published as
a caveat and read it. The REX.B bug came from 167 sites in a category that cannot
exist. This came from 28 accesses labelled an error bar. Neither needed new
machinery — only refusing to leave an anomaly at the level of "reported rather
than resolved".

## Open work

- `string_scan_accesses` is 361 — negative displacements read as an inlined
  character scan. That reading has never been checked against the instructions;
- `image_base_groups_spanning_elements` is 102, published as "the register was
  reused for another array". Also unread;
- 14 accesses still fall outside their element. Whatever those are, they are not
  this.
