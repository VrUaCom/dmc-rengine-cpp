# DMC3 HD SCM runtime provenance map — 2026-09-13

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Machine-readable authority:** `data/reverse/dmc3-scm-runtime-provenance-20260913.json`

## Purpose

The structural SCM reverse is already terminal for the canonical project scope. This pass goes deeper in a different dimension: it records **where each important serialized domain flows at runtime**.

The required chain is:

```text
L0 serialized SCM bytes
  -> L1 SCM materializer / normalizer
  -> L2 runtime manager/object/mesh/node state
  -> L3 geometry/material/render consumer
  -> L4 draw/shader or external companion ownership
```

A field is not renamed merely because it is copied somewhere. A semantic name is promoted only when the downstream technical effect is independently evidenced. Otherwise the terminal result remains `PRESERVED_UNDECODED`.

## 1. Header provenance

### `+0x10` object count

```text
SCM +0x10
  -> 0x1402F9570
  -> manager +0xE8
```

Status: `EXE_CONFIRMED`.

### `+0x11` scene-node count

```text
SCM +0x11
  -> 0x1402F9570
  -> manager +0xEA
  -> scene-node domain / 0x1402F1DB0
```

Status: `EXE_CONFIRMED`.

### `+0x12` texture-slot count is not the live authority

```text
SCM +0x12 = serialized mirror

manager +0x110 = external texture companion
companion +0x00
  -> 0x1402F9570
  -> manager +0xEC = live runtime texture count
```

Status: `EXE_AND_CORPUS_CONFIRMED` mirror/consistency value. Writer coherence must cover both resources.

### `+0x13`

```text
SCM +0x13
  -> 0x1402F9570
  -> manager +0xFA
```

It is runtime-carried, but no stronger independent semantic consumer is established. Retail corpus value is zero.

Status: `PRESERVED_UNDECODED`.

### `+0x14` legacy resource/provenance code

Corpus arithmetic:

```text
raw = family_class * 100000 + model_set * 100 + sub_index
```

Runtime:

```text
SCM +0x14
  -> 0x1402F9570
  -> manager +0xE4
```

The arithmetic decomposition and runtime carry are real. Official class labels are not. Neutral structural naming is therefore required.

## 2. Object `+0x01` reaches final vertex alpha

This is one of the deepest closed SCM chains:

```text
serialized object +0x01
  -> 0x14030303A inside SCM initializer 0x140302F10
  -> runtime object +0x07
  -> bounded C4 / EA compatibility rewrites
  -> runtime +0x178 / +0x17C
  -> 0x140304111..0x140304167
  -> MDL_PARTS_COLOR_PKT.alpha.w
  -> DMC3_STG COLOR0.a
  -> texture/color shader variants
  -> final alpha / alpha-test path where selected shader uses it
```

Projection:

```text
effective <= 0x80 -> alpha.w = effective / 255
effective >  0x80 -> control code retained, alpha.w = 1.0
low source mode != 0 -> effective control forced to 0x80
```

Therefore this byte is `alpha_control`, not plain opacity.

Status: `EXE_CONFIRMED`.

## 3. Object source flags — exact technical projections

Serialized flags live at object `+0x10`. SCM initializer `0x140302F10` preserves the source word into runtime baseline/effective state and projects selected bits.

| Source condition | Runtime effect | Site |
|---|---|---:|
| low nibble != 0 | runtime flag bit 8; helper mode; alpha-control override | `0x1403032E1` + helper path |
| `0x00000020` | runtime flag bit 10 | `0x1403032FB` |
| `0x00020000` | runtime bit 9 + `(1,1,1,0)` vector | `0x140303315` |
| `0x00010000` | runtime bit 7 + secondary-boolean inversion | `0x14030337C` |
| `0x00040000` | runtime bit 4 | `0x140303398` |
| `0x00080000` | runtime bit 5 | `0x1403033E3` |
| high nibble `0x0F000000` | runtime bit 15 + `highNibble-1` at `+0x0D` | `0x1403033B3` |
| `0x00100000` | GS TEST/ZBUF state selection | `0x140302640` |
| `0x00004000` | TEX1 nearest `0` vs linear `0x60` | `0x1402F9890` |

