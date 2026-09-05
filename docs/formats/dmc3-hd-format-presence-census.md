# DMC3 HD format presence census

**Snapshot:** 2026-09-05  
**Base:** `main@fb5623b14ff56b3eed95fbe08dba79a6687af8f1`  
**Canonical analysis EXE SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

This document answers a different question from the canonical [format-purpose catalog](dmc3-hd-format-catalog.md):

> **Have we actually seen bytes for this family in a bound retail/corpus surface, or do we only know that the original executable recognizes/references it?**

Do not infer physical presence from an EXE string, dispatcher branch, extension table, capability table, or subsystem name. Conversely, do not infer absence from one NBZ volume or one supplied corpus.

Machine-readable companion: [dmc3-hd-format-presence-census.json](dmc3-hd-format-presence-census.json).

## 1. Presence vocabulary

| Status | Meaning |
|---|---|
| `RETAIL_VOLUME_CONFIRMED` | the container/volume itself is a bound retail artifact |
| `RETAIL_MEMBER_CONFIRMED` | a file with this extension/name is present directly in a bound retail archive central-directory surface |
| `CORPUS_PAYLOAD_CONFIRMED` | real payload bytes were supplied/extracted and structurally or semantically bound to this family; this does not necessarily establish the original retail filename/extension |
| `WORKING_NAME_OVER_OTHER_EXTENSION` | project semantic name for bytes whose observed physical/extracted filename uses another extension |
| `EXE_ONLY_NOT_BOUND` | original EXE recognizes/references the family, but current canonical documentation has no bound real payload receipt for it |
| `CAPABILITY_ONLY` | executable/media layer supports the format class; shipped DMC3 asset presence is not claimed |
| `SYNTHETIC_ONLY` | DMC Rengine test format only |
| `REJECTED` | historical/superseded identity; not current game truth |

`EXE_ONLY_NOT_BOUND` means **not bound in the current evidence set**, not “the game does not contain this format”.

## 2. Directly measured retail members — `dmc3-0.nbz`

The bound `dmc3-0.nbz` central-directory surface contains 4,333 files. The measured extension counts are:

| Family / extension | Files | Presence | Notes |
|---|---:|---|---|
| `PAC` | 3,725 | `RETAIL_MEMBER_CONFIRMED` | dominant physical member family; most deeper game resources live inside PAC/PNST-style slot containers |
| `TXT` | 565 | `RETAIL_MEMBER_CONFIRMED` | these top-level files are localization message tables, not the stage-config TXT corpus |
| generic `.bin` | 24 | `RETAIL_MEMBER_CONFIRMED` | extension is not semantic authority; several unrelated payload families may be carried under `.bin` |
| `TM2` | 6 | `RETAIL_MEMBER_CONFIRMED` | direct retail members |
| `MOD` | 4 | `RETAIL_MEMBER_CONFIRMED` | direct retail members; additional model payloads may also exist nested in containers |
| `FON` | 4 | `RETAIL_MEMBER_CONFIRMED` | real font assets exist even though the font binary schema remains unreversed |
| `PTX` | 2 | `RETAIL_MEMBER_CONFIRMED` | direct retail texture-bundle members |
| `BD` / `PHD` / `TSB` | 3 total | `RETAIL_MEMBER_CONFIRMED` | sound-system bank/body/table companions |

The NBZ volume itself is `RETAIL_VOLUME_CONFIRMED`; NBZ is not counted as a member extension in the table above.

Source authority: [DMC3 `dmc3-0.nbz` archive normalized-key census — 2026-09-03](../reverse/dmc3-nbz-archive-key-census-2026-09-03.md).

### Important boundary

The central directory only sees top-level NBZ members. It does **not** enumerate the semantic formats inside the 3,725 PAC files. Therefore a family missing from the table above may still be common inside PAC/PNST.

## 3. Real nested or supplied payloads already present in project evidence

The current project has real payload/corpus evidence for the following semantic families even when they are not direct `dmc3-0.nbz` member extensions:

