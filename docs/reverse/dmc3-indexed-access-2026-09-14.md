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

## Closing the gap above stride 8

A SIB scale encodes only 1, 2, 4 and 8. Any other element size is reached by
multiplying the **index** first:

```asm
lea rax,[rcx+rcx*2]      ; index * 3
mov edx,[rbx+rax*8]      ; base + index * 24
```

The image holds **933** of the `lea reg,[a+a*k]` form alone, plus `imul` by a
constant and shifts. Carrying that multiplier through to the read makes those
sizes visible, and 40 reads then show element sizes of **6, 10, 12, 20, 24, 40
and 72 bytes** — none of which a scale field could have expressed.

So the weak half of the finding is now measured rather than assumed: the
absence of indexed access to a name run holds for **every** stride the walk can
name, not only for 8.

## What it did find: 155 indexed arrays

The same machinery, pointed at everything rather than at the name runs, reads
data layout straight out of the instruction stream.

On a held base the displacement is a **field offset inside the element**, so it
has to land inside it. That is a free consistency check, and it splits the 554
such reads three ways:

| | count | meaning |
| --- | --- | --- |
| offset inside the element | **267** | an array walk with a readable field |
| negative offset | 255 | the inlined character scan, `[base + i*1 - 1]` |
| offset at or past the element | **16** | the element size or base is wrong |

### The failures named the missing instruction

That third row started at **32**, and the sites in it were not noise. The
function at `0x274730`:

```asm
lea  r14,[rip+0x3229d1]     # 0x597150      <- base
lea  rbx,[rbx+rbx*4]        ; index * 5
add  rbx,rbx                ; index * 10        <- not modelled
mov  eax,[r14+rbx*8]        ; base + index * 80
movzx eax,BYTE PTR [r14+rbx*8+0x4]
mov  eax,[r14+rbx*8+0x8]
 ... up to +0x1c
```

**Eighty bytes.** The doubling was the step the multiplier tracking did not
model, so the element read as 40 and every offset from +8 to +28 fell outside
it. Adding the self-add form — and the shift form alongside it — took
inconsistent reads from 32 to **16** and consistent walks from 267 to **304**.

The check earned its keep twice: it flagged the gap, and the gap named the
instruction to add. `0x597150` now reads as an 80-byte element with eight
4-byte fields, which is what the instructions say it is.

The remaining 16 — **2.8%** — is the error bar, and worth stating rather than
hiding. Alongside it, **5 of 168 bases** are read at two different element
sizes. A base cannot have two, so one reading of each is wrong and nothing in
the encodings says which; the count is reported rather than a winner picked.

What survives is **178 arrays**, of which **28** are read at more than one
offset and so carry a partial element layout rather than a single observation:

| Array | Element | Fields observed | Sites |
| --- | --- | --- | --- |
| `0x597150` | 80 bytes | +0 +4 +8 +12 +16 +20 +24 +28 | 8 |
| `0x5CEC30` | 4 bytes | +0 +1 +2 +3 | 25, across 5 functions |
| `0x582450` | 16 bytes | +0 +4 +8 +12 | 4 |
| `0x596DE0` | 48 bytes | +16 +20 +24 | 6 |
| `0x5D08A0` | 20 bytes | +4 +8 +12 +16 | 4 |
| `0x5DE5B0` | 24 bytes | +0 +8 +16 | 6 |
| `0x570600` | 12 bytes | +0 +4 +8 | 3 |
| `0xCA13E0` | 10 bytes | +0 +8 | 6 |
| `0x580D20` | 24 bytes | +4 +8 | 4 |

These are runtime state arrays in writable data — which is also why none of
them is a name run. Layout only: where the array is, how wide its element is,
and which offsets inside that element the code reads. No contents.

## The image-base reads, and a layout not invented

778 reads reach their array against the image base, where the array's start is
folded into the displacement. One read cannot separate base from field.

Grouping reads whose addresses fall within one element of each other looked
like the way in, and it yields **18 arrays**. Grouping instead by what is
actually shared — one function, one index register, one element size — shows
why that is unsound:

| groups with more than one read | 60 |
| --- | --- |
| **spanning more than one element** | **53** |
| one array's fields | 7 |

53 of 60 means the index register was reused for a *different* array. Proximity
would have merged them and invented a layout for each. The false-merge rate is
the result here; the 7 surviving layouts are recorded with `base_measured=false`,
because the array may begin before the lowest address observed and only the
offsets between reads are measured.

Two of the seven reproduce a base and element size a register was separately
seen holding — `0x4E9020`, an 8-byte element with fields at +0 and +4. Two
independent routes to the same array, which is corroboration rather than a
second array, so they merge into the measured entry.

## What this changes

Coverage by constant index still means something — it means code reaches those
bytes, and which bytes. It does not mean the run is indexed, and the earlier
reading that treated the two as one should be read with this alongside it.

## Open work

- **the 16 remaining inconsistent sites, and the 5 conflicting bases.** Each is
  individually checkable against the instructions, exactly as the 16 that were
  closed here were;
- **the ~630 single image-base reads.** Each measures that an array of a known
  element size covers a known address, and no more. Separating base from field
  needs a second read through the same index register, which 60 groups have and
  only 7 survive;
- **the receiver of a virtual call**, which needs the same machinery pointed at
  a vtable pointer rather than a table base;
- **the tables no direct reference reaches** — now a sharper question, since a
  base arriving through a load is exactly what this walk gives up on.
