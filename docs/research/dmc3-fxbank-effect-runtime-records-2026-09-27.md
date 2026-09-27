# DMC3 HD FXBANK effect runtime records — V / E / G / P

Date: 2026-09-27  
Reverse branch: `reverse/em034-lady-runtime-20260927`  
Native Reader implementation branch: `feature/pnst-gallery-visual-previews`

## Authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- ImageBase: `0x140000000`

Corpus used for cross-checking:

- `em034_028.pnst`
- FXBANK loader: `0x1402C04C0`
- manifest identity: `(kind, u16 id)`

This document records only fields whose use is visible in the executable. It
does not assign synthetic file extensions or historical child filenames.

## 1. V record — composite child graph + local transform

Runtime consumer: `0x140324680`.

The V payload begins with a signed 16-bit entry count. Entries use stride
`0x2C`. For entry index `i`, with `base = i * 0x2C`:

| offset | use |
|---|---|
| `base + 0x04` | dispatch kind |
| `base + 0x06` | referenced u16 id |
| `base + 0x0C..0x14` | translation float3 |
| `base + 0x18..0x20` | rotation float3 in degrees |
| `base + 0x24..0x2C` | scale float3 |

The rotation triple is converted by:

```text
value / 360.0 * 2*pi
```

before `0x1403304A0` builds the rotation matrix.

The transform construction sequence is:

1. `0x1403304A0` — rotation;
2. `0x140031200` — translation;
3. `0x1403304F0` — scale;
4. compose with the parent transform.

### V dispatch map

The branch at `0x1403247D5..` is now closed:

| dispatch byte | target family | creator |
|---:|---|---:|
| 0 | P | `0x1403127B0` |
| 1 | E | `0x1402E3B90` |
| 2 | G | `0x1402EBA90` |
| 3 | V | recursive `0x140324570` |

This corrects the earlier tentative 1/2 ordering.

Corpus examples:

- `V10`: P22, E879, E13
- `V377`: E669, E699, G214, E892, E892
- `V488`: P18, P3, P16, P2, E1, V8
- `V543`: P18, P3, P16, P2, E1, V8, E61

Status: **EXE_CONFIRMED + CORPUS_CONFIRMED**.

## 2. E record — texture / animation-backed rectangle

Primary runtime object creation: `0x1402E3C20`.

### Texture link

`0x1402E3AA0` reads:

```text
u16 [E + 0x04]
```

and resolves it through the T manager `0x140CF0AA0` using
`0x140322CD0`.

Therefore `E+0x04` is an EXE-confirmed T texture id.

### Optional A animation link

When:

```text
byte [E + 0x06] == 1
u16  [E + 0x08] != 0xFFFF
```

the runtime binds `E+0x08` through the A manager/state path
(`0x140322960`). Frame rectangles are then read through
`0x1403227F0 / 0x140322810`.

Therefore `E+0x08` is an EXE-confirmed A animation id under that gate.

### Direct rectangle

When the A path is not active, `0x1402E3F8D..` copies:

```text
u16 E+0x0C
u16 E+0x0E
u16 E+0x10
u16 E+0x12
```

into the same runtime rectangle members used by the A-frame path. The runtime
then derives right/bottom by adding width/height.

These four fields are therefore the direct texture rectangle:

```text
x, y, width, height
```

Mode byte `E+0x01` is consumed directly; mode 5 bypasses the normal texture
path in several runtime branches.

Corpus example:

```text
E745:
  mode = 1
  T = 5
  A = 110 (gate +0x06 = 1)
  direct rectangle = 0,96,16,16
```

Status: **EXE_CONFIRMED + CORPUS_CONFIRMED**.

## 3. G record — generator runtime envelope

Creator path: `0x1402EBA90 -> 0x1402EBB20`.  
Consumer/init path: `0x1402EBC10`.

The following offsets are directly read by the executable and are safe to
surface visually:

