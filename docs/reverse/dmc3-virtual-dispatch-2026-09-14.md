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
| dispatched on `this` | 105 |
| …inside a method the RTTI binds into a vtable | 98 |
| …bound into exactly one vtable, so the class is unambiguous | **19** |

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

### Why only 19

The walk gives up its register state at every trace root, because a block
reached by a branch has a state that depends on which predecessor ran, and
picking one would be unsound. So only dispatches reachable in straight-line
flow from a method's entry keep the `this` fact. Block-level dataflow with a
proper merge at join points is what would lift this, and it is the next thing
worth building.

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

- **block-level dataflow with merges**, which is what stands between 105 `this`
  dispatches and the thousands that are surely there;
- **the 98 minus 19** — methods bound into several vtables, where the class is
  genuinely ambiguous from `this` alone and would need the call sites of the
  method itself to narrow;
- **receivers from fields.** A dispatch on `this->member` is one step further
  than `this`, and the member's type would come from what the constructor
  stores there.
