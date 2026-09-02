# How DMC3 identifies a file — recovered — 2026-08-26

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes, the canonical analysis build.

**Status:** RECOVERED FROM INSTRUCTIONS / TWO INDEPENDENT CALL SITES

Byte-window receipts for every address below are in
`data/reverse/dmc3-type-identification-windows.v1.json`.

## 1. The whole identification model

The runtime asks two questions, in this order.

**First the name.** `0x1402DB3C0` tests the resource name against a literal
table at `0x140507070` using a substring search:

| literals | type |
|---|---|
| `.ptx` `.PTX` `.Ptx` | 4 — texture pack |
| `.clt` `.CLT` `.Clt` | 5 — palette |
| `.c1d` `.C1D` `.c1D` `.C1d` | 6 |

Case is enumerated, not folded: `.PTx` is a name this runtime does not
recognize. The match is a substring of the whole name, not a suffix, so
`at.ptx.bak` is a texture pack here.

**Then the bytes.** Only if no extension matched does `0x1402DB1F0` read the
payload and return a type code:

| bytes 0..2 | type | handler |
|---|---|---|
| `MOD` | 0 | `0x1402FE3B0` |
| `EFM` | 1 | `0x1402F7A90` |
| `SCM` | 2 | `0x1403051B0` |
| `MRP` | 3 | none — the type is recorded and nothing is called |
| `SHW` | 7 | `0x1403204C0` |
| anything else | −1 | nothing |

**Three bytes, not four.** This is the correction that matters most to the
product. Stage files store `SCM ` with a trailing space and this project had
been requiring it; the runtime stops after `SCM`. A reader that demands the
fourth byte refuses payloads the game accepts.

The same table appears a second time at `0x1401B9FA0`, reached from the
container walk instead of from registration, calling the same four handlers.
Two independent sites agreeing is why the map above is stated as recovered.

## 2. The container walk

`0x1401B92E0` finalizes the loaded-resource pool and walks PAC; `0x1401B9FA0`
dispatches by type and walks PNST, recursing into itself.

```text
if (p[0]=='P' && p[1]=='A' && p[2]=='C')          // three bytes
    count = *(int32*)(p + 4);
    for (i = 2; count--; ++i)
        off = *(uint32*)(p + 4*i);                 // table at p+8
        if (off) dispatch(p + off);                // container-relative
```

PNST is compared as four bytes including the `T`, then walked identically and
recursively.

Three product claims are now proven rather than observed:

- **a zero offset is an absent slot.** The runtime substitutes a null pointer
  and calls nothing. A sparse container is intact, and a slot index is an
  identity rather than a position in a packed list.
- **offsets are container-relative**, added to the container's own base.
- **the count is signed**; a non-positive count ends the walk with no children.

`EFW` is recognized at `0x1401BA00D` and deliberately not walked — a real
state, distinct from unknown, and one a product must not expand.

The pool itself is a fixed array: `0x16B` slots of stride `0x48` from `+0x10`,
with a state field that moves 2 → 3 as each slot is finalized.

## 3. What the runtime never compares

A whole-image sweep for four-character immediates finds exactly two families of
magic comparison outside the tags above: `DDS ` with its FourCC set
(`DXT1/2/3/4/5`, `ATI1/2`, `BC4U/S`, `BC5U/S`, `RGBG`, `GRGB`, `YUY2`, `DX10`)
at `0x140049A8E` and `0x14004A949`, and `VAGp` at `0x140032970`.

There is **no** `PAC\0` comparison, no `PNST` comparison outside the walk, no
`HITS`, no `LIG2`, no `DCA`. Those tags are real — the files carry them — but
the runtime reaches those payloads by position inside a container it was told
to load, not by probing for a signature. That is precisely why relative-slot
containers store no names: nothing downstream needs one.

So the product's magic table is a tool's convenience, and it is now separated
in the code from the comparisons the runtime actually performs.

## 4. The sector

`0x1402EF6A2` converts a file length to bytes with `shl ebx, 0xB` — the file
layer measures in 2,048-byte sectors. The texture pack's size table is in the
same unit, which is where its sector count comes from.

## 5. What this changed in the product

- `ResourceTypeContract` and `RelativeSlotWalkContract` carry the addresses,
  the tables and the geometry, with `static_assert`s tying them to one image.
- The classifier probes the recovered three-byte tags before its observed
  four-byte ones, and emits `mod`, `efm`, `mrp`, `shw` for the first time.
- PAC magic narrowed from four bytes to the recovered three; PNST stays at
  four. `RelativeSlotContainerSpec` now carries how many bytes to compare.
- The integration registry gained the recovered types and the observed stage
  tags, and PTX moved from `recognized/pending` to `structural` with a parser.

## 6. Open

- the `0x70` texture-block descriptor, walked past and unread;
- the four handlers' payload structures;
- the third string in each resource-table entry, filled by the registrar and
  not yet traced to a consumer;
- whether any path reaches the walk with a container the extension table has
  already typed, which would make the two axes interact rather than layer.
