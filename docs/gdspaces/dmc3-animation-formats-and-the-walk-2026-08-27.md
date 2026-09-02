# All six animation kinds, and why the walk never reaches them — 2026-08-27

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

Two questions were asked together: find every animation format the executable
knows, and finish reversing how `PNST` unpacks. They turn out to be the same
question, and the answer to the first is inside the answer to the second.

## 1. The six kinds, read off the comparison chain

A sweep for standalone extension literals in `.rdata` finds exactly two
clusters that are not video or PE section names:

```
0x140507070  .ptx .PTX .Ptx   .clt .CLT .Clt   .c1d .C1D .c1D .C1d
0x1405071D0  .mot .MOT  .mcv .MCV  .cam .CAM  .hid .HID  .tsc .TSC
```

The second block is 80 bytes — ten entries of eight — and it is this project's
`animation-extension-literal-table` window. Read by its bytes alone it says
five formats.

The classifier at `0x1402E01A0` compares **twelve**. The other two are `.clt`
and `.CLT`, and it reaches into the *first* registry's block at `0x140507088`
and `0x140507090` rather than carrying its own copies. That is why measuring
this table by its contiguous bytes undercounts it.

Resolving every comparison against every type-code store gives the chain in
code order:

| order | literals | branch | stored code |
|---|---|---|---|
| 1 | `.mot` `.MOT` | `0x1402E0598` | **0** motion |
| 2 | `.mcv` `.MCV` | `0x1402E051B` | **1** curve |
| 3 | `.cam` `.CAM` | `0x1402E049B` | **2** camera |
| 4 | `.hid` `.HID` | `0x1402E041B` | **3** hide |
| 5 | `.clt` `.CLT` | `0x1402E039A` | **4** palette |
| 6 | `.tsc` `.TSC` | fallthrough | **5** tsc |

The codes this project already recorded are confirmed rather than corrected.
What is new is the literal accounting, the shared pair, and that the chain is
an ordered sequence of comparisons — the first match wins — rather than a table
walk that could be sorted.

### The comparison is `strstr`, and that is what makes `.mot1` work

**Corrected the same day.** This note first said the extension is matched
against the tail of the name, and this project implemented a tail match. It is
wrong. The call at `0x1402E01DC` goes through the import slot at `0x14034F3D0`,
which resolves to **`strstr`** in `VCRUNTIME140.dll`. The extension is searched
for **anywhere in the name**.

That is not a detail. It is why the game reads `pl000.mot1` through
`pl000.mot6` as motions while carrying no numbered literal for any of them — a
whole-image search finds no `.mot1`, `.mot2` or `.mot6` anywhere, because none
is needed. A tail matcher refuses exactly the names the game accepts.

`extension_matched_as_substring` had said so in the contract since the registry
was first recovered. Nothing asserted the implementation against the flag, so
the flag and the code disagreed silently. There is now a test that fails for a
tail matcher and passes for a substring one.

One consequence follows and is worth stating rather than rediscovering: a name
carrying two of these extensions is typed by whichever comes first in the
**chain**, not by whichever appears first in the name. `x.cam.mot` and
`x.mot.cam` are both motions.

And a second: the classifier must be asked the **path**, not the format derived
from it. `pl000.mot1` has the extension `mot1`, which is not one of the six;
only the whole name reaches `strstr`.

Two further details worth holding. The first registry's block carries a third,
capitalized variant `.Clt` that this classifier never compares, so `.Clt` is
not an animation here. And the registrar builds its key with `"%s/%s"` at
`0x140507068` — a resource is identified by a **group and a name**, not by a
name alone, which is what lets two containers hold the same member name.

## 2. What the walk actually does

Reading `0x1401B9FA0` end to end rather than in the parts already needed:

```
dispatch(p):
    if p == NULL: return
    if p[0..2] == "MOD": handler 0x1402FE3B0
    if p[0..2] == "EFM": handler 0x1402F7A90
    if p[0..2] == "SCM": handler 0x1403051B0
    if p[0..2] == "SHW": handler 0x1403204C0
    if p[0..2] == "EFW": return          <- recognized, not walked
    if p[0..2] == "EFE": return          <- likewise, and missed until now
    if p[0..3] != "PNST": return
    count = *(i32*)(p + 4);  if count <= 0: return
    for i in 0 .. count-1:
        off = *(u32*)(p + 8 + 4*i)
        dispatch(off ? p + off : NULL)
```

Three things this settles.

**`EFE` is a second recognized-and-skipped tag.** The compiler collapsed its
comparison into `cmp cl, cl` followed by `cmp [rbx+2], al`, testing the third
byte against the `E` that `al` still holds from the first byte's own
comparison. It reads like dead code and is not.

**The count is re-read from memory on every iteration.** A handler that
rewrites the count mid-walk moves the end of the walk. A reader that hoists it
into a register is not reproducing this loop.

