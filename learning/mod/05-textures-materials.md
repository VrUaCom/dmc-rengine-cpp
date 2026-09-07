# Урок 5 — Textures, companion і material runtime

## MOD не містить bitmap texture як mesh payload

Mesh зберігає:

```text
texture_slot @ +0x02
GS CLAMP REGION_REPEAT @ +0x04..+0x0A
```

Сам texture image приходить із зовнішнього model texture companion.

## Texture companion envelope

Recovered model-family companion:

```text
+0x000 u32 texture_count
+0x004 u32 block_count[texture_count]
payload base = 0x800
block size   = 0x800
```

Payload для entry займає:

```text
block_count * 0x800
```

Confirmed payload magic:

```text
TM2\0
```

Parser fail-closed на truncated table, overflow, truncated allocation і bad TM2 magic.

## Runtime chain

```text
manager +0x110 source companion
 -> 0x140304B30 materializer
 -> intermediate state (+0x120 involved)
 -> 0x14030D040 runtime descriptor builder
 -> manager +0x118 descriptor table
 -> mesh.texture_slot * 0x40
 -> 0x1402F9890 material helper
```

## Header mirror vs authority

MOD header `+0x12` — serialized mirror.

Companion `texture_count` — runtime-authoritative domain size.

Тому canonical validation:

```text
mesh.texture_slot < companion.texture_count
```

Mirror mismatch треба показати як consistency warning, але не вигадувати, що retail game обов’язково fatal-exits.

## Runtime texture descriptor: `0x40`

Це in-memory ABI:

```text
+0x00 pointer/source state
+0x08 pointer/source state
+0x10 pointer/source state
+0x18 pointer/source state
+0x20 GS TEX0 qword
+0x28 GS MIPTBP1 qword
+0x30 mip_record_count - 1
+0x34 base-width carry
+0x38..+0x3F unpromoted
```

`0x1402F9890` прямо consumes TEX0.

## GS CLAMP

Mesh `+0x04/+0x06/+0x08/+0x0A` — `MINU/MAXU/MINV/MAXV`.

Це legacy PS2 GS `CLAMP REGION_REPEAT` state. Register fields мають 10-bit domain. Якщо serialized raw u16 > `0x03FF`, parser зберігає raw value і warning; не mask.

## Не плутай representation layers

```text
MOD texture_slot
!= texture companion entry
!= TM2 internal image
!= DDS
!= PTX bundle
!= runtime 0x40 descriptor
```

DMC Rengine може мати читачі/перетворення для DDS/PTX, але це інші resource formats.

## Writer boundary

Production texture replacement authority ще не закрита, бо для неї потрібні complete-enough TIM2 semantics, companion writer, MOD/companion coherence, reopen, reintegration і original-game acceptance.
