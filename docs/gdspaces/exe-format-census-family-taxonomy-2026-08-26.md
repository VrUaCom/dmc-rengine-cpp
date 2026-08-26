# GDSpaces / DMC3 EXE — Complete Format Census and Family Taxonomy — 2026-08-26

**Tracking:** #225 / PR #226  
**Canonical analysis executable:** `dmc3.exe`, 6,356,432 bytes, SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Scope:** all resource/data formats currently evidenced by the canonical EXE plus already-canonical bounded corpus evidence.  
**Non-claim:** this is not a claim that every middleware extension/string present in the executable occurs in the retail DMC3 corpus.

## Purpose

Build one format authority for GDSpaces from executable evidence instead of maintaining disconnected extension lists. A format identity is not defined by extension alone. The classifier must preserve content signature, parent-container lineage, loader/registry context, logical catalog identity and runtime translation identity.

## Evidence classes

- **RAW-INSTRUCTION** — direct executable code recognizes, transforms or dispatches the format.
- **RAW-CATALOG** — exact entry from an executable-owned resource/path table.
- **RAW-STATIC-TABLE** — executable static registry/descriptor evidence, with code xref where recorded.
- **PROJECT-CANONICAL-CORPUS** — bounded file/corpus evidence already accepted by DMC Rengine but not promoted here as a whole-image generic EXE dispatcher.
- **MIDDLEWARE-CAPABILITY** — executable media/backend code recognizes the representation; retail use is not implied.
- **EXCLUDED-NONRESOURCE** — strings/extensions that must not become GDSpaces resource formats.

---

# 1. Exact 4,039-entry EXE master resource catalog

The executable pointer table at `0x140553050` contains 4,039 logical resource names. Every entry has an extension. Exact extension counts from the canonical image are:

| logical extension | count | bounded meaning |
|---|---:|---|
| PAC | 3398 | dominant logical packed/container-backed game resource identity |
| TXT | 424 | localized/message text identities in the master catalog |
| ADX | 154 | legacy logical audio identities |
| BIN | 24 | event/audio/system metadata identities; not one schema |
| SFD | 19 | legacy logical video identities |
| MOD | 4 | model/geometry resources |
| ICO | 3 | legacy save/UI icons |
| TM2 | 3 | texture/image resources |
| FON | 2 | font resources |
| PTX | 2 | texture package/runtime resources |
| PHD | 1 | sound-bank component |
| TSB | 1 | sound-bank component |
| BD | 1 | sound-bank component |
| SYS | 1 | legacy save/system member (`icon.sys`) |
| PTZ | 1 | texture-related package/resource; exact schema still open |
| AFS | 1 | `endof.afs` logical/legacy catalog identity; not proof of active physical AFS backend |

Total: **4,039**.

### BIN caution

The 24 master-catalog BIN identities include `SpuMap.bin`, `EventTbl00.bin` through `EventTbl21.bin`, and `lei20_4c.bin`. In addition, the EXE uses separate `DATA\\DATA` metadata BIN files such as `TextureDataNameTbl.bin`, `MovieDataNameTbl.bin`, `CurveDataNameTbl.bin`, `TexAnimeDataNameTbl.bin`, `EffectDataNameTbl.bin`, `PtclParamFiles.bin`, `GeneratorDataNameTbl.bin`, `ValueDataNameTbl.bin`, `EfcMdlDataNameTbl.bin`, and `DemoDataNameTbl.bin`.

Therefore `.bin` is only an extension envelope, never a format-schema authority.

---

# 2. Direct executable format registries

## 2.1 Typed post-load / content-magic surface

The central typed dispatcher `0x1401B9FA0` recognizes and/or dispatches the following content families:

- `MOD` -> `0x1402FE3B0`;
- `EFM` -> `0x1402F7A90`;
- `SCM` -> `0x1403051B0`;
- `SHW` -> `0x1403204C0`;
- `PNST` -> recursive member traversal;
- explicit terminal-prefix checks exist for `EFW` and `EFE`.

