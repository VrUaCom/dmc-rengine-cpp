# HITS Pass 10 — Slice 12: Stage CFG PAC → Dynamic Collision Slot Bridge

Date: 2026-08-15  
Canonical target SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
Status: **EXE CONFIRMED / IMPLEMENTED / VALIDATED**

## Promotion

Dynamic collision entry/primitive-descriptor tables used by the reviewed CEm runtime paths are sourced from the current stage **`room\\stXXXcfg.pac`** container.

The previously observed raw offsets `+0xA4/+0xA8` and `+0x60/+0x64` are **PAC slot-offset table entries**, not schema-specific fields in an abstract inner resource blob.

This is a canonical correction to the initial Slice-11 resource-provenance model.

## Stage resource selector — `0x1401AE310`

The selector receives a resource-kind index and stage id. For the stage-table-backed cases it decomposes stage id by `100` and selects the corresponding group/table row.

Relevant profile evidence:
- selector VA `0x1401AE310`;
- stage resource group table root `0x1405C4A50`;
- stage group divisor `100`;
- resource kind `1` selects stage-row subrecord `+0x10`;
- filename pointer for that subrecord is row `+0x18`.

Direct canonical table reads confirm examples:
- stage `1` -> `room\\st001cfg.pac`;
- stage `105` -> `room\\st105cfg.pac`;
- stage `114` -> `room\\st114cfg.pac`;
- stage `421` -> `room\\st421cfg.pac`;
- stage `434` -> `room\\st434cfg.pac`.

`0x1401AF000` calls the selector with `resource_kind=1` and stage id from object `+0x644`, then stores the resolved resource handle at object `+0x650`.

Therefore the later CEm collision paths reading object `+0x650` are consuming the current stage CFG PAC resource.

## PAC slot addressing

Canonical PAC layout used by these paths:
- slot count at PAC `+0x04`;
- u32 slot-offset table begins at PAC `+0x08`;
- slot N offset field = `0x08 + N * 4`.

Thus:
- `+0xA0` = slot 38;
- `+0xA4` = slot 39;
- `+0xA8` = slot 40;
- `+0x5C` = slot 21;
- `+0x60` = slot 22;
- `+0x64` = slot 23.

## Modern observed Stage-CFG collision layout

Representative C260 callsite: `0x14009823F`.

The caller resolves from the Stage CFG PAC:
- PAC slot **39** -> C260 entry table (`RDX`);
- PAC slot **40** -> C260 primitive descriptor table (`R8`);
- nearby PAC slot **38** -> related runtime/source block used by surrounding collision setup.

PAC slot-count gates are exact:
- source slot 38 requires slotCount >= 39;
- entry slot 39 requires slotCount >= 40;
- descriptor slot 40 requires slotCount >= 41.

This pattern is observed across multiple modern CEm-family collision initialization paths.

## Legacy observed CEm008 Stage-CFG collision layout

Representative C260 callsite: `0x1400B6483`.

The caller resolves:
- PAC slot **22** -> C260 entry table;
- PAC slot **23** -> C260 primitive descriptor table;
- PAC slot **21** -> related runtime/source block.

Slot-count gates:
- source slot 21 requires slotCount >= 22;
- entry slot 22 requires slotCount >= 23;
- descriptor slot 23 requires slotCount >= 24.

This is an earlier observed Stage-CFG slot generation, not a different generic blob header format.

## Connection to Slice 9 ownership

Once the Stage-CFG slots are resolved, the already confirmed Slice-9 ownership contract applies:

`entry = entry_table + entry_index * 4`

`descriptor_index = u16(entry+0x02)`

`descriptor = descriptor_table + descriptor_index * 0x50`

Then:
- C8D0 receives the resolved descriptor in `R8`;
- runtime object stores it at `+0x118`;
- CC530 reads descriptor byte `+0x00` and dispatches primitive types `0..6`.

Therefore the modern data-side type-5 target is specifically **Stage CFG PAC slot 40**, while the legacy observed target is **slot 23**.

## Correction

**REJECTED / SUPERSEDED:**
`+0xA4/+0xA8` and `+0x60/+0x64` are schema-specific u32 relative-offset fields inside an abstract resource blob.

**CURRENT:**
those addresses are PAC slot-offset table entries in `room\\stXXXcfg.pac`:
- modern entry/descriptor slots 39/40;
- legacy entry/descriptor slots 22/23.

The offsets still contain u32 values relative to the PAC base, but their semantic identity comes from PAC slot numbering, not an inner-format header.

## Implementation

- `include/dmc_rengine/profiles/dmc3/hits_stage_cfg_pac_evidence.hpp`
- `tests/hits_stage_cfg_pac_evidence_tests.cpp`
- CTest `hits_stage_cfg_pac_evidence`

All public evidence is exact-canonical-SHA gated. Packed SHA `81c7...c7d6` receives no canonical stage/PAC collision-slot descriptors.

## Validation receipt

Exact code head `63acd1bd2e311b7978c0d455e3a7279cc28dfcff` passed GitHub Actions run `31854936132` on both Ubuntu and Windows, including `hits_stage_cfg_pac_evidence`.

Later documentation/evidence corrections do not alter the validated C++ code.

## Current raw-data boundary

The project Library currently exposes some extracted `stXXXcfg_006.ukn` artifacts but not the full Stage-CFG PAC or extracted slot 39/40 artifacts through the available file listing. Direct Library semantic search is currently unavailable through the retrieval connector.

Therefore type-5 raw-record inspection is not promoted from unavailable data. The next evidence-safe paths are:
1. obtain/materialize the actual Stage-CFG PAC slot 40/23 data from existing project corpus if it becomes accessible; or
2. use the already defined C8D0 observation boundary to capture the actually referenced descriptor after Stage-CFG slot resolution.
