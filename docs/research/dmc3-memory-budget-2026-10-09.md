# DMC3 (HD Collection, x64) — main memory, pools and where to enlarge them

Executable studied: `dmc3.exe`, PE32+ x86-64, image base `0x140000000`,
SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Addresses below are RVAs (`dmc3.exe+RVA`). No game bytes are reproduced
beyond the 32-bit immediates a patch has to check.

## Question

Mods that make a PAC (model, textures) larger are reported to make the HUD
warp and then crash the game once enough of them are loaded
([Nexus mod notes](https://www.nexusmods.com/devilmaycryhdcollection/mods/271?tab=posts)).
The question was whether that is a per-file fixed buffer, or a memory budget
the whole game shares — and where it is set.

## Findings

### 1. One block, allocated once (`+30190`)

```
+030194 mov ecx, 0x10400000        ; 260 MiB
+030199 call +490D0                ; VirtualAlloc(0x100000, size, RESERVE) then COMMIT
+0301A1 mov [+5D9EA8], rax         ; block base
+0301A8 lea rcx, [rax+0x10400000]  ; block end
+0301AF mov [+5D9EF0], rcx
```

`+490D0` reserves and commits at the fixed low address `0x100000`
(`VirtualAlloc` with `MEM_RESERVE` then `MEM_COMMIT`, `PAGE_READWRITE`).

### 2. The block's layout (emulated, `tools/research/dmc3_heap_layout.py`)

The routine carves the block sequentially; emulating it with the allocator
stubbed gives every region:

| global | offset in block | size |
|---|---|---|
| `+5D9EA8` … `+5D9DB8` | `0x0` … `0x27BFC0` | system tables, 2 MiB at `+5D9DB0`, 1.52 MiB at `+5D9DB8`, small arrays |
| **`+5D9D30`** | **`0x400000`** | **`0x10000000` (256 MiB) — the main game heap** |
| `+5D9EF0` (end) | `0x10400000` | — |

The first 4 MiB are fixed-size system regions; everything else is the main
heap, which ends exactly at the block end.

### 3. The main heap and its pools (`+2C60E0`, `+2C6010`)

```
+2C60E4 mov rcx, [+5D9D30]          ; heap base (block + 4 MiB)
+2C60ED mov r8d, 0x10000000         ; memset(base, 0, 256 MiB)
+2C6113 mov r8d, 0x10000000         ; heap manager +CA89A0 initialised with 256 MiB, kind 6
```

Allocators `+CA8988` and `+CA89E8` attach to the manager `+CA89A0`
(`+3379B0`). From `+CA8988` the game carves three pools (`+337920` allocate,
`+337780` initialise a pool):

| pool object | size (edx / r9d) | blocks (r8d) | kind |
|---|---|---|---|
| `+CA8910` | `0x4000000` (64 MiB) at `+2C6031` / `+2C604E` | `0x800` | 6 |
| `+CA8938` | `0x500000` (5 MiB) at `+2C6065` / `+2C6082` | `0x400` | 4 |
| `+CA8960` | `0x400000` (4 MiB) at `+2C6099` / `+2C60B6` | `0x200` | 6 |

What is left of the 256 MiB after the pools (~183 MiB) is the general heap
that loaded files are allocated from.

### 4. Independent confirmation

DDMK (serpentiem, zlib licence) patches the same sites on the same build:
`+30195` / `+301AB` (260 → 300 MB, with the block redirected to its own
allocation) and `+2C6065` / `+2C6082` (the 5 MiB pool, to 64 MiB)
([DDMK `Mary/Memory.ixx`](https://github.com/serpentiem/ddmk)). That the
game keeps running with those values is evidence the sites are the ones
that matter; DDMK's code is not used here, only the fact of the sites.

## Conclusion

There is no per-PAC fixed buffer in the loader path examined; the limit a
larger mod runs into is the shared main heap (256 MiB) and its pools. To
give the game more room, a patched copy of `dmc3.exe` must change, together:

| site | original | meaning |
|---|---|---|
| `+30195` | `0x10400000` | block size = 4 MiB + heap |
| `+301AB` | `0x10400000` | block end = same |
| `+2C60EF` | `0x10000000` | heap clear size |
| `+2C6115` | `0x10000000` | heap manager size |

and optionally each pool's two immediates (its size must also be taken from
the heap, so heap growth should cover pool growth).

## Open

- The block is reserved at the fixed address `0x100000`; a larger block
  needs that range free at start-up. DDMK's 1 GiB low allocation suggests
  low space is available, but a reservation failure is the first thing to
  check when a larger value does not start.
- Pointers into this block may be kept in 32-bit fields (the PS2 lineage);
  keeping the block below 2 GiB is the safe assumption until checked.
- Which file kinds each pool serves (kind 4 / 6) is not established yet.
- The effect in the game of a larger heap is to be confirmed by running it.
