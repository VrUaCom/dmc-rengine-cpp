# DMC3 HD Format-Purpose Closure Pass — 2026-08-27

**Target:** canonical unpacked DMC3 HD analysis executable  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Repository base:** `main@f886f27e62ec9a05b6829df7fd074981a06a4b49`  
**Scope:** residual format identity and subsystem-purpose reconciliation. No source/game binary was modified.

## 1. Result

This pass closes the catalog-level question **“what is each currently observed/named DMC3-HD format for?”** as far as direct evidence permits.

It does **not** claim that every binary field of every family is decoded. The closure rule is stricter:

```text
identity != purpose != schema != writer equivalence
```

For every family, the canonical catalog and machine-readable registry now record the strongest evidence-backed purpose. When only subsystem membership is proven, that is the purpose boundary; acronym expansion and field semantics are deliberately left unresolved.

Machine-readable authority:

- `docs/formats/dmc3-hd-format-purpose-registry.json`
- `docs/formats/dmc3-hd-format-catalog.md`

## 2. Method

The pass combined four evidence surfaces:

1. canonical EXE printable-extension/resource-path census;
2. direct x86-64 disassembly of format classifiers and manager dispatchers;
3. direct original-runtime parser inspection for strong signatures such as `SPUMAPDT`;
4. current hash-bound/corpus and project reverse documentation for families that do not expose an extension or magic in the EXE.

Short ASCII hits were not accepted as format proof. In particular, prior `STE`/`EST` substring hits remain rejected as direct magic evidence.

## 3. Direct original-runtime typed resource classifiers

### 3.1 Primary magic classifier — `0x1402DB1F0`

The canonical executable classifies the first bytes of primary resource payloads into typed branches:

| Identifier | Type id | Status | Purpose boundary |
|---|---:|---|---|
| `MOD` | 0 | `EXE_CONFIRMED` | actor/object/model resource |
| `EFM` | 1 | `EXE_CONFIRMED` | effect-system typed resource |
| `SCM` | 2 | `EXE_CONFIRMED` | stage/scene mesh/model resource |
| `MRP` | 3 | `EXE_CONFIRMED` identity | primary model/render/effect-side manager companion; exact schema open |
| `SHW` | 7 | `EXE_CONFIRMED` | shadow/render typed companion |

The caller/dispatcher `0x1402DB3C0` also applies format-specific post-load work to the known typed branches:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
SHW -> 0x1403204C0
```

`MRP` receives a real type id but does not take one of those four immediate fixup calls in this branch. That is enough to promote its **identity and manager membership**, not to invent field semantics.

### 3.2 Primary extension dispatcher — `0x1402DB3C0`

The same primary manager recognizes case variants of:

| Extension | Type id | Promotion |
|---|---:|---|
| `.ptx` | 4 | PTX original-runtime typed identity |
| `.clt` | 5 | CLT original-runtime typed identity |
| `.c1d` | 6 | C1D original-runtime typed identity |

`CLT` and `C1D` sit beside original parser/control strings including `Cloth`, `ClothSim1D...`, `ResetClt...`. The strongest safe interpretation is therefore:

- `CLT`: cloth/deformation simulation companion;
- `C1D`: one-dimensional cloth-simulation companion.

Both purposes are `HIGH_CONFIDENCE`; exact binary schemas remain open.

## 4. Motion/control resource manager — `0x1402E01A0`

A second original manager assigns resource type ids by extension:

| Extension | Type id | Canonical purpose boundary |
|---|---:|---|
| `.mot` / `.MOT` | 0 | motion/animation |
| `.mcv` / `.MCV` | 1 | motion companion/control/curve family |
| `.cam` / `.CAM` | 2 | camera resource/track family |
| `.hid` / `.HID` | 3 | hide/visibility-control family |
| `.clt` / `.CLT` | 4 | cloth/deformation companion |
| `.tsc` / `.TSC` | 5 | motion/control-manager companion; exact semantics unresolved |

Consequences:

- `MCV` is no longer merely a filename candidate: original-runtime subsystem membership is confirmed.
- base `HID` is directly original-runtime registered; `HID2/HID3` remain corpus variants until their exact variant ABI is recovered.
- `TSC` must not be categorized as a texture format. Its identity and motion/control-manager membership are direct; exact semantic role remains `RESEARCH_REQUIRED`.
- `CLT` crossing both primary and motion/control managers is consistent with a model-deformation/cloth simulation resource.

## 5. SPU sound-memory map parser — `0x140339D80`

`SPUMAPDT` is promoted from acronym candidate to `EXE_CONFIRMED` audio metadata.

The function:

1. validates the first eight bytes against `SPUMAPDT`;
2. reads a count-like dword at `+0x08`;
3. consumes up to 16 32-bit region-size entries beginning at `+0x10`;
4. builds cumulative region boundaries;
5. enforces a cumulative limit of `0x200000` bytes.

The canonical static resource table also names `SpuMap.bin` beside the system sound resources.

Safe purpose:

> **SPU sound-memory/sample-bank partition and region-map metadata.**

Still open: exact mapping of each region to `PHD/TSB/BD` or individual sample payloads.

## 6. System sound companions: PHD / TSB / BD

The canonical executable contains both runtime resource names and source-tree-style paths:

```text
SpuMap.bin
snd_sys.phd
snd_sys.tsb
snd_sys.bd

