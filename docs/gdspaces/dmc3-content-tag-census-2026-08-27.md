# What the runtime does with a four-byte tag — 2026-08-27

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

The question started narrow: recover `HITS` from the executable the way `MOT`
and `SCM` were recovered. There are four real `HITS` payloads in the corpus and
a whole image to check them against, which is the best evidence position
available anywhere in this project.

A search for the tag settled it in one step. **`HITS` does not appear in the
image at all** — not in `.text`, not in `.rdata`, not as a dword immediate, not
as a byte chain. So the narrow question became the general one.

## 1. The census

Two sweeps, both over a resynchronizing linear disassembly covering 99.87% of
`.text`:

- every chain of byte comparisons whose displacements run 0, 1, 2 — the shape a
  three- or four-byte tag test takes;
- every instruction whose immediate spells three or four printable ASCII bytes,
  which catches a tag compared as one dword.

Together they enumerate every tag the runtime compares. There are **five**:

| tag | dispatcher `0x1401B9FA0` | probe `0x1402DB1F0` |
|---|---|---|
| `MOD` | `0x1401B9FB4` | `0x1402DB1F3` |
| `EFM` | `0x1401B9FCB` | `0x1402DB206` |
| `SCM` | `0x1401B9FE2` | `0x1402DB21C` |
| `SHW` | `0x1401B9FFB` | `0x1402DB248` |
| `MRP` | — | `0x1402DB232` |

Plus the two container magics, `PAC` and `PNST`, compared by the walks.

`MRP` is compared by the probe alone: no handler dispatches it, and no supplied
file carries it.

## 2. What that makes of the rest

This project reads ten tags at offset zero and has been printing every one of
them the same way:

```
HITS  LIG2  DCA  SEF  CAM  EVE  POS  ITM  STE  MOT
```

**The runtime compares none of them.** This was already established for `MOT`
alone, and the honest generalization is now available: those ten are authoring
conventions. They are true statements about the files and false statements
about the game.

`LIG2` comes closest and still does not qualify — a constructor stores it as an
object's type field at `0x14023ECC9` and nothing ever compares it back.

That is not a reason to stop reading them. A tool that reads authoring
conventions is useful, and this corpus is full of them. It is a reason not to
print a convention and a recovered identification identically, which is the
same discipline already applied to slot names.

So `HITS` cannot be recovered from the executable. Not because the reverse was
insufficient — because the executable does not contain the thing being looked
for. The parser this project has for it stays exactly what it was: corpus-
derived, and now explicitly so.

## 3. Two readers found on the way

Sweeping for ASCII immediates turned up two functions worth having.

### TM2, at `0x1403365B0`

```
if (*(u32*)p != 'TM2\0') goto fallback
data = p + *(u32*)(p + 8)
```

The PlayStation 2 texture format, with a reader in a PC build. The magic is
compared as one **dword including the NUL** — the opposite of `PAC`, whose
fourth byte the runtime never reads, and the difference is the point: match
what the game matches, no more and no less.

This is the standing hypothesis about `at.ptx`, the 12 KiB texture container in
a real volume that no parser here accepts. It is not proof — nothing here has
seen those bytes — but the classifier now recognizes `TM2\0`, so if such a
payload appears it is named instead of left unknown.

One detail worth keeping: on a magic mismatch the reader does **not** report an
error. It constructs a fallback object and reports its dimensions, so "this is
not a TM2" is never announced, and a product inferring from the return value
would learn nothing.

### The DDS FourCC chain, at `0x14004A946`

Ten formats and four aliases, each mapped to a DXGI format code:

| FourCC | code | | FourCC | code |
|---|---|---|---|---|
| `DXT1` | 71 | | `ATI2` | 83 |
| `DXT3` | 74 | | `BC5S` | 84 |
| `DXT5` | 77 | | `RGBG` | 68 |
| `ATI1` | 80 | | `GRGB` | 69 |
| `BC4S` | 81 | | `YUY2` | 107 |

Aliases: `DXT2`→`DXT3`, `DXT4`→`DXT5`, `BC4U`→`ATI1`, `BC5U`→`ATI2` — each
jumps into another entry's store rather than carrying its own.

This project's texture work derived its compression set from the corpus, which
shows two of these. The chain is the runtime's own set, so it moves from
observed to recovered: a texture using `BC5S` is one the game reads, whether or
not any supplied file happens to.

## 4. Where it lives

- `include/dmc_rengine/profiles/dmc3/content_tag_census_contract.hpp`
- `include/dmc_rengine/profiles/dmc3/tm2_contract.hpp` — both readers
- `src/gdspaces/classifier.cpp` — `tm2`, four bytes
- `tests/content_tag_census_tests.cpp`
- `data/reverse/dmc3-type-identification-windows.v1.json` — four new windows
