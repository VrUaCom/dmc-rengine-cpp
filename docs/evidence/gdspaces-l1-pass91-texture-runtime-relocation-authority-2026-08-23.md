# GDSpaces L1 Pass 91 — texture runtime relocation authority — 2026-08-23

Scope: **Layer 1 — Resource Materialization only**  
Status: **writer/output relocation compatibility established against canonical EXE contract; original-game modified-resource acceptance still open**

## Purpose

Pass 91 reconciles the existing compiled DMC3 texture-authoring stack with the canonical read-side materialization contract recovered in Pass 90.

This pass does **not** invent a new texture serializer and does **not** copy the Pass-90 PTX parser branch into the writer ancestry. Instead it asks one narrower question:

> Do the bytes already emitted by the existing texture writers form the exact serialized relative-pointer image consumed by the canonical non-TM2 `gfxTexture` materializer?

The answer for the evidenced descriptor/DDS authoring subset is **yes**.

## Canonical runtime authority used

Canonical target:

- file: `dmc3.exe`
- size: `6,356,432` bytes
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

Pass 90 recovered the relevant original-code contract:

- `0x1403365B0` — per-entry dispatcher; non-`TM2\0` entries enter the serialized `gfxTexture` branch;
- `0x140046510` — installs the `gfxTexture` vtable and resolves serialized qword fields by `address(field) + raw_delta`;
- first relocation field: `entry+0x20`;
- second relocation field: resolved descriptor `+0x08`;
- `0x140046AF0` — consumes descriptor `+0x04` as DDS byte size and descriptor `+0x08` as DDS data pointer;
- `0x140049A10` — DDS-from-memory validation/creation path.

The canonical source image therefore has the following decisive relocation relations:

```text
qword entry+0x00 = 0
qword entry+0x20 = 0x40
  address(entry+0x20) + 0x40 = entry+0x60

u32   entry+0x64 = DDS byte size
qword entry+0x68 = 0x08
  address(entry+0x68) + 0x08 = entry+0x70

DDS starts at entry+0x70
```

## Existing writer reconciliation

The existing texture stack already emitted these values before their runtime ownership was understood.

### `TextureSlotFramingParser`

The Pass-80 structural parser already fail-closes on the complete qword image:

- zero dwords at `+0x00` and `+0x04` -> `qword +0x00 == 0`;
- `u32 +0x20 == 0x40` plus zero dword at `+0x24` -> `qword +0x20 == 0x40`;
- `u32 +0x64 == exact bounded DDS size`;
- `u32 +0x68 == 8` plus zero dword at `+0x6C` -> `qword +0x68 == 8`;
- descriptor size is exactly `0x70`, therefore the attached DDS begins at `entry+0x70`.

Previously these fields had corpus structural authority only. Pass 90 supplies their original runtime relocation meaning.

### Existing size-changing writers

Both existing descriptor builders zero-initialize the complete `0x70` descriptor and then author:

```text
+0x20 = 0x40
+0x64 = authored DDS total size
+0x68 = 8
```

Therefore their high dwords remain zero and the complete qwords exactly encode the canonical relative-pointer image.

No absolute runtime address is authored.

## New code — read-only runtime materialization inspector

Pass 91 adds `TextureSlotRuntimeMaterializationInspector` to the writer ancestry.

It is deliberately read-only. For a bounded serialized entry it:

1. requires the source vtable qword at `+0x00` to be zero;
2. reads the qword delta at `+0x20`;
3. resolves `descriptor = address(field) + delta` exactly as the canonical materializer does;
4. bounds-checks the resolved descriptor;
5. reads descriptor `+0x04` as DDS byte size;
6. reads descriptor `+0x08` as the second qword delta;
7. resolves the DDS pointer by the same `address(field) + delta` rule;
8. bounds-checks the complete DDS byte range;
9. validates `DDS ` magic, DDS header size `0x7C`, and DDS pixel-format size `0x20`.

It does not mutate the serialized image, install a vtable, allocate D3D objects or claim to emulate the complete original runtime.

## Size-changing inverse-serialization regression

The new regression creates a two-record texture bundle and first verifies both source entries through the runtime relocation inspector.

Then it performs an actual existing-writer size-changing rebuild:

```text
texture 0: DXT5 256x256 -> DXT5 512x512
texture 1: untouched DXT1 128x128
```

The first record grows, so the second record moves to a different absolute file offset.

The output is reparsed through `TextureSlotFramingParser`, then **both** output records are independently passed through `TextureSlotRuntimeMaterializationInspector`.

Required result:

```text
record 0 moved/rebuilt -> relocation still resolves descriptor and DDS
record 1 moved unchanged -> relocation still resolves descriptor and DDS
```

This is the essential inverse-serialization property: relocation fields are position-independent relative deltas, so packed reflow does not require absolute pointer patching.

Negative controls reject:

- non-zero source vtable placeholder;
- descriptor delta escaping the bounded record.

## Existing compiled evidence retained

This runtime-authority upgrade sits on top of the already compiled writer evidence:

- Pass 84: exact current compiled texture writer executed successfully on **112/112** real-corpus authoring cases;
- Pass 85: those physical edited texture children were lifted through **45/45** real PNST bottom-up rebuild cases with exact target-child and untouched-sibling validation.

Pass 91 does not repeat those corpus runs. It explains and verifies the newly recovered original runtime relocation contract against the same writer representation.

## What is now proven

For the existing safe texture-authoring subset:

1. the writer emits the canonical serialized relative-pointer image expected by the non-TM2 `gfxTexture` materializer;
2. the image remains valid after a size-changing record reflow that changes absolute file offsets;
3. both changed and displaced-unchanged records resolve their descriptor and DDS pointers using the original `address(field)+delta` rule;
4. DDS range/header gates are compatible with the recovered canonical DDS-from-memory path;
5. no absolute runtime pointer needs to be serialized.

## What is still not proven

This pass does **not** establish:

- direct-retail provenance for the preserved DDS-bearing texture corpus;
- that every retail DMC3 texture resource uses this representation;
- authoring authority for the unresolved non-zero auxiliary descriptor subset;
- authority for opaque/multi-DDS variants outside the bounded parser domain;
- actual execution of modified rebuilt texture bytes by original DMC3;
- final PAC/PNST/NBZ game-consumption acceptance;
- Layer 1 completion.

## Hard freeze

- no absolute runtime pointer serialization;
- no generalized `gfxTexture` object writer beyond the evidenced descriptor image;
- no non-zero source vtable authoring;
- no promotion of unresolved auxiliary texture fields;
- no original-game acceptance claim until an actual rebuilt resource is consumed by canonical DMC3;
- no `Layer 1 COMPLETE` claim from structural/runtime-contract compatibility alone.

## Next Layer-1 gates

1. direct-retail texture-bearing resource reacquisition from canonical `dmc3-0.nbz` with an artifact/source-chain receipt;
2. compare retail child SHA/layout against the preserved transformed DDS-bearing corpus;
3. if representation matches, run a minimal controlled texture edit through the established writer -> parent PAC/PNST -> NBZ path;
4. obtain an original-game consumption receipt for that rebuilt artifact;
5. separately resolve or formally exclude remaining auxiliary/opaque texture variants.
