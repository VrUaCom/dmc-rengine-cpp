# Урок 2 — Document, objects і meshes

## Document header: `0x40`

Канонічний shared model document shell:

| Offset | Type | MOD meaning |
|---|---:|---|
| `+0x00` | char[4] | `MOD ` magic |
| `+0x04` | f32 | version, retail corpus uses ~`1.01` |
| `+0x10` | u8 | outer object count |
| `+0x11` | u8 | node/transform-domain count |
| `+0x12` | u8 | serialized texture-domain mirror |
| `+0x13` | u8 | MOD `default_joint_index` |
| `+0x14` | u32 | runtime-carried, semantics unresolved |
| `+0x20` | u64 | node-domain block offset |
| `+0x40` | — | outer object table begins |

`+0x12` — не live texture authority. Runtime domain size приходить із companion.

`+0x13` EXE-confirmed: serialized byte переноситься в manager `+0xFA`, використовується як `JntNo` fallback і як index у `currentWorld[]`. Не доведено, що це завжди root.

`+0x14` переноситься в manager `+0xE4`, але MOD-specific downstream meaning не встановлено. Не переносити сюди SCM `LegacyResourceCode`.

## Outer object: `0x40`

| Offset | Type | Meaning/status |
|---|---:|---|
| `+0x00` | u8 | child mesh count |
| `+0x01` | u8 | raw alpha/control |
| `+0x02` | u16 | aggregate element count |
| `+0x08` | u64 | mesh table offset |
| `+0x10` | u32 | source flags |
| `+0x18` | f32 | live render parameter for some flags, artistic name open |
| `+0x1C` | u32 | live render parameter for some flags, artistic name open |
| `+0x30` | f32[3] | bounding center |
| `+0x3C` | f32 | bounding radius |

### Object flags

Direct runtime projection already відновлена. Наприклад `0x4000` дає nearest-filter material signal; `0x200/0x400` активують manager bit21 і копіюють `+0x18/+0x1C`; інші підтверджені bits проектуються у runtime object flags. Але ми не даємо їм artistic names без downstream proof.

## Inner mesh: `0x50`

| Offset | Type | Meaning |
|---|---:|---|
| `+0x00` | u16 | element count |
| `+0x02` | u16 | texture slot |
| `+0x04` | u16 | GS CLAMP MINU |
| `+0x06` | u16 | GS CLAMP MAXU |
| `+0x08` | u16 | GS CLAMP MINV |
| `+0x0A` | u16 | GS CLAMP MAXV |
| `+0x0C` | u32 | preserved undecoded |
| `+0x10` | u64 | positions offset |
| `+0x18` | u64 | normals offset |
| `+0x20` | u64 | UV offset |
| `+0x28` | u64 | blend-index stream |
| `+0x30` | u64 | packed weight/topology stream |
| `+0x38` | u64 | preserved undecoded |
| `+0x40` | u64 | generated topology workspace relative offset |
| `+0x48` | u32 | generated topology count |
| `+0x4C` | u32 | preserved undecoded |

## Streams

- position: `float3`, stride 12;
- normal: `float3`, stride 12;
- UV: `int16x2`, stride 4, scale `4096`;
- blend indices: `u8x4`;
- control: `u16`.

Recovered source streams normally мають `0x10` alignment. Canonical parser попереджає про deviation, а не переписує raw bytes.

## Relative coordinate spaces

Не всі offsets мають однакову базу:

- більшість mesh streams post-load relocates як `resourceBase + relative`;
- mesh `+0x40` post-load обчислюється як `meshRecordAddress + relative`.

Це один із типових writer bugs: правильне числове значення з неправильною coordinate base дає валідний-looking, але зламаний resource.

## Unknown fields

`+0x0C`, `+0x38`, `+0x4C` мають typed preservation view. Вони не consumed у підтвердженому runtime mesh path, але це не доводить, що вони padding або globally unused.