The state-2 finalizer separately recognizes a PAC root before traversing members into the same typed path.

### Registry A — `0x1402DB1F0` and surrounding manager

Direct classification gives:

- `MOD` = type 0;
- `EFM` = type 1;
- `SCM` = type 2;
- `MRP` = type 3;
- extension PTX = type 4;
- extension CLT = type 5;
- extension C1D = type 6;
- `SHW` = type 7.

`MRP` is therefore a current raw format identity even though its full semantic role remains open.

## 2.2 Registry B — whole-file clip/timeline/control family

A separate manager around `0x1402E01A0` classifies whole-file `DATA\\DATA` resources by extension:

- MOT;
- MCV;
- CAM;
- HID;
- CLT;
- TSC.

This is a distinct loader context from Registry A.

**Important:** CLT exists in both registries. Extension alone is insufficient to determine family/ABI.

---

# 3. Container and archive representations

## NBZ

DMC3 bootstrap `0x14002E930` probes `%sDMC3-%d.nbz`. Current L1 authority identifies the retail representation as classic ZIP-backed NBZ with STORE/raw-DEFLATE members.

## ZIP

ZIP is the transport/container representation inside NBZ, not the logical GDSpaces game-resource identity.

## PAC

Direct `PAC\0` content recognition and member traversal. PAC is a relative-slot container with sparse/empty-slot behavior under the canonical product/runtime evidence.

## PNST

Direct `PNST` recognition and recursive typed member traversal. Product/corpus evidence preserves sparse slot identity and alias topology.

## LST

Loose-list fallback is runtime-constructed: original resource path can be rewritten to `.lst`; ordinary entries can be rewritten to `.pac`; nested-list and `dummy` semantics are already bounded. LST is a manifest/container-control representation, not PAC itself.

## AFS

`GDataX360.afs/`, `GData.afs/` and `afs/sound/` exist as EXE resolver namespaces, and one `.afs` sentinel occurs in the master catalog. This is **legacy/logical namespace authority only** at this scope. Do not infer an active physical binary AFS backend without separate evidence.

---

# 4. Texture / image formats

## PTX

Registry A extension identity; CPtxManager/gfxTexture runtime evidence exists. Exact PTX schema remains partially open.

## PTZ

One master-catalog identity (`basic.ptz`). Treat as texture-related/open until a direct parser/ABI is recovered.

## CLT

Appears in Registry A and Registry B. Must carry loader-context identity.

## C1D

Registry A auxiliary texture/render resource identity. Exact schema remains open.

## TM2

Direct magic validator `0x1403365B0` recognizes `TM2`; three master-catalog resources are present.

## DDS

Direct DDS magic validation exists in the texture runtime, including DX10-header support. Observed encoding/FourCC variants include:

`DXT1`, `DXT2`, `DXT3`, `DXT4`, `DXT5`, `ATI1`, `ATI2`, `BC4U`, `BC4S`, `BC5U`, `BC5S`, `RGBG`, `GRGB`, `YUY2`, `DX10`.

These are DDS subformats/encodings, not separate top-level GDSpaces families.

## ICO / FON

Catalog/save/UI resource representations. ICO is also part of the legacy save package. FON is font data.

---

# 5. Model / geometry / render family

Current EXE identities:

- MOD — direct magic + typed post-load;
- EFM — direct magic + typed post-load;
- SCM — direct magic + typed post-load; geometry/collision-adjacent representation;
- MRP — Registry-A direct magic, semantic ownership still open;
- SHW — direct magic + typed post-load;
- EFW — direct central typed prefix check;
- EFE — direct central typed prefix check.

Do not force these into one binary ABI merely because one registry groups them. The family groups resource purpose/loader context, not record-layout equivalence.

---

# 6. Clip / timeline / control family

Registry-B current raw extensions:

- MOT;
- MCV;
- CAM;
- HID;
- CLT;
- TSC.

These are loaded through a distinct whole-file path. The precise schema of each remains a per-format reverse target.

