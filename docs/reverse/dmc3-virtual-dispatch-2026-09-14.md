# DMC3 HD virtual dispatch (2026-09-14)

Who does a virtual call call? The honest answer for most of them is *not
knowable from the file* — and the interesting part is which ones are.

**Evidence:** [`dmc3-hdc-virtual-dispatch.evidence.json`](../../evidence/executable/dmc3-hdc-virtual-dispatch.evidence.json)

## A census first

Recording each call through a register base, rather than only its
displacement, gives **10,274 dispatch sites** where 3,410 distinct
displacements were known before.

## The receiver is almost never in the file

The one shape that would resolve statically is a global object whose vtable
pointer is baked into the image:

```asm
lea  rcx,[rip+globalObject]
mov  rax,[rcx]                 ; vtable pointer, readable from .data
call [rax+0x48]
```

It occurs **zero** times. Receivers arrive as arguments, or through pointers
loaded from writable data whose values are established at run time. That is a
property of the programme, not a shortfall of the walk: whole-image
devirtualisation by reading the file is not available, and saying so is more
useful than a number built on guesses.

## The receiver that *is* knowable: `this`

One receiver needs no analysis of what a pointer holds. The Microsoft x64
convention puts the first argument in **rcx**, so in a method that is `this`:

```asm
mov  rbx,rcx                   ; the usual copy into a saved register
 ...
mov  rax,[rbx]                 ; this->vtable
call [rax+0xd0]                ; slot 26
```

The enclosing method's **own binding** says which vtable that is, and the
displacement says which slot — so the target is *read out of the vtable*, not
inferred.

| | count |
| --- | --- |
| dispatch sites | 10,274 |
| dispatched on `this`, or on something inside it | 1,134 |
| …the vtable read straight out of the object (one load) | 352 |
| …the vtable read through a pointer the object holds (two loads) | **782** |
| resolved to a class, slot and target | **47** |
| …of those, through a subobject rather than the object | 17 |

Those 19 resolve to a class, a slot and a target across 15 classes:

| Site | Caller | Class | Slot | Target |
| --- | --- | --- | --- | --- |
| `0x58CC3` | `0x58C10` | `CCameraRail` | 3 | `0x580A0` |
| `0x64FED` | `0x64F80` | `CComEm000` | 37 | `0x61000` |
| `0x7F978` | `0x7F960` | `CComEm008` | 26 | `0x61130` |
| `0x99ECE` | `0x99DE0` | `CEm001` | 21 | `0x9CE60` |

A method bound into more than one vtable is inherited, and `this` then does not
say which one — those are counted and left unresolved rather than attributed to
the first class that fits.

### Why only 19 — and what happened when that was fixed

The walk gave up its register state at every trace root, because a block reached
by a branch has a state depending on which predecessor ran, and picking one
would be unsound. So only dispatches in straight-line flow from a method's entry
kept the `this` fact.

The register analysis now runs as **its own pass** over the instructions the
walk decoded, with a state per instruction and a **meet at every join** — a fact
survives only when every path into the point agrees on it. The walk itself is
untouched, and the code graph comes out byte-identical, which is the check that
says so.

| | single pass | block dataflow |
| --- | --- | --- |
| indexed reads with a nameable base | 1,524 | **1,963** |
| consistent array walks | 368 | **531** |
| recovered arrays | 190 | **278** |
| dispatches on `this` | 105 | **175** |
| resolved | 19 | **30** |

**And the earlier reading was wrong about the ceiling.** Dataflow lifted the
figure by two thirds; the rest is not an analysis problem. Here is a method of
`DMC3::FullMotionVideoManager` at `0x2A9A0`:

```asm
mov rbx,rcx              ; this
 ...
mov rax,[rbx+0x8]        ; a container held in a field of this
mov rcx,[rsi+rax*1]      ; an element pointer out of that container
mov rax,[rcx]            ; the element's vtable
call [rax+0x20]          ; slot 4
```

The receiver is not `this` — it is an element of a container whose contents are
built at run time. Most dispatch sites are that shape, so the 2,487
displacements in single-vtable methods were never reachable by following `this`,
and no amount of dataflow gets there.

## Calls on a subobject

A load through the first argument at a **non-zero** offset is the vtable
pointer of whatever sits there — a subobject's vtable pointer is at its own
offset zero, so `[this + k]` is the vtable of the thing at `k`.

```asm
mov rbx,rcx                 ; this
 ...
mov rax,[rbx+0x60]          ; the vtable of the subobject at +96
lea rcx,[rbx+0x60]          ; and its own `this`, adjusted
call [rax+0xe8]             ; slot 29
```

