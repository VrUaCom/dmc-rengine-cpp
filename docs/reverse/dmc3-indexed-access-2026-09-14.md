# DMC3 HD: which tables the code actually indexes (2026-09-14)

The question left open three times over: is a recovered name run a table the
game indexes, or a run of literals the linker happened to pack evenly?

**Evidence:** [`dmc3-hdc-indexed-access.evidence.json`](../../evidence/executable/dmc3-hdc-indexed-access.evidence.json)

## Why it could not be answered before

A constant index folded into a displacement and a direct load of the literal at
that offset are **the same instruction**. `lea rdx,[rip+0x22e0cf]` says nothing
about whether `0x22e0cf` is element one of a table or a string in its own right.

What does distinguish them is holding a base and scaling an index. That needs
the base, index and scale of a memory operand — which the length decoder parsed
and threw away.

## Surfacing the addressing fields

No length logic changed; three encodings just needed saying out loud:

| Encoding | Meaning |
| --- | --- |
| REX.R / REX.X / REX.B | extend the ModRM reg, SIB index and SIB base to 0–15 |
| SIB index 4, no REX.X | **no index register** — objdump prints this as `riz` |
| SIB index 4, REX.X set | r12, an ordinary index register |
| SIB base 5, mod 0 | **no base register** — a bare disp32 |

Then every indexed memory operand in the image was compared against objdump:

```text
compared 16503, agree 16503, disagree 0, skipped 56
```

The 56 skipped are the `riz` form — which is the no-index encoding the decoder
reports as having no index — and 32-bit addressing.

## Following a base, without pretending to do dataflow

The walk remembers what a RIP-relative `lea` put in a register, and gives it up
on **any** instruction naming that register in its ModRM fields, on **every**
call, and after **64** instructions. Reads invalidate as readily as writes: the
decoder models no mnemonics, so which instructions write which register is not
knowable here, and the honest response is to claim less.

That is deliberately lossy. What makes the residue worth having is the
validation on the other side — a base that survives all of it and lands on a
recovered table *at that table's own element size* is not a coincidence.

## What it found: the image base in a register

**1,303** indexed reads have a base the walk can name. **749** of them hold RVA
zero — the image base:

```asm
1400316a0:  lea   r13,[rip+0xfffffffffffce959]   # 0x140000000   <- __ImageBase
    ...
1400317a0:  movss xmm1,DWORD PTR [r13+rbx*8+0x5e1668]
```

A table of 8-byte elements at RVA `0x5E1668`, reached by displacement alone.
This is the construct the switch-table recovery already depended on; it is now
readable for data tables generally rather than inferred from a candidate list.

## What it found about the name runs: nothing

Of those 1,303 reads, the number reaching a recovered run or record at its own
element size is **zero**.

Four have a base register holding a run's base exactly. All four are this:

```asm
or   rax,-1
inc  rax
cmp  BYTE PTR [rbx+rax*1], r8b     ; base + index*1 - 1
jne  ...
```

An inlined character scan over the **first** name — which is to say, over a
literal. Every other access into a run's bytes loads one element's address
outright, exactly as the eight `lea` instructions at `0x506C38` did.

### So the runs are layout, not access

The fixed stride is the **packer's alignment**, not an array's element size. The
runs are a true fact about how the linker laid these literals out, and not a
fact about how the game reads them.

### How strong is that

- **Strong at stride 8.** A scale of 8 is then the natural encoding, 211 such
  reads exist elsewhere in the image, and none of them names a run.
- **Weaker above 8.** A SIB scale is only 1, 2, 4 or 8, so a stride of 16 or 24
  is reached by computing the element address first — `lea rdx,[rcx+rcx*2]` and
  the like. This walk follows a base into a register; it does not follow
  arithmetic on it. Absence there is consistent with indexing it cannot see.
- **Recall is unmeasured.** objdump counts 16,559 indexed operands in `.text`;
  the walk names a base for 1,303. The rest take their base from loads,
  parameters and arithmetic. The tracking is a floor, not a census.

## What this changes

Coverage by constant index still means something — it means code reaches those
bytes, and which bytes. It does not mean the run is indexed, and the earlier
reading that treated the two as one should be read with this alongside it.

## Open work

- **follow arithmetic on a held base**, which would settle the strides above 8;
- **the receiver of a virtual call**, which needs the same machinery pointed at
  a vtable pointer rather than a table base;
- **the tables no direct reference reaches** — now a sharper question, since a
  base arriving through a load is exactly what this walk gives up on.
