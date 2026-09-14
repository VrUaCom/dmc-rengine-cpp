# DMC3 HD executable structural reverse (2026-09-13)

Complete structural pass over the canonical DMC3 HD executable: every PE
directory parsed, the unwind-backed function inventory enumerated, and the
MSVC run-time type information recovered into a class graph.

**Artifact:** SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`,
6,356,432 bytes — the same canonical research target recorded in
[`dmc3-hdc-phase12.evidence.json`](../../evidence/known-targets/dmc3-hdc-phase12.evidence.json).

**Evidence:** [`dmc3-hdc-structural-reverse.evidence.json`](../../evidence/executable/dmc3-hdc-structural-reverse.evidence.json)
**Full report:** [`dmc3-hdc-structural-reverse.report.json`](../../evidence/executable/dmc3-hdc-structural-reverse.report.json)

Regenerate locally against your own copy:

```bash
dmc-rengine analyze-exe /path/to/dmc3.exe --out report.json
```

The report is deterministic: identical bytes produce byte-identical output.

## What this pass establishes, and what it does not

It reads structure the image declares about itself. That is a real and useful
result — it is the scaffolding any later work hangs on — but it is not
decompilation and must not be summarized as such.

**Established:** build identity, section and directory layout, the complete
import and export surface, the count and extent of compiler-emitted function
ranges, the relocation and TLS directories, and the polymorphic type graph with
per-subobject vtable offsets.

**Not established:** what any of the 12,235 functions does. No function is
named, no parameter list is recovered, no field layout beyond RTTI subobject
offsets is known, and no behavior is claimed. Per-function recovery is a
separate program with its own evidence requirements.

## Build identity

| Field | Value |
| --- | --- |
| Format | PE32+ x86-64, GUI subsystem |
| ImageBase | `0x140000000` |
| Entry point | RVA `0x34615C` |
| SizeOfImage | `0xDAC000` |
| Link timestamp | 1521798082 (`0x5AB4CBC2`) — 2018-03-23T09:41:22Z |
| DllCharacteristics | `0x8120` — `HIGH_ENTROPY_VA`, `NX_COMPAT`, `TERMINAL_SERVER_AWARE` |
| CodeView GUID | `8DACCD58-89B6-4E0C-9B74-EC2A1292FC90`, age 1 |
| PDB path | `C:\dev\dmc\dmc3\build\x64\dmc3.pdb` |

The CodeView record is the strongest fingerprint the image carries: two builds
of the same source differ here even when every other field matches. The PDB
path also corroborates the build-tree root `c:\dev\dmc\dmc3\` that appears in
embedded shader paths.

The timestamp is a timestamp by specification. Whether this build used a real
clock or a reproducible-build substitute is not established.

Two flag observations are worth recording because they are counter-intuitive:

- `DYNAMIC_BASE` is **clear**, yet the image ships 35,936 base relocations and
  sets `HIGH_ENTROPY_VA`. The image is relocatable but does not opt into ASLR,
  so it loads at `0x140000000` unless something else forces a rebase — which
  makes RVA-to-VA reasoning about this build simpler than it usually is for a
  64-bit image.
- `GUARD_CF` is **clear**, yet a `.gfids` section is present. Control-flow
  guard metadata was emitted without the feature being enabled in the final
  link.

## Sections and directories

| Section | RVA | Virtual size | Raw size | Traits |
| --- | --- | --- | --- | --- |
| `.text` | `0x1000` | 3,464,526 | 3,464,704 | code, execute/read |
| `.rdata` | `0x34F000` | 2,110,312 | 2,110,464 | read-only data |
| `.data` | `0x553000` | 8,516,024 | 548,864 | read/write |
| `.pdata` | `0xD73000` | 146,820 | 146,944 | exception data |
| `.gfids` | `0xD97000` | 80 | 512 | control-flow guard |
| `.tls` | `0xD98000` | 9 | 512 | thread-local storage |
| `.rsrc` | `0xD99000` | 3,888 | 4,096 | resources |
| `.reloc` | `0xD9A000` | 73,136 | 73,216 | base relocations |

`.data` reserves 8.5 MB of virtual space against 536 KB of file data: the bulk
is zero-initialized state the image allocates at load rather than ships.

Present directories: export, import, resource, exception, certificate, base
relocation, debug, TLS, load config, IAT. There is no delay-import directory
and no CLR header.

Resources are minimal — one icon, one icon group, one manifest. Nothing of the
game's content lives in the executable's resource tree.

## Function inventory

The x64 exception directory holds **12,235 `RUNTIME_FUNCTION` entries**
covering **3,085,665 bytes**, or 89.1 percent of the `.text` virtual size.

> **Corrected 2026-09-14.** Those 12,235 entries resolve to **7,389 functions**
> plus 4,846 continuation ranges. See
> [the function map](dmc3-function-map-2026-09-14.md#correction-7389-functions-not-12235).

| Function size | Count |
| --- | --- |
| ≤ 16 bytes | 593 |
| ≤ 32 | 919 |
| ≤ 64 | 2,065 |
| ≤ 128 | 2,539 |
| ≤ 256 | 2,618 |
| ≤ 512 | 2,161 |
| ≤ 1,024 | 947 |
| ≤ 2,048 | 296 |
| ≤ 4,096 | 73 |
| ≤ 8,192 | 20 |
| ≤ 16,384 | 3 |
| ≤ 32,768 | 1 |

Two cautions on reading this table. The entry count is a count of *ranges*, not
of functions — the correction above resolves it. And the uncovered 10.9
percent of `.text` is *not* established to be non-code — it is simply not
covered by unwind data, which also describes import thunks and padding.

## Import surface

26 modules, 229 functions, no delay imports.

| Attribution | Modules | Functions |
| --- | --- | --- |
| Render | `d3d11.dll`, `dxgi.dll` | 2 |
| Audio | `fmod64.dll`, `fmodstudio64.dll` | 36 |
| Video | `MFPlat.DLL`, `MFReadWrite.dll` | 8 |
| Input | `DINPUT8.dll`, `XINPUT9_1_0.dll` | 3 |
| Platform service | `steam_api64.dll` | 7 |
| OS and CRT | 17 modules | 173 |

The attribution column is an inference from the dependency set, recorded at
confidence `high` rather than `confirmed`: an imported library can be
initialized conditionally, or linked and never exercised on some paths.

The counts are informative on their own. Two imports from `d3d11.dll` and
`dxgi.dll` is the signature of a device created once and everything else
reached through COM vtables — so the renderer's surface is invisible to the
import table. FMOD's 33 direct imports are the opposite: a C API called
broadly. Media Foundation appears alongside the `FullMotionVideo` RTTI classes
below, which is the one place two independent sources agree on a subsystem.

## Export surface

Two named exports, neither forwarded:

| Ordinal | RVA | Name |
| --- | --- | --- |
| 1 | `0x32BA0` | `FMODGetCodecDescription` |
| 2 | `0x2C5DF0` | `dmc3_main` |

An executable exporting a symbol named `dmc3_main` is a strong lead: it is a
named, stable anchor into the game's own code at VA `0x1402C5DF0`, distinct
from the CRT entry point at RVA `0x34615C`. Its role is an inference from the
name and remains to be confirmed by behavior.

## Recovered type system

MSVC run-time type information survives in full.

| Measure | Count |
| --- | --- |
| Type descriptors | 408 |
| Complete-object locators | 915 |
| Distinct polymorphic types | 396 |
| Vtables located | 915 |

Locators outnumber types because multiple inheritance gives a type one locator
and one vtable per base subobject; the scanner groups by type descriptor and
keeps every vtable with its subobject offset. The 12 descriptors without a
locator belong to types that are not polymorphic or that appear only as bases.

### The object spine

Base-class descriptors give the actual inheritance graph:

| Base | Derived types |
| --- | --- |
| `CWork` | 264 |
| `ICollisionHandle` | 188 |
| `IActor` | 182 |
| `CActor` | 181 |
| `CShell` | 87 |
| `INonPlayer` | 46 |
| `CNonPlayer` | 45 |
| `CStageSet` | 32 |

`CWork` sits under two thirds of all polymorphic types — it is the engine's
common object base. `CActor` with its `IActor` and `ICollisionHandle`
interfaces forms the spine everything animate hangs from.

The deepest declared hierarchies carry 12 bases, for example:

```text
CEm021 : CEm017 : CNonPlayer : CActor : CWork, IActor, ICollisionHandle, ...
```

### Families

| Family | Types | Reading |
| --- | --- | --- |
| `CEm###` | 103 | enemy classes, numbered, several with `Shl`/`Cart` variants |
| `I*` | 21 | pure interfaces |
| `CCamera*` | 12 | camera modes: boss, rail, player, pan, clip, mini-demo |
| `CDamage*` | 7 | damage resolution, including per-enemy specializations |
| `CDraw*` | 7 | draw paths: SCM, shadow, UV, break, crush, operate |
| `CEfc*` / `CEffect*` | 7 | effects |
| `CCom*` | 6 | AI command objects, per enemy |
| `CCns*` | 5 | constraints: chain, IK, matrix, direction, random |

