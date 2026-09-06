# DMC3 HD MOD runtime post-load — canonical EXE verification

Date: 2026-09-06

## Authority

Canonical executable:

- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- MOD post-load helper VA: `0x1402FE3B0`

This pass was re-verified directly against the canonical executable supplied for the project. Historical Wave-3 code/PRs were used only as reconciliation material; they are not the authority for this promotion.

## EXE-confirmed MOD helper contract

The helper mutates an already selected MOD allocation in place.

### Header/object layer

- `u8 @ +0x10` is the object count consumed by this helper.
- Objects begin at resource `+0x40` with stride `0x40`.
- Header qword `+0x20` is converted from resource-relative offset to `resourceBase + relative`.
- Per-object `u8 @ +0x00` is the mesh count consumed by this helper.
- Per-object qword `+0x08` is converted from resource-relative offset to `resourceBase + relative`.

### Mesh layer

MOD meshes use stride `0x50` in this helper.

The following mesh qwords are converted from resource-relative offsets to `resourceBase + relative`:

- `+0x10`
- `+0x18`
- `+0x20`
- `+0x28`
- `+0x30`

MOD does **not** perform the EFM-specific base relocation of mesh `+0x38` in this helper.

Mesh `+0x40` uses a different coordinate space: it is converted as `meshRecordAddress + relative`, not `resourceBase + relative`.

## Packed weights/topology stream

The u16 stream reached through mesh `+0x30` is not a skin-only control stream.

- bit `0x8000` participates in the topology continuation/break logic;
- the helper clears bit 15 in-place (`word &= 0x7FFF`) after consuming the marker;
- the lower 15 bits remain the packed payload already decoded by the MOD skin reader.

This reconciles the structural skin decoder with runtime topology behavior: the serialized word is a combined packed weights/topology value.

## Generated topology-command workspace

The helper reconstructs a u16 command stream from the serialized per-element words and writes it into the mesh-relative workspace referenced by `+0x40`.

The generated **word count** is written as a dword at mesh `+0x48`.

The canonical helper therefore transforms serialized MOD bytes into a runtime image containing process pointers and generated data. That post-loaded image is not a serializable MOD file.

## Canonical implementation boundary

`formats::mod::runtime_postload::apply_in_place()` reproduces the EXE-confirmed state transition while adding host-side safety checks:

- all dereferenced ranges are validated before mutation;
- pointer-addition overflow is rejected;
- the full write plan is built before pointer relocation, so malformed research fixtures fail without partial conversion.

These checks are tooling safeguards, not claims that the original game performs identical validation.

## Evidence status

- helper identity and field accesses: `EXE_CONFIRMED`
- MOD pointer-coordinate spaces: `EXE_CONFIRMED`
- bit-15 topology-break consumption and in-place clear: `EXE_CONFIRMED`
- generated u16 command algorithm and `+0x48` word-count write: `EXE_CONFIRMED`
- host-safe transactional validation behavior: project/tooling behavior, not original-game behavior

## Explicit non-claims

This pass does **not** establish:

- MOD writer authority;
- byte-equivalent serialization from a post-loaded runtime image;
- original dispatcher selection ABI/key;
- EFM post-load authority in the canonical MOD module;
- material/texture/render binding semantics beyond fields actually consumed here;
- real-corpus byte-for-byte equivalence of the reconstructed host helper against an original live post-load allocation.

Representative real MOD corpus/live-runtime comparison remains a separate validation gate.
