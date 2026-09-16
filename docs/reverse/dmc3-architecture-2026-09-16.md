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

## Open work

- **the COM vtable surface behind D3D11.** Those calls are indirect dispatch
  through published interface layouts; resolving them uses public API knowledge
  rather than anything read from the file, which is a different kind of claim
  and should be labelled as one if it is made;
- **which functions form each subsystem.** The import edges name 17 FMOD
  callers and 9 Media Foundation callers; walking the call graph outward from
  those would bound the audio and video layers by reachability;
- **`CWork` itself** — 264 classes derive from it and its layout is the engine's
  universal object header.
