# DMC3 HD Format and Resource-Purpose Catalog

**Snapshot:** 2026-08-31  
**Reconciliation base:** `main@d358a2e69a98b13d36d42b594c353afd6546ffb8`  
**Canonical analysis EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** DMC3-HD resource identities and capabilities currently supported by canonical EXE reverse, retained corpus evidence, or reviewed product code.

Machine-readable authorities:

- [`dmc3-hd-format-purpose-registry.json`](dmc3-hd-format-purpose-registry.json)
- [`../../data/reverse/dmc3-exe-format-census-20260831.json`](../../data/reverse/dmc3-exe-format-census-20260831.json)
- [`../../data/reverse/dmc3-runtime-type-identification-20260831.json`](../../data/reverse/dmc3-runtime-type-identification-20260831.json)
- [`../../data/reverse/dmc3-primary-3d-family-20260831.json`](../../data/reverse/dmc3-primary-3d-family-20260831.json)

This catalog is exhaustive over the currently observed/named evidence set. It is **not** a claim that every schema, consumer or writer is fully recovered.

## 1. Authority rules

Keep four questions independent:

1. **Identity** — what resource family is this?
2. **Purpose** — what subsystem owns/uses it?
3. **Schema** — which byte fields are actually understood?
4. **Product support** — what can clean C++ safely do today?

Evidence order is:

```text
bounded canonical-EXE classifier/content check/owned tag
 -> validated structural magic/grammar
 -> hash-bound corpus structure
 -> contextual/index/path label
 -> unknown
```

A runtime identity does not imply a complete schema. A filename extension does not override stronger byte evidence. Different runtime classifiers must not be collapsed into one global magic rule.

### Evidence statuses

| Status | Meaning |
|---|---|
| `EXE_CONFIRMED` | direct bounded evidence in canonical `dmc3.exe` |
| `DATA_CONFIRMED` | stable corpus/binary evidence |
| `HIGH_CONFIDENCE` | convergent evidence with a remaining promotion boundary |
| `CANDIDATE` | evidence-linked interpretation, not canonical semantics |
| `RESEARCH_REQUIRED` | identity/subsystem exists; exact purpose/schema remains open |
| `CAPABILITY_ONLY` | media/runtime capability, not DMC resource ownership proof |
| `REJECTED` | contradicted/superseded claim |

## 2. Canonical runtime identification paths

### 2.1 Three-byte registry/content probe — `0x1402DB1F0`

```text
MOD -> 0
EFM -> 1
SCM -> 2
MRP -> 3
SHW -> 7
other -> -1
```

Only bytes `0..2` are checked at this site. Therefore `MODX` is still a MOD runtime identity **on this path**. That does not permit `MODX` to enter a decoder proven only for canonical `MOD ` layout.

### 2.2 Register/classify path — `0x1402DB3C0`

Extension-side classification precedes the three-byte content probe:

```text
.ptx -> 4
.clt -> 5
.c1d -> 6
```

Known normal handlers for content classes are:

```text
MOD -> 0x1402FE3B0
EFM -> 0x1402F7A90
SCM -> 0x1403051B0
SHW -> 0x1403204C0
MRP -> no generic handler established here
```

### 2.3 Container dispatcher — `0x1401B9FA0`

The materialized-child dispatcher handles `MOD`, `EFM`, `SCM`, `SHW`, explicitly compares `EFE` and `EFW` as three-byte sentinels, and recursively dispatches exact four-byte `PNST`.

`EFE` / `EFW` are therefore **EXE-confirmed identities**, even though their exact semantic purpose and binary schema remain `RESEARCH_REQUIRED`.

### 2.4 Four-byte family-mask classifier — `0x1402FD650`

Here the fourth byte **is significant** and must be ASCII space:

```text
MOD  -> 0x10000000
EFM  -> 0x20000000
SCM  -> 0x30000000
MRP  -> 0x40000000
MCV  -> 0x50000000
SHW  -> 0x60000000
```

This independently confirms `MCV ` as a runtime family identity. `MCV` is absent from the three-byte registry probe, so the two classifiers must remain separate.

### 2.5 Motion/control extension dispatcher — `0x1402E01A0`

```text
.mot -> 0
.mcv -> 1
.cam -> 2
.hid -> 3
.clt -> 4
.tsc -> 5
```

These mappings establish runtime identity/subsystem registration, not complete field schemas.

## 3. Newly closed direct content/tag evidence

### `VAGp`

At `0x140032970` the executable directly compares the first DWORD against `0x70474156`:

```text
56 41 47 70 = VAGp
```

Identity is `EXE_CONFIRMED_MAGIC`. Exact DMC3 bank/container ownership remains open.

### `DDS `

Direct first-DWORD checks exist at:

```text
0x140049A8E
0x14004AD9D
44 44 53 20 = DDS<space>
```

