# PTX runtime callbacks and arena initialization

Date: 2026-09-23. Branch: **Ада-Астра**.
Canonical executable SHA-256:
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

**Result: 10 EXE/C++ differential scenarios PASS.** This pass closes the
callback-record allocation, enqueue, drain and recycle path used by the shared
block allocator, and the bounded initializer for one runtime block arena. It
does not claim the callback targets' behavior or full owner-level startup and
shutdown order.

## Callback queue

The canonical instruction windows are `0x140329240..0x140329290`
(enqueue), `0x1403292A0..0x1403292F8` (drain), and
`0x140329300..0x140329378` (record acquisition). The free-list head is the
BSS global at `0x140CF3240`. The preallocation count is the image dword at
`0x1405D1130`; its canonical value is 32.

| Record offset | Value passed or stored |
|---|---|
| `+0x00` | callback function address |
| `+0x08` | callback context passed in `RCX` |
| `+0x10` | callback payload passed in `RDX` |
| `+0x18` | queue/free-list link |

Records are 0x20 bytes. Enqueue acquires a record, writes the four fields, and
pushes it at the caller-owned queue head. If the global free list is empty,
the acquisition function allocates 32 records with size 0x20, clears their
four qwords, and chains them onto the free list before popping one.

Drain saves each record's next pointer before invoking its callback. After the
callback returns, it pushes the processed record onto the global free list,
then advances the caller-owned queue head to the saved next pointer. The
queue is therefore drained in head-first order; enqueue itself is LIFO. The
drain code rereads the caller-owned head after invoking a callback. The C++
model preserves that ordering and permits a callback to mutate the head, but
the differential fixtures use non-reentrant callbacks.

## Arena setup

`0x140337780` initializes a 0x28-byte arena state from a caller-provided
backing span. Its arguments are state pointer (`RCX`), backing pointer
(`RDX`), requested block bytes (`R8D`), backing byte count (`R9D`), and
alignment shift (stack argument 5).

- `+0x00`: occupancy map start;
- `+0x08`: aligned data-region start;
- `+0x10`: signed block capacity;
- `+0x14`: block byte size rounded up to `1 << alignment_shift`;
- `+0x18`: backing byte count, set to zero when the minimum-size check fails;
- `+0x1C`: supplied alignment shift;
- `+0x20`: active allocation count, reset to zero;
- `+0x24`: untouched by this initializer.

For the tested non-overlapping backing spans, capacity is
`(backing_bytes - alignment) / (aligned_block_bytes + 1)`. The data region
starts at `align_up(backing_address + capacity, alignment)`. The initializer
fills the supplied backing byte count with `0xFF`, then clears the first
`capacity` occupancy bytes. A too-small span changes only `+0x18` to zero and
returns `AL=0`.

The one-time setup routine `0x1402C6010` calls this initializer for three
states at `0x140CA8910`, `0x140CA8938`, and `0x140CA8960`, after requesting
backing spans from `0x140337920`:

| State | Backing bytes | Requested block bytes | Alignment shift |
|---|---:|---:|---:|
| `0x140CA8910` | `0x04000000` | `0x800` | 6 |
| `0x140CA8938` | `0x00500000` | `0x400` | 4 |
| `0x140CA8960` | `0x00400000` | `0x200` | 6 |

The C++ slice implements and differentially checks the per-state initializer.
The backing allocator and its setup caller are recovered in
[`dmc3-ptx-temporary-backing-2026-09-23.md`](dmc3-ptx-temporary-backing-2026-09-23.md).

## Artifacts and verification

- [C++ callback queue API](../../include/dmc_rengine/reverse/deferred_callback_queue.hpp)
- [C++ callback queue implementation](../../src/reverse/deferred_callback_queue.cpp)
- [Arena API and implementation](../../include/dmc_rengine/reverse/runtime_block_allocator.hpp)
- [Differential bridge](../../tests/reverse/ptx_callback_arena_bridge.cpp)
- [Differential verifier](../../scripts/reverse/verify_ptx_callback_arena.py)
- [Canonical instruction evidence](../../data/reverse/ptx-callback-arena-20260923/evidence.json)
- [Differential scenario results](../../data/reverse/ptx-callback-arena-20260923/verification.json)

The verifier executes the canonical callback queue functions and arena
initializer in Unicorn and compares returned state with the C++ implementation.
It compiles the implementation with C++20, `-Wall -Wextra -Wconversion -Werror`.
The earlier palette context/allocator gate also remains green at 310 scenarios.

## Boundaries and next step

Callback target bodies are replaced with a synthetic `RET`; operator-new
failure, callback reentrancy, concurrency, and aliasing are outside this pass.
Arena overlap and invalid-pointer CPU-fault behavior are also outside the
fixture domain. These checks do not establish that a real game callback leaks
or that a production caller violates initialization order.

Next: continue the previously queued graphics configuration and finalizer
links. The relative runtime order between global context `0x140CF1030` and
embedded context `this+0x5E0` remains open; available evidence identifies their
separate initialization and cleanup callsites but does not connect their
lifetimes in one runtime trace.
