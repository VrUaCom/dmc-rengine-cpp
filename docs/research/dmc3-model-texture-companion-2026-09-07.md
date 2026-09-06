# DMC3 HD model texture companion — serialized envelope and runtime chain

Date: 2026-09-07

## Authority

Canonical executable:

- `dmc3.exe`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- companion materialization helper: `0x140304B30`
- runtime texture-table builder: `0x14030D040`
- TIM2/TM2 signature consumer: `0x1403365B0`
- shared model material consumer: `0x1402F9890`

## Recovered serialized envelope

The source texture companion consumed through model-manager state has the following confirmed envelope:

- `u32 +0x000` — texture payload count;
- `u32[] +0x004` — per-texture allocation counts in `0x800`-byte blocks;
- `+0x800` — first texture payload;
- subsequent payloads are consecutive and advance by `block_count * 0x800`;
- each texture payload presented to the TIM2 path begins with little-endian `0x00324D54`, bytes `TM2\0`.

The outer envelope does not currently have a promoted standalone magic of its own. It must therefore not be mislabeled as a TIM2 file: it is a companion allocation containing one or more TM2-backed payload spans.

## Runtime manager chain

The canonical model-manager path distinguishes serialized/source and runtime materialized state:

1. manager `+0x110` supplies the source texture-companion allocation;
2. `0x140304B30` walks its count/block table and texture payloads;
3. manager `+0x120` participates in the intermediate source/index state used during materialization;
4. `0x14030D040` creates the runtime descriptor table stored at manager `+0x118`;
5. runtime descriptor entries have stride `0x40`;
6. shared material helper `0x1402F9890` reads serialized mesh texture slot `+0x02`, selects `texture_slot * 0x40` from the runtime table and consumes descriptor state including entry `+0x20`.

This closes the previously open ownership link between the serialized model mesh texture slot and the external TM2-backed companion path.

## Canonical C++ boundary

`model_family::ModelTextureCompanionAbi` records only the serialized envelope that is directly required to locate payload allocations:

- texture count field `0x000`;
- block-count table `0x004`;
- payload base `0x800`;
- allocation block size `0x800`;
- TM2 payload magic `0x00324D54`.

`parse_texture_companion()` performs host-safe structural validation:

- count table must fit the input and remain before payload base;
- block-count multiplication must not overflow;
- every allocated payload span must fit the input;
- every payload must be large enough for and begin with `TM2\0`.

It returns the entry index, block count, payload offset and allocated span size. It deliberately does not parse TIM2 internals.

## Validation fixture

The model-family regression constructs a two-texture companion:

- texture 0: `1 * 0x800` bytes at `0x800`;
- texture 1: `2 * 0x800` bytes at `0x1000`;
- total required allocation: `0x2000` bytes.

Regression also verifies:

- a bad second TM2 magic is rejected;
- truncation inside the second allocated span is rejected.

## Evidence status

- source manager pointer `+0x110`: `EXE_CONFIRMED`;
- runtime descriptor table pointer `+0x118`: `EXE_CONFIRMED`;
- intermediate manager state `+0x120`: `EXE_CONFIRMED` as observed runtime materialization state, detailed semantic naming still bounded;
- count `+0x000`: `EXE_CONFIRMED`;
- block-count table `+0x004`: `EXE_CONFIRMED`;
- `0x800` payload base/block unit: `EXE_CONFIRMED`;
- TM2 magic path: `EXE_CONFIRMED`;
- runtime descriptor stride `0x40`: `EXE_CONFIRMED`;
- mesh texture slot -> runtime descriptor selection: `EXE_CONFIRMED`.

## Explicit non-claims

This pass does not establish:

- a writer for the companion envelope;
- the complete TIM2/TM2 internal image schema;
- the complete semantic layout of each runtime `0x40` descriptor;
- a filename/extension rule for every companion allocation;
- that manager `+0x120` is itself a serialized file format;
- production authoring authority for replacing texture payloads.
