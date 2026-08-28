# GDSpaces / DMC3-HD residual format census — 2026-08-26

**Pass class:** residual static/corpus format census and contradiction cleanup.  
**Canonical analysis target:** `dmc3.exe`  
**Size:** `6,356,432` bytes  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Primary output:** `docs/formats/dmc3-hd-format-catalog.md`.

This pass is intentionally narrower than a general L3 completion pass. Its job is to find resource families that can be missed by extension/string scans, reconcile them against current product code and corpus evidence, and prevent obsolete names from being promoted as format truth.

## 1. Census method

The residual search uses multiple independent surfaces because no single surface is complete:

```text
path/extension strings
+ direct 3/4/8-byte immediate comparisons in executable code
+ typed post-load dispatcher branches
+ StageCfg physical-slot access
+ hash/corpus/index labels
+ current C++ parser/registry behavior
+ preserved research documents
```

Rules:

- an ASCII substring alone is not format evidence;
- a source extension is not semantic authority;
- a middleware capability string does not prove shipped retail presence;
- a corpus label can establish a family target without proving original-runtime consumer semantics;
- unknown purpose is preserved as `RESEARCH_REQUIRED`, never filled from acronym intuition.

## 2. LIG2 promotion

Residual immediate/signature inspection finds `LIG2` on a direct executable identification path rather than only as a corpus label.

Combined evidence:

- canonical executable identification path: `EXE_CONFIRMED` identity;
- clean C++ `lig2` scanner: `0x20` header, `0x30` record stride;
- project corpus expectation: 48 records;
- stage context: lighting family.

### Promotion

`LIG2` is a real DMC3-HD binary resource identity and a stage-lighting resource family.

### Remaining boundary

Current clean scanner is structural and does not yet expose proven semantics for the individual 0x30-byte fields. The direct identifier evidence and product probe policy should be reconciled in a future code slice.

## 3. ITM hidden behind `.ukn`

Four real stock fixtures previously surfaced externally as `.ukn`:

```text
st001cfg_006.ukn  size 48   records 1
st010cfg_006.ukn  size 48   records 1
st105cfg_006.ukn  size 64   records 2
st114cfg_006.ukn  size 400  records 19
```

The semantic identity is ITM, not unknown.

Evidence-backed ITM envelope:

```text
+0x00 char[4] "ITM\0"
+0x04 u16     version/family
+0x06 u16     record count
+0x08..0x0F   preserved raw/reserved

record start  = 0x10
record stride = 0x14

record:
+0x00 u32 itemId
+0x04 f32 X
+0x08 f32 Y
+0x0C f32 Z
+0x10 f32 rotationY
```

Verified stock-size relation uses `align16(0x10 + count*0x14)` with zero canonical padding on the reviewed fixtures.

### Promotion

`.ukn` is explicitly rejected as format authority. ITM is the static item-placement family; behavior, persistence and registry requirements live in other systems.

## 4. SCH contradiction cleanup

A legacy project statement described `st001.pac` as containing:

```text
PTX, SCM, SCH
```

Later corpus reconciliation resolves the real `st001` surface into texture bundle, SCM, HITS resources, text, PNST and nested PAC, while misleading extraction labels can remain.

No independent canonical header + original consumer currently re-establishes SCH.

### Decision

`SCH` is **REJECTED as current canonical format identity**.

It may remain an acquisition/search hint in historical material, but must not appear in canonical format lists as confirmed until independently reacquired.

## 5. STE / EST executable-string false positives

Raw ASCII hits for the short tokens `STE` and `EST` were reviewed in context.

The reviewed hits include unrelated larger strings/metadata rather than a clean file-magic comparison. Therefore those hits do not promote either family to EXE-magic-confirmed.

### Current status

- `STE`: real StageCfg/corpus resource family; purpose is scene/effect transform data at `HIGH_CONFIDENCE`; direct magic not proven by the rejected ASCII hits.
- `EST`: real StageCfg/corpus label with a strong runtime slot-9 dependency/control correlation; exact slot mapping remains `CANDIDATE`.

## 6. StageCfg physical slot 9

A direct StageCfg path around `0x1401AF000` has a concrete physical-slot dependency:

```text
if cfg PAC slotCount >= 10:
    read offset table entry at +0x2C
    -> physical slot 9 payload
    -> call semantic scanner 0x1401A9BC0
    -> dependency/control results feed enemy-resource demand
```

Corpus comparison:

```text
st001cfg.pac: 10 slots; semantic children include EST
st114cfg.pac:  9 slots; EST absent
```

### Promotion boundary

The relation is strong enough for:

> `EST` = stage dependency/control payload candidate with direct slot-9 runtime correlation.

It is **not** strong enough for:

> `physical slot 9 is universally EST`.

That stronger statement requires an independent exact slot map or raw payload/header identity.

## 7. Direct identifier/magic residuals

The residual machine-code/signature surface confirms dedicated recognition/handling for:

```text
LIG2
TM2/TIM2 family
DDS
VAGp
```

The DDS-adjacent values below are encoding/layout tags, not separate resource families:

```text
DXT1 DXT2 DXT3 DXT4 DXT5
ATI1 ATI2
BC4 BC5
RGBG GRGB YUY2
```

## 8. Typed-postload format surface

The already canonical L3 dispatcher evidence is retained as a format census source:

