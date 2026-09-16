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

## Open work

- 105 of 11,434 is still under 1%. The remaining mass is `field of a copied
  register` (30.7%) and `field of a field` (17.9%) — chains the analysis follows
  only two loads deep;
- an interprocedural step would do more than any local one: a site on argument
  two is unresolved here but a caller knows what it passes;
- nothing in this note reads a field's contents, and the resolution it gains is
  structural throughout.
