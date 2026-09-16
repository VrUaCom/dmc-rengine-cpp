# DMC3 HD: the platform surface and the layers behind it (2026-09-16)

What a port has to provide, and what the game is made of — both read from the
compiler's own records.

**Evidence:** [`dmc3-hdc-architecture.evidence.json`](../../evidence/executable/dmc3-hdc-architecture.evidence.json)

## The platform, symbol by symbol

26 modules, 221 symbols, 870 call edges. The *shape* of each says what it is:

| Subsystem | Module | Symbols | Functions |
| --- | --- | --- | --- |
| Render | `d3d11.dll`, `dxgi.dll` | **2** | 1 each |
| Audio | `fmod64.dll`, `fmodstudio64.dll` | 36 | 17 + 3 |
| Video | `MFPlat.DLL`, `MFReadWrite.dll` | 8 | 9 + 2 |
| Input | `XINPUT9_1_0.dll`, `DINPUT8.dll` | 3 | 6 + 1 |
| Platform | `steam_api64.dll` | 7 | 12 |
| Runtime | CRT + `VCRUNTIME140` + `KERNEL32` | 120 | 228 / 201 / 123 |

**Rendering enters through exactly two symbols:**

```text
d3d11.dll   D3D11CreateDeviceAndSwapChain   (1 function)
dxgi.dll    CreateDXGIFactory               (1 function)
```

Everything after device creation goes through **COM interfaces**, not imports —
which is also why the image has so many indirect calls. A renderer port
replaces one entry point and then a vtable surface.

Audio is FMOD. Video is Media Foundation (`MFStartup`,
`MFCreateSourceReaderFromURL`, `MFCreateSample`). Input is `XInputGetState` /
`XInputSetState` with a single `DirectInput8Create` beside it.

The heaviest module by callers is not the graphics stack at all — it is the C
runtime's **maths** library, called from **228** functions.

## Three naming conventions, three layers

| Convention | Count | What it is |
| --- | --- | --- |
| `C*` | 360 | the engine |
| `I*` | 21 | its interfaces |
| `DMC3::*` | **5** | the PC port's shim |

And the five in the `DMC3` namespace are *all* video and audio:
`FullMotionVideo`, `FullMotionVideoManager`, `PCFullMotionVideo`,
`PCFullMotionVideoManager`, `PlaySoundStreamVoiceContext` — an abstraction and
its `PC`-prefixed implementation. That is precisely the shape a port adds, and
it is the whole of what this one added under its own name.

### The platform boundary is not polymorphic

Of the functions calling FMOD, Direct3D, DXGI, XInput or DirectInput, **not one
is a virtual method or a constructor.** Only Media Foundation reaches classes at
all, and they are the shim's own. `DMC3::PCFullMotionVideo` names `IUnknown`
among its bases, as a Media Foundation callback must.

So the port boundary here is a set of free functions, not a hierarchy — which
is a cheaper thing to reimplement than it might have been.

## The engine's design map

The hierarchy descriptors declare **1,341 base relationships**. How widely each
base is named *is* the design:

| Base | Named by | Reading |
| --- | --- | --- |
| `CWork` | **264** | the root of the engine |
| `ICollisionHandle` | 188 | taking part in collision |
| `IActor` | 182 | being an actor |
| `CActor` | 181 | and its implementation |
| `CShell` | 87 | a large intermediate |
| `INonPlayer` | 46 | enemies |
| `ICom` / `IComAction` / `IComActionState` | 29 / 29 / 30 | the command layer, one set per enemy |
| `IPlayer` | **5** | `CPlDante`, `CPlLady`, `CPlNewVergil`, `CPlVergil` |

Collision is declared *more* widely than actorhood — 188 against 182. The
command triple sitting at 29–30 apiece, one per enemy type, is the enemy AI
layer counted exactly.

**Eleven bases carry no vtable of their own** and are absent from any class list
built from vtable coverage. They are read from the descriptors instead, which is
why the hierarchy is taken from there and not assembled from what happens to
have a vtable — the interfaces are exactly the part that would have gone
missing.

> **On content.** Type names and hierarchy the compiler emitted, and imported
> symbol names. No game data.

## How big is each subsystem

Bounding a layer is a question about **edges**, so the 19,420 direct call edges
between inventory functions are now carried through rather than only counted.
Asking which functions reach a module's imports within three calls sizes it:

| Subsystem | Functions within 3 calls | Direct callers |
| --- | --- | --- |
| Audio (`fmod64`) | **70** | 17 |
| Steam | 28 | 12 |
| Input (`XINPUT`) | 21 | 6 |
| Video shim (`MFPlat`) | 14 | 9 |
| **Render (`d3d11`, `dxgi`)** | **4** each | 1 each |
| `KERNEL32` | 3,036 | 65 |
| CRT maths | 2,212 | 171 |

**Four functions.** The renderer's entry point is an island — which is the
import list's two symbols saying the same thing a second way: after device
creation the renderer is a COM surface, and almost nothing funnels into the
call that creates it.

At the other end, two fifths of the image sits within three calls of an
operating-system service.

### The caveat was right for the wrong reason

These figures were first published with a note that 1,177 unresolved indirect
jumps left holes in the call graph. Classifying them shows what they actually
are:

| Form | Count | What it is |
| --- | --- | --- |
| `jmp reg` | 644 | a compiled switch — **637 tables recovered, 98.9%** |
| `jmp [reg+slot]` | **1,160** | a virtual call *in tail position* |
| `jmp [rip+slot]` | 10 | an import thunk |

So the switch recovery was never the problem: **17** jumps are genuinely
unresolved, not 1,177. The rest are dispatch, and lumping them together made a
dispatch site look like a failure to read a table.

Read as dispatch, they belong with the call sites — the census rises from
10,274 to **11,434**. Almost none is on `this` (the receiver analysis gains
exactly one), so the reachability figures remain lower bounds, because a tail
call's target is still unknown. The caveat stands; its cause was misnamed.

And the classes behind the audio layer are **scene** classes — `CSceneGame`,
`CSceneDemo`, `CSceneMisStart`, `CSceneStartMenu` — not an audio hierarchy.
Audio is driven from scene code, which agrees with the platform boundary being
made of free functions.

## Open work

- **the COM vtable surface behind D3D11.** Those calls are indirect dispatch
  through published interface layouts; resolving them uses public API knowledge
  rather than anything read from the file, which is a different kind of claim
  and should be labelled as one if it is made;
- **the 1,160 tail dispatches**, whose targets are unknown for the same reason
  every other virtual call's is, and which keep the reachability figures lower
  bounds;
- **`CWork` itself** — 264 classes derive from it and its layout is the engine's
  universal object header.
