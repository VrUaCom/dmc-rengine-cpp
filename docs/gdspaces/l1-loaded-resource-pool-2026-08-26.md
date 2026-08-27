# L1 — the loaded-resource pool is a fixed partition — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
Receipts in `data/reverse/dmc3-type-identification-windows.v1.json`.

**Status:** RECOVERED FROM INSTRUCTIONS AND FROM IMAGE DATA

Thirty byte windows of this layer were acquired in an earlier pass and none of
them became code. The addresses sat in documents while the product modelled
resource lifetime its own way. This closes that gap for the pool itself.

## 1. It is an array, not a list

`0x1401B8380` initializes **363 records of stride `0x48`**, and
`0x1401B92D0` walks the same 363 with the same stride. Two routines agreeing
is what makes the geometry recovered rather than inferred.

```text
record, stride 0x48
  +0x00  int32  group id      <- written at init from the partition table
  +0x04  int32  state
  +0x20  qword  payload
  +0x28  embedded object      <- constructed at init, destroyed at release
pool
  +0x6760  byte flag          <- cleared last, past the record array (0x6618)
```

## 2. Seven groups, with capacities the image carries as data

Two `u16` tables at `0x140581A10` and `0x140581A20`:

| group | base | capacity | wrapper |
|---|---|---|---|
| 0 | 0 | 4 | `0x1401B8F50` |
| 1 | 4 | 136 | `0x1401B90B0` |
| 2 | 140 | 60 | `0x1401B9160` |
| 3 | 200 | 28 | `0x1401B8FF0` |
| 4 | 228 | **1** | `0x1401B8D60` |
| 5 | 229 | 128 | not acquired |
| 6 | 357 | 6 | `0x1401B9270` |
| | | **363** | |

The partition tiles the pool exactly, and the contract refuses to compile if it
ever stops doing so.

Each wrapper reads its own base out of `0x140581A20` — group 4 from
`+0x08`, group 0 from `+0x00` — so the tables are the authority and the
wrappers are seven views onto one array.

**A resource does not go "somewhere in the pool".** It goes into a specific
group with a fixed capacity, and group 4 holds exactly one record. When a group
is full the original runtime has nowhere else to put a resource, and any model
of loading that treats the pool as one undifferentiated space is wrong about
what the game can hold.

## 3. The state machine

| state | meaning |
|---|---|
| 0 | free |
| 1 | requested |
| 2 | loaded |
| 3 | relocated — the payload's offsets are now pointers |
| 4 | releasing |

`0x1401B8DC0` moves 1 → 2. `0x1401B92D0` walks records in 2, relocates each
payload through the type dispatch, and leaves them at 3. A group wrapper skips
a record already at 3, which is how a second acquisition of a loaded resource
becomes a no-op rather than a reload.

That last point ties L1 to the type layer: the finalizer is where `PAC` is
walked and every child dispatched by tag, so *relocation is loading*. A
resource is not usable until L1 has moved it to state 3.

## 4. What this changes for the product

Nothing about correctness yet — the product does not emulate the pool. What it
changes is what may be claimed: any statement about how many resources of a
kind can be live, or about reload behavior, now has a table to be checked
against instead of an assumption.

## 5. Open

- group 5's wrapper, the largest group at 128 records, is not in the acquired
  set;
- what selects a group for a given resource;
- the `0x28` embedded object's own layout;
- the `0x148` bytes of pool fields between the record array and the flag.
