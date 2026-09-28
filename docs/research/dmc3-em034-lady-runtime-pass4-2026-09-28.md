# DMC3 HD — em034 Lady canonical equipment binding — pass 4

Date: 2026-09-28  
Branch: `reverse/em034-lady-runtime-20260927`  
Canonical executable SHA-256:  
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

Promote the completed em034 reverse into one reusable upstream contract for
Native Reader and later DMC Rengine runtime/editor work.

This pass does not build an APK and does not change the Android/Windows Reader.
It defines the canonical model that those clients must consume.

Reverse authorities:

- Pass 1: CEm034 ownership, placement records, MotionScript controllers.
- Pass 2 / 2G: state/action/channel -> component placement transitions.
- Pass 2E: CEm034Shl00..05 retire-state conditions.
- Pass 2F: persistent-component visibility closure.
- Pass 2H: equipment role and active/stowed semantic promotion.

## Canonical implementation

Header:

`include/dmc_rengine/profiles/dmc3/em034_lady_equipment_contract.hpp`

Regression test:

`tests/em034_lady_equipment_contract_tests.cpp`

CMake target stem:

`em034_lady_equipment_contract`

## Model boundary

The canonical Lady model has two different resource domains.

### Persistent equipment components

Exactly five CEm034-owned persistent components exist:

| component | MOD slot | role | manager | CCnsMatrix host |
|---:|---:|---|---:|---:|
| 0 | 20 | launcher | +0x08E8 | +0x4000 |
| 1 | 21 | handgun A | +0x0900 | +0x40C0 |
| 2 | 22 | handgun B | +0x0908 | +0x4180 |
| 3 | 23 | bowgun | +0x0910 | +0x4240 |
| 4 | 24 | machine gun | +0x0930 | +0x4300 |

Each owns two placement records:

- `stowed` = former preset0;
- `active_held` = former preset1.

These are placement states, not visibility states.

Loaded persistent components remain present. The canonical CEm034 surface has no
independent per-component hide/show flag.

### Dynamic CShell actors

`CEm034Shl00..05` are not persistent equipment components. They are
independent `CShell -> CActor` instances with their own actor/collision
lifecycle.

Dedicated dynamic model resources currently proven:

| actor | model resource |
|---|---|
| CEm034Shl02 | slot25 + texture slot19 — missile/rocket projectile |
| CEm034Shl03 | slot26 + texture slot19 — grappling blade/hook |
| CEm034Shl03 | slot30 + texture slot29 — tether/cable strip |

Slots25/26/30 must therefore never be inserted into the default body-equipment
attachment list.

## Special component0 control domain

Component0 / slot20 has two independent state axes:

1. placement: `stowed | active_held`;
2. control domain:
   - `body_constraint`;
   - `independent_motion_script`.

The independent controller is:

`CEm034+0x52B0 = em034_013`

with control gate:

`CEm034+0x4020`.

A Reader model that collapses this into one weapon-state byte is non-canonical.

## Special component3 active parent

Component3 / slot23 active record serializes body joint13, but the runtime
consumer does not use the ordinary body-joint parent.

Effective active parent:

`CEm034+0x43C0`

Therefore the contract stores:

- serialized joint = 13;
- parent kind = internal runtime transform;
- effective body joint = none;
- runtime parent offset = `0x43C0`.

Any implementation that simply attaches slot23 active placement to body
joint13 loses canonical runtime behavior.

## MotionScript transition contract

The profile stores placement transitions separately from state-entry overrides.

Important regression anchors include:

- action44/state0x7F:
  - component1 -> active;
  - component2 -> active.
- action46/state0x81:
  - component3 -> active on entry;
  - lane1/ch2 value1 -> stowed.
- action50/state0x85:
  - component4 stowed on entry;
  - lane1/ch1 value1 -> active;
  - lane1/ch1 value2 -> stowed.

The former action50 mapping:

`frame39 / lane1 channel0 value2 -> component4 stowed`

is rejected. Frame39 belongs to the separate +0x57DD flag/timer path. The
actual placement reset is frame64 / lane1 channel1 value2.

## Native Reader migration status

The pre-reverse viewer candidate that grouped `slots20..26 + 30` on body
joint9 is **REJECTED and removed** from
`fix/em034-lady-assembly-info-export`.

The Reader now implements the canonical split:

1. only slots20..24 are persistent equipment;
2. both costumes default to the exact stowed placement records;
3. slot20 placement and independent MotionScript control-domain state are
   separate axes;
4. slot23 active placement uses the exact RuntimeBodyRootScaled parent;
5. slots25/26/30 remain outside the seven-part static Lady composite;
6. Shl02/03 resources are retained as latent runtime visuals;
7. state/channel transitions drive the persistent components;
8. `em034_012` and `em034_013` are retained simultaneously;
9. raw MOT playback is separate from Script Play;
10. script playback is lane-aware and uses the recovered state/action map;
11. slot30 is skinned from the EXE-confirmed five per-frame tether matrices;
12. no Lady-specific all-weapons->joint9 fallback remains.

## Evidence-safe human names

Promoted role names:

- launcher;
- handgun A;
- handgun B;
- bowgun;
- machine gun;
- missile/rocket projectile;
- grappling blade/hook;
- tether/cable.

Still intentionally not promoted:

- boss slot20 proper name "Kalina Ann" from EXE/corpus alone;
- negative-X vs positive-X hand -> human left/right label;
- exact gameplay labels for Shl00/Shl01/Shl04/Shl05.

These naming gaps do not block the binding model.

## Test contract

The regression test asserts:

- exactly five persistent component slots: 20..24;
- slots25/26/30 are dynamic, never persistent;
- component0 independent MotionScript metadata;
- component3 active special-parent behavior;
- Shl02 and Shl03 resource ownership;
- action44 paired-handgun entry;
- action46 bowgun return-to-stowed signal;
- action50 channel1 value1/value2 placement pair;
- absence of an action50 channel0 placement transition.

## Current gate status

PASS 1 — reverse skeleton/ownership: **closed for binding scope**  
PASS 2 — channel/lifecycle/visibility matrix: **closed for binding scope**  
PASS 3 — end-to-end script/runtime bridges: **closed for required binding transitions**  
PASS 4 — canonical Rengine EquipmentBinding contract: **implemented**  
PASS 5 — Native Reader canonical migration: **implemented**  
PASS 6 — Reader host/native integration tests: **implemented**  
PASS 6A — exact-head CI before final slot30 fidelity pass: **GREEN**  
PASS 6B — final slot30/source-scene exact-head CI: **running**  
PASS 7 — APK/device acceptance: **blocked only on final green exact-head gate**

Validated Reader behavior now includes:

- both Lady costumes through one CEm034 runtime;
- five persistent component bindings with exact placement records;
- exact component3 body-root/scale bridge;
- two simultaneous MotionScript controllers;
- five-channel opcode3 timelines;
- lane-exact script state/action mapping;
- raw MOT and Script Play as separate modes;
- corrected action50 signal trace;
- Shl02 exact spawn pose with external gameplay-target trajectory boundary;
- Shl03 actor presentation;
- slot30 per-frame five-bone tether skinning through the canonical MOD skin
  palette.

PR #104 remains DRAFT and must not merge until physical Android acceptance.