| Family | Presence | Evidence boundary |
|---|---|---|
| `PNST` | `CORPUS_PAYLOAD_CONFIRMED` | real relative-slot containers; distinct from PAC despite envelope similarity |
| `SCM` | `CORPUS_PAYLOAD_CONFIRMED` | 68 unique SCM resources used by the structural reverse; stage/static geometry |
| `HITS` | `CORPUS_PAYLOAD_CONFIRMED` | real collision payloads; `HITS$` is rejected |
| `DDS` | `CORPUS_PAYLOAD_CONFIRMED` | nested/extracted texture payloads and texture-bundle children |
| `DCA` | `CORPUS_PAYLOAD_CONFIRMED` | StageCfg camera/cinematic records |
| `LIG` / `LIG2` | `CORPUS_PAYLOAD_CONFIRMED` | StageCfg lighting payloads; structural record boundaries recovered |
| `EVE` | `CORPUS_PAYLOAD_CONFIRMED` | event/trigger-volume corpus |
| `POS` | `CORPUS_PAYLOAD_CONFIRMED` | stage placement/position corpus |
| `ITM` | `CORPUS_PAYLOAD_CONFIRMED` | real item-placement payloads, including files historically extracted as `.ukn` |
| `STE` | `CORPUS_PAYLOAD_CONFIRMED` | stage/effect transform corpus |
| `EST` | `CORPUS_PAYLOAD_CONFIRMED` | real payload label/bytes exist, but exact universal semantic identity remains candidate-level |
| `CAM` | `CORPUS_PAYLOAD_CONFIRMED` | camera family is present in stage corpus in addition to EXE registration evidence |
| `MOT` / numbered MOT variants | `CORPUS_PAYLOAD_CONFIRMED` | corpus variants exist; exact variant ABI remains open |
| `SHW` | `CORPUS_PAYLOAD_CONFIRMED` | hash-bound real payload established self-contained shadow-hull geometry |
| stage `TXT` | `CORPUS_PAYLOAD_CONFIRMED` | StageSet/room/door/effect text corpus; distinct from top-level localization TXT files |
| `EventTblNN.bin` | `CORPUS_PAYLOAD_CONFIRMED` | mission/event bytecode lives under a generic `.bin` filename family; full VM/opcode semantics remain open |

This table records **payload presence**, not completion of the reverse. Many of these formats still have partially unknown fields or no writer.

## 4. Working semantic names that must not be mistaken for physical extensions

### `SO`

`SO` is currently a project working name for a recovered typed object/structure graph. The bound reverse record explicitly states that observed leaves are `.bin` and that no original `.so` extension authority is claimed.

Presence classification:

```text
SO semantic payload: CORPUS_PAYLOAD_CONFIRMED
physical .so extension: NOT ESTABLISHED
observed carrier naming: .bin / contextual slot identity
```

This distinction is mandatory for Native Reader, GDSpaces naming, export, and any future writer.

### `.ukn`

`.ukn` is not a game format. It is an extraction/naming fallback. Real semantic payloads such as `ITM` and `HITS` can appear under `.ukn`, so routing by `.ukn` alone is forbidden.

### `.bin`

`.bin` is likewise a generic filename extension, not a semantic format. `EventTblNN.bin`, SO-working-name payloads, and other unrelated records must be identified from bytes/context rather than the suffix.

## 5. EXE-known families without a currently bound real payload receipt

The following families are real original-runtime identities or references, but current canonical documentation must **not** say that their payload files were already found merely because the EXE knows them:

| Family | Presence | What is actually proven |
|---|---|---|
| `EFM` | `EXE_ONLY_NOT_BOUND` | primary typed effect-model family and handler/fixup are EXE-confirmed; real payload receipt is not currently canonical |
| `MRP` | `EXE_ONLY_NOT_BOUND` | registry/family-mask recognition is confirmed; exact schema remains open |
| `MCV` | `EXE_ONLY_NOT_BOUND` | motion/control manager + family-mask identity confirmed |
| `CLT` | `EXE_ONLY_NOT_BOUND` | cloth/deformation manager identity confirmed |
| `C1D` | `EXE_ONLY_NOT_BOUND` | ClothSim1D identity/subsystem confirmed |
| `HID` | `EXE_ONLY_NOT_BOUND` | hide/visibility-control manager identity confirmed; corpus variant labels do not by themselves bind a current raw payload receipt |
| `TSC` | `EXE_ONLY_NOT_BOUND` | motion/control manager identity confirmed; semantic role/schema open |
| `PTZ` | `EXE_ONLY_NOT_BOUND` | canonical asset-table/reference relationship to texture resources confirmed |
| `EFE` | `EXE_ONLY_NOT_BOUND` | container-dispatch sentinel/prefix recognized; no normal handler/schema recovered |
| `EFW` | `EXE_ONLY_NOT_BOUND` | container-dispatch sentinel/prefix recognized; no normal handler/schema recovered |
| `VAGp` | `EXE_ONLY_NOT_BOUND` | direct marker check confirmed in EXE; current DMC3 payload ownership not bound |
| `SPUMAPDT` / `SpuMap.bin` | `EXE_ONLY_NOT_BOUND` | parser/path identity confirmed; no current raw payload receipt is promoted here |
| `options.sav` | `EXE_ONLY_NOT_BOUND` | persistence path/subsystem known; exact binary schema remains open |
| `ICO` / `icon.sys` | `EXE_ONLY_NOT_BOUND` | executable asset catalog references exist; HD runtime use/payload interpretation remains open |

`SEF` is deliberately not promoted to this table as EXE-only: current catalog evidence is corpus/data-led and its direct original consumer still needs reacquisition.

## 6. Capability-only media identifiers

`PSS`, `THP`, `PAM`, `XMV`, `PMF`, `AVI`, `MPG`, `BIK`, and `MP4` are media-layer capabilities exposed by the executable. Their presence in capability logic is **not proof that DMC3 ships assets in those formats**.

`SFD` and `ADX` are logical/legacy runtime identities; HD physical representation may instead be `WMV` and `OGG`. Presence and representation must remain separate claims.

## 7. Reverse-priority consequence

When choosing the next format to reverse, distinguish two queues:

### Queue A — real bytes already available

Prefer these when the goal is fast structural closure or Native Reader coverage:

`FON`, `PHD`, `TSB`, `BD`, `EventTbl`, `EVE`, `POS`, `STE`, `EST`, `MOT`, plus remaining unknown `.bin`/`.ukn` payloads.

### Queue B — EXE-known but payload acquisition still required

Do not start by inventing structures from dispatcher code alone. First locate/acquire a real payload for:

`MRP`, `EFM`, `MCV`, `CLT`, `C1D`, `HID`, `TSC`, `PTZ`, `EFE`, `EFW`, and other EXE-only families.

For Queue B, a successful acquisition should record:

1. physical source (`NBZ -> member -> PAC/PNST -> slot` where applicable);
2. exact `ResourceId` / physical slot;
3. original observed filename/extension separately from semantic identity;
4. SHA-256 and byte size;
5. why the semantic family is established (magic, structural grammar, EXE consumer, or contextual evidence).

## 8. Non-claims

- This census does not prove that an `EXE_ONLY_NOT_BOUND` family is absent from DMC3.
- The measured retail central-directory surface covers `dmc3-0.nbz` only.
- A family present under `.bin`/`.ukn` must not be renamed physically merely because its semantic identity is known.
- A real payload does not imply a complete parser, writer, or original-game authoring equivalence.
- Media capability identifiers are not shipped-asset evidence.
- `SO` is not promoted as an original `.so` file extension.
- `PNX` is not added to the canonical DMC3 format inventory in this change because no current repository evidence record establishes that identity; it should be added only with a bound artifact/consumer contract.