```text
MOD  -> dedicated typed fixup
EFM  -> dedicated typed fixup
SCM  -> dedicated typed fixup
SHW  -> dedicated typed fixup
PNST -> recursive child dispatch
PAC  -> child/root traversal where applicable
```

### Product reconciliation finding

Current `FormatIntegrationRegistry` represents `MOD`, `SCM` and `PNST`, but still lacks explicit clean integration descriptors for `EFM` and `SHW` despite direct original-runtime branches.

This is a product migration gap, not evidence that EFM/SHW are unknown to the game.

## 9. HITS contradiction cleanup

The repository had a stale format document describing:

```text
HITS$
+ universal 0x18060001 record marker
+ marker followed by thirteen floats
```

Current C++ parser/integration evidence contradicts that model.

Canonical current structure uses:

```text
magic: HITS
header: 0x44
bounds + cell size + grid dimensions
spatial-table relative offset
triangle-array relative offset
triangle records: 0x38
```

Each triangle contains raw flags, three points, normal and plane-D. `0x18060001` is an observed raw flag value, not a universal delimiter.

### Decision

The old `HITS$` document is superseded and must be replaced by the current collision-grid model.

## 10. DCA semantic/product split

Current clean C++ DCA code safely exposes only:

```text
header = 0x10
record = 0x410
```

Preserved reverse research is stronger about the runtime purpose: the two-channel mini-demo camera interpretation maps one channel to camera eye/world position and the second to look-at target, with view construction from target-eye.

### Catalog rule

DCA purpose may be recorded as mini-demo/cinematic camera data while the product parser remains labelled **structural-only**. Purpose maturity must not be confused with field-schema maturity.

## 11. EVE boundary

EVE remains a spatial-zone format, not the mission scripting engine.

Evidence-backed geometry:

```text
+0x00 "EVE\0"
+0x04 u16 version/family (0x0100 observed)
+0x06 u16 count
records @ 0x10, stride 0x80
+0x00 u32 id candidate
+0x04 u32 type candidate
+0x08 u32 tag/script candidate
+0x20 four vec4 polygon corners
+0x60 float height
+0x64..0x7F raw tail
```

The geometry purpose is data-confirmed. `id/type/tag` semantics remain candidate. Mission/event control remains a separate EventTbl concern.

## 12. Save-family split

The persistence census must distinguish:

- `dmc3.sav` — game progress/persistent gameplay state; canonical envelope size `0x4A30` in current reverse tooling;
- `options.sav` — configuration/options persistence; exact schema remains open.

They are not variants of one proven binary schema.

## 13. HD media representation split

Existing canonical Wave-2 evidence remains:

```text
logical ADX -> physical OGG by basename translation
logical SFD -> physical WMV by extension translation
```

The residual media string/capability surface is broader:

```text
PSS THP PAM XMV WMV PMF AVI MPG BIK MP4
```

Only supported playback capability may be inferred from this wider set. Retail/shipped presence is not promoted for every family. `SFD -> WMV` remains the DMC3-HD-specific EXE-confirmed video translation.

## 14. Remaining named format frontier

After extension, immediate, dispatcher and StageCfg passes, the remaining high-value named families are concentrated in subsystem-private binary chunks rather than obvious new extensions:

```text
PTZ
PHD
BD
TSB
SPUMAPDT
MRP
C1D
EFE
EFW
MCV
HID/HID2/HID3
TSC
CLT
POS
STE
EST
```

For the first thirteen families above, exact purpose/schema must remain `RESEARCH_REQUIRED` or bounded candidate until a direct payload + consumer relation is recovered.

`POS/STE/EST` already have StageCfg associations, but their exact original consumer ABIs remain priority reverse targets.

## 15. Purpose census outcome

The residual pass supports a complete **current-observation** catalog with three classes:

1. **purpose confirmed/bounded:** NBZ, PAC, PNST, SCM, MOD, HITS, DDS, PTX, TM2, CAM, LIG/LIG2, EVE, ITM, TXT, DCA, ADX/OGG, SFD/WMV, dmc3.sav;
2. **purpose high-confidence/candidate:** POS, STE, EST, SHW subsystem meaning, MOT/MCV/HID variants, audio-bank companion families;
3. **identity/label only — research required:** PTZ/EFE/EFW/MRP/C1D/TSC/CLT and unresolved exact audio-bank splits.

This classification is preferable to either dropping unresolved families or assigning invented semantic expansions.

## 16. Non-claims

This pass does not claim:

- every possible DMC3-HD format has now been discovered;
- every observed label is a standalone physical file type;
- every media capability is shipped in the retail corpus;
- EST is definitively physical slot 9;
- STE/EST have EXE-confirmed ASCII magic;
- every DCA field is semantically decoded;
- EFM/SHW clean-product support is implemented;
- format breadth changes L1/L2/L3 completion status.

## 17. Follow-up priority

Highest-value next reverse sequence:

```text
EST / StageCfg slot9 -> 0x1401A9BC0 exact grammar
 -> POS original consumer
 -> STE original consumer
 -> EFM/SHW typed-layout details
 -> audio-bank PHD/BD/TSB/SPUMAPDT split
 -> effect/render private chunks PTZ/EFE/EFW/MRP/C1D/TSC/CLT
 -> MOT/MCV/HID variant binding
 -> final contradiction sweep
```

Every promoted result must update `docs/formats/dmc3-hd-format-catalog.md` in the same change.