---

# 7. Stage / gameplay / structured-data family

## TXT

Two distinct contexts must remain separate:

1. master-catalog localized/message TXT identities;
2. nested stage/config/effect TXT grammar (`_004.txt`, `cfg_000.txt`, `_effect_000.txt`) with recovered tokens such as `DOOR`, `BoxIn`, `NextRoom`, `ORBREAK`, `SEAL`, `SWITCH`, `LIFT`, `SE`, `DUMMY`.

## BIN / EventTbl

Twenty-two `EventTbl00..21.bin` identities are directly registered. Exact record schema remains open. Other BIN tables are separate schemas.

## LIG2

Project-canonical corpus evidence identifies the LIG2 structure, and the canonical EXE contains a constructor path writing `LIG2`; classify as current raw-supported structured stage data.

## ITM V1

Project-canonical placement format with 0x10 header used by Item Editor. No generic whole-image ITM magic dispatcher is promoted from this census; identify by parent/slot/context and validated schema.

## HITS$

Project-canonical magic `HITS$`, marker `0x18060001`, 56-byte records. Preserve as a separate gameplay/spatial-data format; do not claim it is part of the generic typed dispatcher unless a direct raw xref is separately recovered.

## DCA

Project-canonical `DCA\0`, 0x10 header, known 0x410 record size in bounded samples. The current census does not promote a generic whole-image DCA dispatcher.

---

# 8. Audio family

## ADX -> OGG translation

The master catalog contains exactly 154 legacy ADX logical names. The EXE contains a separate 154-record OGG descriptor table at `0x14055C610`, stride 0x10, containing filename pointer + loopStartMs + loopEndMs.

The ADX basename set and OGG basename set correspond; runtime constructs/searches the `.ogg` representation. Preserve both identities:

`logical ADX identity -> physical/runtime OGG representation`.

## VAGp

Direct `VAGp` magic check exists in legacy sound parsing. Treat as a supported legacy audio representation, not proof of current retail external files.

## PHD / TSB / BD

Single master-catalog sound-bank components (`snd_sys.*`). Exact individual schemas remain open.

## SpuMap.bin

Master-catalog audio/system metadata BIN. Do not merge its schema with EventTbl/effect metadata BIN files.

## Sound PAC families

Stage sound, enemy sound and weapon/style sound are PAC-backed resource sets; PAC is the container representation while the family/consumer identity belongs to audio/RCP metadata.

---

# 9. Video family

## SFD -> WMV translation

The master catalog has exactly 19 SFD logical identities. PC/HD media code rewrites the legacy extension to `.wmv` before loading. Preserve both logical and physical/runtime identities.

## Middleware capability extensions

The media classifier recognizes:

`PSS`, `THP`, `PAM`, `XMV`, `WMV`, `PMF`, `AVI`, `MPG`, `BIK`, `MP4`.

These are **MIDDLEWARE-CAPABILITY** entries. Recognition does not prove each format exists in the DMC3 retail corpus.

---

# 10. Save / system / UI family

- `dmc3.sav` — PC gameplay/progression persistence; detailed envelope/checksum ABI is already separately canonical.
- `options.sav` — PC configuration persistence.
- `icon.sys` — legacy package/system member.
- `icon00.ico`, `icon01.ico`, `icon02.ico` — legacy save/UI icon members.
- `euro28.fon`, `china20m.fon` and related font/atlas resources — localization/UI.

Save persistence is not part of the L1 NBZ/PAC resource container family merely because both are file formats.

---

# 11. Embedded shader family

The canonical EXE contains embedded `DXBC` shader bytecode blobs with chunks such as `SHDR`, `ISGN`, `OSGN`.

Classification: **F_SHADER_EMBEDDED**. This is executable-embedded shader data, not an ordinary GDSpaces file request.

HLSL/FXH path/source strings are not promoted as external game-resource formats without a proven file-open/load path.

---

# 12. Family taxonomy

