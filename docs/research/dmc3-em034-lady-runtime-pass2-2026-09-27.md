# DMC3 HD — em034 Lady runtime reconstruction — pass 2

Date: 2026-09-27  
Branch: `reverse/em034-lady-runtime-20260927`  
Depends on: `dmc3-em034-lady-runtime-pass1-2026-09-27.md`

## Purpose

Close the CEm034 state-entry -> motion-script lane -> attachment transition path for the physical test actions 13, 44, 46 and 50, and recover the lifecycle boundary for dynamic resource slots 25/26/30.

No weapon-name or hand/back semantic labels are promoted in this pass.

## CEm034 state setter

Function `0x14016A410` writes the incoming state id to:

`CEm034+0x5990`

and dispatches a per-state entry handler.

**EXE_CONFIRMED**

The CEm034 secondary object at `+0x250` has vtable `0x1404D84A0`. Its vtable slot `+0x40` points to adjustor:

`0x14016AD40`

which subtracts `0x250` from `this`, forces `r8d=-1`, and tail-calls `0x14016A410`.

Therefore state changes made through this virtual path start both em034_012 script object lanes.

## Script start helper

`0x140171AB0(this, bank, action, laneSelector)` routes to the two em034_012 controllers:

- selector 0 -> `CEm034+0x5070`
- selector 1 -> `CEm034+0x5190`
- selector -1 -> starts both controllers

Each selected controller is started by `0x14005A1F0(controller, bank, action)`.

`0x14005A1F0` stores bank/action in the controller, resolves the script pointer through its bank/action tables, clears the channel bytes, runs the interpreter, and refreshes the bound motion-resource state.

**EXE_CONFIRMED**

## Bank-4 actor-state mapping

For the weapon-heavy state range, CEm034 uses:

`bank4 action = CEm034.state(+0x5990) - 0x53`

This is directly emitted by the state-entry handlers and by the delayed slot20-script start sites.

Important exact mappings:

| CEm034 state | em034_012 bank4 action |
|---:|---:|
| 0x56 | 3 |
| 0x57 | 4 |
| 0x58 | 5 |
| 0x59 | 6 |
| 0x5E | 11 |
| 0x60 | 13 |
| 0x61 | 14 |
| 0x7F | 44 |
| 0x81 | 46 |
| 0x85 | 50 |
| 0x8F | 60 |

**EXE_CONFIRMED**

## Slot20 independent animation gate

CEm034 owns a byte at:

`+0x4020`

The per-frame script update path at `0x14017105A..` always ticks the two em034_012 controllers (`+0x5070`, `+0x5190`), but ticks the em034_013 controller (`+0x52B0`) only when:

`CEm034+0x4020 == 0`

The main slot20 attachment setup writes `+0x4020 = 1`.

State paths that start the slot20 em034_013 controller write `+0x4020 = 0`.

**EXE_CONFIRMED**

This establishes two distinct slot20 runtime modes:

1. body-joint attachment mode using the slot20 attachment record;
2. independent slot20 MOT-script mode using em034_013 / PAC11.

This is not merely a visibility bit. It gates whether the independent slot20 script controller advances.

## Delayed slot20 script start

States `0x59` and `0x61` correspond to bank4 actions 6 and 14.

Their update branch waits for em034_012 lane signal channel 0 == 1. When observed it:

1. clears the signal;
2. restores slot20 through `0x1401713F0(index=0, mode=0)`;
3. starts em034_013 controller `+0x52B0` at bank4 action `state-0x53`;
4. writes `+0x4020 = 0`, enabling independent slot20 controller ticking.

Therefore:

- state 0x59 -> delayed em034_013 bank4/action6;
- state 0x61 -> delayed em034_013 bank4/action14.

**EXE_AND_CORPUS_CONFIRMED**

The corresponding PAC11 MOT slots 6 and 14 exist in the retail corpus.

## Physical-test action 13

CEm034 state `0x60` = em034_012 bank4/action13.

State entry starts the body script on both object lanes and immediately applies:

- slot20: mode1
- slot24: mode1

using `0x1401713F0`.

The action13 opcode-3 stream is:

- frame 2:  `(1,0,0,0,0)`
- frame 3:  `(0,1,0,0,0)`
- frame 5:  `(1,0,0,0,0)`
- frame 8:  `(1,0,0,0,0)`
- frame 14: `(1,0,0,0,0)`
- frame 17: `(1,0,0,0,0)`
- frame 20: `(1,0,0,0,0)`

The state-specific update consumes object-lane-1 channels 0 and 1 for other runtime actions, but does not issue another slots20/24 attachment-table change during this state.

**EXE_AND_CORPUS_CONFIRMED**

## Physical-test action 44

CEm034 state `0x7F` = em034_012 bank4/action44.

State entry applies:

- slot21: mode1
- slot22: mode1

and starts action44 through the both-lane path.

Action44 signal stream:

- frame14: `(1,1,0,0,0)`
- frame21: `(1,0,0,0,0)`
- frame28: `(1,1,0,0,0)`

The state update consumes object-lane-1 channel0 and channel1. Channel0 == 1 reaches a dynamic `CEm034Shl00` creation path; the signal is cleared after consumption.

**EXE_AND_CORPUS_CONFIRMED**

The exact gameplay name of the Shl00 actor remains **PRESERVED_UNDECODED**.

## Physical-test action 46

CEm034 state `0x81` = em034_012 bank4/action46.

State entry applies:

- slot23: mode1

and starts action46 on both body-script lanes.

Action46 signal stream:

- frame1:  `(0,1,0,0,0)`
- frame27: `(1,1,0,0,0)`
- frame32: `(1,1,0,0,0)`
- frame37: `(1,1,0,0,0)`
- frame42: `(1,1,0,0,0)`
- frame78: `(0,0,1,0,0)`

The state update consumes object-lane-1:

- channel0 == 1 -> dynamic `CEm034Shl05` creation path;
- channel1 == 1 -> runtime parameter at the slot23/effect domain is changed to 1.5;
- channel2 == 1 -> signal is cleared and slot23 is restored with `index=3, mode=0`.

Therefore slot23 has an exact attachment-state transition for action46:

`mode1 at state entry -> mode0 at frame78`

**EXE_AND_CORPUS_CONFIRMED**

## Physical-test action 50

CEm034 state `0x85` = em034_012 bank4/action50.

State entry starts action50 on both script lanes and performs the common
`0x140168B00` baseline reset. Therefore component4/slot24 begins this state
in preset0.

Action50 signal stream:

- frame10: `(0,1,0,0,0)`
- frame25: `(1,0,0,0,0)`
- frame39: `(2,0,0,0,0)`
- frame64: `(0,2,0,0,0)`

Live canonical-EXE revalidation of the consumer at `0x140169F2A` corrects
the earlier interpretation:

- lane1 channel1 == 1 -> `slot24 / component4, preset1`;
- lane1 channel1 == 2 -> `slot24 / component4, preset0`;
- lane1 channel0 == 1 -> starts the separate `+0x57DD` flag/timer path;
- lane1 channel0 == 2 -> clears that flag/timer path.

Thus the exact slot24 placement transition is:

- state entry -> preset0;
- frame10 -> preset1;
- frame64 -> preset0.

Frame39 is **not** the placement reset. It is lane1/channel0 value2 and belongs
to the separate flag/timer path.

**EXE_AND_CORPUS_CONFIRMED**

## Dynamic slot25 / CEm034Shl02 lifecycle

The CEm034 update branch shared by states:

- 0x56 -> action3
- 0x57 -> action4
- 0x58 -> action5

reads em034_012 object-lane-1 channel0.

Retail script actions 3/4/5 each emit:

`frame4: (1,0,0,0,0)`

When channel0 == 1, CEm034 clears the signal and calls factory:

`0x140173620 = CEm034Shl02`

Pass 1 proved Shl02 directly owns resource pair slots19 + 25.

Therefore slot25 is a dynamically created runtime actor resource activated from actions3/4/5 at the frame4 signal.

**EXE_AND_CORPUS_CONFIRMED**

Exact gameplay naming remains **PRESERVED_UNDECODED**.

## Dynamic slots26/30 / CEm034Shl03 lifecycle

A CEm034 behavior path transitions the actor through the secondary-vtable state setter to:

`state 0x5E = bank4/action11`

and immediately afterwards constructs:

`0x1401745F0 = CEm034Shl03`

using a transform derived from the slot20 runtime domain.

Pass 1 proved Shl03 directly owns:

- slots19 + 26
- slots29 + 30

The retail action11 script contains no opcode-3 signals, matching the immediate external creation path rather than a delayed channel event.

Therefore slots26/30 are dynamic Shl03 actor resources associated with the state0x5E/action11 transition, not permanent CEm034 body attachments.

**EXE_AND_CORPUS_CONFIRMED**

