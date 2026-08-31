# DMC3 LIG2 light-category routing reverse — 2026-08-31

**Status:** CANONICAL RESEARCH ADDENDUM  
**Canonical EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## 1. Main correction

The first LIG2 record byte must not be described as a physical `point / spot / directional` light-type enum from current evidence.

The original runtime uses the low values primarily as **light routing/categories**. The physical distance behavior is controlled independently by other record fields.

## 2. Exact selector rules

The light selector at `0x1402EE560` takes a query mask and filters candidate light records.

For the observed low categories:

```text
record category 1 -> accepted when queryMask has bit 0x01 OR 0x02
record category 2 -> accepted when queryMask has bit 0x02
record category 3 -> accepted when queryMask has bit 0x01
record category 4 -> accepted when queryMask has special bit 0x10
```

The selector additionally uses:

```text
query bit 0x04 -> scan CLightMgr list at +0x20
query bit 0x08 -> scan CLightMgr list at +0x38
```

Thus category filtering and light-source-list selection are separate mechanisms.

The dynamic-list branch additionally strips/tests bit `0x08` from the light record category/flags byte before applying the same low-category logic, so the first record byte should remain modeled as `category/flags`, not a strict enum.

## 3. MOD/EFM CDraw default proves category 2 routing

General `CDraw` constructor:

```text
0x140089270
```

initializes:

```text
CDraw + 0x744 = 2
```

The general lighting update at `0x14008B500` reads this value and passes it into the model-lighting builder `0x1402FD040`.

That builder ORs in source-list bits `0x04 | 0x08` before calling `0x1402EE560`.

Therefore the normal general-model query is effectively:

```text
routing mask 2
+ static/dynamic source lists
```

and the selector admits:

```text
category 1
category 2
```

but not category 3 or the special shadow category 4.

Evidence-safe semantic description:

> **LIG2 category 2 = general CDraw / MOD-EFM-facing lighting category.**

Do not rename it to `point light` or `directional light`.

## 4. CDrawSCM default proves category 3 routing

`CDrawSCM` constructor:

```text
0x140089320
```

writes:

```text
DWORD [CDrawSCM + 0x570] = 0x00010000
```

Little-endian field split gives:

```text
+0x570 = 0
+0x571 = 0
+0x572 = 1   <- SCM light routing mask
+0x573 = 0
```

The SCM lighting update at `0x14008B5D0` reads byte `CDrawSCM + 0x572` and passes it into `0x1402FD2B0`, whose per-mesh path calls the same selector.

With routing mask `1`, the selector admits:

```text
category 1
category 3
```

but not category 2 or category 4.

Evidence-safe semantic description:

> **LIG2 category 3 = SCM/stage-geometry-facing lighting category.**

## 5. Category 1 is shared

Because category 1 is accepted by both routing masks `1` and `2`, it is shared by the normal model and SCM lighting paths.

Evidence-safe semantic description:

> **LIG2 category 1 = shared/general lighting category consumed by both CDraw and CDrawSCM.**

This is a routing statement, not a physical-light-shape statement.

## 6. Category 4 remains shadow-specific

The companion shadow reverse already proved:

```text
CDrawShadow query mask = 0x1C
```

and the special `0x10` selector bit admits category/flag `0x04`.

Evidence-safe semantic description:

> **LIG2 category 4 = SHW shadow-projection light category in the recovered path.**

It supplies the world-space point used to derive shadow projection direction/tilt.

## 7. Correct category map

Current canonical interpretation:

| Value | Runtime ownership/routing | Status |
|---:|---|---|
| `0` | inactive / not registered | EXE_CONFIRMED |
| `1` | shared between general CDraw and CDrawSCM | EXE_CONFIRMED |
| `2` | general CDraw / MOD-EFM-facing | EXE_CONFIRMED |
| `3` | CDrawSCM / stage-geometry-facing | EXE_CONFIRMED |
| `4` | shadow-projection category selected by CDrawShadow | EXE_CONFIRMED |

The byte can contain additional flag bits, so tooling should preserve the raw value and expose decoded routing flags rather than enforce only values 0..4.

## 8. Distance behavior is independent of category

The evaluator `0x1402EE2A0` treats record byte `+0x02` independently from category.

When:

```text
record +0x02 == 0
```

runtime computes:

```text
weight = max(1 - distance * record[+0x20], 0)
```

Therefore:

```text
record +0x20 = linear distance-falloff coefficient
zero-contribution range = 1 / record[+0x20]
```

when `+0x20 > 0`.

When:

```text
record +0x02 != 0
```

runtime sets the distance weight to exactly `1.0`.

Thus a category-2 light can be either distance-attenuated or unattenuated; category does not define `point` versus `directional` behavior.

## 9. Real-corpus examples

### st114

Five active category-2 local records use approximately:

```text
falloff = 1.11111e-5
range   ≈ 90,000 game units
RGB-like raw channels = (255, 67, 47)
```

Three category-1 records use approximately:

```text
falloff = 6.25e-6
range   ≈ 160,000 game units
RGB-like raw channels = (0, 50, 200)
```

The category-4 shadow record uses the no-distance-attenuation mode.

### st001

The active category-3 records are consumed by SCM routing and use distance attenuation with differing ranges. Category-2 records are consumed by general-model routing. The type-4 record remains the SHW projection source.

## 10. Colour channel promotion

`0x1402EE2A0` reads:

```text
record +0x24 -> u16 -> float
record +0x26 -> u16 -> float
record +0x28 -> u16 -> float
```

into three consecutive contribution channels.

Together with corpus RGB-like distributions, these may be safely exposed as **light colour/contribution channels** in a research editor. Exact final shader normalization and display colour-space handling remain to be traced before claiming a final normalized RGB ABI.

## 11. Editor implication

Do not expose:

```text
Type 1 = Point
Type 2 = Spot
Type 3 = Directional
```

That would be false according to current runtime evidence.

Expose instead:

```text
Routing / category:
  Shared
  Model (CDraw/MOD-EFM)
  Stage (CDrawSCM)
  Shadow projection

Distance attenuation:
  Enabled / Disabled

Position:
  X / Y / Z

Range:
  derived from 1 / falloff

Colour:
  raw R/G/B contribution channels
```

A later pass can recover additional physical-light behavior if other fields or shader consumers encode spot cones, directional vectors, or other modes.

## 12. Effect/dynamic-light boundary

The same selector can scan two separate CLightMgr lists (`+0x20` and `+0x38`) through query bits `0x04` and `0x08`.

This proves that the runtime can combine at least two light-source pools under the same category filtering.

It does **not yet** prove that `_effect.pac` is the producer of the second list. That relation remains a dedicated next reverse target: trace registrations into `CLightMgr +0x38` back to effect runtime / SEF / stage effect commands.
