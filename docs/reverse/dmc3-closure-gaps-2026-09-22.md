# The widest closure is not the widest: 362 of the 1,641 are reachable by edges the file records

2026-09-22. Target: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes. Evidence packet `dmc3-hdc-closure-gaps`.

The function map flags 1,641 functions `outside_every_closure`, and the
reachability note describes them as the ones "the entry point cannot reach under
any assumption the file supports." That guarantee is too strong. The closure
follows direct calls and virtual dispatch. The file records two more kinds of
edge, and following them absorbs 362 of the 1,641.

## First, the exception metadata, parsed rather than bounded

The last note found 332 unreached functions referenced from unwind data and
called it a lower bound, because it measured each unwind structure only up to the
handler pointer. It also found 526 referenced from "ordinary `.rdata`" and called
those the likeliest place for a second entry mechanism. The second statement was
wrong, and parsing the handler data shows why.

Every unwind structure with a handler names one of four routines, and the handler
data behind each has a known shape:

| handler | entries | data |
| --- | ---: | --- |
| `__GSHandlerCheck` | 1,408 | a stack-cookie offset, no funclets |
| `__CxxFrameHandler3` (thunk `0x346BDE`) | 518 | RVA of a `FuncInfo` |
| cookie check with C++ EH (`0x3458E8`) | 279 | RVA of a `FuncInfo`, then cookie data |
| `__C_specific_handler` (thunk `0x346C08`) | 5 | an SEH scope table |

All 797 `FuncInfo` pointers land on a valid magic number — none misses — which
identifies `0x3458E8` empirically as the cookie-checking C++ handler without
naming it. Parsing them gives 792 distinct `FuncInfo`, 2,240 unwind states,
4,150 IP-to-state entries — and **five try blocks with five catch clauses in the
entire image**. The engine uses C++ exceptions almost exclusively for destructor
unwinding. A port can treat it as RAII with next to no catch semantics.

With the full metadata extents, the 526 "ordinary `.rdata`" references were
mostly `FuncInfo`, unwind maps and IP maps beyond the short extents the last note
used. Only 99 of the no-caller functions are referenced by data or code that is
not exception metadata.

## An exhaustive partition of the 1,641

| | functions |
| --- | ---: |
| called by another unreached function | 794 |
| no caller; an exception-handling target | 383 |
| no caller; named only as the parent of a split fragment | 108 |
| no caller; referenced by other data or code | 99 |
| no caller; referenced nowhere outside `.pdata` | 257 |
| **total** | **1,641** |

The 383 exception targets are 327 unwind actions, 49 found only in IP maps, 5
catch handlers and 5 SEH filters or handlers. The 108 are referenced, but by the
chained unwind record of their own split-off code, which is a reference and not a
route in. The last note's "250 referenced nowhere" becomes 257 here because a
dword that falls inside exception metadata without being one of its target fields
is now counted as a coincidence rather than a reference.

## The second mechanism is in code, not in tables

Of the 99, verified instruction by instruction, three sites in reached code take
an address with `lea`:

```text
0x254BB3  lea rax,[rip+0x146]   ; 0x254D00
          mov [rsp+0x20],rax    ; fifth argument
          lea r9,[rip+0x7A]     ; 0x254C40, fourth argument
          mov r8d,0x27 ; mov edx,0xE00
          call 0x345B24
0x254CCA  lea r9,[rip+0x2F]     ; 0x254D00
          mov edx,0xE00 ; mov r8d,0x27
          call 0x345B94
```

That is MSVC's array construction and destruction machinery: `0x345B24` takes an
array, an element size (`0xE00`), a count (`0x27`), a constructor and a
destructor; `0x345B94` takes the array, size, count and destructor. The element
destructor `0x254D00` is only ever called by the runtime helper through the
pointer it was handed. The closure does not follow a taken address, so the
destructor looks unreachable. `0x345B24` has 145 callers and `0x345B94` has 360.

The census behind it, with a control. Every `lea r64,[rip+disp32]` pattern in
`.text` is 13,225 candidates. Their targets hit **49** unreached function starts,
**23** of them from inside reached code, and **0** of 1,641 aligned addresses
drawn from the same span that are not function starts. The raw-dword matches in
`.text` that the previous scan counted — 260 of them — are coincidences: x64 code
does not take a function's address as an absolute 32-bit value.

## Re-closing

Starting from the tool's 5,748 and closing over its own call edges reproduces
exactly 5,748, which confirms the reconstruction. Adding the two edge kinds:

| edges added | absorbs of the 1,641 |
| --- | ---: |
| funclets of reachable parents | 314 |
| addresses taken inside reachable code | 48 |
| both | **362** |

**1,279 remain, over 557,004 bytes.** By count the flagged population falls by
22%. By mass it barely moves — from 587,983 bytes — because funclets are small.
So the conclusion of the population note survives in substance: the unreached
mass is game code entered by routes this file does not record. What changes is
the guarantee attached to the flag, which was stated as "under any assumption the
file supports" and should have been "over calls and dispatch."

## Reproducible

`research/exe/measure_closure_gaps.py` computes every figure in the re-closing
section from a local copy of the image and a map report produced with `--all`,
identifies handlers by the import they jump through and `FuncInfo` by its magic
number rather than by address, and is SHA-gated. Its nine tests need no
executable and all eight mutations of its guards are killed.

## What is still unread

- The 1,279 that remain, and above all the 257 nothing refers to.
- Whether the tool should follow these two edge kinds itself. The measurement
  says it should; changing `outside_every_closure`'s definition moves a published
  counter and wants its own increment with the evidence records it would touch.