..\..\mw\CSE\DATA\SpuMap.bin
..\..\mw\CSE\DATA\snd_sys.phd
..\..\mw\CSE\DATA\snd_sys.tsb
..\..\mw\CSE\DATA\snd_sys.bd
```

This promotes their **sound-subsystem identity** to direct executable evidence.

The safest purpose split remains:

- `PHD` — global sound-system bank/header descriptor companion;
- `TSB` — global sound-system table/bank companion;
- `BD` — global sound-system bank body/sample-data companion.

Those are subsystem-purpose descriptions, not asserted acronym expansions or decoded schemas. Exact cross-link fields remain open.

## 7. PTZ

The canonical static asset table contains:

```text
i001_90.tm2
basic.ptz
basic.ptx
at000.mod
```

Historical product-routing evidence also groups `PTZ` with texture resources.

Promotion:

- identity/static presence: `EXE_CONFIRMED`;
- purpose: `HIGH_CONFIDENCE` texture-side companion associated with PTX;
- exact parser/consumer/schema: still open.

No direct `.ptz` typed-dispatch branch was found in this pass, so PTZ is not promoted beyond that boundary.

## 8. Newly cataloged families omitted by the previous “all formats” inventory

### 8.1 FON

Canonical executable resource paths include:

```text
font\euro28.fon
font\china20m.fon
font\china20z.fon
```

Purpose: font/glyph definition resource for the UI/text subsystem. Identity/static subsystem context is direct; glyph/atlas schema remains open.

### 8.2 Legacy save/icon sidecars

Canonical executable tables include:

```text
icon.sys
icon00.ico
icon01.ico
icon02.ico
```

Purpose boundary: legacy save/icon metadata and image sidecars retained in the asset catalog. Exact HD runtime use and binary interpretation remain open.

## 9. StageCfg families

Current corpus evidence keeps these StageCfg semantic families distinct:

```text
LIG / LIG2
CAM
EVE
POS
ITM
STE
DCA
EST
```

Important proven/corrected boundaries:

- `ITM` is not generic `.ukn`; stock `stXXXcfg_006.ukn` samples validate the `ITM\0` structure.
- `EVE` is a spatial event-volume format, not mission bytecode.
- `EventTblNN.bin` holds mission/event control logic and remains distinct from EVE.
- `STE` is a scene/effect transform family; direct short ASCII `STE` hits were false positives and do not constitute EXE magic proof.
- `EST` remains a stage dependency/control candidate.

### StageCfg slot-9 relation

At `0x1401AF000`, the original code, when the cfg PAC has at least ten physical slots, reads offset-table entry `+0x2C` (physical slot 9) and passes that payload to `0x1401A9BC0`. The downstream scan contributes dependency/enemy-resource demand.

Corpus correlation:

```text
st001cfg: 10 slots; EST observed among semantic children
st114cfg:  9 slots; EST absent
```

This is strong correlation, but it is **not** enough to assert `slot 9 == EST` universally. That promotion still requires an independent exact raw slot map/header.

## 10. Extension/string census and exclusions

The canonical executable contains large numbers of resource-looking strings, but frequency is not semantic proof. The pass explicitly separates resource families from build/debug/platform noise.

Examples observed in the executable include high counts for `.pac`, `.txt`, `.adx`, `.ogg`, plus smaller typed families such as `.ptx`, `.c1d`, `.clt`, `.mot`, `.mcv`, `.cam`, `.hid`, `.tsc`, `.phd`, `.tsb`, `.bd`, `.fon`, `.ico` and `.ptz`.

The following are **not promoted as DMC3 resource families merely because they occur as extension-like strings**:

- `HLSL`, `FXH` — shader source/build/debug paths;
- `PDB` — debug-symbol path;
- `DLL` — runtime dependency names;
- `DXT1..DXT5`, `ATI1/ATI2`, `BC4/BC5`, `RGBG/GRGB/YUY2` — DDS/pixel encodings or layout tags.

## 11. Media capability versus shipped resources

The executable/media layer exposes support strings for:

```text
PSS THP PAM XMV WMV PMF AVI MPG BIK MP4
```

Only direct distribution/runtime evidence should promote a family to a shipped DMC3-HD resource. The catalog therefore keeps `PSS/THP/PAM/XMV/PMF/AVI/MPG/BIK/MP4` as `capability-only` while `SFD -> WMV` remains an original-runtime logical-to-physical translation.

## 12. Exact semantics still open after purpose closure

“Purpose closure” does not mean “schema closure.” The remaining exact reverse frontier is intentionally explicit:

```text
PTZ       exact consumer/schema
PHD       exact fields and cross-links
TSB       exact fields and cross-links
BD        exact fields and cross-links
EFE       exact original consumer/schema
EFW       exact original consumer/schema
POS       exact record schema/consumer
STE       exact record schema/consumer
EST       exact identity/header/universal slot mapping
MRP       exact field semantics
MCV       exact field semantics
HID*      exact track schema/variant ABI
TSC       exact semantic role and schema
CLT/C1D   exact binary schemas
```

These are not “unknown formats” anymore in the loose sense: most now have a bounded subsystem purpose. What remains unknown is the exact byte contract or finer semantic meaning.

## 13. Canonical documentation changes from this pass

This pass requires the following `main` authority to stay synchronized:

- `docs/formats/dmc3-hd-format-catalog.md` — human-readable purpose catalog;
- `docs/formats/dmc3-hd-format-purpose-registry.json` — machine-readable family registry;
- `docs/formats/README.md` — format documentation entry point;
- `docs/README.md` — project documentation index.

No product writer, game binary, extracted retail resource or original executable is modified by this documentation/reverse pass.