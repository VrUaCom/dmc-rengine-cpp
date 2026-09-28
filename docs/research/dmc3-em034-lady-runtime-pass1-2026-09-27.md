# DMC3 HD — em034 Lady runtime reconstruction — pass 1

Date: 2026-09-27  
Branch: `reverse/em034-lady-runtime-20260927`  
Canonical executable SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Scope

This pass records only evidence recovered from the canonical DMC3 HD executable and the retained retail `em034` corpus. It does not promote guessed weapon names, hand/back semantics, or attachment meaning beyond the observed runtime behavior.

## CEm034 runtime anchors

**EXE_CONFIRMED**

- RTTI: `0x1405D68E8`
- constructor: `0x140168220`
- destructor/deleting destructor: `0x140168600` / `0x140168840`
- allocation size: `0x5A40`
- primary vtable: `0x1404D80E8`
- secondary vtables: `0x1404D8168`, `0x1404D8260`, `0x1404D8280`, `0x1404D8338`, `0x1404D84A0`
- vptr object offsets: `+0x0`, `+0x60`, `+0xD0`, `+0x110`, `+0x180`, `+0x250`
- factory: `0x14016EB80`
- factory registration: `0x140326310`, class selector `edx=6`
- incoming resource pointer: `+0x7E0`
- incoming matrix domain: `+0x7A0..+0x7D0`

`0x14016FBF0` is the PAC physical child resolver used by this path. For the ordinary PAC layout it reads the count at raw `+4` and slot offsets from the table beginning at raw `+8`.

## CEm034 top-level model ownership

**EXE_CONFIRMED**

The factory directly resolves and creates runtime node arrays for these top-level MOD slots:

| PAC slot | CEm034 field |
|---:|---:|
| 1 | `+0x7E8` |
| 17 | `+0x8A0` |
| 20 | `+0x8E8` |
| 21 | `+0x900` |
| 22 | `+0x908` |
| 23 | `+0x910` |
| 24 | `+0x930` |

The node allocator helper is `0x140168150`; the MOD node count comes from header byte `+0x11`.

## Retail MOD observations

**CORPUS_CONFIRMED**

| slot | bytes | nodes | header +0x14 |
|---:|---:|---:|---:|
| 1 | 129280 | 23 | 900 |
| 17 | 19664 | 9 | 900 |
| 20 | 38720 | 3 | 717400 |
| 21 | 22048 | 1 | 717400 |
| 22 | 23648 | 1 | 717400 |
| 23 | 23904 | 4 | 717400 |
| 24 | 34000 | 1 | 717400 |
| 25 | 2800 | 1 | 717400 |
| 26 | 8800 | 2 | 717400 |
| 30 | 3744 | 5 | 717400 |
| 32 | 141968 | 23 | 1597 |
| 34 | 19536 | 9 | 1597 |

## Appearance selection

**EXE_CONFIRMED**

`CEm034+0x5A2F` selects the appearance:

- false: body slot 1, hair slot 17, equipment PTX slot 19
- true: body slot 32, hair slot 34, equipment PTX slot 33

The same main equipment MODs, slots 20–24, are used for both appearances.

Observed controller domains:

- body: `+0x940`
- hair: `+0x10C0`
- slot 20: `+0x1840`
- slot 21: `+0x1FC0`
- slot 22: `+0x2740`
- slot 23: `+0x2EC0`
- slot 24: `+0x3640`

This promotes the costume split and the slots 20–24 PTX 19/33 choice to EXE evidence. It does **not** promote the previous Reader pairing of slots 25/26 with PTX 33.

## Main equipment attachment state tables

The attachment setter is `0x1401713F0(this, index, mode)`.

**EXE_CONFIRMED**

- index 0 -> slot 20
- index 1 -> slot 21
- index 2 -> slot 22
- index 3 -> slot 23
- index 4 -> slot 24
- mode 0 table: `0x14057AFC0`
- mode 1 table: `0x14057B0B0`
- record stride: `0x30`
- each record stores translation vec4, rotation XYZ vec4, and host-joint byte

