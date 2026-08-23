# GDSpaces L1 Pass 89 — storage DDS header ↔ runtime TM2 correlation

Date: 2026-08-23  
Scope: **Layer 1 — Resource Materialization only**  
Status: **corpus-confirmed correlation; executable materializer mapping remains open**

## Why this pass exists

Passes 87–88 recovered the runtime-facing PTX/TM2 envelope and the embedded DDS bridge. The remaining boundary was phrased broadly as “storage → runtime materializer”. The preserved Phase 16 PAC corpus now narrows that boundary substantially.

The key correction is that the DDS-bearing storage representation is not an arbitrary DDS collection with unrelated framing. For a 50-entry aligned cohort it already uses the same outer allocation geometry as the recovered runtime PTX path:

```text
u32 count                 @ +0x00
u32 blockCount[count]     @ +0x04
entry area                @ +0x800
entry allocation          = blockCount[i] * 0x800
```

Inside each aligned storage entry, the DDS begins at a stable `entry + 0x70`.

This does **not** prove that the storage representation is retail-original TM2. No `TM2\0` signature occurs in these five Phase 16 PAC samples, and Pass 88 still requires executable data-flow evidence before storage and runtime identities can be unified.

## Evidence population

The Phase 16 sample analysis contains 91 DDS signatures and zero TM2 signatures across five PAC samples:

| sample | DDS | TM2 | aligned `0x70`-header cohort |
|---|---:|---:|---:|
| `st001.pac` | 17 | 0 | 17 |
| `id100.pac` | 21 | 0 | 21 |
| `pl000.pac` | 4 | 0 | 4 |
| `em000.pac` | 19 | 0 | 8 |
| `m09_b01.pac` | 30 | 0 | 0 |
| **total** | **91** | **0** | **50** |

`m09_b01.pac` and part of `em000.pac` contain an opaque multi-DDS variant and are deliberately excluded from the aligned-entry claims below.

## 50/50 byte-level invariants

For every aligned entry in the cohort:

1. DDS starts at entry-relative **`+0x70`**.
2. `u16 +0x10` / `u16 +0x12` equal DDS width / height.
3. `u32 +0x08 == 0x00020000 | (DDS mipCount << 8)` for the observed mip counts `0, 1, 8, 9`.
4. `u32 +0x64 == u32 +0x38 + 0x80`.
5. The `+0x64` value equals the complete DDS byte count represented by the entry; `+0x38` is that count minus the legacy `0x80` DDS header. The arithmetic relation is byte-confirmed; the semantic labels remain corpus-derived until executable consumers are found.
6. `u16 +0x44/+0x46 == width/2,height/2`.
7. `f32 +0x48/+0x4C == 2/width,2/height`.
8. `blockCount == ceil((0x70 + u32[+0x64]) / 0x800)`.

For `st001.pac` slot 1 specifically, `count=17`, the block table starts at `+0x04`, the first entry begins at `+0x800`, and the first DDS begins at `0x800 + 0x70 = 0x870` within the slot. This matches the recorded DDS signature position exactly.

## Correlation with Pass 88 runtime TM2 ABI

Pass 88 independently recovered these runtime reads:

```text
TM2 +0x08      ddsRelativeOffset
TM2 +0x3C      ddsByteSize
TM2 +0x50..67  24-byte metadata copied into gfxTexture
TM2 +0x58      width (u16)
TM2 +0x5A      height (u16)
```

That produces three high-value acquisition hypotheses:

### H89.1 — storage `+0x64` → runtime TM2 `+0x3C`

The storage field is the exact complete DDS byte count required by the recovered runtime field.

**Status:** strong corpus/runtime correlation, not instruction-confirmed.

### H89.2 — runtime TM2 `+0x08 = 0x70`

All 50 aligned storage entries place DDS at `+0x70`, exactly the quantity the runtime TM2 bridge expects as a relative DDS pointer displacement.

**Status:** strong corpus/runtime correlation, not instruction-confirmed.

### H89.3 — storage `+0x08..+0x1F` → runtime TM2 `+0x50..+0x67`

Both ranges are exactly 24 bytes. Under this mapping, storage width/height at `+0x10/+0x12` land exactly at runtime TM2 `+0x58/+0x5A`, which Pass 88 already proved are width/height before the block is copied into `gfxTexture`.

**Status:** high-value mapping hypothesis; needs a copy/remap data-flow proof.

## Narrowed materializer model

The evidence now supports a much narrower model than “decode arbitrary DDS and build an unrelated TM2 object”. A plausible boundary is a fixed-size **`0x70` header canonicalization** around a DDS payload that can remain at the same relative location:

```text
storage entry
  [0x70-byte storage header]
  [DDS bytes]

        ↓ materializer / adapter — still unproven

runtime entry
  [0x70-byte TM2 runtime header]
  [same logical DDS byte image]
```

This could be in-place, temporary-buffer, or separately allocated. The ownership mode is not yet known, so none is promoted.

## Exact next reverse targets

The next acquisition pass should trace the source pointer and destination pointer through:

- `0x140314E00` — CPtxManager load variant A;
- `0x140314FA0` — CPtxManager load variant B;
- `0x140336A70` — PTX bundle parse variant B;
- `0x140336BB0` — PTX bundle parse variant A;
- `0x1403366E0` — PTX runtime entry builder;
- `0x1402F11C0` — loaded-resource registration/coordinator candidate.

The decisive evidence is not another magic hit. We need one or more of the following exact data flows:

```text
source +0x64  -> runtime +0x3C
0x70          -> runtime +0x08
source +0x08..+0x1F -> runtime +0x50..+0x67
```

and the branch/discriminator that chooses this path for the DDS-bearing storage representation.

## Hard freeze

Until that instruction evidence exists:

- do not rename the storage header to TM2;
- do not implement a DDS→TM2 writer from these correlations;
- do not state that retail storage→runtime materialization is closed;
- do not treat runtime/synthetic envelope tests as original-game write compatibility;
- preserve the opaque multi-DDS variant as a separate unresolved representation.

## Promotion boundary

Pass 89 closes a **search-space problem**, not the materializer itself.

Before:

```text
91 DDS on disk / 0 TM2
        ?
TM2 -> DDS runtime bridge
```

After this pass:

```text
0x800-block storage bundle
  -> 0x70-byte DDS-bearing entry header
  -> exact field correlations
  -> [small executable adapter boundary still to recover]
  -> 0x70-byte runtime TM2 entry header
  -> DDS bridge
```

The next promotion requires executable instruction evidence for that adapter boundary.