Exact gameplay naming remains **PRESERVED_UNDECODED**.

## Reader model corrections now closed by evidence

The following current/previous Reader assumptions are now **REJECTED**:

- one persistent attachment host for slots20–26/30;
- one scalar weapon state sufficient for Lady;
- one MotionScriptFile per assembled actor PAC;
- slot20 always follows a host joint during animation.

The minimum canonical em034 runtime model needs:

- main equipment slots20–24 with two EXE attachment records each;
- five-channel script signals per object lane;
- two em034_012 object lanes;
- a separate em034_013 controller for slot20;
- slot20 runtime mode gating via `+0x4020`;
- dynamic child-actor lifecycle for slot25 and slots26/30;
- state/action-specific attachment events, at least the exact action13/44/46/50 transitions above.

## Still unresolved

- proper-name promotion for the launcher (role is closed; exact franchise name is not);
- negative-X versus positive-X hand -> human left/right labels;
- exact gameplay names for Shl00/Shl01/Shl04/Shl05 beyond their proven runtime domains;
- final actor-manager unlink/free internals after shared Shl retire completion, only if exact runtime-manager emulation requires them;
- canonical Lady EquipmentBinding/WeaponBinding implementation in Native Reader.

## Pass 2D — Shl actor inheritance and destruction entrypoints

### All six Lady Shl classes are CShell actors

**EXE_CONFIRMED**

The canonical executable RTTI/class-hierarchy census closes the inheritance boundary for every dynamic Lady child actor:

```text
CEm034Shl00..05
  -> CShell
     -> CActor
        -> CWork
        -> IActor
        -> ICollisionHandle
```

Therefore Shl00..05 are independent actor/collision objects. They are not persistent CEm034 equipment attachments and their lifetime must not be represented as a visibility bit on slots20..24.

This also separates two previously conflated questions:

1. core component presentation/control for persistent slots20..24;
2. spawn/update/retire/destruction lifecycle for Shl00..05.

### Exact inherited polymorphic-head layout

**EXE_CONFIRMED**

The canonical RTTI Complete Object Locators for `CActor`, `CShell`, and every `CEm034Shl00..05` instance agree on the same three object heads:

```text
+0x000  primary CWork/CActor/CShell-derived head
+0x060  secondary actor interface head
+0x0D0  secondary collision interface head
```

The two secondary heads retain the CActor multiple-inheritance layout. Each Lady Shl class has six bases in its class-hierarchy descriptor, versus five for CShell, consistent with one further derived class layer over the shared shell actor.

| class | primary COL | +0x60 COL | +0xD0 COL | primary vtable |
|---|---:|---:|---:|---:|
| CEm034Shl00 | 0x5157C0 | 0x515860 | 0x515888 | 0x1404D8588 |
| CEm034Shl01 | 0x5158B0 | 0x515950 | 0x515978 | 0x1404D8738 |
| CEm034Shl02 | 0x5159A0 | 0x515A40 | 0x515A68 | 0x1404D88E8 |
| CEm034Shl03 | 0x515A90 | 0x515B30 | 0x515B58 | 0x1404D8A98 |
| CEm034Shl04 | 0x515B80 | 0x515C20 | 0x515C48 | 0x1404D8C48 |
| CEm034Shl05 | 0x515C70 | 0x515D10 | 0x515D38 | 0x1404D8DF8 |

The corresponding secondary vtables are recorded in
`data/reverse/dmc3-em034-lady-runtime-pass2d-20260928.json`.

### Primary vtable slot 0 is the deleting-destructor entry

**EXE_CONFIRMED**

The RTTI/vtable atlas records `CEm034` primary vtable
`0x1404D80E8` with first virtual target `0x140168840`. Independent Pass-1
reverse already identifies `0x140168840` as the CEm034 deleting destructor.
This provides a direct control for interpreting the same primary-vtable position
in the six Shl classes.

The recovered deleting-destructor entries are therefore:

| class | deleting destructor | factory |
|---|---:|---:|
| CEm034Shl00 | 0x1401721D0 | 0x140172240 |
| CEm034Shl01 | 0x140172960 | 0x1401729D0 |
| CEm034Shl02 | 0x1401735E0 | 0x140173620 |
| CEm034Shl03 | 0x140174010 | 0x1401745F0 |
| CEm034Shl04 | 0x1401751A0 | 0x140175210 |
| CEm034Shl05 | 0x140175AA0 | 0x140175B10 |

The first targets of the +0x60/+0xD0 vtables are adjustor entries into the same
derived destruction boundary and are retained in the machine-readable Pass-2D
record.

### Lifecycle boundary now closed, destruction trigger still open

The following model is now **REJECTED**:

```text
Shl actor disappears
  == hide one persistent equipment slot
```

Correct architectural split:

```text
slots20..24
  persistent CEm034-owned component/control domains
  placement preset + optional independent MotionScript control

Shl00..05
  independent CShell/CActor instances
  own actor + collision lifecycle
  spawn through the already recovered action/phase paths
  retire through actor lifecycle and ultimately the class deleting destructor
```

What is **not** yet promoted:

- the exact per-class condition that marks a Shl actor for retire/remove;
- the actor-manager/remove function that reaches the deleting destructor;
- whether a class becomes non-rendered for an interval before retirement;
- gameplay/artistic names for Shl00..05.

Those require the live canonical EXE call/xref path around the class update
methods / CShell retire path. The retained RTTI atlas proves the class and
destruction boundary, but it does not contain the required call-xref graph.

Status: **EXE_CONFIRMED structural lifecycle boundary; retire trigger PRESERVED_UNDECODED**.


## Pass 2E — live canonical EXE Shl retire conditions

### Artifact re-verification

**EXE_CONFIRMED**

The live executable used for this pass was supplied again and hashed before
analysis:

```text
SHA-256 e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082
PE32+ x86-64
.text VMA 0x140001000
```

This pass therefore no longer depends only on the retained RTTI atlas.

### Shared Shl actor state machine

**EXE_CONFIRMED**

All six `CEm034Shl00..05` primary update dispatchers begin by calling
`0x1403261B0` and dispatch on actor byte `+0x08`.

The common state grammar is:

```text
state 0 -> secondary/interface initialization path
state 1 -> class-specific phase/update
state 2 -> class-specific phase/update
state 3 -> completion/retire path through 0x1403261E0
other   -> no class update
```

`0x1403261E0` operates on the common actor state at `+0x1C/+0x20`.
For actor substate 1/2 it invokes virtual slot `+0x20` and promotes both
fields to 3; state 4 routes through `0x140326240`. This is shared actor
lifecycle machinery, not a Lady-only visibility helper.

### Shl00 retire condition

**EXE_CONFIRMED**

`0x140172590` maintains a countdown at `Shl00+0x52C`:

```text
remaining = [Shl00+0x52C] - [Shl00+0x14]
if remaining < 0:
    Shl00+0x08 = 3
```

The same phase clears child pointer `+0x520` once that child's common actor
state `+0x1C` is 0, 3, or 4. In Shl00 state 3 the dispatcher first sends
the retained `+0x520` child through `0x1403261E0`, then sends Shl00
itself through the same path.

### Shl01 retire condition

**EXE_CONFIRMED**

`0x140172B20` initializes `Shl01+0x538 = 1.0f`, decrements it by the
common actor delta `+0x14`, and writes `Shl01+0x08 = 3` when the
countdown becomes negative.

It independently tracks children at `+0x520` and `+0x528`. State 3
routes both non-null children through `0x1403261E0` before retiring Shl01.

### Shl02 retire condition

**EXE_CONFIRMED**

`0x140173800` initializes:

```text
Shl02+0xD68 = 3.0f
```

and decrements the timer by `Shl02+0x14`. A negative timer writes
`Shl02+0x08 = 3`.

The class retains a child at `+0xD60`; state 3 sends that child through
`0x1403261E0` and then retires Shl02 through the same common path.

### Shl03 owner-action lifetime gate

**EXE_CONFIRMED**

`0x140174550` is the decisive Shl03 lifetime gate.

It reads owner pointer `Shl03+0x1510`, enters owner subobject `+0x250`,
calls virtual slot `+0x08`, and treats the returned integer as the owner
action/state selector.

The jump table covers values 94..144 exactly.

Shl03 is retained for:

```text
94, 95, 96, 98, 99, 144
```

Shl03 is moved to actor state 3 for:

```text
97
100..143
any value outside 94..144
```

Both class phase paths call this gate:

```text
0x140174D2C -> 0x140174550
0x14017508F -> 0x140174550
```

Therefore Shl03 lifetime is explicitly coupled to a narrow set of owner
actions rather than a generic fixed-duration timer.

### Shl04 two-phase retire path

**EXE_CONFIRMED**

Shl04 uses two class phases:

```text
state 1 -> 0x1401756E0
state 2 -> 0x1401753A0
state 3 -> child cleanup + 0x1403261E0
```

The state-1 path decrements `Shl04+0x530`; expiry writes the word at
`+0x08 = 0x0002`, simultaneously selecting actor state 2 and resetting
the adjacent class phase byte `+0x09`.

The state-2 path initializes `Shl04+0x530 = 3.0f`, decrements it by
`+0x14`, and on expiry writes `Shl04+0x08 = 3`.

Shl04 tracks children at `+0x520` and `+0x528`; state 3 sends both
through `0x1403261E0` before retiring the parent actor.

### Shl05 retire conditions

**EXE_CONFIRMED**

Shl05 has two recovered countdown surfaces.

`0x140175C50` initializes `Shl05+0x528 = 1.0f`, decrements it by
`+0x14`, and writes `+0x08 = 3` on expiry.

`0x140175ED0` maintains the alternate phase countdown at `+0x52C` and
also writes `+0x08 = 3` when that countdown becomes negative.

The state-3 dispatcher retires the optional child at `+0x520` first and
then Shl05 itself through `0x1403261E0`.

### Promotion

The previous open item:

```text
spawn/despawn conditions for CEm034Shl00..05
```

is now split more precisely:

- spawn surfaces: **EXE_CONFIRMED** from Pass 2B/2C;
- per-class transition into retire state 3: **EXE_CONFIRMED** in this pass;
- shared actor completion path `0x1403261E0`: **EXE_CONFIRMED**;
- final actor-manager unlink/free after common completion: still
  **PRESERVED_UNDECODED** and not required for the Lady binding model unless
  ownership implementation needs exact manager internals.

For the canonical Lady equipment model, Shl lifecycle is now sufficiently
separated from persistent component visibility.


## Pass 2F — persistent component visibility closure

### Result

**EXE_CONFIRMED bounded closure**

For the five persistent CEm034 component managers:

```text
component0 / slot20 -> CEm034+0x08E8
component1 / slot21 -> CEm034+0x0900
component2 / slot22 -> CEm034+0x0908
component3 / slot23 -> CEm034+0x0910
component4 / slot24 -> CEm034+0x0930
```

the canonical CEm034 code contains no separate per-component gameplay
visibility enable/disable state.

The previously used vocabulary `mode0/mode1` is therefore a placement-preset
selector, not a hidden/visible selector.

### The component hosts are CCnsMatrix constraints

RTTI identifies the five local placement hosts as `CCnsMatrix` objects.
The shared evaluator at `0x1402CBBE0` proves the meaning of host `+0x28`:

```text
host+0x28 == 1 -> consume external parent matrix from host+0x30
host+0x28 == 2 -> consume inline matrix at host+0x40
other          -> generic constraint path 0x14030E9B0
```

The CCnsMatrix initializer at `0x1402CBCB0` initializes `+0x28=0`,
`+0x30=null` and an inline identity matrix. This is constructor state, not a
Lady render-disable state.

Every recovered Lady placement path for slots20..24 writes:

```text
manager+0x100 = component CCnsMatrix host
host+0x30     = selected parent runtime transform
host+0x28     = 1
```

and supplies the selected local transform in host `+0x80..+0xB0`.

Thus `host+0x28=1` means external-parent matrix constraint mode.

### Exact persistent host map

| component | resource manager | CCnsMatrix host | auxiliary/local domain |
|---:|---:|---:|---:|
| 0 / slot20 | +0x08E8 | +0x4000 | +0x4080 |
| 1 / slot21 | +0x0900 | +0x40C0 | +0x4140 |
| 2 / slot22 | +0x0908 | +0x4180 | +0x4200 |
| 3 / slot23 | +0x0910 | +0x4240 | +0x42C0 |
| 4 / slot24 | +0x0930 | +0x4300 | +0x4380 |

### Placement helper 0x1401713F0

`0x1401713F0(CEm034*, componentIndex, preset)` selects between:

```text
preset0 table base = 0x14057AFC0
preset1 table base = 0x14057B0B0
record stride      = 0x30
```

and rebinds the selected component's CCnsMatrix parent/local transform.

For component3/slot23, preset1 has the already-recovered special parent
`CEm034+0x43C0`. This remains a placement exception, not visibility.

Slot20 additionally updates its independent-controller gate `+0x4020`
when returning to the body-constraint domain. The gate controls whether
em034_013 ticks; it does not detach or hide the slot20 model.

### Direct-reference census

Within the complete canonical CEm034 class code block through the first
CEm034Shl class boundary, direct references to the five manager fields are:

```text
slot20 +0x08E8: 11
slot21 +0x0900: 10
slot22 +0x0908:  7
slot23 +0x0910:  6
slot24 +0x0930: 10
```

The recovered references classify into:

- resource load/binding destination;
- baseline placement setup;
- state/script placement rebind;
- source/runtime matrix reset;
- transform sampling/update for actor/effect creation.

No CEm034 gameplay path in this bounded surface:

- clears a loaded component manager field;
- writes `manager+0x100 = null`;
- switches the component CCnsMatrix host to a disabled state;
- uses `host+0x28=0` as a state transition;
- changes MOD alpha/object flags as a script-conditioned visibility operation;
- routes opcode-3 channels to a separate render enable/disable primitive.

The only zero state observed for the CCnsMatrix mode is construction/
initialization before runtime placement binding.

### Canonical visibility model for slots20..24

For Reader/runtime reconstruction the minimum evidence-backed state is now:

```text
resource presence:
  present when the component resource was successfully loaded and CEm034 owns it

placement:
  preset0 | preset1

control domain:
  component0 additionally has body-constraint vs independent em034_013 control

visibility:
  no independent script-driven per-slot toggle recovered
```

Consequently, a Reader field such as:

```text
bool visible;
```

must not be synthesized from `preset0/preset1`.

If a presentation field is required by the UI, the canonical static model is
`persistent-present` for loaded slots20..24, with placement/control state kept
separate.

This does not claim that renderer-global culling, scene-level actor removal or
resource-load failure can never make a component absent on screen. It closes
the narrower and relevant question: **CEm034 has no recovered independent
per-slot weapon visibility state for slots20..24.**

### Promotion

The previous open item:

```text
visibility enable/disable for slots20–24
```

is now closed as:

**EXE_CONFIRMED — no separate CEm034-local visibility toggle; placement and
control-domain transitions are the canonical runtime state.**


## Pass 2F — persistent slots20..24 presentation/visibility boundary

### Scope

This pass uses the re-verified canonical executable directly and performs a
CEm034-side reference census for the five persistent runtime MOD managers:

| component | PAC slot | CEm034 manager pointer |
|---:|---:|---:|
| 0 | 20 | +0x8E8 |
| 1 | 21 | +0x900 |
| 2 | 22 | +0x908 |
| 3 | 23 | +0x910 |
| 4 | 24 | +0x930 |

No gameplay/artistic weapon names are assigned.

### Persistent ownership

**EXE_CONFIRMED**

The five managers are allocated into persistent CEm034 fields by the common
resource acquisition helper `0x140168150`. State/action transitions do not
create or destroy these managers.

The direct CEm034 references to these five pointers resolve to:

- manager transform source/current transform at `+0x108/+0x110`;
- manager host/control pointer at `+0x100`;
- CEm034-owned component transform/control blocks;
- placement rebinding through `0x1401713F0`;
- slot20's independent MotionScript control gate `CEm034+0x4020`.

### Placement helper is not a visibility helper

**EXE_CONFIRMED**

`0x1401713F0(CEm034*, componentIndex, mode)` selects one of two 0x30-byte
placement records and rebuilds the component transform/host binding.

For components 0..4 it writes the selected runtime MOD manager's `+0x100`
host/control pointer and the CEm034 component binding fields `+0x28/+0x30`.

For component 3 / mode1 it uses the already recovered special parent
`CEm034+0x43C0`.

For component 0 it additionally restores `CEm034+0x4020 = 1`, switching
slot20 back to body-placement control.

The helper contains no render-enable/render-disable operation.

Therefore:

```text
mode0 != hidden
mode1 != visible
```

and the old interpretation of the two records as visibility states is
**REJECTED**.

### Direct CEm034 render-state census

**EXE_CONFIRMED for the CEm034 ownership/control surface**

Across the CEm034 code surface, direct accesses rooted at the five persistent
manager fields were classified. The observed operations are allocation,
retention, transform reads/writes, host/control rebinding and transform-derived
effect/spawn work.

No CEm034-side direct write was found that independently toggles a render or
visibility field on only slots20..24 during the recovered action transitions.

This is a negative result with a precise boundary: it closes the need for a
Lady-specific persistent-component `visible` boolean in the canonical binding
model. It does not claim that the generic MOD renderer has no culling,
object-level flags, alpha rules or global actor visibility machinery.