### Mode 0 / default table

| index | slot | joint | translation | rotation XYZ radians |
|---:|---:|---:|---|---|
| 0 | 20 | 3 | (-2,-20,-17) | (-1.57079625,0,1.08210409) |
| 1 | 21 | 14 | (-1,-4,13) | (1.86750221,0.048869215,2.40855432) |
| 2 | 22 | 16 | (-9.2,-13,-9.1) | (0,0,1.65806270) |
| 3 | 23 | 19 | (10,-15,-2.5) | (0,0,-1.65806270) |
| 4 | 24 | 14 | (17,-5,-16) | (-1.22173047,-0.23561944,0.62831849) |

### Mode 1 / alternate table

| index | slot | joint | translation | rotation XYZ radians |
|---:|---:|---:|---|---|
| 0 | 20 | 9 | (-8.4,-1.0,-1.3) | (0,0,pi) |
| 1 | 21 | 9 | (-7.5,-0.6,-0.8) | (0,0,0) |
| 2 | 22 | 13 | (7.7,-0.8,0.5) | (0,0,pi) |
| 3 | 23 | 13 | (7.2,-1.2,2.7) | (0,-0.17453292,0) |
| 4 | 24 | 13 | (7.2,-0.8,-0.4) | (0,0,pi) |

The tables prove two placement modes. The semantic labels “hand”, “back”, “holster” remain **PRESERVED_UNDECODED** until the consumer meaning is closed.

Observed calls to `0x1401713F0` include mode changes for indices 0, 1, 2, 3 and 4 in the CEm034 update path.

## Motion-script channel semantics

The canonical script controller uses:

- interpreter: `0x140058FE0`
- controller byte accessor: `0x140059350`
- clear accessor: `0x140058FB0`
- controller channel bytes: `+0x91 + object*5 + channel`

Opcode 3 is six bytes. Its five payload bytes are copied to the five controller channels.

**EXE_CONFIRMED**

Therefore the existing Reader abstraction that reduces opcode 3 to only `p+2 & 0x3F` is incomplete for CEm034. Lady consumes multiple channel bytes.

## em034_012.bin

**EXE_AND_CORPUS_CONFIRMED**

- physical slot: 12
- size: 7872
- enemy/direct script form
- bank script counts: `[15,27,4,37,70]`

CEm034 initializes the same script twice:

- controller `CEm034+0x5070`, resource object selector 0
- controller `CEm034+0x5190`, resource object selector 1

Both target the body controller/node domain:
- controller/resource anchor around `CEm034+0x948`
- body node array `CEm034+0x7E8`

For bank 4 actions 13, 44, 46 and 50 the motion-resource rows contain two records, object 0 and object 1, pointing to the same MOT id. These are independent script object lanes of the same body script.

Observed opcode-3 channel streams:

- action 13: after frames 2/3/5/8/14/17/20 -> channel changes including `(1,0,0,0,0)` and `(0,1,0,0,0)`
- action 44: frames 14/21/28 -> `(1,1,0,0,0)`, `(1,0,0,0,0)`, `(1,1,0,0,0)`
- action 46: frames 1/27/32/37/42/78 -> ch1 activation, repeated ch0+ch1 activation, then channel 2 activation
- action 50: frame 10 `(0,1,0,0,0)`; frame 25 `(1,0,0,0,0)`; frame 39 `(2,0,0,0,0)`; frame 64 `(0,2,0,0,0)`

The previous shorthand “action 50 state 1 -> 2” is **REJECTED** as a complete representation. It collapses independent channels.

## em034_013.bin — separate slot-20 script

**EXE_AND_CORPUS_CONFIRMED**

This is a second MotionScript resource:

- physical PAC slot: 13
- size: 512
- enemy/direct form
- bank counts: `[1,1,1,1,43]`

CEm034 resolves slot 13 and initializes a third controller at `CEm034+0x52B0`:

