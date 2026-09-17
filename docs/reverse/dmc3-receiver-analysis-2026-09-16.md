# DMC3 HD: where a dispatch receiver comes from (2026-09-16)

11,434 virtual call sites. The receiver analysis could name 1,135 of them. This
asks why, and the answer turned out to be one instruction.

**Evidence:** [`dmc3-hdc-receiver-analysis.evidence.json`](../../evidence/executable/dmc3-hdc-receiver-analysis.evidence.json)

## First, measure where they are lost

For each site, what instruction produced the register it reads the vtable
through?

| Producer | Sites | |
| --- | ---: | ---: |
| `mov reg, [reg+d]` — a field | **9,878** | 86.4% |
| something else | 1,399 | 12.2% |
| `mov reg, [rsp/rbp+d]` — a stack slot | 116 | 1.0% |
| `mov reg, [rip+d]` — a global | 24 | 0.2% |

That kills the obvious suspicion. MSVC spills `rcx`–`r9` to home space in many
prologues, and I expected the analysis to be losing arguments there. **1% of
sites.** It is not the spill.

Walking one step further back, **2,529** of those field-load bases were produced
by a `lea` — the address of something *inside* another object.

## The one instruction

```asm
lea rax, [rcx+0x10]     ; the address of an embedded member
mov rdx, [rax]          ; its vtable
call qword ptr [rdx]    ; dispatch
```

The analysis dropped `this` at the `lea`, because taking an address was not
modelled as carrying the value. **Taking the address of something inside an
object does not stop it being the object.**

Carrying it, with the offset accumulated:

| | Before | After |
| --- | ---: | ---: |
| dispatch sites on `this` | 1,135 | **1,855** |
| sites inside a class-bound function | 258 | **767** |
| **fully resolved virtual calls** | 47 | **105** |
| receivers that are a member, not the object | 17 | **75** |
| vtable stores into the first argument | 265 | **274** |
| class field layout entries | 44 | **47** |
| size floors corroborated | 177 | **189** |
| size floors on a lone outlier | 124 | **114** |

Every figure moved the way the change predicts, and the lone-outlier count
falling while corroboration rises is what more evidence per class should do.

## The check that had to come with it

The accumulated offset has to reach the **store** consumers too. A vtable
written through an interior address would otherwise read as a store at offset
zero — which is exactly what names a function a constructor of that vtable's
class. Getting that wrong would invent constructors.

A test pins it: a vtable stored through `lea rax,[rcx+0x20]` is a store at 32,
the function is not a constructor, and removing the accumulation makes the test
fail.

The older check holds too. Stores of a taken address into the first argument
were published at **265 of 270** landing on a known vtable — the 98% that says
the following is sound rather than accidental. After the change it is **274 of
279: 98.2%** against 98.1% before. The new stores are the same kind of thing as
the old ones.

## A negative result worth keeping

The convention passes the first four arguments in `rcx`, `rdx`, `r8`, `r9`.
Naming all four, so a dispatch on something a method was *handed* can be told
apart from one on `this`, buys:

```text
12 sites out of 11,434
```

The engine dispatches on the object a method belongs to, or on something reached
through it, and **hardly ever on a parameter**. The classification stays — it is
free, and a site on argument two is a different thing from an unnamed one — but
it is recorded as a measurement that did not pay.

## Then measure again, properly

The census above used a backward scan over the instructions. That is a guess at
what the analysis knows, not the analysis's own answer. Recording the verdict
**per site** put 167 sites in a category that cannot exist:

> the register the vtable is read through held a **constant**

A vtable pointer is never a small constant. Chasing those 167 found the cause two
layers down, in the decoder.

The x86-64 forms that carry their register **inside the opcode** — `mov reg, imm`
at `B8`–`BF`, `push`, `pop`, `xchg` — have no ModRM field to extend, so **REX.B
is the only thing that distinguishes `rax` from `r8`**. Nothing surfaced it, and
the register tracking read the low three bits alone.

```asm
mov rax, [rcx]          ; the vtable
mov r8d, 0x27c          ; 41 b8 ... — read as a write to RAX
call qword ptr [rax+60] ; the vtable fact is gone
```

`mov r8d, imm` between a vtable load and the dispatch through it is a shape the
image is full of. It was destroying the fact and inventing a constant. The
invalidation block had the same blind spot and conservatively forgot *both*
halves of the register file, which is what did most of the damage.

After surfacing REX.B the impossible category is **empty** — that is the check on
the fix — and 21 measured figures move, all towards more facts recovered:

| | Before | After |
| --- | ---: | ---: |
| dispatch sites on `this` | 1,855 | **1,880** |
| in a class-bound function | 767 | **785** |
| pointer stores into `this` | 187 | **207** |
| indexed accesses | 1,966 | **2,023** |
| global-block call sites | 3,582 | **3,753** |
| size floors corroborated | 189 | **192** |
| **constant-argument callees** | 179 | **130** |

That last one is a **correction, not a gain**. `41 b9 imm32` is `mov r9d, imm`,
and it was being recorded as putting a constant in `rcx` — inventing a constant
first argument for the next call. The published figure of "1,311 calls over 179
functions" becomes **1,319 over 130**: the call count barely moves and the
function count was overstated by **27%**.

The separate reading of the function at `0x2E7CA0` is unaffected, because it was
read off its own compare chain rather than inferred from this census. That is the
whole reason the compare chain was read.

## Where the sites actually stand

| The analysis's verdict | Sites | |
| --- | ---: | ---: |
| nothing known | **9,535** | 83.4% |
| through an argument | 1,892 | 16.5% |
| a taken address | 7 | 0.1% |
| a constant | **0** | — |

Of those read through an argument: **1,085** one load deep (the object's own
vtable — the resolvable case) and **782** two loads deep (a pointer member's
pointee, which the enclosing class's layout cannot describe).

Naming a receiver is not resolving a call: 1,880 on `this` → 785 in a
class-bound function → **105 resolved** across 32 classes.

## Open work

- 105 of 11,434 is still under 1%, and the remaining mass is **9,535 sites the
  analysis knows nothing about** — not the `copied register` and `field of a
  field` buckets the backward scan suggested, which were an artefact of that
  scan;
- an interprocedural step would do more than any local one: a site on argument
  two is unresolved here but a caller knows what it passes;
- nothing in this note reads a field's contents, and the resolution it gains is
  structural throughout.