| offset | access/use |
|---|---|
| `+0x01` | mode selector |
| `+0x02` | u16 C id when mode == 1; lookup via `0x1402D3B70` |
| `+0x18` | signed/int scalar converted to float |
| `+0x20` | signed/int scalar converted to float; also returned by `0x1402EBBF0` except a mode special case |
| `+0x30` | mode/special-case byte |
| `+0x38`, `+0x3C` | float endpoints used by a runtime interpolation path |
| `+0x40` | u16 step/count used by that interpolation |
| `+0x50`, `+0x54` | float range used by a randomized runtime path |
| `+0x59` | byte used by the randomized path |

Exact gameplay names for the scalar/range fields are intentionally **not**
assigned yet.

Corpus:

```text
G214:
  +20 = 700
  +30 = 1
  +38 = 1.5
  +3C = 1.0
  +40 = 15
  +50 = 1.0
  +54 = 1.0

G8:
  +20 = 2
  +30 = 0
  +38 = 1.0
  +3C = 1.0
  +40 = 0
  +50 = 1.0
  +54 = 1.0
```

Status: field access **EXE_CONFIRMED**; gameplay labels for several scalars
remain **PRESERVED_UNDECODED**.

## 4. P record — rebased relative graph + six runtime subtypes

Registrar `0x140314B80` parses the raw record through `0x140312DC0`.

Parser contract:

1. `u32 +0x00 == 2`; otherwise reject.
2. relative base is raw `+0x10`.
3. signed dword `+0x10` resolves the root object.
4. signed dword `+0x14` resolves the pointer-list destination.
5. `u16 [root + 0xC0]` is the number of relative child targets.
6. the relative target dwords begin at raw `+0x18`;
7. each target is rebased against raw `+0x10`;
8. the resulting pointers are written into the resolved list destination.

Runtime creator `0x140312840` switches on:

```text
byte [parsed_root + 0x01]
```

with supported subtype values 0..5 and six distinct constructor paths.

em034 corpus:

| P id | bytes | root | subtype | target count |
|---:|---:|---:|---:|---:|
| 22 | 704 | 0x20 | 4 | 2 |
| 325 | 336 | 0x20 | 3 | 0 |
| 337 | 336 | 0x20 | 3 | 0 |
| 307 | 336 | 0x20 | 3 | 0 |
| 18 | 528 | 0x20 | 4 | 1 |
| 3 | 528 | 0x20 | 3 | 1 |
| 16 | 336 | 0x20 | 3 | 0 |
| 2 | 528 | 0x20 | 1 | 1 |
| 257 | 336 | 0x20 | 3 | 0 |
| 258 | 336 | 0x20 | 3 | 0 |

Status: **EXE_CONFIRMED + CORPUS_CONFIRMED** for the relative graph and subtype
selector. Semantic names of subtype 0..5 remain open.

## 5. Native Reader visualization contract

Branch: `feature/pnst-gallery-visual-previews`.

The gallery contract is now:

- T — decoded real DDS texture;
- A — sprite/frame atlas over its linked T texture;
- E — linked T texture plus EXE-confirmed direct rectangle or linked A frames;
- V — child dependency graph (P/E/G/V) plus per-entry translation / degree
  rotation / scale;
- G — EXE-read runtime envelope/range visualization with source offsets shown
  for fields whose gameplay names are still unknown;
- P — rebased relative-layout graph, root/list/targets and runtime subtype 0..5;
- M — normal MOD/EFM native 3D renderer via lazy gallery materialization;
- any registered record not yet typed — explicit byte-map visual, never an
  empty tile.

Gallery thumbnails are bounded to 256x256; opening a child still uses the full
native viewer.

Synthetic `.fxg/.fxv/.fxa/... ` filenames are not naming authority and have
been removed from the gallery presentation. Manifest `kind + id` remains
visible because it is real runtime identity.

## 6. Remaining reverse gates

The visual surface no longer depends on these names, but deeper semantic
reconstruction still needs:

1. G scalar/range gameplay meaning beyond the confirmed runtime access pattern;
2. semantic names and exact render behavior of P subtypes 0..5;
3. C record structure for banks that actually contain C;
4. full cross-record simulation if the Reader later needs to reproduce the
   complete live effect rather than an evidence-backed static preview.