DDS identity is therefore upgraded from corpus-only recognition to **EXE-confirmed content identity**. DXT/ATI/BC/etc. FourCCs remain DDS subformats, not independent DMC resource families.

### `TM2\0`

At `0x1403365BA`:

```text
cmp DWORD PTR [rcx], 0x00324D54
54 4D 32 00 = TM2\0
```

This corrects the older `TIM2`-only assumption. `TIM2` can remain an alias/historical tooling label, but the direct canonical EXE content authority on this path is exactly `TM2\0`.

### `LIG2`

The constructor around `0x14023ECB0` writes:

```text
0x14023ECC9: mov DWORD PTR [rcx+0x08], 0x3247494C
4C 49 47 32 = LIG2
```

This upgrades LIG2 from “one printable occurrence” to an **EXE-owned FourCC/type tag**. It does not prove that object `+0x08` maps directly to the first four bytes of every retail LIG2 file, nor does it close record field semantics.

## 4. Containers, namespaces and metadata

| Family | Purpose | Evidence | Product boundary |
|---|---|---|---|
| `NBZ` | numbered top-level HD volume | `EXE_CONFIRMED` path + product validation | ZIP-backed source/materialization; overlay authoring is not Capcom packer equivalence |
| `PAC` / `PAC0` | relative-slot container | `EXE_CONFIRMED` + corpus | sparse/empty/alias physical topology is authority |
| `PNST` | distinct recursive relative-slot family | `EXE_CONFIRMED` + corpus | PAC-like envelope does not imply same semantic slot schema |
| `PACK` | legacy/observed label | `DATA_CONFIRMED` | original runtime binary writer/parser remains open |
| `.index` | extraction/display naming metadata | `DATA_CONFIRMED` | not runtime lookup or semantic-format authority |
| `.lst` | loose/list-driven PAC fallback manifest | `EXE_CONFIRMED` | full grammar/lifetime/error semantics open |
| `GData.afs/` / `GDataX360.afs/` | logical resource namespaces | `EXE_CONFIRMED` | not proof of a binary AFS backend on current HD path |
| `.ukn` | placeholder extension | `DATA_CONFIRMED` | never route semantically by extension alone |
| `.bin` | generic leaf extension | EXE path corpus | bytes/context decide identity |

Runtime strings include `%sDMC3-%d.nbz` at `0x14036E930` and `GData.afs/` at `0x140363188`.

## 5. Geometry, model, render and collision families

| Family | Purpose | Identity | Schema/product boundary |
|---|---|---|---|
| `MOD` | actor/object mesh-bearing model | `EXE_CONFIRMED` | validated model-family behavior; full variants/writer open |
| `EFM` | mesh-bearing effect model | `EXE_CONFIRMED` | shares model factory branch with MOD; real retail EFM needed for exact stream binding |
| `SCM` | stage/scene model | `EXE_CONFIRMED` | dedicated factory branch; complete material/ownership schema open |
| `MRP` | render/model-side companion | `EXE_CONFIRMED` identity | no generic handler/factory or standalone mesh proof |
| `MCV` | motion/model companion identity | `EXE_CONFIRMED` by family mask + extension dispatcher | exact semantics and downstream owner open |
| `SHW` | shadow geometry/topology companion | `EXE_CONFIRMED` | index triplets + external spatial pool; not proven self-contained textured mesh |
| `HITS` | spatial collision grid/triangle data | `DATA_CONFIRMED` | structural parser exists; **not established as canonical EXE runtime tag by this census** |

### HITS correction

The bounded canonical EXE type-identification census reports **zero ASCII `HITS` occurrences**. Therefore:

- four-byte `HITS` remains a real `DATA_CONFIRMED` collision payload identity from corpus/parser evidence;
- `HITS` must not be labeled `EXE_CONFIRMED` merely because the clean parser recognizes it;
- historical `HITS$` is `REJECTED` and must not be reintroduced.

## 6. Texture/effect families

| Family | Evidence | Boundary |
|---|---|---|
| `DDS` | direct EXE magic checks + corpus | standard texture identity; editing/export integration separate |
| `PTX` | EXE extension classifier + corpus | bundle semantics partial |
| `TM2` / alias `TIM2` | direct `TM2\0` EXE check + runtime filenames | exact schema/HD conversion chain open |
| `PTZ` | `basic.ptz` runtime reference | exact consumer/schema open |
| `SEF` | corpus/effect context | direct typed consumer open |
| `EFE` | EXE dispatcher sentinel | exact purpose/schema open |
| `EFW` | EXE dispatcher sentinel | exact purpose/schema open |

## 7. Cloth, animation, camera and visibility/control

