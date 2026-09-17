# DMC3 HD: the last two unread caveats (2026-09-17)

Two more numbers I had published as interpretations and never checked against
the instructions. One holds. One does not.

**Evidence:** [`dmc3-hdc-indexed-access.evidence.json`](../../evidence/executable/dmc3-hdc-indexed-access.evidence.json)

## Holds: the negative displacements

> A negative displacement is the inlined character scan over a string, not an
> array walk.

Published without ever reading one. Reading all 361:

| | |
| --- | ---: |
| displacement exactly **−1** | 361 / 361 |
| element size **1** | 361 / 361 |
| scale **1** | 361 / 361 |
| opcode **`3A`** (`cmp r8, r/m8`) | 361 / 361 |
| distinct functions | 60 |

```asm
41 3a 44 08 ff    cmp r8b, byte [rax + rcx*1 - 1]
```

A byte-by-byte comparison indexed from one. The reading was right, and the
uniformity is far stronger than the claim, which said only that the displacement
was negative.

Worth saying plainly: **reading a caveat sometimes confirms it.** The method is
not only a way to find bugs.

## Does not hold: the spanning groups

> Most such groups span more than one element, **meaning the register was reused
> for a different array**.

The spans run from one element to **7.5 MB**. The largest cannot be one array —
the image is 6.3 MB. But:

| | |
| --- | ---: |
| groups spanning more than one element | 102 |
| every read a whole multiple of the element apart | **95** (93.1%) |
| spanning fewer than 64 elements | 70 (68.6%) |
| smallest span | **2 bytes — one element** |

A group of three reads two bytes apart is not a reused register. And reads at
exact multiples of the element are what *one* array read at a few constant
indices looks like.

**What is provable:** reads that do not all fall inside one section cannot be one
array, whatever else is true.

```text
reads in several sections    9    <- demonstrably a reused register
undecided                   93    <- nothing in the encoding separates them
```

**9 of 102**, not "most". The 93 are still dropped — that part was right and
stays — but as *undecided*, not as reuse. The two counts are now reported apart,
and tests pin both cases.

## Why the distinction matters

Dropping a group is conservative either way. Saying *why* is not. "The register
was reused" is a claim about what the code does; "nothing here decides it" is a
claim about what the file says. Only the second was supported.

## What this did not touch

The separate finding that the image base is held in a register and indexed
against — 1,303 indexed reads, 749 of them at RVA zero — is unaffected and
stands.

## Method, four rounds on

| Caveat read | Outcome |
| --- | --- |
| 167 sites reading through a constant | decoder bug (REX.B) |
| 28 accesses outside their element | resolvable, error bar halved |
| 361 negative displacements | **confirmed**, and sharpened |
| 102 spanning groups | reading wrong, 9 not 102 |

Three corrections and one confirmation, none needing new machinery. The
remaining caveats in this reverse are now all ones I have actually read.