The helper path for low mode is numeric evidence, not permission to invent material-mode names:

```text
lowMode = flags & 0xF

lowMode == 0:
  helperMode = 9
  selector   = 0x5080B

lowMode != 0:
  helperMode = lowMode
  lowMode == 4          -> 0x50007
  else flags&0x100000   -> 0x5010D
  else                  -> 0x5000D
```

## 4. Bit `0x00200000`: deeper negative provenance

Current corpus:

```text
19 / 254 SCM objects
```

The bit is copied into baseline/effective runtime source flags. It survives the recovered low-mode mutation path. But the stronger whole-image census does **not** establish its semantic effect.

### Rejected candidate A — `0x1402F4C21`

```text
runtimeRecord +0x304
  AND 0x00200000
```

The field has an independent producer:

```text
constructor/runtime input +0x128
  -> & 0xF0000000
  -> runtimeRecord +0x304
```

This is not serialized SCM object `+0x10` provenance.

### Rejected candidate B — `0x140303F2F`

```text
manager +0xE0
  AND 0x00200000
```

`manager+0xE0` is built separately by the manager/family classifier path through `0x1402F9570` and `0x1402FD650`.

Again, this is not source object `+0x10`.

### Bit-test census

The local SCM/runtime area contains `BTS` operations at `0x140302CF9` and `0x140302D59`, but they **set manager bit 21** from a separate control word. They are not consumers of serialized object bit 21.

Terminal result:

```text
PRESERVED_UNDECODED_WITH_BOUNDED_NEGATIVE_EVIDENCE
```

Required policy:

```text
preserve exactly
no semantic editor toggle
do not call globally unused
do not call globally reserved
```

A stronger promotion requires dynamic differential evidence, a real semantic producer, an indirect transformed consumer, or controlled original-game mutation.

## 5. Mesh texture/material chain

### Texture index `+0x02`

```text
mesh +0x02 u16
  -> 0x1402F9890
  -> runtimeTextureTable + index * 0x40
  -> textureRecord +0x20
  -> runtime mesh texture/descriptor
```

The table comes from the external companion, not SCM payload bytes.

### GS CLAMP `+0x04..+0x0B`

```text
MINU +0x04
MAXU +0x06
MINV +0x08
MAXV +0x0A
```

`0x1402F9890` packs:

```text
(MINU << 4)
| 0x0F
| (MAXU << 14)
| (MINV << 24)
| (MAXV << 34)
```

The low nibble means:

```text
WMS = 3 = REGION_REPEAT
WMT = 3 = REGION_REPEAT
```

`MINU == 0` selects the observed disabled/sentinel packed value `0`.

All 481 retail-corpus meshes use zero values, but canonical executable support is live. This is **not padding**.

### `+0x0C`

Primary materialization `0x1402F9BB0` does not assign a promoted semantic. Material helper `0x1402F9890` reads the preceding CLAMP words and skips this lane. Direct `st001/st114` census is zero.

Terminal result:

```text
RESERVED_OBSERVED_ZERO
+
PRESERVED_UNDECODED
```

## 6. Vertex and topology path

```text
mesh +0x10 -> positions float3[]
mesh +0x18 -> normals float3[]
mesh +0x20 -> UV signed i16 / 4096
```

Primary runtime mesh materialization: `0x1402F9BB0`.

Physical continuation:

```text
mesh +0x28 = 0x50 non-final
mesh +0x28 = 0 final
```

Post-load normalizer `0x1403051B0` uses the topology stream at `+0x38` and reconstructs runtime u16 indices into the mesh-relative workspace.

Confirmed topology rule:

```text
vertex record = RGB + topology flags
flags & 0x02 -> break / skip triangle run
```

Workspace contract:

```text
mesh +0x40 -> workspace relative offset
capacityBytes = align16(6 * (vertexCount - 2))
retail first u16 = 0x1212
retail serialized mesh+0x48 = 0
0x1403051B0 -> generated u16 index sequence
               -> mesh+0x48 generated word count
```

Other topology bits remain preservation-gated.

