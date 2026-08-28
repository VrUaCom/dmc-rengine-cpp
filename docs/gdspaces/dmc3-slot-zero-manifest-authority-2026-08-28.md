# Does the game read slot 0? No — and it is a proof, not a search — 2026-08-28

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

`st001.pac` slot 0 is exactly:

```
st001.ptx\r\nst001.scm\r\nst001.sch\r\n
```

and slots 1, 2, 3 are a texture pack, an `SCM` and a `HITS`. The open question
has been whether those three lines are a **runtime naming authority** that
binds those slots (model A) or **pack-time metadata the game never consults**
(model B).

**It is B.** And the shape of the answer matters: the runtime does not skip
slot 0. It reaches it, dispatches it, finds it is of no known type, and
returns. The payload is consumed and discarded.

## 1. Slot 0 is reached

The PAC walk initialises its index register to 2 at `0x1401B9316`, and the
load is `[base + index*4]`. The first iteration therefore reads byte offset 8 —
the first offset-table entry, which is slot 0. In `st001.pac` that entry is
`0x30`, non-zero, so the walk forms a pointer and calls the dispatcher.

There is no special case for index 0 anywhere in the loop.

## 2. The dispatcher recognizes nothing in it

Every branch of the dispatcher at `0x1401B9FA0` turns on the payload's first
byte, tested against `M`, `E`, `S`, `P`. The manifest begins `st001…`, so the
first byte is `s`, `0x73`:

```
cmp p[0], 0x4D ('M')  -> 0x73 != -> skip MOD
cmp p[0], 0x45 ('E')  -> 0x73 != -> skip EFM
cmp p[0], 0x53 ('S')  -> 0x73 != -> skip SCM / SHW
cmp p[0], 0x45 ('E')  -> 0x73 != -> skip EFW / EFE
cmp p[0], 0x50 ('P')  -> 0x73 != -> skip the PNST walk
```

Every branch is skipped. The dispatcher returns having done nothing.

## 3. Nothing else walks a container

This is what turns the result from "no consumer found" into a proof.

A sweep of every **container-relative** slot walk — an indexed dword load from
a base, added back to that base to form a pointer — finds **46** sites
image-wide. Most are static tables, vertex arrays and jump tables. Only two are
preceded by a comparison that establishes the base is a relative-slot
container, and the tag census already showed there is exactly **one** such
comparison per magic in the whole image:

- `PAC` at `0x1401B92FE` — the pool finalizer
- `PNST` at `0x1401BA029` — the dispatcher

So exactly two routines in the executable ever treat a payload as a container.
Neither special-cases slot 0, and the handlers they call receive a *child*
pointer and never the container, so a handler cannot reach back to slot 0
either.

Two ranges were checked specifically because they are where a naming path would
most plausibly live, and both are clear of any container walk:

| range | what it is | walk sites |
|---|---|---|
| `0x14002F000`–`0x140030000` | the namespace resolver | none |
| `0x140338000`–`0x140339000` | semantic materialization | none |

## 4. Even read, the names could not have been used

Two of the three extensions **do not exist anywhere in the image**, in any
case, and none can be constructed:

| searched | occurrences |
|---|---|
| `.sch` `.SCH` `.Sch` | **0** |
| `.scm` as a string | **0** |
| `st001.ptx` `st001.scm` `st001.sch` | **0** |
| `%s.ptx` `%s.scm` `%s.sch` `st%` `%03d` | **0** |

`SCM` exists only as the three-byte magic the dispatcher compares — never as an
extension. So a `.scm` line says nothing the payload's own tag does not already
say, and a `.sch` line matches nothing at all: it has no entry in either
registry, and the animation registry refuses an unmatched name outright.

`.ptx` is the one line that *does* resolve — it has a real entry in the first
registry — which is exactly why the manifest looked like a runtime authority.
One line out of three resolving is not a naming system.

Nor is `st001.ptx` among the 4,039 names in the archive catalogue, which
carries the four files a stage actually is: `st001.pac`, `st001cfg.pac`,
`st001_effect.pac`, `snd_r001.pac`.

## 5. What this changes, and what it does not

**Changes:** the manifest's authority. It is not what the game uses to bind
slots to names, and no product should present it as though it were.

**Does not change:** its accuracy. The three lines are a true statement about
the file, made by whoever built it, and this project still reads and shows
them — attributed as the container's own text, corroborated where the payload's
independently read type agrees. All three do corroborate, once `.sch` and
`HITS` are known to be one record under two names.

## 6. The useful half of a negative result

The question was not unanswerable. It was aimed at the wrong file.

The runtime's own naming authority for a container **is** recovered, and it is
`.lst`: the representation selector at `0x1401B79E0` prefers the packed `.pac`
and rewrites the extension to lowercase `lst` when it is absent, through the
helper at `0x1401B9390`. This pass independently corroborated that from the
other direction — three sites write `lst\0` into a path buffer at field
`+0x6618` (`0x1401B7A7B`, `0x1401B865E`, `0x1401B93EC`) against one that writes
`pac\0` (`0x1401B81F9`) — and found the parser's literal pool at `0x1404DC4CC`
holding a separator, `dummy` and `lst` side by side.

So the ordering to keep is:

```
runtime semantic name   ≠   .lst loose member name
                        ≠   slot 0 build manifest
                        ≠   physical PAC slot
                        ≠   detected binary format
```

Five distinct things, and the middle one is the only container-side name the
game itself reads.

## 7. Where it lives

- `include/dmc_rengine/profiles/dmc3/slot_zero_manifest_contract.hpp`
- `tests/slot_zero_manifest_authority_tests.cpp`
- `data/reverse/dmc3-type-identification-windows.v1.json` — three new windows
- the `.lst` recovery it points at:
  [`dmc3-loose-container-list.md`](dmc3-loose-container-list.md)