### Canonical Reader model consequence

The minimum persistent Lady component state is now:

```text
component 0 / slot20:
    placement_mode = 0 | 1
    control_domain = body-placement | independent em034_013 MotionScript

components 1,2,3,4 / slots21..24:
    placement_mode = 0 | 1

component 3 / slot23:
    mode1_parent = CEm034+0x43C0
```

There is no evidence-backed per-component Lady runtime field:

```text
visible = true | false
```

to reproduce for slots20..24.

If a future whole-renderer trace finds a generic manager/object visibility
mechanism, it belongs to the shared MOD/render contract and must not be
retroactively conflated with Lady placement mode.

### Status

- slots20..24 persistent ownership: **EXE_CONFIRMED**
- mode0/mode1 as placement states: **EXE_CONFIRMED**
- slot20 control-domain gate: **EXE_CONFIRMED**
- separate Lady-specific visibility toggle for slots20..24:
  **REJECTED on the recovered CEm034 control surface**
- generic renderer culling/visibility semantics: outside this Lady pass

This closes the persistent-component visibility blocker for the canonical
CEm034 binding model.


## Pass 2G — canonical 5 x 2 placement table extraction

**EXE_CONFIRMED**

The two placement tables consumed by `0x1401713F0` were extracted directly
from the re-verified canonical executable:

```text
preset0 base 0x14057AFC0
preset1 base 0x14057B0B0
record stride 0x30
component count 5
```

Each record contains:

```text
+0x00 float4 translation
+0x10 float4 rotation XYZ + homogeneous lane
+0x20 signed byte serialized body-node selector
+0x21..+0x2F zero in these ten canonical records
```

The exact decoded records are:

| component / slot | preset | translation xyz | rotation xyz (rad) | serialized node |
|---|---:|---|---|---:|
| 0 / 20 | 0 | (-2.0,-20.0,-17.0) | (-1.570796,0,1.082104) | 3 |
| 0 / 20 | 1 | (-8.4,-1.0,-1.3) | (0,0,3.141593) | 9 |
| 1 / 21 | 0 | (-1.0,-4.0,13.0) | (1.867502,0.048869,2.408554) | 14 |
| 1 / 21 | 1 | (-7.5,-0.6,-0.8) | (0,0,0) | 9 |
| 2 / 22 | 0 | (-9.2,-13.0,-9.1) | (0,0,1.658063) | 16 |
| 2 / 22 | 1 | (7.7,-0.8,0.5) | (0,0,3.141593) | 13 |
| 3 / 23 | 0 | (10.0,-15.0,-2.5) | (0,0,-1.658063) | 19 |
| 3 / 23 | 1 | (7.2,-1.2,2.7) | (0,-0.174533,0) | 13* |
| 4 / 24 | 0 | (17.0,-5.0,-16.0) | (-1.221730,-0.235619,0.628318) | 14 |
| 4 / 24 | 1 | (7.2,-0.8,-0.4) | (0,0,3.141593) | 13 |

`* component3/preset1` is the canonical runtime exception already recovered in
Pass 2B: although the serialized table byte is 13, `0x1401713F0` does not
resolve body node 13 for this case. It binds the component to the internal
runtime transform at `CEm034+0x43C0`.

Therefore a correct implementation must preserve both facts:

1. raw placement record node byte = 13;
2. effective runtime parent = `CEm034+0x43C0`.

Replacing the raw byte with a fabricated node or treating node13 as the
effective parent is **REJECTED**.

No human labels such as hand/back/holster are assigned to either preset.
Those remain semantic interpretation rather than runtime structure.

## Pass 2G — complete script-channel to component placement matrix

### MotionScript signal getter ABI

**EXE_CONFIRMED**

Live disassembly closes the ABI of `0x140059350`:

```text
uint8_t GetSignal(controller, row, channel)
{
    if (channel >= 5)
        return 0;
    return controller[(row + 0x1D) * 5 + channel];
}
```

For the CEm034 consumers in this pass, `row == 0`, so `r8d` is the exact
MotionScript channel index `0..4`.

Controller identity is also exact:

```text
CEm034+0x5070 = em034_012 lane0
CEm034+0x5190 = em034_012 lane1
CEm034+0x52B0 = em034_013 independent slot20 controller
```

The main CEm034 update surface contains 22 direct `0x140059350` calls.
Two additional lane1/channel0 consumers exist in the `+0x5994` phase
controllers at `0x14016DB92` and `0x14016DD80`. Neither is a persistent
component placement consumer.

### Exact state-switch decoding

The CEm034 update function `0x140169060` bounds state to `0x00..0x8F` and
dispatches through:

```text
byte map    0x14016A374
offset map  0x14016A324
```

The state-entry setter `0x14016A410` uses:

```text
byte map    0x14016ACAC
offset map  0x14016AC48
```

and has the same exact `state <= 0x8F` bound.

This closes both the entry-placement and per-frame channel-consumer sides.

### Persistent component matrix

| component | slot | state/action surface | controller/channel | value | canonical effect |
|---:|---:|---|---|---:|---|
| 0 | 20 | baseline `0x140168B00` | none | - | preset0 |
| 0 | 20 | states `0x53..0x59,0x5E,0x61` | entry | - | preset1, body-constraint domain |
| 0 | 20 | states `0x5F,0x62` | entry | - | preset1, body-constraint domain |
| 0 | 20 | states `0x60,0x63` / actions 13,16 | entry | - | preset1, body-constraint domain |
| 0 | 20 | state `0x7B` / action40 | entry | - | preset0 + independent em034_013 domain |
| 0 | 20 | state `0x7B` / action40 | lane0/ch0 | 1 | preset1 + body-constraint domain |
| 0 | 20 | states `0x5A..0x5D,0x64..0x73` | entry | - | preset0 + independent em034_013 domain |
| 0 | 20 | states `0x5C,0x5D` / actions9,10 | lane0/ch0 | 1 | preset1 + body-constraint domain |
| 0 | 20 | states `0x59,0x61` / actions6,14 | lane0/ch0 | 1 | preset0 + start independent em034_013 |
| 0 | 20 | states `0x7C,0x7D` / actions41,42 | entry | - | preset1 with independent em034_013 running |
| 1 | 21 | baseline | none | - | preset0 |
| 1 | 21 | state `0x2A` | lane0/ch0 | 1 | preset1 |
| 1 | 21 | state `0x2A` | lane0/ch0 | 2 | preset0 |
| 1 | 21 | state `0x7F` / action44 | entry | - | preset1 |
| 2 | 22 | baseline | none | - | preset0 |
| 2 | 22 | state `0x7F` / action44 | entry | - | preset1 |
| 3 | 23 | baseline | none | - | preset0 |
| 3 | 23 | state `0x81` / action46 | entry | - | preset1 |
| 3 | 23 | state `0x81` / action46 | lane1/ch2 | 1 | preset0 |
| 4 | 24 | baseline | none | - | preset0 |
| 4 | 24 | state `0x2B` | lane0/ch0 | 1 | preset1 |
| 4 | 24 | state `0x2B` | lane0/ch0 | 2 | preset0 |
| 4 | 24 | states `0x60,0x63` / actions13,16 | entry | - | preset1 |
| 4 | 24 | state `0x85` / action50 | entry | - | preset0 |
| 4 | 24 | state `0x85` / action50 | lane1/ch1 | 1 | preset1 |
| 4 | 24 | state `0x85` / action50 | lane1/ch1 | 2 | preset0 |

No direct MotionScript-channel placement consumer for component2/slot22 was
found. Its recovered Lady-specific transition is state-entry preset1 at
action44, followed by later common baseline reset to preset0.

### Direct 0x140059350 consumer census

