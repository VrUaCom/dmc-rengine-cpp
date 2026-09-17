# DMC3 HD: the startup path (2026-09-17)

Every caveat I had flagged has now been read. So: the one route through this
image that is sound end to end, and which I had never actually looked at.

**Evidence:** [`dmc3-hdc-startup-path.evidence.json`](../../evidence/executable/dmc3-hdc-startup-path.evidence.json)

## Why this path is different

Everything else in the reverse carries an assumption about dispatch. This does
not. Direct calls and tail jumps from the entry point reach **370 of 7,389**
functions, and nothing about that closure is inferred.

| Depth | Functions | Bytes |
| ---: | ---: | ---: |
| 0 | 1 | 18 |
| 1–4 | 49 | 11.0 KB |
| **5–8** | **290** | **116.7 KB** |
| 9–17 | 136 | 37.8 KB |

The entry stub reaches the runtime's own startup by a **tail jump**, not a call.
A closure following calls alone stops at *two* functions instead of 370 — which
is why the depth counts transfers, and why I had to fix it after the first
measurement came back as `2`.

## The whole platform is inside it

97 imported symbols across 18 modules:

| Module | First depth | Functions | Symbols |
| --- | ---: | ---: | ---: |
| `KERNEL32.dll` | 1 | 30 | 32 |
| `VCRUNTIME140.dll` | 2 | 41 | 5 |
| `USER32.dll` | 4 | 3 | 16 |
| `fmod64.dll` | 4 | 3 | 3 |
| `DINPUT8.dll` | 5 | **1** | 1 |
| `d3d11.dll` | 6 | **1** | 1 |
| `dxgi.dll` | 6 | **1** | 1 |
| `steam_api64.dll` | 6 | 2 | 3 |
| `XINPUT9_1_0.dll` | 7 | 4 | 2 |

Depth is the shortest chain of transfers. **It is not an execution order** — it
says how few steps can reach a function, not when it runs.

## Not one object is built here

| | |
| --- | ---: |
| functions on the path | 370 |
| **constructors among them** | **0** |
| functions containing a dispatch | 30 |
| dispatch sites | 68 |
| fixed addresses operated on | 67 |

The boundary is sharp. The sound path brings the platform up and hands off;
everything the engine builds is built on the far side of a virtual call. This is
the same fact that made the rapid-type tightening of
[the reachability bound](dmc3-reachability-2026-09-16.md) collapse to four
vtables — stated here at its source rather than as a symptom.

## Four functions named by what they import

The import table is the compiler's own record, so this is reading, not guessing.

**`0x42890`** — 2,024 bytes, sole caller of either symbol, 6 dispatch sites:
```text
dxgi.dll!CreateDXGIFactory
d3d11.dll!D3D11CreateDeviceAndSwapChain
```
The 6 dispatch sites are the calls on the interfaces it creates.

**`0x47B40`** — 1,217 bytes, sole caller:
```text
RegisterClassExW  CreateWindowExW  ShowWindow  UpdateWindow
GetSystemMetrics  MonitorFromWindow  GetMonitorInfoW  SetWindowsHookExW
DINPUT8.dll!DirectInput8Create
```
Window creation and input-device creation in one function.

**`0x3A9E0`** — `SteamInternal_ContextInit` beside `OutputDebugStringW`.

**`0x41BC0`** — `XInputGetState`, guarded by `GetFocus`.

Each is a single point a port has to replace.

## One function read end to end

**`0x48970`** is 157 bytes, one caller, **no dispatch sites**:

```asm
48998  call [PeekMessageW]
489A0  jz   → return 1                  ; no message
489A6  cmp  eax, 0x12
489A9  ja   → translate/dispatch
489AB  mov  ecx, 0x50004
489B0  bt   ecx, eax                    ; selects 2, 16, 18
489B3  jnc  → translate/dispatch
489CA  call 0x3454C8 → [0x34F700]       ; FMOD_System_UnlockDSP
489CF  xor  al, al                      ; return 0
```

`0x3454C8` is `jmp [rip+0xA232]`, landing on the IAT slot at `0x34F700`, which
the import directory names `fmod64.dll!FMOD_System_UnlockDSP`.

So the audio call is on the **way out**, not once a frame. That mattered: an
FMOD call inside a message pump looked wrong enough to check, and checking it
turned a puzzle into a reading.

> The Win32 headers name the values 2, 16 and 18 `WM_DESTROY`, `WM_CLOSE` and
> `WM_QUIT`. That naming is **public interface knowledge, not read from this
> file**. The values are what the file gives.

## Open work

- the 68 dispatch sites on the path are the handoff. Six of them are in the
  device-creation function and are COM calls on D3D11 interfaces, which would
  need published interface layouts to resolve — a different kind of claim;
- depth is not order. Within one function the call sites are in address order,
  which for straight-line code is execution order, but nothing here establishes
  that across functions;
- 67 of the 200 global blocks are touched during startup. Which ones, and in
  what state they must be, is not measured.
