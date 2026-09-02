# DMC3 em000 SO + hit-volume companion reverse — 2026-09-02

**Status:** EXE-CONFIRMED SLOT BINDINGS + CORPUS-CONFIRMED FORMATS + PARTIAL SEMANTICS  
**Branch:** `research/dmc3-primary-3d-format-abi-20260831`

## Scope

User-supplied adjacent `em000` payloads:

```text
slot_0038.bin  size 6144
slot_0039.bin  size   96
slot_0040.bin  size 1840
```

Canonical target: DMC3 HD `dmc3.exe`, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## 1. Corrections to earlier hypotheses

The direct EXE loader materially corrects several earlier corpus-only interpretations.

1. `slot_0038` first u16 values are **relative offsets**, not recursive type IDs. Earlier `TYPE 6 -> TYPE 8/10/4` wording is rejected.
2. `slot_0039` has **no 4-byte file header**. Its 96 bytes are exactly `24 * 4-byte` binding records.
3. `slot_0039 +2` is a **u16 index into slot_0040's 0x50 record table**, not a next/link byte.
4. `slot_0039 byte0` is a **flags bitfield**; EXE tests bits `0x1`, `0x2`, and `0x4` independently.
5. Physical slots 38, 39, and 40 are resolved directly by the canonical EXE from the PAC/PNST offset table.
6. `CSpreadBone` is a separate `0x40`-stride secondary-bone physics system and is not the slot39/40 subsystem.

These corrections supersede earlier notes in this research line.

## 2. Direct physical-slot proof from dmc3.exe

A canonical call site resolves the relevant PAC/PNST payloads directly by physical slot index.

PAC/PNST layout uses an offset table beginning at `+0x08`, so:

```text
slot 38 offset entry = 0x08 + 38*4 = +0xA0
slot 39 offset entry = 0x08 + 39*4 = +0xA4
slot 40 offset entry = 0x08 + 40*4 = +0xA8
```

The EXE checks the container count against `0x27/0x28/0x29`, reads these exact table positions and obtains the three raw pointers.

### slot39 + slot40 pairing

The engine passes:

```text
slot39 pointer -> RDX
slot40 pointer -> R8
actor hit-volume manager -> RCX
call 0x14005C260
```

`0x14005C260` stores:

```text
manager +0x108 = slot39 base
manager +0x110 = slot40 base
```

This is direct on-disk-to-runtime proof that slots 39 and 40 form one subsystem.

### slot38 loader

The immediately adjacent physical slot 38 pointer is passed to `0x1400594B0`, the large `.SO` runtime loader/controller.

## 3. slot_0039.bin — HitVolumeBinding table

Exact physical layout:

```text
96 bytes = 24 * 4-byte records
```

There is no header. The first bytes `06 00 00 00` are record 0.

Recovered structure:

```cpp
struct HitVolumeBinding { // 0x04 bytes
    uint8_t  flags;
    uint8_t  attachmentIndex;
    uint16_t volumeIndex;
};
```

### Direct consumer

Functions around `0x14005C630` / `0x14005C740` do:

```text
binding = slot39Base + index*4
volumeIndex = *(u16 *)(binding + 2)
volume = slot40Base + volumeIndex*0x50
attachmentIndex = binding[1]
```

The multiplication to reach the geometry record is explicit: `volumeIndex * 0x50`.

Therefore:

- `volumeIndex`: **EXE-CONFIRMED**.
- `attachmentIndex`: **EXE-CONFIRMED transform/attachment selector**; a bone/joint interpretation is very strong but exact owner vocabulary still needs the final object-table trace.
- `flags`: **EXE-CONFIRMED bitfield**.

### Flags

`0x14005C8D0` and downstream `0x1402CCD60` test the first byte independently with:

```text
bit 0 -> 0x01
bit 1 -> 0x02
bit 2 -> 0x04
```

These bits alter runtime hit-mask/category behavior. Exact canonical names for the bits remain open.

### em000 bindings

```text
#  flags attachment volume
0  0x6      0       0
1  0x2      3       1
2  0x1      9       2
3  0x4     17       3
4  0x4     20       4
5  0x2      3       5
6  0x3      3       1
7  0x0      0       0
8  0x0      0       0
9  0x0      0       0
10 0x1      9      10
11 0x1      0      11
12 0x1      0      12
13 0x1      9      13
14 0x1      9      14
15 0x1      0      15
16 0x1     13      16
17 0x1      0      17
18 0x4      9      18
19 0x4      9      19
20 0x4      9      20
21 0x1      9      21
22 0x1      9      22
23 0x0      0       0
```

This also proves binding count need not equal geometry-record count and that several bindings may reference the same geometry record.

## 4. slot_0040.bin — raw typed hit-volume geometry

Exact layout:

```text
1840 bytes = 23 * 0x50-byte records
```

The file is headerless. Each `0x50` record begins with its own raw shape type.