| call site | state surface | lane/channel | consumed value / role |
|---|---|---|---|
| 0x140169162 | 0x7B | lane0/ch0 | 1 -> component0 preset1/body domain |
| 0x14016928F | 0x2A | lane0/ch0 | 1 -> component1 preset1 |
| 0x14016937F | 0x2A | lane0/ch0 | 2 -> component1 preset0 |
| 0x14016948A | 0x2B | lane0/ch0 | 1 -> component4 preset1 |
| 0x14016957A | 0x2B | lane0/ch0 | 2 -> component4 preset0 |
| 0x14016966B | 0x34..0x36,0x3B..0x3D,0x42..0x44,0x49..0x4B,0x50..0x52 | lane1/ch0 | 1 -> Shl00 creation surface |
| 0x14016984F | 0x5C,0x5D | lane0/ch0 | 1 -> component0 preset1/body domain |
| 0x1401698A3 | 0x59,0x61 | lane0/ch0 | 1 -> component0 preset0 + independent em034_013 |
| 0x140169919 | 0x56..0x58 | lane1/ch0 | 1 -> Shl02 creation |
| 0x1401699FB | 0x60,0x63 | lane1/ch0 | 1 -> runtime/effect trigger |
| 0x140169A39 | 0x60,0x63 | lane1/ch1 | 1 -> runtime/effect trigger |
| 0x140169ABC | 0x53..0x55,0x5A,0x5B,0x5E,0x64..0x73 | lane1/ch0 | 1 -> Shl01 creation surface |
| 0x140169BB2 | 0x7F | lane1/ch0 | 1 -> Shl00 creation |
| 0x140169BDD | 0x7F | lane1/ch1 | nonzero -> runtime selector `+0x57D7` |
| 0x140169D9E | 0x81 | lane1/ch0 | 1 -> Shl05 creation |
| 0x140169ED7 | 0x81 | lane1/ch1 | 1 -> runtime parameter 1.0 -> 1.5 |
| 0x140169EF7 | 0x81 | lane1/ch2 | 1 -> component3 preset0 |
| 0x140169F62 | 0x85 | lane1/ch1 | 1 -> component4 preset1 |
| 0x140169F7B | 0x85 | lane1/ch1 | 2 -> component4 preset0 |
| 0x140169FA4 | 0x85 | lane1/ch0 | 1 -> start `+0x57DD` flag/timer path |
| 0x14016A013 | 0x85 | lane1/ch0 | 2 -> clear `+0x57DD` flag/timer path |
| 0x14016A0B4 | 0x8F | lane1/ch0 | 1 -> Shl04 creation loop |
| 0x14016DB92 | `+0x5994` phase controller | lane1/ch0 | 1 -> acknowledge/clear phase signal |
| 0x14016DD80 | `+0x5994` phase controller | lane1/ch0 | 1 -> acknowledge/clear phase signal |

### Action50 correction promoted to canonical authority

The previous Pass-2 interpretation assigned the component4 preset0 transition
to action50 frame39/channel0 value2. Live re-disassembly rejects that mapping.

Correct trace:

```text
frame10  lane1/ch1 = 1 -> component4 preset1
frame25  lane1/ch0 = 1 -> flag/timer on
frame39  lane1/ch0 = 2 -> flag/timer off
frame64  lane1/ch1 = 2 -> component4 preset0
```

This correction is applied both to this document and to
`data/reverse/dmc3-em034-lady-runtime-pass2-20260927.json`.

### Pass-4 boundary

The structural component/channel/placement/visibility/lifecycle matrix is now
sufficient to stop modeling Lady equipment as guessed joints.

The remaining semantic work is deliberately separate:

- human names for component0..4 / slots20..24;
- human names for Shl00..05;
- human names for preset0/preset1.

Until those are independently proven, the canonical implementation should use
neutral identifiers and exact state/preset/control-domain data.


## Pass 2E — live canonical EXE closure: placement/control matrix, visibility boundary, Shl retirement, and slot identity

Date: 2026-09-28

### Canonical artifacts revalidated

The live reverse pass used the uploaded retail artifacts directly, not only the previous notes/RTTI atlas:

| artifact | size | SHA-256 |
|---|---:|---|
| dmc3.exe | 6,356,432 | e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082 |
| em034.pac | 2,690,416 | 1a5a245c8348dee3fa37ef1a83da39f15f5c1576e252daf5adc897e349f56dff |
| em034-extract original ZIP | 2,413,153 | 26f96362317a4cfa122237a3362f4430cbb383d5e5c57202e1a9b0ad50a43e65 |
| em034-extract corrected ZIP | 2,401,140 | e2d7f05f1df3cca83c73e0cc3f8cf67e8d8d5ac64d1ac4adfd6c0e8dec6ff875 |

Status: **EXE_AND_CORPUS_CONFIRMED input authority**.

### Placement-preset semantics are now human-readable

The two 0x30-byte attachment-record tables used by `0x1401713F0` were re-read directly from the canonical executable.

Preset 0, table base `0x14057AFC0`:

| component | slot | node | translation |
|---:|---:|---:|---|
| 0 | 20 | 3 | (-2.0, -20.0, -17.0) |
| 1 | 21 | 14 | (-1.0, -4.0, 13.0) |
| 2 | 22 | 16 | (-9.2, -13.0, -9.1) |
| 3 | 23 | 19 | (10.0, -15.0, -2.5) |
| 4 | 24 | 14 | (17.0, -5.0, -16.0) |

Preset 1, table base `0x14057B0B0`:

| component | slot | node | translation |
|---:|---:|---:|---|
| 0 | 20 | 9 | (-8.4, -1.0, -1.3) |
| 1 | 21 | 9 | (-7.5, -0.6, -0.8) |
| 2 | 22 | 13 | (7.7, -0.8, 0.5) |
| 3 | 23 | 13 | (7.2, -1.2, 2.7) |
| 4 | 24 | 13 | (7.2, -0.8, -0.4) |

The canonical body MOD `em034_001.mod` has 23 nodes. Its hierarchy establishes:

- node 9 = terminal node of one arm/hand chain;
- node 13 = terminal node of the opposite arm/hand chain;
- node 3 = torso/spine domain;
- node 14 = pelvis/lower-body root;
- nodes 16 and 19 = leg/hip-chain nodes.

Therefore the two preset semantics can now be promoted without inventing per-slot “back/holster” labels:

```text
preset0 = BodyStowed
preset1 = HandHeld
```

The per-slot stowed location still comes from the exact node index; it must not be flattened to a universal `Back` or `Holster` enum.

Status: **EXE_AND_CORPUS_CONFIRMED**.

### `0x1401713F0` is placement/control, not render visibility

Fresh disassembly of `0x1401713F0` confirms that mode 0/1:

- selects the corresponding 0x30-byte placement record;
- rebuilds the component runtime transform;
- changes the parent/world-matrix source;
- writes runtime attachment state;
- for component0 restores `CEm034+0x4020 = 1`, returning slot20 to attached-control mode.

There is no render-enable/render-disable operation in this function.

A bounded census over the CEm034 runtime code region `0x140168600..0x1401721D0` finds 33 direct references to the five core manager fields:

```text
slot20 +0x8E8 : 8 references
slot21 +0x900 : 8 references
slot22 +0x908 : 5 references
slot23 +0x910 : 4 references
slot24 +0x930 : 8 references
```

All 33 are loads/address derivations. No state-dependent write/nulling of those five persistent manager pointers is present in the bounded CEm034 code.

The recovered state/script paths manipulate placement, transform ownership, slot20 independent-MOT control, and dynamic Shl actors. No separate per-slot CEm034 script visibility toggle was found.

Canonical Reader conclusion:

```text
slots20..24 = persistent/resident core component domains
state/script transition = placement/control-domain transition
mode0/mode1 != hidden/visible
```

This does not claim that the generic renderer can never cull a model; it closes the CEm034-owned per-slot visibility question.

Status: **EXE_CONFIRMED bounded negative closure**.

### Exact state-entry component/control matrix

The `0x14016A410` state setter dispatches through the byte selector table at
`0x14016ACAC`. The full 0..143 selector table was decoded against the case
jump table at `0x14016AC48`.

The standard component reset helper `0x140168B00` materializes the
`BodyStowed` records for components0..4 and restores component0 attached
control (`+0x4020 = 1`).

Only the following entry groups override that baseline for the core equipment domain:

| state(s) | entry result |
|---|---|
| 0x2E, 0x30..0x52 | component1/slot21 = HandHeld; direct materialization of the same preset1 record |
| 0x53..0x59, 0x5E, 0x61 | component0/slot20 = HandHeld attached |
| 0x5A..0x5D, 0x64..0x73 | component0 remains BodyStowed; start independent em034_013 control; `+0x4020=0` |
| 0x5F, 0x62 | component0 = HandHeld attached |
| 0x60, 0x63 | component0 = HandHeld attached; component4/slot24 = HandHeld |
| 0x7B | component0 remains BodyStowed; independent em034_013 control |
| 0x7C, 0x7D | component0 initialized at the HandHeld transform, then independent em034_013 control |
| 0x7F | component1/slot21 = HandHeld; component2/slot22 = HandHeld |
| 0x81 | component3/slot23 = HandHeld |
| 0x85 | no entry placement override; action50 performs runtime component4 transitions |
| 0x8F | no core placement override; action60 drives dynamic Shl04 creation |

All other entry cases that execute the normal reset path leave the five core
components in their BodyStowed placement unless a later per-frame consumer
changes them.

Status: **EXE_CONFIRMED**.

### Per-frame script-to-component transitions

The canonical opcode-3 accessor remains `0x140059350`; channel bytes are
independent five-byte lanes.

Recovered placement/control consumers:

| Lady state / action | controller lane/channel | value | result |
|---|---|---:|---|
| 0x5C / action9 | lane0 ch0 | 1 | component0 -> HandHeld attached; independent slot20 control stops |
| 0x5D / action10 | lane0 ch0 | 1 | component0 -> HandHeld attached; independent slot20 control stops |
| 0x59 / action6 | lane0 ch0 | 1 | component0 -> BodyStowed; start em034_013 action6; `+0x4020=0` |
| 0x61 / action14 | lane0 ch0 | 1 | component0 -> BodyStowed; start em034_013 action14; `+0x4020=0` |
| 0x81 / action46 | lane1 ch2 | 1 | component3 -> BodyStowed |
| 0x85 / action50 | lane1 ch1 | 1 | component4 -> HandHeld |
| 0x85 / action50 | lane1 ch0 | 2 | component4 -> BodyStowed |

Important correction to the earlier shorthand: the delayed slot20 transitions in states
0x59/0x61 consume **lane0 (`+0x5070`) channel0**, not lane1.

Action50 lane1 channel0 value1 is not a placement rewrite. At frame25 it starts a
separate flag/timer path, and after that timing gate the update calls
`0x140171C70(this,1)`, creating a Shl00 actor from the slot24 firing domain.

Status: **EXE_AND_CORPUS_CONFIRMED** where frame/opcode data is involved; otherwise **EXE_CONFIRMED**.

### Common Shl retire/remove/destructor lifecycle is now closed end to end

All six Lady Shl classes use the common actor lifecycle.

Class-local phase termination writes:

```text
Shl +0x08 = 3
```

The class primary dispatcher then calls `0x1403261E0`.

`0x1403261E0`:

- lifecycle `actor+0x1C == 1 or 2`: invokes virtual slot +0x20,
  then sets `+0x1C = 3` and `+0x20 = 3`;
- lifecycle 3: remains in retire countdown;
- lifecycle 4: calls `0x140326240`.

Actor-manager traversal `0x1403265A0` decrements `actor+0x20` while
lifecycle==3. When the three-tick countdown reaches zero at `0x140326644`,
it writes lifecycle 4.

`0x140326240` then:

1. invokes virtual slot +0x10 for lifecycle 1/4;
2. removes the actor from global actor manager `0x140CF2520` through
   `0x140326800`;
3. writes `actor+0x1C = 0`;
4. calls primary vtable slot0 with deleting flag 0, entering the class
   destructor path.

This closes:

```text
class condition
 -> phase3
 -> common retire request
 -> 3 manager ticks
 -> lifecycle4
 -> actor-manager remove
 -> virtual class destructor
```

Status: **EXE_CONFIRMED**.

### Complete direct phase3 trigger surface for CEm034Shl00..05

Within the six Lady Shl class implementations there are exactly seven direct
writes of `Shl+0x08 = 3`:

| write site | class/path | condition |
|---:|---|---|
| 0x1401725F0 | Shl00 phase1 | hard lifetime expires |
| 0x140172BCD | Shl01 phase2 | post-impact timer expires |
| 0x1401738D4 | Shl02 phase2 | post-collision/query timer expires |
| 0x14017459C | Shl03 helper 0x140174550 | owner Lady state leaves allowed set |
| 0x1401753DB | Shl04 phase2 | post-primary-lifetime timer expires |
| 0x140175C8B | shared Shl00/Shl05 phase2 | 1-second post-impact timer expires |
| 0x140175F30 | Shl05 phase1 | hard lifetime expires |

Per-class lifecycle:

**Shl00**

- phase1 hard timeout: 120.0 seconds;
- collision object `+0x268` with nonzero hit field, or `+0x270 flags & 3`,
  enters phase2;
- phase2 starts a 1.0-second linger timer;
- timer expiry -> phase3 -> common retire chain.

**Shl01**

- phase1 collision/query condition enters phase2;
- phase2 starts a 1.0-second timer;
- expiry -> phase3.

**Shl02**

- phase1 collision at `+0x278`, or successful world/query helper
  `0x140244810`, enters phase2;
- phase2 starts a 3.0-second timer;
- expiry -> phase3.

**Shl03**

- owner CEm034 pointer is retained at `Shl03+0x1510`;
- helper `0x140174550` reads the owner's current Lady state;
- Shl03 remains active only for owner states:

```text
0x5E, 0x5F, 0x60, 0x62, 0x63, 0x90
```

- state0x61, all states0x64..0x8F, and states outside the bounded
  0x5E..0x90 range drive phase3 retirement;
- phase1 may first advance to phase2 when `Shl03+0x2A8 & 0x20000`;
  phase2 continues applying the same owner-state guard.

Thus Shl03 lifetime is owner-state-bound rather than fixed-duration.

**Shl04**

- factory `0x140175210` stores caller-supplied XMM2 directly to
  `Shl04+0x530`; this is its primary phase1 lifetime;
- action60/state143 creation uses `120.0 + 30.0 * i` seconds;
- the Shl03 phase3 creation surface uses `90.0 + 30.0 * derived_index`;
- phase1 expiry -> phase2;
- phase2 always runs a 3.0-second timer;
- expiry -> phase3.

**Shl05**

- phase1 hard timeout: 120.0 seconds;
- collision/hit condition enters phase2;
- shared phase2 timer = 1.0 second;
- expiry -> phase3.

Status: **EXE_CONFIRMED**.

### Geometry-backed slot identity

The corrected retail MODs were parsed using the canonical MOD topology algorithm recovered from
`0x1402FE3B0`: the packed-control `0x8000` topology break bit was used to regenerate
the triangle-strip command stream before visual classification.

The resulting geometry plus runtime ownership establishes the following structural identities:

| slot | structural identity | evidence |
|---:|---|---|
| 20 | large primary launcher body; host for independent em034_013 motion, rocket spawn domain and grappling-system source | EXE_AND_CORPUS_CONFIRMED |
| 21 | handgun A | CORPUS_CONFIRMED geometry + EXE hand/stow behavior |
| 22 | handgun B | CORPUS_CONFIRMED geometry + EXE hand/stow behavior |
| 23 | compact crossbow/dagger-launcher form; thigh/body stow -> hand use; source domain for Shl05 projectile actor | EXE_AND_CORPUS_CONFIRMED structural |
| 24 | compact submachine-gun form with front blade; source domain for repeated Shl00 projectile creation | EXE_AND_CORPUS_CONFIRMED structural |
| 25 | long rocket/missile projectile owned by Shl02 | EXE_AND_CORPUS_CONFIRMED |
| 26 | grappling blade/hook head owned by Shl03 | EXE_AND_CORPUS_CONFIRMED structural |
| 30 | articulated chain/cable segment owned by Shl03 | EXE_AND_CORPUS_CONFIRMED structural |

The higher-level human name `Kalina Ann` for slot20 is a
**SEMANTIC_CANDIDATE** rather than a raw EXE string claim: the canonical
resource is the unique large Lady launcher that produces the rocket and
grappling domains. Keep the structural label as the machine authority.

Likewise, `crossbow/dagger launcher` for slot23 is a human-facing description
of the recovered geometry/action role, not a serialized name field.

### Pass-2 closure status after live EXE pass

Closed:

- component0..4 persistent placement ownership;
- exact BodyStowed vs HandHeld preset semantics;
- all state-entry placement/control groups;
- known script-channel placement/control transitions;
- explicit CEm034 visibility question: no separate per-slot toggle found;
- creation surface for Shl00..05;
- common Shl actor retirement/removal/destruction chain;
- per-class retire conditions for Shl00..05;
- structural slot identities for the weapon/projectile/grapple resource family.

Still deliberately not promoted:

- left-hand/right-hand naming for slots21/22 until body coordinate handedness is explicitly named;
- exact serialized/artistic name for slot23;
- one-to-one semantic names for Shl00/Shl01/Shl04 beyond their recovered runtime roles;
- Reader implementation / APK build.

The reverse gate for the canonical Lady equipment-binding architecture is now substantially closed.
The next phase may define the C++20 canonical binding model from this evidence without restoring
the rejected joint9 shortcut.


## Pass 2H — equipment semantic identity and human placement labels

### Scope and evidence rule

This pass assigns human-readable *role* labels only where the live canonical
EXE behavior and the extracted canonical MOD corpus converge.

Proper-name promotion remains stricter. In particular, the boss resource does
not expose a canonical `Kalina Ann` string, so the launcher role can be
promoted while the exact proper-name binding remains unpromoted.

### Canonical corpus identity

The following MOD payloads were re-read from the supplied corrected em034
extract. Their triangle strips were reconstructed from the canonical MOD
control stream rather than inferred from filenames.