**No size is ever computed.** Both walks form `container + stored_offset` and
pass that bare pointer; the handler finds its own end from its own contents. So
a container does not record how long a slot is, and this project's "up to the
next non-zero offset" is a product decision, not a recovered rule. That is now
`walk_computes_child_size = false` in the contract, and it is the honest reason
an unpacked extent can be wrong at the last slot.

## 3. The finding that answers the original question

**The dispatcher does not walk a nested `PAC`.**

The `P` branch requires all four of `P`, `N`, `S`, `T`. A payload beginning
`PAC\0` fails at the second byte and returns. Only the pool finalizer at
`0x1401B92E0` walks a PAC, and only the one held in a pool slot.

`st001.pac` slot 7 is a `PAC`, and the only motion in the entire corpus is
inside it. **The runtime's own dispatcher would never descend there.** This
project's expander does — which is a tool going further than the game, not a
shape the game reads.

So animation is not missing from the walk because the walk is incomplete. It is
missing because the walk was never the path to it: animation is reached by
name, through the second registry, and a relative-slot container stores no
names at all.

## 4. The pool finalizer, more completely

```
rsi = pool + 0x10 ; 0x16B slots of stride 0x48
    if slot.state != 2: next                 ; state at slot + 0x04
    payload = *(void**)(slot + 0x20)
    if payload[0..2] == "PAC": walk its slots, dispatching each
    else:                      dispatch(payload)
    if slot.finalizer:  finalizer(slot)      ; pointer at slot + 0x10
    slot.state = 3
```

A slot's payload does not have to be a container: when it is not a `PAC` the
finalizer dispatches it directly, so a slot can hold a bare `SCM`, `MOD` or
`PNST`. And the state machine is real — a slot is acted on only in state 2 and
is left in state 3, so this pass runs once per slot per load.

## 5. What this project can and cannot do with the six

| kind | code | named | read |
|---|---|---|---|
| `mot` | 0 | yes | **yes** — structure recovered from 69 tracks |
| `mcv` | 1 | yes | no corpus |
| `cam` | 2 | yes | no corpus |
| `hid` | 3 | yes | no corpus |
| `clt` | 4 | yes | no corpus |
| `tsc` | 5 | yes | no corpus |

`ResourceClassification` now carries `animation_type` and
`animation_structure_recovered`, so the distinction is a fact the code holds
rather than a note in a document that drifts.

The consequence is worth stating plainly, because it decides what would help:
**five of the six cannot be found inside a container at all.** The container
stores no name, and without one real payload of a kind there is no structure to
probe for. `mot` is the exception only because one file gave 69 tracks to check
a reading against.

One `.mcv`, one `.cam`, one `.hid`, one `.clt` or one `.tsc` — extracted by
name from a real volume — is what turns each row of that table. A numbered
motion (`.mot1` … `.mot6`) needs nothing: it is already read, because the
numbering never changed the kind.

## 7. Both registries share the comparison, and one had no matcher at all

The first registry — `.ptx`, `.clt`, `.c1d` — calls through the **same import
slot**, `0x14034F3D0`. One `strstr` over two literal tables. So both of its
properties carry: substring rather than suffix, and case enumerated rather than
folded.

`ResourceTypeContract` had recorded `extension_matched_as_substring` since it
was written and implemented **nothing**. There was no matcher to disagree with
the flag, which is precisely how the animation side got away with disagreeing
with its own. It now has one, and a test asserts the two together.

This also settles `at.ptx` in principle, if not yet in bytes: by the runtime's
own rule it is a texture pack, because its name contains `.ptx`. What this
project cannot do is *read* it. Those are different statements and the code now
makes both of them separately.

`.clt` is claimed by both registries with different codes — 5 in the first, 4
in the animation one. A type code means nothing without the registry that
issued it, and the test says so.

## 8. An extent runs to the next greater offset, and that is a decision

Since the walk stores no size, the extent of a slot is this project's reading.
The reading is "up to the next **greater offset in the file**", which is not
the same as "up to the next slot": an offset table need not ascend, and in a
sparse container it frequently does not start at slot 0.

The parser already sorted distinct populated offsets and bounded each entry by
the next greater one — the correct reading — but nothing tested a table whose
slots address the file out of order. There is now a container whose slot 0
points last, and it comes back with every extent right.

## 9. A volume family is numbered

`0x14036E930` holds `%sDMC3-%d.nbz`. A real archive is `dmc3-0.nbz` because the
game builds the name that way, which means a volume has siblings and a member
absent from one may sit in another.

## 6. Where it lives

- `include/dmc_rengine/profiles/dmc3/animation_type_contract.hpp` — the
  literal accounting, the shared pair, the key format, the ordered chain.
- `include/dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp` — `EFE`,
  the four dispatched tags and their handlers, the nested-PAC negative, the
  no-size negative, the re-read count, the pool slot state machine.
- `include/dmc_rengine/gdspaces/classifier.hpp` — `animation_type`,
  `animation_structure_recovered`.
- `tests/animation_formats_and_walk_tests.cpp`
- `data/reverse/dmc3-type-identification-windows.v1.json` — six new windows.
