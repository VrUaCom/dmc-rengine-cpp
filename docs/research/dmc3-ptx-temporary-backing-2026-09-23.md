# PTX temporary backing allocator

Date: 2026-09-23. Branch: **Ада-Астра**.
Canonical executable SHA-256:
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

**Result: 25 EXE/C++ differential scenarios PASS.** This pass recovers the
temporary backing descriptor, nested lease checkpoint, aligned bump allocation,
and cursor rollback. It also confirms the direct caller and the three arena
requests made by one-time setup.

## Backing descriptor and lease

`0x140337A30` initializes a 0x28-byte descriptor from a caller-owned backing
span. It stores the base at `+0x00`, limit (`base + bytes`) at `+0x08`, aligned
cursor at `+0x10`, byte count at `+0x18`, active lease depth at `+0x1C`,
alignment shift at `+0x20`, and zero at `+0x24`. The cursor is aligned upward;
the limit is not rounded.

The 0x18-byte lease state is:

| Offset | Meaning |
|---|---|
| `+0x00` | backing descriptor pointer |
| `+0x08` | cursor checkpoint captured at acquire |
| `+0x10` | active lease depth at acquire |

`0x1403379B0` acquires only into an empty lease. It captures the arena cursor,
increments the active depth and a second live lease counter at `+0x24`, and
stores the new depth in the lease. Nested leases therefore form a stack.
`0x1403379E0` releases only the current top lease: it checks the saved depth,
decrements both counters, restores the saved cursor, and clears the lease.
Releasing an outer lease while an inner lease is active returns false without
changing state.

## Allocation behavior

`0x140337920` takes the lease pointer in `RCX` and raw requested bytes in `EDX`.
It returns null if the lease is empty, its saved depth does not equal the arena's
active depth, or the requested raw byte range exceeds the arena limit. The
capacity check occurs **before** alignment rounding. It then rounds the size up
to `1 << alignment_shift`, returns the old cursor, and advances by the rounded
size. Since there is no second capacity check after rounding, a request that
fits before rounding can advance the cursor beyond the descriptor limit by up
to one alignment unit minus one. The C++ implementation preserves this order.

The initializer `0x1402C6010` first acquires the global lease from descriptor
`0x140CA89A0` through owner state `0x140CA8988`. On success it makes three
allocations from that lease and initializes the returned spans:

| Allocation callsite | Backing bytes | Arena state | Block bytes | Shift |
|---|---:|---:|---:|---:|
| `0x1402C603C` | `0x04000000` | `0x140CA8910` | `0x800` | 6 |
| `0x1402C6070` | `0x00500000` | `0x140CA8938` | `0x400` | 4 |
| `0x1402C60A4` | `0x00400000` | `0x140CA8960` | `0x200` | 6 |

Direct callsite `0x14004F49F` enters this one-time setup routine. Setup returns
false without allocating if lease acquisition fails; after a successful lease,
the initializer calls the three backing allocations and arena initializers in
sequence.

## Context lifecycle boundary

Static instruction evidence confirms the global PTX context initialization call
at `0x1402C0594` for `0x140CF1030`, and context cleanup calls at `0x1402C035B`
and `0x140245D67`. The latter passes `this+0x5E0` only when the guard byte
`this+0x638` is nonzero. These are distinct code paths; this pass does not
establish their relative runtime order or prove that the setup routine above
dominates either context path.

## Artifacts and verification

- [Temporary backing allocator API](../../include/dmc_rengine/reverse/temporary_backing_allocator.hpp)
- [Temporary backing allocator implementation](../../src/reverse/temporary_backing_allocator.cpp)
- [Differential bridge](../../tests/reverse/ptx_temporary_backing_bridge.cpp)
- [Differential verifier](../../scripts/reverse/verify_ptx_temporary_backing.py)
- [Canonical instruction and callsite evidence](../../data/reverse/ptx-temporary-backing-20260923/evidence.json)
- [Differential scenario results](../../data/reverse/ptx-temporary-backing-20260923/verification.json)

The verifier executes the canonical EXE instructions in Unicorn and compares
descriptor and lease bytes with the C++ model. It compiles with C++20,
`-Wall -Wextra -Wconversion -Werror`.

Limits: callback targets remain bounded synthetic stubs in the prior pass;
concurrent access, reentrant acquisition, and invalid-pointer CPU-fault behavior
are not modeled. A process-level runtime trace is still needed to establish
cross-owner startup and shutdown order.
