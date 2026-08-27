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
| 5 | 229 | 128 | `0x1401B8DF0` |
| 6 | 357 | 6 | `0x1401B9270` |
| | | **363** | |

The partition tiles the pool exactly, and the contract refuses to compile if it
ever stops doing so.

Each wrapper reads its own base out of `0x140581A20` — group 4 from
`+0x08`, group 0 from `+0x00` — so the tables are the authority and the
wrappers are seven views onto one array.

**A resource does not go "somewhere in the pool".** It goes into a specific
group with a fixed capacity, and group 4 holds exactly one record. Any model of
loading that treats the pool as one undifferentiated space is wrong about what
the game can hold.

### Only one group is a pool

Six wrappers take the index the caller names. **Group 5 alone searches** — it
scans its 128 records for the first in state 0. So what picks a group is the
caller, by calling that group's wrapper; group 5 is the only one that allocates
in the usual sense.

It is not the largest. Group 1 holds 136 records to group 5's 128 and still
takes a named index, so *dynamic* here describes how a record is chosen and
never how many there are. The obvious reading — that the biggest group must be
the pool — is wrong, and a test says so, because it was the first thing this
pass got wrong.

### There is no failure path at capacity

When the scan finds nothing it falls out of its loop with a **null** record and
stores into it immediately. The original runtime writes through a null pointer
rather than reporting a full pool. The completion helper does the same on an
odd handle.

This bounds what may honestly be said about the game: loading does not degrade
gracefully at capacity, it crashes. A tool must neither claim otherwise nor
reproduce it.

## 3. The state machine closes

Every state has a routine that leaves it. Nothing is reachable and unleavable.

| from | to | routine | what happens |
|---|---|---|---|
| 0 free | 1 requested | `0x1401B84E0` | allocate the payload, materialize it |
| 1 requested | 2 loaded | `0x1401B8DC0` | the load completed |
| 2 loaded | 3 relocated | `0x1401B92D0` | offsets become pointers, each payload dispatched by tag |
| 3 relocated | 0 free | `0x1401B9530` | destroy the embedded object, free the record |
| 1 requested | 4 releasing | `0x1401B8430` | abort an in-flight load |
| 2 loaded | 4 releasing | `0x1401B8430` | abort a load that never got relocated |
| 4 releasing | 0 free | `0x1401B8F00` | the deferred sweep over every record marked for release |
| any | 0 free | `0x1401B95E0` | full reset of all 363 |

Free is reached three ways. That is not redundancy — an ordinary release, a
deferred sweep and a full reset are three different lifetimes ending the same
way.

**Cancellation is deferred.** `0x1401B8430` marks every record in state 1 or 2
as `releasing` and leaves the destroying to the sweep; a record already at 3 is
not cancelled, because a load that got that far is finished rather than in
flight. Marking instead of destroying is what makes this safe while a loader is
still working, and it is why states 4 and 0 exist separately at all.

Its loop is unrolled three records at a time, 121 iterations — 363 again, from
a fourth routine.

Relocation *is* loading: the finalizer is where `PAC` is walked and every child
dispatched by tag, so a resource is not usable until L1 has moved it to state
3. A group wrapper skips a record already at 3, which is how a second
acquisition becomes a no-op rather than a reload.

## 4. The pool is one global, and a handle is a byte offset

`0x140C99D30`, confirmed three ways: acquire computes a handle by subtracting
it, the completion helper adds it back, and the deferred sweep walks from it.

A handle is that record's **byte offset**, not an index. The completion helper
traps deliberately on an odd handle — it writes through a null pointer rather
than continuing — because the stride is even and an odd offset can never name a
record.

## 5. Where L1 meets the loose-container layer

`0x1401B8CA0` is the junction, and it is the caller the loose-container
contract never had.

```text
request = record[0x18]
if (*(uint16*)request != 0)          -> generic materializer 0x1402EF4D0
else                                    // container-backed, kind 0
    switch (selector 0x1401B79E0):
        0        -> refuse
        1        -> packed wins: generic materializer 0x1402EF4D0
        anything -> the .lst list representation 0x1401B85C0
```

Every address in `LooseContainerContract` now has a recovered caller, and its
`container_backed_kind16 = 0` is exactly the `u16` this dispatch tests. Two
contracts recovered a week apart agree at the branch, and a test says so.

## 6. The alternate allocator

When the pool flag at `+0x6760` is 1 and the descriptor pointer at `+0x6720`
is non-null, acquire allocates through the pool's own arena at `+0x6718`
instead of the shared loader. That is what the `0x148` bytes between the record
array and the flag are for, in part.

## 7. What this changes for the product

Nothing about correctness — the product does not emulate the pool. What it
changes is what may be claimed. Any statement about how many resources of a
kind can be live, about reload behavior, or about when a materialized resource
becomes usable, now has a table and a state machine to be checked against
instead of an assumption.

## 8. Still open

- group 5's wrapper, the largest group at 128 records, is not in the acquired
  set;
- what selects a group for a given resource;
- the `0x28` embedded object's own layout, and the destructor at `0x140337710`;
- the rest of the `0x148` bytes of pool fields.

Two addresses in this range are accounted for and deliberately not modelled:
`0x1401B985D`–`0x1401B9860` are a divide-by-100 inside a formatting call, part
of a diagnostic print rather than of the pool's structure.