## F_ARCHIVE

- NBZ
- ZIP transport
- AFS legacy/logical namespace only at current scope

## F_CONTAINER

- PAC
- PNST
- LST manifest/fallback

## F_MODEL_GEOMETRY_RENDER

- MOD
- EFM
- SCM
- MRP
- SHW
- EFW
- EFE

## F_TEXTURE_IMAGE

- PTX
- PTZ (open schema)
- CLT when Registry-A context applies
- C1D
- TM2
- DDS + DDS encoding variants
- ICO

## F_CLIP_TIMELINE_CONTROL

- MOT
- MCV
- CAM
- HID
- CLT when Registry-B context applies
- TSC

## F_STAGE_GAMEPLAY_DATA

- TXT (message and stage grammars as separate variants)
- BIN (schema-qualified, never generic)
- EventTbl*.bin
- LIG2
- ITM
- HITS$
- DCA

## F_AUDIO

- ADX logical -> OGG runtime
- VAGp
- PHD
- TSB
- BD
- SpuMap.bin
- PAC-backed stage/enemy/weapon/style sound resource sets

## F_VIDEO

- SFD logical -> WMV runtime
- middleware capability: PSS/THP/PAM/XMV/WMV/PMF/AVI/MPG/BIK/MP4

## F_SAVE_SYSTEM_UI

- dmc3.sav
- options.sav
- icon.sys
- ICO save members
- FON/font resources

## F_SHADER_EMBEDDED

- DXBC

---

# 13. Classification precedence for GDSpaces

A correct DMC3 classifier should use this order:

```text
1. exact content magic/signature
2. parent container + member/slot lineage
3. original loader registry/context
4. logical EXE catalog identity
5. runtime translation identity
6. namespace/path context
7. extension fallback
8. UNKNOWN / fail closed
```

Extension-only classification is forbidden when stronger evidence exists.

### Required provenance fields

At minimum preserve:

- `logical_name`;
- `logical_extension`;
- `physical_source`;
- `physical_representation`;
- `parent_container_chain`;
- `content_magic`;
- `loader_context`;
- `format_family`;
- `format_variant`;
- `evidence_status`.

Examples:

```text
Battle_00.adx -> logical ADX / runtime OGG
Title.sfd -> logical SFD / runtime WMV
PAC child with SCM magic -> F_MODEL_GEOMETRY_RENDER / SCM regardless of missing child filename
.clt -> family decided by Registry-A vs Registry-B loader context, not extension alone
```

---

# 14. Explicit exclusions

Do not promote the following as original DMC3 resource formats without separate evidence:

- `.index` extraction/tool metadata;
- DLL/EXE/PDB/certificate/CRL/HTTP strings;
- arbitrary compiler/source/debug paths;
- HLSL/FXH strings without a proven external loader;
- every middleware-recognized video extension as retail-corpus presence;
- physical binary AFS solely from `GData.afs/` namespace strings or `endof.afs` catalog sentinel.

---

# 15. Acceptance state

This census materially supersedes the older small `05_DMC3_EXE_Data_Formats` list as the architecture inventory, while preserving its bounded format-specific findings.

What is now closed at census level:

- exact 4,039 master-catalog extension distribution;
- direct Registry-A identities;
- direct Registry-B identities;
- core PAC/PNST/NBZ/ZIP identities;
- ADX->OGG and SFD->WMV translation identity separation;
- loader-context requirement for ambiguous extensions such as CLT;
- family taxonomy and classifier precedence.

Still format-specific/open:

- complete binary schemas for MRP, EFW, EFE, C1D, MOT, MCV, CAM, HID, CLT, TSC, PTZ, PHD/TSB/BD and many BIN variants;
- full semantic-success/failure ABI for typed families;
- complete real-retail corpus occurrence census for nested formats not represented by the 4,039 logical catalog;
- writer/repack equivalence per format.

The format census is an inventory/identity authority. It does not by itself claim parser completeness, writer completeness or original-game equivalence for every listed format.
