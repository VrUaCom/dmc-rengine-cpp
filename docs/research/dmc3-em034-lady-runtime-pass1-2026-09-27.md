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


## Pass 2B — baseline placement reset, slot20 independent-script gate, and remaining Shl script edges

### 0x140168B00 is the five-component baseline reset

**EXE_CONFIRMED**

`0x140168B00(CEm034*)` rebuilds the placement objects for all five core
equipment components from the mode-0 table:

```text
slot20/component0 -> mode0 record
slot21/component1 -> mode0 record
slot22/component2 -> mode0 record
slot23/component3 -> mode0 record
slot24/component4 -> mode0 record
```

The bank-4 state-entry paths call this helper before applying any state-specific
mode-1 overrides. Therefore the state-entry contract is:

```text
start script action
-> reset components20..24 to canonical mode0 baseline
-> apply state-specific mode1 overrides
```

This is stronger than treating each component's current placement as an
independent sticky state.

### Correction: slot23/component3 mode1 has a special parent path

**EXE_CONFIRMED correction**

The mode-1 record for component3/slot23 contains node byte 13 in the serialized
placement table, but `0x1401713F0` does **not** use that body-node byte when
`componentIndex == 3 && mode == 1`.

Instead it sets the slot23 placement parent pointer directly to:

```text
CEm034 + 0x43C0
```

The `+0x43C0` transform is maintained by CEm034's runtime update path and is
seeded from the runtime transform associated with `CEm034+0x850`.

Therefore the earlier shorthand “slot23 mode1 -> body node13” is
**REJECTED** as an effective-parent description.

Correct interpretation:

- slot23 mode0 -> body node19 from the mode0 record;
- slot23 mode1 -> CEm034 internal transform `+0x43C0`;
- the mode1 table's node-byte 13 is present in data but bypassed by this
  component-specific code path.

No gameplay label is assigned to `+0x43C0` yet.

### Slot20 has two runtime control modes

**EXE_CONFIRMED**

Component0/slot20 is special because it has its own MotionScript controller
(`CEm034+0x52B0`, sourced from `em034_013.bin`).

The byte at `CEm034+0x4020` gates whether that controller is advanced by the
main CEm034 update:

```text
if CEm034+0x4020 == 0:
    update controller +0x52B0
else:
    skip controller +0x52B0
```

Every component0 path through `0x1401713F0` re-establishes a body-attached
placement and writes:

```text
CEm034+0x4020 = 1
```

Several CEm034 state entries instead write `+0x4020 = 0` and explicitly start
the slot13 MotionScript at the matching bank4 action.

Thus slot20 has two confirmed control domains:

1. body-placement domain through component0 / `0x1401713F0`;
2. independent slot13 MotionScript domain through controller `+0x52B0`.

Calling those modes “held/independent”, “launcher”, or similar remains
**PRESERVED_UNDECODED** until semantic identity is separately proven.

### Bank4 action40 / state123 closes the slot20 handoff

**EXE_AND_CORPUS_CONFIRMED structural routing**

CEm034 state123 maps to bank4/action40.

At state entry:

- both slot12 body-script lanes are started for action40;
- slot13 controller `+0x52B0` is started at bank4/action40;
- `CEm034+0x4020 = 0`, so the slot13 controller is advanced.

Retail action40 contains:

```text
frame4 -> opcode3 [1,0,0,0,0]
```

State123 update target `0x140169147` reads the first slot12 controller
(`+0x5070`) byteIndex0. When it sees value1 it:

1. clears that signal;
2. rebuilds slot20 using the exact component0 mode1 placement data:
   - translation `(-8.4,-1.0,-1.3)`;
   - rotation `(0,0,pi)`;
   - body node9;
3. rebinds/commits the slot20 runtime controller;
4. writes `CEm034+0x4020 = 1`.

This gives an exact script-driven handoff:

```text
state123/action40
 -> slot20 independent slot13-script mode
 -> frame4 body channel0=1
 -> slot20 component0 mode1 body placement
 -> stop advancing slot13 controller
```

No human weapon/hand semantic label is assigned.

### Shl02 is driven by bank4 actions3/4/5

**EXE_AND_CORPUS_CONFIRMED**

CEm034 states86/87/88 map to bank4 actions3/4/5.

Their common update target `0x140169901` reads controller1
(`CEm034+0x5190`) byteIndex0.

On value1 it:

- clears the signal;
- builds the child transform;
- creates `CEm034Shl02` through factory `0x140173620`.

Retail corpus:

```text
action3 frame4 -> [1,0,0,0,0]
action4 frame4 -> [1,0,0,0,0]
action5 frame4 -> [1,0,0,0,0]
```

Therefore all three actions have a canonical script-channel edge to Shl02
creation. Shl02's gameplay name remains **PRESERVED_UNDECODED**.

### Shl01 is concretely triggered by action31

**EXE_AND_CORPUS_CONFIRMED**

The shared update target `0x140169A9D` serves states:

```text
83,84,85,90,91,94,100..115
```

which correspond to bank4 actions:

```text
0,1,2,7,8,11,17..32
```

It reads controller1 byteIndex0. A value1 is cleared and enters a bounded loop
that creates `CEm034Shl01` through factory `0x1401729D0`.

In the retail slot12 corpus, among those mapped actions only action31 contains
opcode3 byteIndex0 events:

```text
action31 frame1  -> [1,0,0,0,0]
action31 frame13 -> [1,0,0,0,0]
```