Runtime path:

```text
slot39.volumeIndex
  -> slot40Base + index*0x50
  -> 0x14005C8D0 stores raw-volume pointer in runtime hit object
  -> shape dispatchers consume raw record type
```

### Direct type dispatcher

`0x1402CC0E0` dispatches the raw type:

```text
type 2 -> 0x1402CC3F0
type 3 -> 0x1402CC110
type 4 -> 0x1402CC300
```

A broader dispatcher at `0x1402CC530` supports raw type IDs `0..6`, with distinct handlers for each. Current actor corpus primarily exposes 2, 3, and 4.

### Raw type 2 — sphere

The type-2 handler transforms the vector at raw `+0x10` as the center and consumes the scalar at raw `+0x20` as the size/radius term.

Representative `em000` record:

```text
center = (0, 50, 0, 1)
radius = 50
```

Semantic status: **sphere / VERY HIGH, direct EXE geometry semantics**.

### Raw type 3 — oriented box / OBB

Handlers `0x1402CC110` / `0x1402CC610` consume:

```text
+0x10..+0x18  center XYZ
+0x1C..+0x24  rotation XYZ
+0x28..+0x30  extents XYZ
```

The rotation values are converted and used to build orientation matrices before spatial tests.

Corpus examples include:

```text
center   = (0, 60, 100)
rotation = (25, 0, 0)
extents  = (50, 100, 50)
```

Semantic status: **oriented box / OBB, VERY HIGH / EXE-CONFIRMED geometry behavior**.

### Raw type 4 — capsule

Handlers `0x1402CC300` / `0x1402CC890` consume:

```text
+0x10  endpoint A vec4
+0x20  endpoint B vec4
+0x30  radius scalar
```

The runtime derives midpoint/axis geometry from the two endpoints and applies the radius term.

`em000` example:

```text
A      = (0, 231, 106, 1)
B      = (0,-120, 106, 1)
radius = 95
```

Semantic status: **capsule, VERY HIGH / direct EXE geometry behavior**.

### Other raw types

The engine has handlers for raw types `0`, `1`, `5`, and `6`, but these are not yet semantically mapped in this pass. Do not infer their names from the separate text parser enum.

### Important enum separation

The canonical text hit parser independently supports vocabulary:

```text
sphere
box
cylinder
capsule
hit_rot
hit_jnt
hitattr
charhit
camhit
```

Its text-parser enum numbering is different from the raw slot40 numbering. The vocabulary corroborates the primitive concepts but must not be numerically conflated with raw types 2/3/4.

## 5. slot_0038.bin — large SO offset-bank format

Historical extraction corpus still strongly identifies this family as `.so`, but direct EXE loading corrects its binary interpretation.

### File header

Raw first bytes:

```text
06 00 98 0B FF FF
```

`0x1400594B0` loads them as three u16 relative offsets:

```text
+0x00 -> 0x0006
+0x02 -> 0x0B98
+0x04 -> 0xFFFF  // absent optional bank/section
```

Therefore these values are **offsets, not type IDs**.

A safe file-level model is:

```cpp
struct SoHeader {
    uint16_t bankOffset[3]; // 0xFFFF means absent
};
```

### Bank A at file +0x0006

Starts:

```text
08 00 CC 09 18 0B FF FF
```

This is another relative-offset directory:

```text
+0x0008
+0x09CC
+0x0B18
+0xFFFF terminator
```

Relative to bank-A base `0x0006`, the three present subtables begin at:

```text
A0 = 0x000E
A1 = 0x09D2
A2 = 0x0B1E
```

### Bank B at file +0x0B98

Starts:

```text
08 00 8E 0A FE 0B FF FF
```

Present relative subtables:

```text
+0x0008
+0x0A8E
+0x0BFE
```

Absolute starts:

```text
B0 = 0x0BA0
B1 = 0x1626
B2 = 0x1796
```

### Subtable structure

Each subtable begins with its own u16 record-offset list terminated by `0xFFFF`, followed by variable-size compact records.

Recovered record counts for this `em000` sample:

```text
A0 @0x000E : 93 records
A1 @0x09D2 : 10 records
A2 @0x0B1E :  5 records
B0 @0x0BA0 : 78 records
B1 @0x1626 : 10 records
B2 @0x1796 :  5 records
```

Exact semantic names for the six subtables remain open; do not call them type4/type6/type8/type10.

## 6. SO is a runtime control/sequence program and directly drives the hit-volume subsystem

This is now direct EXE evidence, not only a corpus hypothesis.

`0x1400594B0` stores an optional linked runtime-subsystem pointer in the SO manager. At the actor call site that linked subsystem is the hit-volume manager initialized from slots 39 and 40.

### SO selects hit volumes

`0x140059650` consumes an index byte from the SO execution stream and calls `0x14005C840`.

`0x14005C840` performs the equivalent of:

```text
manager->currentVolume = manager->slot40Base + index*0x50
```

