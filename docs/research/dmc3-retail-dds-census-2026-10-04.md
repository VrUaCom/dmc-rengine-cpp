# DMC3 retail DDS corpus census — 2026-10-04

## Authority

Source archive:

- `DMC 3 RENGINE (6).zip`
- size: 237,658,858 bytes
- SHA-256: `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`

The scan validates standard DDS headers and exact complete DXT1/DXT5 mip payload lengths. Duplicate appearances inside parent PAC images and extracted children are retained as occurrences, then deduplicated by exact DDS SHA-256 for the unique-image census.

## Population

- valid DDS occurrences across the archive: **409**
- standalone extracted `.dds` files: **243**
- unique DDS images by SHA-256: **154**
- sum of byte sizes of 154 unique DDS images: **24,872,432 bytes**
- average unique DDS size: about **161,509 bytes**

The 409 count is not a count of simultaneously resident textures. The archive intentionally contains parent containers plus extracted descendants, so the same DDS can occur more than once.

## Compression

Unique-image population:

- DXT5: **94**
- DXT1: **60**

## Dimension distribution

| dimensions | unique DDS |
|---|---:|
| 128x64 | 6 |
| 128x128 | 17 |
| 256x128 | 13 |
| 256x256 | 46 |
| 256x512 | 11 |
| 512x256 | 4 |
| 512x512 | 53 |
| 1024x1024 | 3 |
| 1024x2048 | 1 |

Only one unique DDS in this corpus exceeds 1024 on either axis.

## Mip counts

| mip levels | unique DDS |
|---:|---:|
| 8 | 23 |
| 9 | 59 |
| 10 | 68 |
| 11 | 3 |
| 12 | 1 |

Every entry in this bounded census carries the expected full chain for its dimensions.

## Retail maximum specimen

Largest width/height area, largest byte size and largest mip count all belong to:

`analysis_inputs/stage_drops/plwp_grenade/plwp_grenade_000/plwp_grenade_000_000.dds`

Properties:

- 1024 x 2048
- DXT5
- 12 mips
- 2,796,368 bytes including the 128-byte DDS header
- SHA-256 `00f03a616da7e8ae2a44f5a9a386a21b705ff74118a64bd72734504db23e40c1`

Status: CORPUS_CONFIRMED.

The largest 1024x1024 images in this corpus are DXT1 and occupy 699,192 bytes each.

## Texture-set counts in the preserved extraction tree

The archive contains 243 standalone DDS children grouped under 77 immediate parent directories.

Largest observed sibling sets:

- 17 textures — multiple stage texture sets, including `st001_001` and `m20_s00_004_001`;
- 16 textures — `m20_b00_004_001`, `m20_c00_004_001`, `st445_001`;
- smaller observed sets include 8, 6, 4, 3, 2 and 1.

The largest 17-texture sibling set by aggregate encoded DDS bytes in this corpus is `m20_s00_004_001` at 3,541,440 DDS bytes.

This is CORPUS_CONFIRMED extraction-tree evidence. It is **not** promoted as the runtime maximum number of textures, because PTX record fanout, mip/source-record allocation and simultaneous residency are separate runtime domains.

## Comparison against canonical HD loader

Fresh canonical-EXE reverse establishes a DMC3-source-proven Texture2D DDS loader guard of 16,384 x 16,384.

Therefore:

```text
retail observed maximum axis: 2,048
canonical HD loader maximum axis: 16,384
```

The retail content uses only a small portion of the representable/loader dimension domain. This large gap is exactly why Rengine must keep separate labels for:

- retail observed maximum;
- executable hard loader limit;
- product authoring safety;
- practical tested modding limit.

## Current product implication

The existing `Dmc3DdsSafety.max_dimension = 1024` cannot be described as a corpus-wide maximum because one hash-bound retail DDS is 1024x2048.

Do not automatically set the authoring limit to 16,384. A safe replacement policy should be chosen only after original-game stress and residency tests.

## Next census expansion

This receipt covers the exact historical v6 archive above. The next expansion should add the complete game/NBZ population and retain source lineage so counts can be reported separately for:

- stage;
- enemy;
- player/weapon;
- effect;
- UI;
- other texture families.

That wider census is required before calling 1024x2048 the maximum texture used anywhere in the complete DMC3 HD distribution.