| slot | size | nodes | vertices | SHA-256 |
|---:|---:|---:|---:|---|
| 20 | 38720 | 3 | 955 | `160fa4ac612ddf6c62bd00ca248ed612f7410488f406f4e4990cb5c10b6be74b` |
| 21 | 22048 | 1 | 544 | `ee62e8b87b66d0b01609df40089fea11d308dc6b02238c7a07ac2acaec8a4218` |
| 22 | 23648 | 1 | 584 | `8843fb42321896b5846ddb4faf93741dc1193a590fe31767ccf95ae96c699464` |
| 23 | 23904 | 4 | 587 | `b1ca603658913e06e18468ee801deea2ac237bf8b0ea1ab50de7a8eab0591fec` |
| 24 | 34000 | 1 | 842 | `e0a70ac54cf82ad69cb1c607554d22bf292048dae107738c7fa0dc989c975c97` |
| 25 | 2800 | 1 | 62 | `f65eeb79ebdcc07e2091eed191929fcc97516901d4bd6af396deb5e056385169` |
| 26 | 8800 | 2 | 212 | `002f5d83b49f6ee498c745f158fb524da2aaee91d42a4eb8ef0c23fe8d347833` |
| 30 | 3744 | 5 | 82 | `3f2f749edfd3c52bb27c3d0013b4e0eeb2b31f71e9adcf70012b72de280a1dcc` |

Axis-aligned geometry dimensions from decoded float3 position streams:

| slot | dimensions |
|---:|---|
| 20 | 150.34 × 32.68 × 27.00 |
| 21 | 21.89 × 2.54 × 13.65 |
| 22 | 25.75 × 2.54 × 14.42 |
| 23 | 30.90 × 4.69 × 18.55 |
| 24 | 42.28 × 4.67 × 20.14 |
| 25 | 6.81 × 6.81 × 111.78 |
| 26 | 38.53 × 3.98 × 11.59 |
| 30 | 28.00 × 1.67 × 1.67 |

### Human role map

**EXE_AND_CORPUS_CONFIRMED role semantics**

| resource | canonical role | evidence |
|---|---|---|
| component0 / slot20 | launcher assembly | long launcher geometry; three-node articulated MOD; independent em034_013 motion controller; active placement during missile and grapple state families |
| component1 / slot21 | handgun A | complete handgun silhouette; one-node rigid MOD; active hand-end placement; paired with component2 in action44 |
| component2 / slot22 | handgun B | second complete handgun silhouette; one-node rigid MOD; opposite hand-end placement; paired with component1 in action44 |
| component3 / slot23 | vertical bowgun / crossbow | reconstructed mesh has stock/body plus the characteristic transverse/vertical bow limb; four-node MOD; action46 active placement and repeated Shl05 projectile creation |
| component4 / slot24 | machine gun / SMG-class automatic firearm | compact long-magazine automatic-firearm silhouette; action50 active placement; Shl00 creation path is sourced from its runtime transform |
| slot25 / Shl02 | missile / rocket projectile | very long narrow projectile geometry; dynamically owned by Shl02; actions3/4/5 spawn Shl02 at frame4; Shl02 has a recovered raw lifetime countdown initialized to 3.0 (time unit not yet promoted) |
| slot26 / Shl03 | grappling blade / hook head | pointed/serrated hook/blade geometry; dynamic Shl03 ownership; paired with slot30 in the same actor |
| slot30 / Shl03 | tether / cable / chain strip | thin five-node segmented strip geometry; dynamic Shl03 ownership; lifetime is coupled to the owner grapple state family |

The previous semantic candidate `slot23 = hook` is **REJECTED**. The hook/tether
domain is the dynamic Shl03 pair `slot26 + slot30`.

The exact proper name of slot20 is not promoted from these bytes. Its role is
canonically a launcher assembly; associating that boss weapon role with a
franchise proper name is a separate semantic layer.

### em034_001 body skeleton proves active versus stowed placement

The supplied canonical body MOD `em034_001.mod` has 23 transform-domain
nodes. Its bind-pose local rotations are zero to floating-point noise, so the
relevant body chains can be classified directly from the parent/order arrays
and accumulated translations.

Relevant bind nodes:

| node | parent | bind/world position | structural role |
|---:|---:|---|---|
| 3 | 2 | approximately (0, 117.52, -0.52) | central torso/back chain; parent of both arm chains |
| 9 | 8 | approximately (-61.75, 136.84, -2.18) | negative-X arm endpoint / hand endpoint |
| 13 | 12 | approximately (+61.75, 136.84, -2.18) | positive-X arm endpoint / hand endpoint |
| 14 | 1 | approximately (0, 104.70, 0) | pelvis/root of the two leg chains |
| 16 | 15 | approximately (-8.04, 51.83, 0.37) | negative-X lower-leg chain |
| 19 | 14 | approximately (+8.04, 94.43, 0.37) | positive-X hip/upper-leg chain |

The two arm chains are exact mirrors:

```text
3 -> 6  -> 7  -> 8  -> 9
3 -> 10 -> 11 -> 12 -> 13
```

and the two leg chains branch from node14.

This closes the human meaning of the two placement tables:

```text
preset1 = active / held placement
preset0 = stowed / body-storage placement
```

**EXE_AND_CORPUS_CONFIRMED**

The evidence does not yet assign the words `left` and `right` to the
negative-X and positive-X hand endpoints; that requires a separate
model-coordinate handedness/front-axis proof. The canonical model therefore
uses `negative_x_hand` and `positive_x_hand` if a side-distinguishing
identifier is required.

### Per-component human placement meaning

| component | preset0 | preset1 |
|---:|---|---|
| 0 / launcher | back/torso-stowed, body node3 | active/held on hand endpoint node9 |
| 1 / handgun A | pelvis/holster-stowed, node14 | active/held on hand endpoint node9 |
| 2 / handgun B | leg/holster-stowed, node16 | active/held on hand endpoint node13 |
| 3 / bowgun | hip/body-stowed, node19 | active/held through the special runtime hand-domain transform `CEm034+0x43C0` |
| 4 / SMG | pelvis/body-stowed, node14 | active/held on hand endpoint node13 |

For component3, the serialized preset1 record contains node13 but the effective
runtime parent remains the already-proven special transform
`CEm034+0x43C0`; therefore a Reader must keep the special parent override and
must not simplify this row to an ordinary node13 attachment.

### Semantic promotion boundary

Now closed for canonical Lady binding:

- component0..4 resource roles;
- dynamic slot25/26/30 roles;
- preset0 = stowed;
- preset1 = active/held;
- back/torso versus pelvis/leg/hip stow domains;
- exact hand-endpoint node indices without inventing left/right handedness.

Still deliberately unresolved:

- proper-name promotion of the launcher to a franchise weapon name;
- negative-X hand versus positive-X hand -> human left/right labels;
- exact gameplay names for Shl00, Shl01, Shl04 and Shl05 as actor classes
  beyond their recovered source/attack domains.

These remaining naming details do not block the canonical
EquipmentBinding/WeaponBinding structural model.


## Pass 2I — complete CEm034 MotionScript entry dispatcher

### Canonical state-entry map

**EXE_CONFIRMED**

Live disassembly of `0x14016A410` closes the complete CEm034 state-entry
MotionScript start map for states `0x00..0x8F`. The function uses the
switch byte map at `0x14016ACAC` and offset table at `0x14016AC48`.

The important architectural result is that CEm034 does **not** always start
one shared action on both body-script controllers.

The two `em034_012` controllers are:

```text
lane0 = CEm034+0x5070
lane1 = CEm034+0x5190
```

For ordinary both-lane entries, `0x140171AB0(..., selector=-1)` starts the
same bank/action on both. States 55..82 instead use direct controller starts
and can run different actions concurrently.

### Full recovered body-script starts

```text
bank0
 state 0..6   -> action 0..6   both lanes
 state 7      -> no body script
 state 8..14  -> action 8..14  both lanes

bank1
 state 15..20 -> action 0..5   both lanes
 state 21     -> no body script
 state 22     -> action 7      both lanes
 state 23     -> no body script
 state 24..26 -> action 9..11  both lanes
 state 27..34 -> no body script
 state 35     -> action 20     both lanes
 state 36..37 -> no body script
 state 38..41 -> action 23..26 both lanes

bank2
 state 42..45 -> action 0..3   both lanes

bank3
 state 46     -> action 0      both lanes
 state 47     -> no body script
 state 48..54 -> action 2..8   both lanes

 state 55..61:
   lane0 -> bank0/action1
   lane1 -> bank3/action9..15

 state 62..68:
   lane0 -> bank3/action16
   lane1 -> bank3/action16..22

 state 69..75:
   lane0 -> bank3/action23
   lane1 -> bank3/action23..29

 state 76..82:
   lane0 -> bank3/action30
   lane1 -> bank3/action30..36

bank4
 state 83..115 -> action 0..32 both lanes
 state 116..122 -> no body script
 state 123..130 -> action 40..47 both lanes
 state 131..132 -> no body script
 state 133 -> action 50 both lanes
 state 134..142 -> no body script
 state 143 -> action 60 both lanes
```