- resource object selector: 0
- direct/enemy table mode: 1
- controller/resource anchor: `CEm034+0x1848`
- node array: `CEm034+0x8E8`

The target node array is the slot-20 model. Therefore slot 13 is the script for the main slot-20 equipment object.

Bank 4 resource IDs map to MOT group 4. The resource slot set is exactly:

`{0,6,10,14,30,31,32,40,41,42}`

Retail nested PAC slot 11 contains exactly those MOT slots. Thus:

`em034_013 bank 4 -> top-level PAC slot 11 -> slot20 model`

is **EXE_AND_CORPUS_CONFIRMED**.

The current Native Reader only retains the first recognized top-level `MotionScriptFile`; this design cannot represent both slot 12 and slot 13 and is insufficient for canonical em034 playback.

## Dynamic CEm034Shl actors

The Shl classes are separately allocated and registered runtime child actors, not embedded CEm034 subobjects.

**EXE_CONFIRMED**

| class | factory | size | observed CEm034 factory call sites |
|---|---:|---:|---|
| CEm034Shl00 | 0x140172240 | 0x540 | 0x1401696F4, 0x140169CDE, 0x140171DD0 |
| CEm034Shl01 | 0x1401729D0 | 0x560 | 0x140169B64 |
| CEm034Shl02 | 0x140173620 | 0xD80 | 0x140169972 |
| CEm034Shl03 | 0x1401745F0 | 0x1540 | 0x14016CC41 |
| CEm034Shl04 | 0x140175210 | 0x540 | 0x14016A269, 0x14016CE66 |
| CEm034Shl05 | 0x140175B10 | 0x540 | 0x140169E8C |

### Slot ownership recovered

**EXE_CONFIRMED**

CEm034Shl02 initialization directly resolves:
- slot 19
- slot 25

and supplies the pair to its runtime controller. Therefore slot 25 belongs to the Shl02 runtime actor.

CEm034Shl03 initialization directly resolves:
- slot 19 + slot 26
- slot 29 + slot 30

Therefore slots 26 and 30 belong to the Shl03 runtime actor.

Consequences:

- slots 25/26/30 are **not** part of the persistent main slots20–24 attachment table;
- the previous Reader model that attaches all slots20–26/30 persistently to one body joint is **REJECTED**;
- exact semantic names for Shl02/Shl03 remain **PRESERVED_UNDECODED**.

## CPlWpLadyGun boundary

**EXE_CONFIRMED**

- RTTI: `0x1405D7568`
- ctor: `0x14022B8A0`
- delete/dtor: `0x14022B9F0`
- factory: `0x14022BA30`
- allocation size: `0xAC0`
- vtables: `0x1404E1338`, `0x1404E13B8`, `0x1404E14B0`
- factory registration class selector: `edx=5`
- byte `+0x112 = 9`
- byte `+0x113 = caller subtype`

CPlLady creates CPlWpLadyGun with subtypes 9 and 10 and stores the resulting runtime weapon pointers in CPlLady-owned arrays around `+0x64B0`.

No direct `CEm034 -> CPlWpLadyGun` factory edge has been found. The earlier working assumption that enemy Lady directly constructs this player-Lady weapon class is therefore **REJECTED**. CPlWpLadyGun remains useful comparative evidence, but cannot be used as direct CEm034 authority without a proven edge.

## Reader corrections implied by pass 1

Do not implement yet as a guessed approximation. Required architecture for the next Reader pass:

1. Main static CEm034 equipment = slots20–24 only.
2. Preserve both EXE-confirmed attachment modes per slot.
3. Replace the single scalar Lady weapon state with the canonical multi-channel script signal model.
4. Retain multiple actor motion scripts in one assembled PAC:
   - slot12 body script, object lanes 0 and1;
   - slot13 slot20 equipment script.
