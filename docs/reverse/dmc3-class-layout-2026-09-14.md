# DMC3 HD class layout (2026-09-14)

What a class *contains*, at which offset, with types — read out of what its
constructor writes.

Follows [virtual dispatch](dmc3-virtual-dispatch-2026-09-14.md), which found
that most dispatches go through fields rather than through `this`, and left the
types of those fields as the way in.

**Evidence:** [`dmc3-hdc-class-layout.evidence.json`](../../evidence/executable/dmc3-hdc-class-layout.evidence.json)

## The idea

A constructor writes its class's vtable into the object at **offset zero**.
That alone names the function — no symbol needed:

```asm
lea rax,[rip+vtableOfCScene]
mov [rcx],rax                  ; offset 0  -> this function constructs a CScene
```

And everything else it writes into the object at a **non-zero** offset names
what sits there:

```asm
lea rax,[rip+vtableOfCGameLoader]
mov [rcx+0xa80],rax            ; offset 2688 -> a CGameLoader lives here
```

The register analysis already knows which register holds the first argument and
which holds an address a `lea` produced, so both of these fall out of it.

## What came out

| | count |
| --- | --- |
| stores of a taken address into the first argument | 270 |
| …where the address is a known vtable | **265** |
| functions storing one at offset zero — constructors and destructors | **146** |
| distinct typed fields recovered | **44** |
| …embedded members (a different class) | 15 |
| …offsets the RTTI independently agrees with | **29** |

**265 of 270 is a check on the method, not a result of it.** An address written
into an object at a fixed offset, taken by `lea`, in a function whose first
argument that object is, turns out to be a vtable 98% of the time. If the
register following were loose, that number would not hold.

## The layouts

`CScene` — nine typed members:

| Offset | Type |
| --- | --- |
| 64 | `CLightMgr` |
| 88 | `CGameW` |
| 112 | `CChain` |
| 592 | `CEventMission` |
| 2688 | `CGameLoader` |
| 3312 | `CList<CNonPlayer>` |
| 4256 | `CChain` |
| 7552 | `CLightStatic` |
| 22832 | `CCameraMiniDemo` |

`CEm021`, `CEm029Cart`, `CNonPlayer` and others carry **base subobjects** at
offsets 96, 208, 272, 384 and 592 — the shape of multiple inheritance, and each
of those offsets is *also* recorded by the RTTI as that vtable's subobject
offset.

That is the validation: where the vtable belongs to the constructor's own class,
two independent sources — an instruction the compiler emitted and a structure
the compiler emitted — say the same offset. They agree on 29 of the 44 entries.
The other 15 are embedded members, where the RTTI has nothing to say because the
member is not a base.

> **On content.** Offsets and type names only. No field values are read.

## Open work

- **fields whose type has no vtable.** A member with no virtual functions leaves
  no vtable store, so this sees only the polymorphic parts of an object. The
  gaps between recovered offsets are where everything else lives;
- **dispatch on `this->member`**, now that the member's type is known at an
  offset: a load of `[this + k]` followed by a dispatch resolves against the
  class at `k`. This is the route to the bulk of the 10,274 dispatch sites;
- **constructors that do not store a vtable** — a non-polymorphic class has
  none, so its constructors stay unnamed by this route.
