# A hard cap of one hundred open resources, and two sections that were already closed

2026-09-19. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-open-resource-cap`.

## Sections 1C and 1E hold, and were never open

I set out to check §1C and §1E of the 2026-09-05 reconciliation against the
bytes, having found §1A, §1B and §1D sound. They hold. They were also not open:
`docs/gdspaces/l2-exe-reconciliation-2026-08-26.md` §1.1 states that "inside
`OpenGameResource` the raw instructions confirm" basename extraction after the
last separator, the `0x400` candidate buffer, the six prefixes from the table,
provider phase 1 before phase 2, the resolver at `0x327430`, and early failure
when candidate construction fails. §1.2 and §1.3 add the mask-bit gating per
provider type and the literal `0x0E` normalisation. That document marks the
static reverse CLOSED.

My re-read agrees with every one of those points and adds nothing to them. For
the record, what I read: the request is reduced to its basename by a backward
scan for `/` or `\`; the prefix table at `0x55AEF8` holds exactly six pointers,
five prefixes and an empty string at index five; the search is a double loop with
the **prefix as the inner** index over six and the provider pass as the outer over
two; the mask handed to the resolver is literally `pass + 1`, so 1 then 2, built
from the pass index rather than from two constants; and a join failure exits both
loops rather than skipping that prefix.

§1E likewise: in `0x327473` a lookup miss jumps to `0x327565`, the next mount,
while a construction failure jumps to `0x327572`, which frees the stream and
returns zero. Two different destinations for the two failures, which is exactly
the distinction §1E draws.

What follows is what was not already there.

## One hundred slots, and that is the ceiling

Before any of the search, `OpenGameResource` calls `0x48B20`, and a null result
makes it return `-1` immediately. `0x48B20` is:

```text
EnterCriticalSection(0xC19AE0)
rdx = 0xC18E78 ; ecx = 0
while [rdx] != 0:
    ecx++ ; rdx += 0x20
    if ecx >= 0x64: break        ; 100
if a free slot was found:
    slot = 0xC18E60 + ecx*0x20
    [slot+0x18] = 1              ; in use
    [slot+0x10] = 0
    [slot+0x08] = 0
    [slot+0x00] = -1
LeaveCriticalSection(0xC19AE0)
return slot                      ; null if none was free
```

So the open-resource table is **100 records of 0x20 bytes at `0xC18E60`**, the
in-use marker is the byte at `+0x18`, and allocation is a linear scan under one
critical section. When all hundred are in use the allocator returns null and the
request fails with `-1` — not a wait, not a growth, not an eviction.

The record fills out from its users:

| offset | set by | holds |
| --- | --- | --- |
| 0x00 | the allocator, to `-1` | an identifier |
| 0x08 | `OpenGameResource` | the resolved stream |
| 0x10 | `OpenGameResource` | the resource's size |
| 0x18 | the allocator and the release | the in-use byte |
| 0x19 | read at `0x2FE16` | a flag set elsewhere |

`0x48DF0` releases a slot: lock, `[slot+0x18] = 0`, tail-jump to
`LeaveCriticalSection`. That is the whole of it — it does **not** close the stream
at `+0x08` or reset `+0x00` and `+0x10`. On the one path that calls it here, the
candidate-construction failure at `0x2FE20`, no stream has been assigned yet, so
nothing leaks; but the release itself provides no guarantee about that.

A hundred is a hard number a port has to decide about, and it is enforced in one
place by one scan.

## Where the size comes from

On success the open does this:

```text
0x2FE0B  mov rcx,rax          ; the stream just resolved
         call 0x3272A0        ; the size dispatcher
         movsxd rcx,eax
         movzx eax,[rbp+0x19]
         mov [rbp+0x10],rcx   ; the size, into the slot
```

`0x3272A0` is the third stream dispatcher — the one yesterday's correction found
by a byte scan after claiming there were two — and its archive branch is the
two-byte accessor at `0x0629C0` that returns `[entry+0x20]`, the uncompressed
size. So the chain closes from both ends: a caller learns a resource's size
through a tagged-union dispatch into a two-instruction getter, and that is why
the size accessor exists outside the file layer at all.

## Two modes the shipped call surface never selects

The prefixed search is gated:

```text
0x2FD2D  eax = mode & 3
         cl = (al != 3)
         al = ((mode & 2) == 0)
         if (al & cl) == 0 -> 0x2FE2A
```

`(mode & 2) == 0` already implies `mode & 3 != 3`, so the first test can never
fail when the second passes: the gate reduces to **bit 1 of the mode being
clear**. With bit 1 set, control reaches `0x2FE2A`, which tests **bit 9** and,
when it is set, derives the executable's own directory with
`GetModuleFileNameA` and `strrchr` exactly as the bootstrap does; when bit 9 is
clear it goes elsewhere again, to `0x2FF12`.

All three direct callers load `edx = 1` — prior art from August, and it still
holds. So bits 1 and 9 are never set by any caller in the image, and both
alternative resolution paths are unreachable from the shipped call surface. They
are in the binary and nothing selects them.

## What is still unread

- `0x2FF12`, the third path, and whether it differs from the other two in more
  than where it looks.
- Section 1F of the 2026-09-05 reconciliation, the collision receipt.
- What sets `[slot+0x19]`, read on the success path and written by nothing here.
- The 1,641 functions no path reaches, still untouched as a population.