Thus an SO command can directly select a raw hit-volume descriptor.

### SO activates/builds hit volumes

`0x140059770` consumes IDs from SO records and invokes `0x14005C630`, which combines:

```text
slot39 HitVolumeBinding
+
slot40 raw geometry
+
attachment transform
```

to build/activate runtime hit objects.

Additional SO handlers around `0x140059820` and `0x1400598C0` manipulate active runtime hit instances/groups.

### ASCII opcode class

SO execution dispatchers around `0x140059270` / `0x140059CF0` read the first byte of compact records, subtract ASCII `'0'` (`0x30`) and dispatch through a jump table for opcode characters `'0'..'9'`.

This proves at least one SO record stream is a compact binary/script-like command language with ASCII numeric opcodes and opcode-specific record sizes.

Exact semantics of opcodes `0..9` remain open.

## 7. Historical corpus reconciliation

Retail-derived extraction manifests include:

```text
em035_011.so   5568
em035_012.ukn   144
em035_013.ukn  2880

pl011_005.so   73120
pl011_006.so    1008
pl011_007.ukn  20080
```

The corrected slot39 interpretation resolves previous count confusion:

```text
em000:  slot39  96/4  = 24 bindings; slot40 1840/80  = 23 volumes
em035:  compact 144/4 = 36 bindings; volume 2880/80 = 36 volumes
pl011:  compact 1008/4=252 bindings; volume 20080/80=251 volumes
```

This is consistent with a binding table that can contain disabled/default entries or multiple references to one geometry record.

Historical extensions are useful naming evidence but are not sufficient alone to infer magic or structure.

## 8. CSpreadBone correction

Direct relationship to slot39/40 is **REJECTED**.

`CSpreadBone` is independently EXE-confirmed as a secondary-bone physics system with:

```text
entry stride = 0x40
maximum entries = 44
runtime update writes back to bone matrices
```

It belongs to the secondary-physics research line, not this hit-volume pair.

## 9. Current actor subsystem model

```text
em000 actor package
|
+-- slot38: .SO control/sequence resource
|   +-- 3*u16 file-level relative-offset header
|   +-- bank A -> 3 present subtables
|   +-- bank B -> 3 present subtables
|   +-- each subtable: u16 record-offset list + variable records
|   +-- compact opcode execution, including ASCII '0'..'9'
|   +-- directly selects/activates hit volumes
|
+-- slot39: HitVolumeBinding[24]
|   +-- uint8 flags
|   +-- uint8 attachmentIndex
|   +-- uint16 volumeIndex
|
+-- slot40: RawHitVolume[23], stride 0x50
    +-- type2 sphere
    +-- type3 oriented box / OBB
    +-- type4 capsule
    +-- types0/1/5/6 supported by engine, semantics still open
```

This is no longer merely a structural correlation: the canonical EXE directly connects all three physical slots to the actor runtime.

## 10. Evidence status

- physical slot 38 -> SO loader `0x1400594B0`: **EXE-CONFIRMED**.
- physical slots 39/40 -> paired hit-volume manager `0x14005C260`: **EXE-CONFIRMED**.
- slot39 record size = 4 and no header: **EXE-CONFIRMED**.
- slot39 `+2` = u16 volume index: **EXE-CONFIRMED**.
- slot39 byte0 = flags bitfield: **EXE-CONFIRMED**.
- slot39 byte1 = attachment/transform selector: **EXE-CONFIRMED role**, exact object vocabulary open.
- slot40 stride = 0x50: **EXE-CONFIRMED**.
- raw type2 sphere: **VERY HIGH / direct geometry semantics**.
- raw type3 oriented box: **VERY HIGH / direct geometry semantics**.
- raw type4 capsule: **VERY HIGH / direct geometry semantics**.
- SO file header/bank/subtable offset structure: **EXE + DATA CONFIRMED**.
- SO directly drives hit-volume selection/activation: **EXE-CONFIRMED**.
- SO acronym expansion: **OPEN**.
- SO six-subtable semantic names: **OPEN**.
- SO ASCII opcode `0..9` meanings: **OPEN**.
- raw hit-volume types 0/1/5/6: **OPEN**.
- CSpreadBone direct relation: **REJECTED**.

## 11. Next reverse gates

1. Decode the SO ASCII opcode handlers `'0'..'9'` and recover exact command semantics.
2. Name the six SO subtables by tracing all consumer fields and cross-referencing actor state/action calls.
3. Resolve `HitVolumeBinding.flags` bits 0/1/2 to canonical hit categories.
4. Trace `attachmentIndex` all the way to skeleton/object ownership and lock bone-vs-object semantics.
5. Reverse raw hit-volume types `0`, `1`, `5`, and `6` from their direct geometry handlers.
6. Acquire authoritative `em000.index` if available to restore original extraction names for slots 38/39/40 without overwriting physical slot identity.
