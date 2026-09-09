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

## Object flag `0x00100000`: exact GS state selector

Цей source bit більше не є просто “render flag with unknown effect”. Canonical object-state path доводить exact legacy GS projection для активного low-nibble mode (окрім special mode 4).

`0x140302640` формує packet, який потім ідентифікований як GS A+D descriptor із register targets:

```text
ZBUF_1  register 0x4E
TEST_1  register 0x47
ALPHA_1 register 0x42
PRIM    register 0x00
```

Для `0x00100000`:

```text
source bit clear:
  TEST_1 = 0x5000D
  AREF   = 0
  ZBUF_1.ZMSK = 1   // depth writes masked

source bit set:
  TEST_1 = 0x5010D
  AREF   = 16
  ZBUF_1.ZMSK = 0   // depth writes enabled by this selector
```

В обох випадках:

```text
ATE  = 1
ATST = GREATER
ZTE  = 1
ZTST = GEQUAL
```

Тому технічний semantic — **legacy GS alpha-test-reference/depth-write-state selector**. Не називай його `cloth`, `transparent`, `alpha material` або іншою artistic категорією без окремого evidence.

## Object flag `0x00200000`: carried, але не interpreted у двох audited renderer paths

Source bit21 переноситься в baseline/effective runtime flags і може бути відновлений із baseline state. Confirmed GS packet builder `0x140302640` його не тестує і не витягує арифметично.

Follow-up whole-model census додав незалежний material path:

```text
runtime object +0x14 effective flags
 -> read @ 0x1402F9ED9
 -> call 0x1402F9890

runtime object +0x14 effective flags
 -> read @ 0x1402FA042
 -> call 0x1402F9890
```

Обидва paths передають **цілий dword** effective flags у common material helper. Але direct disassembly `0x1402F9890` показує тільки один interpreted flag mask:

```text
0x00004000 -> legacy GS TEX1 filtering selector
```

`0x00200000` там не тестується, не shift-иться і не проектується. Окремо `0x1402F28E0` читає effective `+0x14` і ставить bit17 (`0x00020000`), зберігаючи існуючий bit21 без його semantic interpretation.

Отже bounded evidence тепер сильніший:

```text
baseline/effective carriage         EXE_CONFIRMED
state restoration participation     EXE_CONFIRMED
local GS helper bit21 use           EXE_CONFIRMED: none
common material helper bit21 use    EXE_CONFIRMED: none
external effective-word mutator     preserves bit21, does not interpret it
high-level semantic                 PRESERVED_UNDECODED
```

Equal mask `0x00200000` в manager `+0xE0` або runtime object `+0x304` не означає спільну semantic provenance. Поки не доведений copy/derivation edge, high-level meaning source bit21 лишається `PRESERVED_UNDECODED`.

Критично: “два renderer paths не читають bit21” не означає “bit21 globally unused”. Для semantic promotion усе ще потрібен provenance-confirmed terminal consumer або доведений derivation edge в інший runtime domain.

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

Production texture/material replacement authority ще не закрита. Навіть для EXE-confirmed GS selector треба знати повну serialization/reintegration policy, а для texture replacement додатково потрібні complete-enough TIM2 semantics, companion writer, MOD/companion coherence, reopen, reintegration і original-game acceptance.
