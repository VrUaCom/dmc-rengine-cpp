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
| dispatched on `this` | 175 |
| …inside a method the RTTI binds into a vtable | 158 |
| …bound into exactly one vtable, so the class is unambiguous | **30** |

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
- **receivers from fields.** A dispatch on `this->member` is one step further
  than `this`, and the member's type would come from what the constructor stores
  there. This is where the bulk of the sites are, and it is the only route to
  them that does not require running the game.