| Family | Evidence | Boundary |
|---|---|---|
| `MOT` / `MOT2..6` | base MOT EXE dispatcher; variants corpus-backed | variant ABI/bindings open |
| `MCV` | `MCV ` family mask + `.mcv` class 1 | fields/relationship to MOT/model consumers open |
| `CAM` | EXE manager registration + corpus | full track schema/writer open |
| `HID` / `HID2/3` | base HID EXE registration; variants corpus-backed | track schema open |
| `CLT` | EXE primary + motion/control registration | cloth/deformation schema open |
| `C1D` | EXE primary extension classifier | 1-D cloth schema open |
| `TSC` | EXE motion/control registration | exact semantics open |

## 8. Stage/gameplay families

| Family | Purpose | Evidence/product boundary |
|---|---|---|
| `LIG` | stage lighting | `DATA_CONFIRMED`; structural record scanner |
| `LIG2` | lighting record/type family | EXE-owned object tag + corpus 0x20 header / 0x30 records; fields open |
| `EVE` | event/trigger volume | `DATA_CONFIRMED`; field semantics partial |
| `POS` | stage placement/transform companion | `HIGH_CONFIDENCE`; exact schema open |
| `ITM` | item placement | EXE/corpus-backed; item ID + XYZ + Y rotation known |
| `STE` | scene/effect transform | `HIGH_CONFIDENCE`; exact consumer/schema open |
| `EST` | stage dependency/control candidate | `CANDIDATE`; do not universalize slot identity |
| stage `TXT` | StageSet/room/door/effect config | EXE parser/token evidence; structural product support |
| `EventTblNN.bin` | mission/event control bytecode | EXE load/scan + high-confidence semantics; full opcodes open |
| `DCA` | cinematic/mini-demo camera records | corpus structural parser; exact fields partial |

## 9. Audio and bank families

| Family | Evidence | Boundary |
|---|---|---|
| `ADX` | EXE logical audio references | HD physical representation may translate to OGG |
| `OGG` | EXE physical filename corpus | loop metadata ownership remains EXE-side |
| `VAGp` | direct EXE first-DWORD check | exact DMC3 ownership/container relation open |
| `PHD` | `snd_sys.phd` | exact fields/linkage open |
| `TSB` | `snd_sys.tsb` | exact fields/linkage open |
| `BD` | `snd_sys.bd` | exact fields/linkage open |
| `SPUMAPDT` / `SpuMap.bin` | explicit EXE string + parser evidence | region-to-bank/sample linkage open |

## 10. Video/media capability boundary

Runtime/movie identities include `SFD` and `WMV`. The executable also contains a media capability extension classifier over approximately `0x14002A5D1..0x14002A779` for:

```text
PSS THP PAM XMV WMV PMF AVI MPG BIK MP4
```

These are **capability-only** unless independent DMC resource ownership evidence exists. DMC Rengine must not register generic media support as if every such format were a native DMC3 resource family.

## 11. Other runtime/path-reference families

Canonical EXE runtime references additionally include:

- `.pac` resource-name corpus;
- `.adx`, `.ogg`, `.sfd`, `.tm2`, `.txt` filename corpora;
- `basic.ptz`, `LOADERICON.dds`;
- font `.fon` names;
- `icon00.ico`, `icon01.ico`, `icon02.ico`, `icon.sys`;
- `options.sav`, `dmc3.sav`;
- `snd_sys.phd`, `snd_sys.tsb`, `snd_sys.bd`;
- `SpuMap.bin`, `EventTblNN.bin`;
- explicit `SPUMAPDT` at VA `0x140508988`.

A path/reference is identity/reference evidence, not automatic proof of complete binary grammar.

## 12. Explicit exclusions

The following must **not** become standalone DMC resource families without new evidence:

```text
DDS encoding/layout tags:
DX10 DXT1 DXT2 DXT3 DXT4 DXT5 ATI1 ATI2 BC4U BC4S BC5U BC5S RGBG GRGB YUY2

shader compiler/container metadata:
RDEF SPDB D3DSHDR SHEX

source/debug/dependency artifacts:
HLSL FXH PDB DLL
```

Printable instruction fragments and PE section names are not format evidence unless tied to a bounded classifier, handler, direct content check, constructor/object tag or owned runtime path.

## 13. Product integration policy

DMC Rengine must keep `identity evidence` and `decoder support` independent:

- confirmed identity may be exposed by registries/classifiers without inventing a parser;
- structurally understood formats may be inspected read-only;
- unresolved families remain fail-closed;
- only validated canonical mesh layouts enter mesh decode;
- `MODX`/`SCMX` style three-byte runtime matches do not automatically become renderable resources;
- media capability recognition does not create system/file ownership.

The current open frontier includes MRP/MCV exact consumers and fields, real EFM stream binding, SHW external-pool ownership, CLT/C1D schemas, EFE/EFW semantics, VAGp container ownership, TM2 HD conversion/schema, and the exact relation between the LIG2 object tag and retail on-disk representation.