5. Bind slot13 bank4 to nested PAC11 MOTs.
6. Treat slots25,26,30 as dynamic Shl actor resources, not permanent body attachments.
7. Keep semantic visibility/spawn labels unresolved until the update consumers are closed.
8. Do not build the next APK until the remaining state/visibility/lifecycle pass is complete.

## Remaining pass-2 targets

- map every CEm034 consumer of slot12 channel 0/1/2;
- map `0x14005A1F0` calls that start/synchronize the slot13 controller;
- recover the exact relationship between body action state and slot20 bank4 action selection;
- map Shl00..05 spawn/destruction conditions to script channels;
- recover visibility enable/disable paths for slots20–24;
- only then assign human semantic labels (pistol, launcher, hook, SMG, cable, hand/back).


## Pass 2A — exact CEm034 state/action/channel routing

### +0x250 state-machine vtable

**EXE_CONFIRMED**

The CEm034 subobject at `+0x250` uses vtable `0x1404D84A0`.

Relevant entries:

```text
vtable +0x38 -> 0x140169060  state/update consumer
vtable +0x40 -> 0x14016AD40  state-change thunk
```

`0x14016AD40` adjusts `this` by `-0x250`, forces the third argument to
`-1`, and jumps to `0x14016A410`.

`0x14016A410(CEm034*, state, selector)` stores the new state at
`CEm034+0x5990` and starts the matching script action. The helper
`0x140171AB0` interprets selector `-1` as **start both slot12 controllers**:

- `CEm034+0x5070` — object lane 0;
- `CEm034+0x5190` — object lane 1.

Therefore ordinary virtual state changes through the CEm034 state-machine
subobject execute both object lanes of `em034_012.bin`.

This closes the previous action-selection uncertainty for states reached through
that vtable path.

### Exact bank-4 state/action mapping

**EXE_CONFIRMED**

The state selector in `0x14016A410` maps the target Lady states to bank 4:

| CEm034 state | slot12 bank/action | additional entry behavior |
|---:|---|---|
| 96 | bank4 action13 | component0/slot20 preset1; component4/slot24 preset1 |
| 127 | bank4 action44 | component1/slot21 preset1; component2/slot22 preset1 |
| 129 | bank4 action46 | component3/slot23 preset1 |
| 133 | bank4 action50 | no placement write required at entry |

Other recovered bank-4 mappings include:

- states83..89 -> actions0..6;
- state94 -> action11;
- state95 -> action12;
- state97 -> action14;
- state98 -> action15;
- state99 -> action16;
- states100..115 -> actions17..32;
- state143 -> action60.

States90..93 and 100..115 also start the slot13 controller at
`CEm034+0x52B0` with the corresponding bank4 action, which is the
slot20-specific MotionScript path recovered earlier.

### Action 13 / state 96

**EXE_AND_CORPUS_CONFIRMED structural routing**

State96 uses update target `0x1401699D7` and reads the second slot12 controller
(`CEm034+0x5190`).

- opcode3 byteIndex0 == 1:
  - clears that byte;
  - when the internal count gate permits, calls `0x140171C70(this,0)`.
- opcode3 byteIndex1 == 1:
  - clears that byte;
  - under its separate internal gate, enters the event path ending at
    `0x140338940`.

`0x140171C70` has now been structurally closed:

- it takes its source transform from the runtime object of **MOD slot24**
  (`CEm034+0x930`);
- it derives a position/orientation through the CEm034 body/runtime-node path;
- it creates **CEm034Shl00** through factory `0x140172240`.

Retail bank4/action13 has:

```text
frame 2   [1,0,0,0,0]
frame 3   [0,1,0,0,0]
frame 5   [1,0,0,0,0]
frame 8   [1,0,0,0,0]
frame 14  [1,0,0,0,0]
frame 17  [1,0,0,0,0]
frame 20  [1,0,0,0,0]
```

Thus the repeated byteIndex0 signals feed the Shl00 creation path sourced from
slot24. The gameplay/artistic name of Shl00 remains **PRESERVED_UNDECODED**.