Widest vtables by total slot count: `CComEm006` and `CComEm008` at 271 slots
across 2 vtables, `CComEm000` at 265. The `CEm01x` enemies carry 6 vtables
each at 148 total slots — that shape (six subobjects) is a direct readout of
multiple inheritance in the object layout.

Only two template instantiations appear: `CList<CLockOnTarget>` and
`CList<CNonPlayer>`.

Names are the compiler's own decorated names, reconstructed mechanically. What
each class *does* is not established by its name or its position in the graph.

## Method

Everything above was produced by the repository's own Binary Inspector, which
this pass extended:

- `PeReader` now parses header fields and the data-directory array;
- `PeDirectoryReader` parses imports (including delay imports and
  ordinal-only entries), exports with forwarder detection, the exception
  table, the debug directory with CodeView identity, base relocations, TLS and
  the resource tree;
- `RttiScanner` recovers type descriptors, complete-object locators, class
  hierarchy descriptors, base-class descriptors and vtables, and reconstructs
  readable names from MSVC decorated names;
- `to_json` emits the deterministic report.

Two design choices matter for trust. Locators are accepted only when the
locator's self-reference field equals its own RVA, which rejects essentially
every coincidental byte pattern and is what makes an unanchored scan
believable. And each directory is recovered independently, so a malformed
import table does not cost the caller the exception table.

Every parser is covered by synthetic PE32+ fixtures built inside the test
files. No original executable bytes appear in the repository.

## Open work

- per-function recovery across the 7,389 functions;
- tying `dmc3_main` to observed behavior rather than to its name;
- COM vtable recovery for the D3D11 render path, which the import table cannot
  see;
- field layouts within the recovered classes — RTTI gives subobject offsets,
  not members;
- correlating the class graph against the resource families already documented
  under [`docs/formats/`](../formats/README.md).
