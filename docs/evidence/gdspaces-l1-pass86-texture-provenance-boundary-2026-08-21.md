# GDSpaces L1 Pass 86 — Texture provenance boundary — 2026-08-21

## Scope

Layer 1 only. This pass corrects the provenance classification of the DDS-bearing texture evidence used by Passes 78–85. It does not invalidate the generic NBZ/PAC/PNST materialization and reflow work; it prevents transformed/extraction-side texture evidence from being promoted to original-retail PTX/TIM2 authority without a source-chain receipt.

## Evidence sources

### Preserved v6 source artifact

- `DMC 3 RENGINE (6).zip`
- size: `237,658,858` bytes
- SHA-256: `7680a9ddb700b958ca1591be0629c2ff1da53efa1b723141bbee0ae4b4c7ff6f`
- corpus root used by texture passes: `analysis_inputs/stage_drops`

The corpus README describes these as **external stage/file packs for analysis**. It does not claim that every child or texture representation is a byte-exact direct extraction from retail `dmc3-0.nbz`.

The historical v6 `GDTextureBundleReader` does not implement PTX/TIM2-to-DDS conversion. It scans an already supplied physical slot for literal `DDS ` signatures and exposes bounded virtual DDS children. Therefore the v6 GDS reader is a consumer of the DDS-bearing representation, not evidence of the producer/transform that created it.

### Negative raw-TIM2 corpus control

A raw-signature control scan was run over the preserved v6 source package specifically to distinguish binary texture payloads from source/documentation strings.

- `analysis_inputs/stage_drops`: `2,239` files, approximately `86.5 MiB` scanned;
- true binary `TM2\0` / TIM2 payload samples found in stage drops: **0**;
- whole `DMC 3 RENGINE (6).zip` raw payload scan: true binary `TM2\0` / TIM2 samples found: **0**;
- textual `TIM2` occurrences elsewhere in the archive are source/documentation references and are not accepted as format payload evidence.

This is a negative corpus receipt only. It does **not** prove that retail DMC3 lacks PTX/TIM2. It proves that the preserved v6 analysis corpus used by Passes 78–85 does not supply a raw TIM2 sample from which original-retail TIM2 serialization can be validated.

### Phase 15 container-runtime package

- package: `dmc3_exe_container_runtime_phase15_complete.zip`
- package SHA-256: `cd50266dd3cc8cff61a6a61e5762b67f7bc270c42423cfc30987bfecc9374ad5`
- Phase15 raw PAC path: `/mnt/data/st001.pac`
- raw `st001.pac` SHA-256: `90b3a89abeb6d4475d81d066b7b19460f7a799741742416c3a35c2eeb992e7d1`
- comparison extraction: `/mnt/data/st001.zip`

Phase 15 proves strong byte relations for the raw PAC:

- top-level slots `0,2,3,4,5,6,7` are byte-identical to their extracted counterparts;
- nested sparse PNST slot identity is preserved in the raw PAC while `.index` is lossy;
- texture slot `1` contains 17 embedded `DDS ` signatures;
- `st001.zip` contains 17 extracted DDS files for that slot;
- **0/17 extracted DDS files are exact byte substrings of raw texture slot 1**.

The Phase15 interpretation is therefore retained verbatim in substance: the extracted DDS files are transformed/reordered or reconstructed and are not proven binary-identical slices.

Critically, the Phase15 build script only consumes pre-existing `/mnt/data/st001.pac` and `/mnt/data/st001.zip`; it does not record how either artifact was acquired. Current preserved evidence does not prove whether raw `st001.pac` came directly from retail `dmc3-0.nbz`, from an earlier extractor, or from another intermediate/repacked source.

Phase15 also explicitly leaves **`PTX/DDS bundle exact binary layout and transformations` unresolved**.

### Cross-dataset `st001` reconciliation

The preserved v6 tree contains `analysis_inputs/stage_drops/st001 - copia/st001 - copia/`. It is closely related to the Phase15 extraction, but it is **not the same immutable dump**.

For the Phase15 top-level comparison set:

- `st001_000.ukn`: exact same SHA-256 `7efcf182...3faa5`;
- `st001_002.scm`: exact same SHA-256 `3ed787cc...26ba5`;
- `st001_003.ukn`: exact same SHA-256 `80b4b643...ab77a`;
- `st001_005.pac`: exact same SHA-256 `500dadbc...cc4b0`;
- `st001_006.ukn`: exact same SHA-256 `0f3d9952...99f64`;
- `st001_007.pac`: exact same SHA-256 `f0c0a225...a5487`;
- **`st001_004.txt` differs**: Phase15 extraction is `256` bytes, SHA-256 `36d40827c90e7ce0750a56c0f267be6721dd2b2e4bf30f4100a0cd6ecdabc07e`; the v6 stage drop is `154` bytes, SHA-256 `cfcf480c0076919fd6022dc554ddccd26f89aeafaa5052d557912111e38894b5`.

