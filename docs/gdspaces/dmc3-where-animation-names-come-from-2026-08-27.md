# Where an animation's name comes from — 2026-08-27

**Image:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

An open question has been sitting under this project's whole naming argument.
The animation registry types a resource **by name and never by bytes**. A
relative-slot container **stores no names**. Both were established with
instruction evidence. So where does the name come from?

Following the registrar's callers answers it.

## 1. The key is a pair, and the group is formatted

The registrar builds its key with `"%s/%s"` from a group and a name. Three call
sites reach it, at `0x1402D50C4`, `0x1402D5414` and `0x1402D5854`, and all
three build the group the same way:

```
lea  r8,  [rdi + 0x28]              ; a name held on the demo object
lea  rdx, [rip -> 0x140506BC8]      ; "demo/%s"
lea  rcx, [rsp + 0x140]             ; buffer
call 0x14002FF80                    ; format
lea  rcx, [rdi + 0xB4B0]            ; the animation registry table
lea  r8,  [rsp + 0x40]              ; the name token
lea  rdx, [rsp + 0x140]             ; the formatted group
call 0x1402E0020                    ; find-or-register
```

So a motion is registered as **`demo/<demo name>/<file>`**.

## 2. The name half is a token in a text script

Each of the three sites sits inside the same loop shape: pull the next token,
skip a line beginning with `#`, compare the token against a keyword, and on a
match format the group and register.

The keywords are:

| site | keyword | animation type |
|---|---|---|
| `0x1402D5854` | `Motion` | 0, motion |
| `0x1402D50C4` | `Camera` | 2, camera |
| `0x1402D5414` | `Hide` | 3, hide |

Three commands, three of the six animation type codes, matching exactly.

**So the names are neither invented by this tool nor stored in the container.
They are written in a script the container never sees.** That is the honest end
of the naming question for animation, and it explains why an unpacked folder
can never carry them: the folder is not where they live.

## 3. The script's vocabulary

The keyword pool at `0x140506950` holds the whole command set:

```
Load  Set  Offset  Model  Motion  Hide  Facial  Shape  WorkRate
Camera  Effect  EffectI  Light  PadVibe  Fade  Quake  Program
Message  Sound  ScrEfc  Clip  SetFrame
```

and the argument keywords that follow them:

```
SkipFrame  CutFrame  ClipScale  ChangeType  Life  SrcValue  DistValue
DistRGBA  No  Arg  Msg  LMotorSt  LMotorEd  SMotorSt  SMotorEd
```

with three path shapes: `demo/%s`, `/demo/%s`, `/demo/%s/%s`.

Three of those commands are already known to name a resource. The rest name
something too, and no script file exists in any supplied corpus, so the
**vocabulary is recovered and the grammar is not**. `script_corpus_available`
is `false` and stays that way until a script is in hand.

## 4. `.c1d` identified: ClothSim1D

The first resource registry has carried a third type since it was recovered —
`.c1d`, code 6 — with nothing behind it. Nobody here had ever looked at one.

At `0x1402C8D2D` its parser takes the file's first token and compares it for
equality against `ClothSim1D`, bailing when it differs. So `.c1d` is a **text
format whose first token names it** — the only self-identifying text format
found in this game so far.

The keyword pool immediately after that literal is its vocabulary:

```
Gravity  SpringForce  Damping  MaxSpeed  FloorLevel  Cut  End
ClothNo  Wind  WindLocal  WindParent  Stiffness  WindType
LimitLength  Bone  NX  NY  NZ  ClothNum
```

Cloth simulation. The classifier now recognizes the first token, which costs a
prefix compare and claims nothing about the grammar. No `.c1d` file exists in
any corpus, so `corpus_available` is `false` and the grammar is deliberately
unguessed.

`End` is the one token both vocabularies share — both parsers use it to close a
block — and the test says so rather than letting the overlap look accidental.

## 5. What this settles and what it does not

Settled: for `Motion`, `Camera` and `Hide`, a resource's name is a script
token, and its identity is a group/name pair rather than a bare filename. A
tool that shows a bare filename for one of these is showing half an identity.

Not settled: the script grammar, the remaining nineteen commands, and every
`.c1d` field. All three need one real file, not more reading.

## 6. Where it lives

- `include/dmc_rengine/profiles/dmc3/demo_script_contract.hpp` — both
  contracts, `DemoScriptContract` and `ClothSim1dContract`
- `src/gdspaces/classifier.cpp` — `c1d` by first token
- `tests/demo_script_naming_tests.cpp`
- `data/reverse/dmc3-type-identification-windows.v1.json` — six new windows