So action31 has a direct canonical edge to the Shl01 creation path.

### Shl04 is concretely triggered by action60/state143

**EXE_AND_CORPUS_CONFIRMED**

State143 maps to bank4/action60 and uses update target `0x14016A095`.

That target reads controller1 byteIndex0. On value1 it clears the signal and
enters a bounded spawn loop whose factory call is:

```text
0x140175210 -> CEm034Shl04
```

Retail action60 contains:

```text
frame18 -> [1,0,0,0,0]
```

Therefore action60 frame18 directly feeds the Shl04 creation path.

### Shl actor script coverage after Pass 2B

| child actor | canonical script/lifecycle evidence |
|---|---|
| Shl00 | actions13,44,50 structural creation paths confirmed |
| Shl01 | action31 byteIndex0 creation path confirmed |
| Shl02 | actions3/4/5 byteIndex0 creation paths confirmed |
| Shl03 | separate lifecycle path recovered; script-channel trigger not yet established |
| Shl04 | action60 byteIndex0 creation path confirmed |
| Shl05 | action46 byteIndex0 creation path confirmed |

Only Shl03 remains without a closed script/lifecycle trigger in this matrix.

### Remaining reverse gates after Pass 2B

- close the Shl03 lifecycle transition around the state144 path;
- determine whether core slots20..24 use explicit render visibility toggles or
  are represented canonically by placement/control-domain changes alone;
- recover semantic resource identities only from stronger executable/corpus
  evidence;
- encode the completed runtime model in C++20 only after these gates close.


## Pass 2C — Shl03 lifecycle phase machine

### CEm034+0x5994 is a six-phase child-actor sequence

**EXE_CONFIRMED**

Function `0x14016CA30` dispatches on byte `CEm034+0x5994` with six phases
(0..5). This phase machine is separate from the ordinary opcode3 channel
consumer, but it drives the same CEm034 state setter through the `+0x250`
state-machine vtable.

The recovered state progression is:

```text
phase0
  -> set CEm034 state93  (bank4 action10)
  -> phase1

phase1
  wait for state144 + timing condition
  -> set state94        (bank4 action11)
  -> create CEm034Shl03
  -> store actor at CEm034+0x5998
  -> phase2

phase2
  wait for state144
  -> state95 (action12) when internal selector +0x5A1C < 1
     OR
     state96 (action13) when selector +0x5A1C >= 1
  -> phase3

phase3
  movement/runtime interaction stage
  -> later set state98  (bank4 action15)
  -> phase4

phase4
  movement/collision gate
  -> set state97        (bank4 action14)
  -> phase5

phase5
  wait for state144
  -> return through the enclosing actor-state path
```

State144 itself is a real CEm034 completion state. At the tail of
`0x140169060`, when the first slot12 controller reports completion through
`0x140059590`, the state-machine vtable is called with state `0x90` (144).

Because the normal switch table handles only states0..143, state144 acts as the
completion rendezvous consumed by higher-level lifecycle code such as
`0x14016CA30`.

Status: **EXE_CONFIRMED**.

### Shl03 creation is now closed

**EXE_CONFIRMED**

In phase1, after the state144/timing gate:

1. `+0x5994` advances from phase1 to phase2;
2. state94 / bank4 action11 is started;
3. a child transform is built from CEm034 runtime transforms;
4. factory `0x1401745F0` creates `CEm034Shl03`;
5. the returned actor pointer is retained at `CEm034+0x5998`.

Therefore Shl03 is not an always-visible attachment and does not require a
direct opcode3 byte trigger. It belongs to this higher-level phase sequence.

This closes the last Shl class that lacked a recovered creation trigger.

### Shl04 has two distinct creation surfaces

**EXE_CONFIRMED**

The earlier direct script edge remains:

```text
state143 / bank4 action60
controller1 byteIndex0 == 1
-> CEm034Shl04 factory 0x140175210
```

A second Shl04 creation loop exists inside phase3 of the Shl03 lifecycle
function. That loop derives positions from runtime/world state and calls the
same `0x140175210` factory without using the action60 opcode3 trigger.

Therefore:

- Shl04 is a reusable dynamic child actor;
- action60 is one canonical creation path;
- the Shl03 phase sequence is another canonical creation surface.

Any one-to-one semantic equation such as “Shl04 == weapon slot X” is
**REJECTED**.

### Updated Shl coverage

| child actor | creation authority |
|---|---|
| Shl00 | script-driven paths in actions13/44/50 |
| Shl01 | action31 script-driven path |
| Shl02 | actions3/4/5 script-driven paths |
| Shl03 | six-phase lifecycle, created between action10 completion and action11 |
| Shl04 | action60 script path + Shl03 phase3 runtime loop |
| Shl05 | action46 script-driven path |

All six CEm034Shl classes now have a recovered creation/lifecycle surface.
Their artistic/gameplay names remain **PRESERVED_UNDECODED**.

## Pass 2C correction to the canonical component model

The core equipment architecture is now more precise:

- slots21,22,24 use body-attached placement records;
- slot23 mode0 uses its body-node placement record, while mode1 uses the
  CEm034-owned transform at `+0x43C0`;
- slot20 can switch between body-placement control and its independent
  `em034_013.bin` MotionScript controller;
- slots25/26/30 belong to dynamic Shl actor resource paths and are not
  persistent body attachments.

This is the minimum topology the later Native Reader implementation must
preserve.