The v6 `st001 - copia` tree also contains a non-game `dmc_mesh_patch_v1` JSON artifact. This is additional evidence that the directory functioned as a working/analysis drop rather than a sealed pristine-retail image.

Therefore the v6 stage-drop tree and Phase15 `st001.zip` must be treated as **related but versioned/transformed analysis datasets**. Hash agreement for selected children does not establish retail provenance for the whole tree or for the DDS-bearing texture representation.

## Runtime representation boundary

Canonical EXE reverse evidence keeps the following representations separate:

`original resource bytes -> PTX/TIM2 parsing path -> runtime texture bundle / gfxTexture`

and a separate DDS-from-memory path exists. The recovered original PTX path expects 0x800-aligned TIM2 entries and validates `TM2\0`.

No current receipt proves that the DDS-bearing `0x70 descriptor + DDS` / `0x800 bundle` profile from the preserved stage-drop corpus is the original PTX/TIM2 resource image consumed by that runtime path.

Therefore direct EXE-consumer reverse of stage-drop descriptor fields such as `+0x3C/+0x40` is **not yet an evidence-backed assumption**. Those fields may belong to a transformed/export/editor-facing representation whose producer or consumer is outside the original PTX parser.

## Corrected authority classification

### Still strongly evidenced

The following work remains valid within its demonstrated domain:

- 154 descriptor/DDS structural relationships in the preserved v6 corpus;
- 58 DDS-bearing bundle framings;
- same-layout DDS payload reintegration;
- safe-domain size-changing DDS-bearing slot rebuild;
- compiled-current C++ texture-profile execution receipts;
- compiled real PNST parent reflow using those complete physical child images;
- generic PAC/PNST writer correctness for supplied physical child images;
- generic NBZ materialization/repack work.

### Correct name for the texture domain

Until provenance closes, these are:

> **validated transformed DDS-bearing texture-slot profile authority**

They are **not yet**:

> original-retail PTX/TIM2 serializer authority

or:

> original-game texture writeback equivalence.

## Impact on Passes 78–85

Passes 78–85 are not rejected. Their tests, hashes, corpus coverage and compiled execution remain useful and reproducible for the preserved DDS-bearing profile.

However, any wording that can be read as `real retail DMC3 texture representation` must be interpreted with this correction: `real/preserved corpus bytes of the transformed DDS-bearing profile`, unless a direct retail-source provenance receipt is attached.

No Layer-1 completion percentage should gain credit for original-retail texture writeback from these passes alone.

## Mandatory provenance gates

Before promoting the texture path to original-retail authority, at least one of the following must close with hashes and reproducible source-chain evidence:

1. reacquire `st001.pac` directly from the retail `dmc3-0.nbz` / `GData.afs` resource path and prove its SHA against the Phase15 raw PAC; or
2. prove the exact extractor/transformer that produced Phase15 `st001.pac` and/or the stage-drop DDS-bearing representation, including transformation rules; or
3. obtain a separate raw retail PTX/TIM2 sample and recover an original-resource serializer independently.

If direct retail `st001.pac` matches Phase15 SHA `90b3...e7d1`, the next task is to trace which runtime path consumes texture slot 1 and reconcile the apparent DDS-bearing representation with the recovered PTX/TIM2 path.

If it does not match, the DDS-bearing writer remains an extraction/editor-profile writer and original writeback must pivot to the actual retail PTX/TIM2 representation.

## Immediate reverse target

1. recover source-chain provenance for `/mnt/data/st001.pac` and `/mnt/data/st001.zip` from preserved artifacts/history;
2. identify the producer of `analysis_inputs/stage_drops` texture representation;
3. reacquire a direct retail texture-bearing PAC from `dmc3-0.nbz` when the 960 MB archive becomes available;
4. compare raw retail slot bytes against the Phase15 and v6 DDS-bearing profiles;
5. recover the EXE-confirmed original PTX envelope structure separately from the transformed DDS-bearing profile;
6. only then resume semantic reverse of `+0x3C/+0x40` against the correct producer/consumer layer.

## Layer-1 status

Layer 1 remains **NOT COMPLETE**. This provenance correction narrows the claimed texture authority; it does not roll back the already validated generic container/repack architecture.