That `lea rcx,[rbx+0x60]` is the giveaway: the receiver gets an **adjusted
this**, which is exactly how a call on a base subobject is made.

Which vtable to read comes from the **RTTI**, which records where each of a
class's vtables sits. That matters more than it sounds: a derived class
inherits its layout from a base whose constructor did the storing, so the
classes making these calls — `CEm025`, `CEm002`, `CEm007` — are *not* the
classes whose constructors write the vtables. Waiting for a constructor store
would have resolved none of them.

And it must be that subobject's vtable, not the class's primary one: at the
same slot they hold different functions, and the subobject's is what runs.

## One load or two

A load out of the object at a non-zero offset is *not* always the vtable of
what sits there. It is, for an **embedded subobject** — its vtable pointer is
at its own offset zero. It is not, for a **pointer member**, where the value is
an address and the vtable is a second load away.

Counting how many loads deep the value is separates them:

```asm
mov rax,[rbx+0x60]     ; one load  -> the subobject's vtable
call [rax+0xe8]

mov rax,[rbx+0xe0]     ; one load  -> a pointer
mov rdx,[rax]          ; two loads -> the pointee's vtable
call [rdx+0x30]
```

**782 of the 1,134 are the second kind**, and 485 of those go through a single
offset — **+224** — a pointer at a fixed place in some widely shared base.

A pointer member's vtable belongs to the pointee, and the enclosing class's
layout describes the *pointer*, not the object. So these are counted and left
alone rather than resolved against the wrong class.

> This distinction was nearly lost. The field carrying the depth went into the
> wrong slot of an aggregate initialiser — it set the multiplier instead — and
> all 782 came out labelled as direct subobject loads. The jump from 352 to
> 1,134 with *zero* at depth two was what gave it away. The constructions now
> name their fields.

## What a pointer field holds comes from a factory

Typing those fields looked like a short step: follow what a call returned into
the store that puts it in the object, and the callee names the type.

**187 such stores exist. Not one callee is a constructor.** The commonest by
far — `0x2E7CA0`, 86 of them.

I first called that an allocator, from its size and its lack of any vtable
binding. **It is not.** Its call sites pass a small constant and it hands back
an object ready to file away:

```asm
mov  r9d,8
xor  r8d,r8d
lea  ecx,[r9-7]            ; the selector: 1
call 0x2e7ca0
mov  [rbx+0xa8],rax        ; straight into this+168 — no constructor between
test rax,rax
```

The function takes four arguments and, when its third is null, copies a 64-byte
block out of read-only data at `0x35D580` — a default transform — before going
on. It is a **factory keyed by a selector**.

So the field's type is whatever that factory returns for *that* selector, and
reaching it means reading the factory's own body rather than hunting for a
constructor that is not there. Tracking the constant selector at each call site
is the next step.

> Correcting the record: "allocator" was an inference from weak signals,
> published as if it were read. The call site is what settles it.

## The calling convention is evidence

The walk used to give up **every** register at a call, on the grounds that
which ones survive was a claim it had no right to make.

That was wrong. It is not a claim — the Microsoft x64 convention names rax,
rcx, rdx and r8–r11 volatile and the rest preserved, and compiler-generated
code follows it. Forgetting only the volatile ones is what lets a `this`
pointer copied into a saved register at entry still be `this` after the method
has called something.

It lifts recall everywhere the same register state is used:

| | before | after |
| --- | --- | --- |
| indexed reads with a nameable base | 1,357 | **1,524** |
| consistent array walks | 304 | **368** |
| recovered arrays | 178 | **190** |

A test pins both halves: a base in rcx is lost across a call, a base in rbx is
not.

## Open work

- **the 158 minus 30** — methods bound into several vtables, where the class is
  genuinely ambiguous from `this` alone and would need the call sites of the
  method itself to narrow;
- **stack slots.** The analysis tracks registers only, so a `this` spilled to
  the frame and reloaded is lost. Tracking frame offsets while the frame pointer
  is fixed would recover those;
- **the 782 through a pointer member**, which need the factory's own body read,
  keyed by the constant selector its call sites pass;
- **the 100 member dispatches at offsets nothing describes.** Their class is
  known and the offset is not a base subobject or a recovered member, so what
  sits there is a field whose type nothing in the file states. The constructor
  store route reaches only polymorphic members;
- **the 77 whose enclosing class is unknown** — a function neither bound into a
  vtable nor storing one, so `this` has no type;
- **receivers from containers**, which is the remaining bulk and needs what the
  container holds at run time.