### Independent slot20 controller starts

**EXE_CONFIRMED**

The separate `em034_013` controller at `CEm034+0x52B0` is started in
addition to the body controllers for these recovered state-entry ranges:

```text
states 90..93   -> bank4 actions 7..10
states 100..115 -> bank4 actions 17..32
state 123       -> bank4 action40
state 124       -> bank4 action41
state 125       -> bank4 action42
```

States 89 and 97 retain the already recovered delayed slot20-script start
through lane0/channel0 rather than starting it at entry.

### Runtime consequence

The previous simplification

```text
one CEm034 state -> one MotionScript action -> duplicate signal stream on both lanes
```

is **REJECTED**.

Canonical playback requires two independent lane timelines. This matters
concretely for state `0x59`: lane1 runs bank3/action13 while lane0 runs
bank0/action1; the lane0 signal is the trigger that hands slot20 to the
independent `em034_013` controller.

The Native Reader Script Play implementation now reconstructs both entry
controllers separately and evaluates each lane's own opcode-3 timeline.

Status: **EXE_CONFIRMED**.


## Pass 2J — dynamic Shl02/Shl03 visual transform authority

### Shl02 / slot25 spawn transform

**EXE_CONFIRMED**

The only recovered call to `CEm034Shl02::factory 0x140173620` is at
`0x140169972`.

At that site the caller operates through the `CEm034+0x250` secondary
subobject. Resolving those offsets back to primary CEm034 proves:

```text
[rdi+0x698] == CEm034+0x8E8 == component0 / slot20 runtime manager
```

The caller reads that manager's current matrix at `manager+0x110` and passes
its translation/current-origin row to the Shl02 factory. The factory copies:

```text
spawn vector -> Shl02+0x80
direction     -> Shl02+0x140
runtime arg   -> Shl02+0x530
mode byte     -> Shl02+0xD6D
```

The Shl02 visual manager lives at `Shl02+0x540`. Its update path passes
`Shl02+0x1A0` to the visual manager through vslot `+0x198`.

Therefore the canonical visual rule is:

```text
slot25 visual
  -> CEm034Shl02 actor transform +0x1A0
  -> spawned from the live component0/slot20 world domain
```

It is **not** a body-joint attachment.

The orientation path builds the actor basis from the direction vector through
`0x14032FD90`; that helper normalizes/crosses basis vectors and writes the
4x4 actor orientation domain.

### Shl03 / slots26+30 transform

**EXE_CONFIRMED**

The CEm034 call at `0x14016CC41` creates `CEm034Shl03`.

Before the call CEm034 constructs a spawn position from a live manager current
matrix plus an EXE transform offset, supplies the scaled CEm034 direction
domain, and passes the primary CEm034 owner as `r9`.

The Shl03 factory stores:

```text
spawn position -> Shl03+0x80
direction      -> Shl03+0x140
runtime arg    -> Shl03+0x560
CEm034 owner   -> Shl03+0x1510
```

Shl03 constructs two visual-manager domains at approximately:

```text
Shl03+0x580
Shl03+0xD00
```

The recovered visual update path at `0x140175010` supplies the same actor
transform `Shl03+0x1A0` to both manager domains through vslot `+0x198`.

Combined with the resource ownership already closed earlier:

```text
Shl03 visual domain A -> PAC slot26
Shl03 visual domain B -> PAC slot30
```

the canonical presentation boundary is now:

```text
slot26 + slot30
  -> one independent CEm034Shl03 actor
  -> common actor transform +0x1A0
```

The internal chain/tether point simulation maintained by the larger Shl03
update path remains a separate deformation layer. A Reader may reproduce the
actor transform immediately, but must not claim exact slot30 cable deformation
until that point-array simulation is ported.

### Dynamic texture companions

**CORPUS_CONFIRMED**

The retained canonical corpus pairing is:

```text
costume1:
  slot25 -> PTX19
  slot26 -> PTX19
  slot30 -> PTX29

costume2:
  slot25 -> PTX33
  slot26 -> PTX33
  slot30 -> PTX29
```

These resources remain dynamic/latent and must not be restored to the
persistent seven-part Lady composite.

### Promotion

The following are now closed for dynamic presentation:

- slot25 visual owner = Shl02;
- slots26+30 visual owner = Shl03;
- visual transform authority = actor `+0x1A0`, not a guessed body joint;
- spawn domains are recovered from live CEm034 runtime matrices;
- texture companions are corpus-confirmed.

Still open for pixel/runtime parity:

- exact Shl02 actor travel/collision integration throughout its lifetime;
- exact Shl03 cable point-array deformation;
- full visual-resource identity for Shl00/Shl01/Shl04/Shl05 where no dedicated
  top-level MOD slot is yet promoted.

Status: **EXE_AND_CORPUS_CONFIRMED visual ownership/transform boundary**.


## Pass 2K — small Shl visual census and Shl03 tether target chain

### Shl00 / Shl01 / Shl04 / Shl05 do not own dedicated MOD managers

**EXE_CONFIRMED bounded class-range census**

The class implementation ranges for the four small shell actors were scanned for
the DMC3 runtime MOD-manager construction/access surfaces used by Shl02/Shl03
(`0x140089270`, `0x140089DE0`, manager transform vslot `+0x198`).

Results:

```text
CEm034Shl00: no dedicated visual-manager construction
CEm034Shl01: no dedicated visual-manager construction
CEm034Shl04: no dedicated visual-manager construction
CEm034Shl05: no dedicated visual-manager construction
```

A `0x140089270` occurrence at the upper boundary of the Shl01 census belongs
to the following Shl02 constructor, not Shl01.

These four actors are therefore retained as shell/effect/collision runtime
events. Reader must not invent a top-level MOD attachment for them. Dedicated
latent 3D MOD presentation remains confined to the evidence-backed:

```text
Shl02 -> slot25
Shl03 -> slots26 + 30
```

### Shl02 lifecycle correction

**EXE_CONFIRMED**

The raw `Shl02+0xD68 = 3.0` value is not the complete actor lifetime and must
not be described as three seconds.

The recovered default-timescale phase sequence is:

```text
spawn
state1:
    CShell helper 0x140244870
    flags = 3
    normalized direction * scalar 30.0
    position += actorDelta * velocity
    Shl02 overrides shell phase timer +0x17C = 1.0
    -> transition to state2 after the first default actor update

state2:
    initialize +0xD68 = 3.0
    subtract actorDelta each update
    transition to state3 only when remaining < 0

state3:
    shared retire path 0x1403261E0
```

`CActor::tick 0x1403261B0` computes actor delta `+0x14` from the global
time-scale selector and actor-local `+0x18`; the base constructor initializes
the local factor to 1.0. Thus, in the Reader's default game-frame domain, the
no-world/no-early-collision presentation path retires on the sixth actor update
from spawn, not at `spawn + 3`.

Gameplay collision/steering may terminate or redirect the shell earlier and
remains authoritative when world collision is present.

### Shl03 slot30 target chain

**EXE_CONFIRMED**

`0x140174050` updates the five-node slot30 visual domain from a five-point
chain. The owner anchor is slot20 node2:

```text
CEm034+0x8F8 -> slot20 node2 manager -> current matrix +0x110
A = node2 world translation
B = Shl03 actor position (+0x80)
```

The target positions written into the Shl03 chain container are:

```text
P0 = A
P1 = A + 0.25 * (B - A)
P2 = A + 0.50 * (B - A)
P3 = A + 0.75 * (B - A)
P4 = B
```

The chain container at `Shl03+0x1480` is allocated by
`0x1402C8F60(..., count=5, spacing=70)`; each element has stride `0x80`.
The visual-manager domain at `Shl03+0xD00` exposes five nodes.

For each visual node, the update writes:

- translation from the corresponding current chain point;
- orientation through `0x14032EEE0` from the chain direction;
- a longitudinal scale derived from segment length divided by the canonical
  spacing 70.0.

This closes the geometry target and node-projection contract.

### Remaining tether solver boundary

The chain container stores target/current/history fields and a current sample
index. The temporal target-to-current/history evolution is a shared generic
chain subsystem rather than Lady-specific code. Its allocation/init family is
`0x1402C8F60..`; target projection is fully known, but the exact temporal
smoothing/history update used by Shl03 has not yet been promoted.

Therefore:

- Shl03 actor transform: **EXE_CONFIRMED**
- slot30 five-point target chain: **EXE_CONFIRMED**
- slot30 node projection/orientation/segment scale: **EXE_CONFIRMED**
- generic temporal tether smoothing/history: **PRESERVED_UNDECODED**

A Reader may show the exact actor-level slot30 transform and exact straight
target chain, but must label temporal cable lag/smoothing as non-parity until
the shared chain solver is closed.
