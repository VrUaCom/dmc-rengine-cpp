# DMC3 HD: what is actually in the vtables (2026-09-16)

264 classes derive from `CWork`. This reads its interface, and every other
base's, without naming a single method.

**Evidence:** [`dmc3-hdc-vtable-slots.evidence.json`](../../evidence/executable/dmc3-hdc-vtable-slots.evidence.json)

## One instruction is enough to sort a slot

A vtable slot is a code address. Decoding only the **first instruction** at that
address splits every slot in the image three ways:

| | Slots | Distinct targets |
| --- | ---: | ---: |
| reaches the `_purecall` import | **276** | 1 |
| starts with a return | **4,246** | **1** |
| anything else | 9,372 | 3,313 |
| **total** | **13,894** | |

The first case is not pattern-matching. RVA `0x346BF0` is six bytes —
`jmp [rip+0x87C2]` — reaching the import-address-table slot at `0x34F3B8`, and
the **import directory** names that slot `VCRUNTIME140.dll!_purecall`. The
linker wrote that name. The same encoding pointed at any other import would
mean nothing at all, which is what the test for it asserts.

The second case is blunter than expected. All 4,246 slots hold **one address**,
`0x24EA30`, whose entire body is three bytes: `C2 00 00`, a return. **Thirty-one
per cent of the image's polymorphic surface resolves to a function that does
nothing.**

One honest limit on both numbers: the linker folds identical functions, so
slots sharing a target need not have been written once. 3,313 is a *lower*
bound on distinct implementations, and 4,246 an *upper* bound on inert surface.

## Comparing a base against its inheritors

For each base and each slot of the base's own vtable, count the classes that
inherit it and what they put there. The trap is which table to read on the
derived side. `IActor` sits **96 bytes** into a `CActor`; a class's *primary*
vtable at offset 0 is a different interface entirely. The class hierarchy
descriptor records the displacement, so the comparison runs against the vtable
of the base's **own subobject** at that offset.

This is not a detail. Read through primary vtables, `IActor` slot 1 looks like
165 distinct implementations across 182 classes. Read through the subobject
vtables, it is **11**. The first reading is simply a different interface.

702 base-and-slot pairs come out, over 53 bases. Exactly **one** base-to-class
pairing in the image names an offset the class carries no vtable at; it is
reported, not guessed at.

### The measure checks itself

Slot 0 scores **0.96 to 1.00** for every base with more than twenty inheritors.
Nothing told it to. MSVC puts the scalar deleting destructor at slot 0 and every
class needs its own, so a near-1.0 ratio there is exactly what a correct
measurement must produce.

## What the ratio shows

`CWork`, over the same 264 classes in all seven rows:

| Slot | What `CWork` puts there | Distinct implementations | Ratio | Left empty |
| ---: | --- | ---: | ---: | ---: |
| 0 | a body | 253 | 0.96 | 0 |
| 1 | `_purecall` | 234 | 0.89 | 5 |
| 2 | `_purecall` | 139 | 0.53 | 92 |
| 3 | `_purecall` | 137 | 0.52 | 93 |
| 4 | `_purecall` | 102 | 0.39 | 125 |
| 5 | the empty body | 51 | 0.19 | **200** |
| 6 | the empty body | 33 | 0.13 | **216** |

The spread *is* the result. The early slots are where each class does its own
thing; the last two are kept as the empty body by 200 and 216 of the 264.

Two more shapes:

- **`IActor`** declares 30 slots. Slots 1–16 score between **0.011 and 0.187**
  across 182 classes — a wide interface that a handful implement and the rest
  leave empty.
- **`ICollisionHandle`**'s two non-destructor slots score **0.011**: 188 classes
  reach *two* addresses between them. That is one implementation shared through
  adjustor thunks, not 188 behaviours.

What any slot *does* is still unknown. Only how widely it varies is measured.

## `I` means abstract; `C` means nothing

Of the 53 measured bases, 21 carry the project's `I` prefix.

- **Not one** of the 21 defaults a slot to the empty body. 21 of 21, no
  exceptions.
- 19 of the 21 declare at least one slot pure. The two that do not,
  `INonPlayerDeath` and `IPlayer`, carry **exactly one slot each** — the
  destructor — so they have nothing else to declare. Among the bases that have a
  slot to declare, it is 19 of 19.

Each is paired with a `C`-named base supplying bodies for the same slots:

| Interface | Slots declared | Implementation | Slots defaulted |
| --- | ---: | --- | ---: |
| `IActor` | 30 | `CActor` | 8 |
| `INonPlayer` | 22 | `CNonPlayer` | 14 |
| `ICom` | 24 | `CCom` | 44 |

The prefix is only a name, but the vtables split along it exactly — in one
direction. The other direction does not hold: **nine** `C`-named bases declare a
slot pure, `CWork` among them, which declares four of its seven and is abstract
despite the prefix.

## `CWork`

The engine's universal base, answering the open item from the architecture note.
Seven slots: a body at `0x4C960`, four `_purecall`, then the empty body twice.
It is an **abstract base with two optional hooks** — not a fat object header.
264 classes inherit it; 200 and 216 of them take the two hooks as given.

## Querying it

```sql
SELECT base_name, slot, derived_classes, distinct_implementations, override_ratio
FROM v_exe_slot_override WHERE base_name = 'CWork' ORDER BY slot;

SELECT * FROM v_exe_base_interface_shape WHERE inheritors >= 15;
```

## Open work

- the 4,246 slots on the empty body are one address; **which classes and slots**
  form that set is a map of where the engine declares behaviour it never needed,
  and the table already holds it;
- slot *semantics* remain unestablished. Nothing in the file names a slot, and
  the override ratio is not evidence of what a slot is for.