`mesh+0x30` and `mesh+0x4C` remain exact-preservation zero domains with no promoted semantic in the recovered SCM paths.

## 7. Scene-node provenance

Scene block starts with four relative offsets:

```text
+0x00 parentByOrderPosition
+0x04 nodeAtOrderPosition
+0x08 objectBindingByNodeIndex
+0x0C transformByNodeIndex
```

Runtime path:

```text
SCM setup 0x140303C10
  -> common node binder 0x1402F1DB0
  -> hierarchy/object-binding/transform runtime domain
```

`+0x10..+0x1F` remains zero in the bounded corpus. The audited planner reads `+0x10` once into a dead local; the raw shell pointer does not escape the recovered model path.

Terminal result:

```text
RESERVED_OBSERVED_ZERO
+
PRESERVED_UNDECODED
+
dormant/no effect in audited canonical path
```

This is stronger than simply saying “unknown,” but still does not justify rewriting the bytes as padding.

## 8. Transform provenance

Serialized transform stride: `0x20`.

```text
+0x00 float3 translation
+0x0C float  translation magnitude
+0x10 float3 XYZ radians
+0x1C float  preservation-only fourth lane
```

Runtime construction:

```text
0x140303C10 SCM setup
  -> 0x1402FA360 SCM local transform initializer
     -> 0x140330450 XYZ rotation helper
        -> Rz * Ry * Rx
     -> 0x140031200 translation helper
  -> hierarchy parent/root pointer setup
  -> 0x1402F9700 world update
     -> world = local * parentOrRootWorld
```

`+0x0C` is Euclidean translation length, not homogeneous W. `+0x1C` is outside the confirmed SCM matrix construction and remains preservation-only.

The historical SCM attribution to `0x1402FA080` is rejected; that function belongs to MOD/EFM.

## 9. External texture companion provenance

Resource pair:

```text
0x140304AE0
  manager +0x108 = SCM
  manager +0x110 = texture companion
```

Companion envelope recovered around `0x140304B30`:

```text
+0x000 u32 textureCount
+0x004 u32 blockCount[textureCount]
+0x800 payload[0]
payload size = blockCount[i] * 0x800
```

Direct payload path `0x1403365B0` checks `TM2\0`; fallback paths exist, so TM2 must not be asserted for every possible companion slot.

This establishes a strict authoring boundary: SCM owns slot references; the companion owns texture payloads.

## 10. SCM draw bridge

The alpha/color ABI reaches the actual SCM draw subsystem:

```text
MDL_PARTS_COLOR_PKT (0x30 bytes)
  -> CDrawSCM path
  -> 0x1402FD040, strict 0x30 packet stride
  -> SCM draw submission 0x1402FC850
  -> embedded DMC3_STG shader ABI
```

This is useful provenance for the reader: an object alpha byte is not merely “runtime-used”; its effect can be shown all the way to shader-visible `COLOR0.a`.

It does **not** mean every adjacent source flag has a shader semantic.

## 11. What the deep reader should expose

For any selected SCM byte or field, Binary Inspector / Native Reader should eventually be able to show:

```text
serialized owner
field/bit status
materializer / initializer
runtime destination
known downstream consumers
external companion dependency, if any
negative provenance / rejected candidates
preservation policy
```

This is more useful than a flat list of field names because it distinguishes:

- direct semantics;
- derived runtime state;
- cross-resource ownership;
- preservation-only bytes;
- rejected false-positive consumers.

## 12. Fresh evidence boundary

No fresh canonical executable bytes are available in the currently connected artifacts.

Available saved material is insufficient for a new disassembly pass:

- `original-exe-bytes.json` has the correct canonical SHA but only an unrelated four-byte item-registry window;
- the saved `dmc3_drive_exe_audit.json` points to a different executable SHA and reports `profileMatch=false`.

Therefore this pass does **not** pretend that historical reverse notes are newly disassembled evidence.

The next real executable step is to materialize the validated SCM deep-reader window packet from the canonical target and compare the acquired bytes against this provenance graph. The packet is intentionally expanded to include the world-transform, draw-submission and texture-payload boundaries so the next pass can trace SCM data beyond materialization into final consumption.