### Action 44 / state 127

**EXE_AND_CORPUS_CONFIRMED structural routing**

State entry moves:

- slot21/component1 -> preset1;
- slot22/component2 -> preset1.

Update target `0x140169B90` consumes the second slot12 controller:

- byteIndex0 == 1:
  - clears it;
  - enters a transform/setup path that creates **CEm034Shl00**
    via factory `0x140172240`.
- byteIndex1 controls an internal branch flag at the state-machine subobject
  `+0x57D7`, selecting between two runtime source paths in the following
  operation.

Retail action44:

```text
frame14 [1,1,0,0,0]
frame21 [1,0,0,0,0]
frame28 [1,1,0,0,0]
```

Therefore all three byteIndex0 events enter the Shl00 path, while byteIndex1
selects the secondary branch state.

No gameplay name is assigned to that branch yet.

### Action 46 / state 129

**EXE_AND_CORPUS_CONFIRMED structural routing**

State entry moves slot23/component3 -> preset1.

Update target `0x140169D7C` consumes the second slot12 controller:

- byteIndex0 == 1:
  - clears it;
  - builds a child transform;
  - creates **CEm034Shl05** via factory `0x140175B10`.
- byteIndex1 == 1:
  - writes runtime float `1.5` to the state-machine field corresponding to
    `CEm034+0x43?0` / subobject-relative `+0x41B0`;
  - the exact gameplay label remains **PRESERVED_UNDECODED**.
- byteIndex2 == 1:
  - clears it;
  - moves slot23/component3 -> preset0.

Retail action46:

```text
frame 1  [0,1,0,0,0]
frame27  [1,1,0,0,0]
frame32  [1,1,0,0,0]
frame37  [1,1,0,0,0]
frame42  [1,1,0,0,0]
frame78  [0,0,1,0,0]
```

This closes the structural sequence:

```text
state129 entry -> slot23 preset1
frames27/32/37/42 -> Shl05 creation
frame78 -> slot23 preset0
```

### Action 50 / state 133

**EXE_AND_CORPUS_CONFIRMED**

Update target `0x140169F2A` consumes the second slot12 controller.

byteIndex1 is the exact placement switch for slot24/component4:

```text
value 1 -> slot24 preset1
value 2 -> slot24 preset0
```

Retail action50:

```text
frame10 [0,1,0,0,0] -> slot24 preset1
frame25 [1,0,0,0,0]
frame39 [2,0,0,0,0]
frame64 [0,2,0,0,0] -> slot24 preset0
```

byteIndex0 has a separate lifecycle path:

- value1 clears the signal and arms/starts an internal state which can enter
  `0x140171C70(this,1)`;
- value2 clears/resets that state.

Since `0x140171C70` is the slot24-sourced Shl00 creator, action50 has both a
slot24 placement channel and a separate Shl00 lifecycle channel.

The old single scalar interpretation “weapon state 1 -> 2” is therefore
**REJECTED**.

## Pass 2A conclusions

The following are now closed:

- ✅ actions13/44/46/50 have exact CEm034 state numbers;
- ✅ ordinary state entry starts both slot12 object lanes;
- ✅ action13 byteIndex0 -> slot24-sourced Shl00 path;
- ✅ action44 byteIndex0 -> Shl00 path;
- ✅ action46 byteIndex0 -> Shl05 path;
- ✅ action46 byteIndex2 -> slot23 preset0;
- ✅ action50 byteIndex1 -> slot24 preset1/preset0;
- ✅ action50 byteIndex0 is a separate Shl00 lifecycle lane.

Still open:

- complete visibility enable/disable writes for slots20..24;
- exact semantic names of slots20..24;
- exact gameplay names of Shl00/Shl05;
- Shl01/Shl02/Shl03/Shl04 script/lifecycle conditions;
- human labels for placement preset0/preset1.

No Native Reader implementation is authorized from this section alone; the
remaining visibility/lifecycle gates must be closed first.
