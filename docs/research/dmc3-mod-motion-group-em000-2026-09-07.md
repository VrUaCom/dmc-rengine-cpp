# DMC3 MOD — motion-group node-domain promotion (2026-09-07)

## Authority

Canonical executable:

`dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

Retail corpus supplied for this pass:

`em000-extract.zip` SHA-256 `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

The corpus contains 35 MOD payloads, 147 meshes, 14,804 vertices and 226 serialized node positions.

## Provenance correction

The third relative table in the MOD node-domain block (`serialized node-domain +0x08`) was previously retained as `adapter_table` with semantics intentionally open.

Canonical EXE now closes its behavioral role.

### Serialized table -> manager

`0x1402F1DB0` materializes the four node-domain pointers. For the third table:

- `0x1402F1E12` reads relative dword `nodeDomain +0x08`
- `0x1402F1E19` adds the node-domain base
- `0x1402F1E23` stores the resulting pointer at model manager `+0x18`

### Manager -> CMotion binding

`0x14030F850` binds model node domains into CMotion:

- `0x14030F870..877`: manager `+0x10` (`nodeAtOrderPosition`) -> CMotion `+0x08`
- `0x14030F87E..882`: manager `+0x08` (`parentByOrderPosition`) -> CMotion `+0x10`
- `0x14030F886..88A`: manager `+0x18` (third table) -> CMotion `+0x18`
- `0x14030F99A`: read third-table byte at hierarchy order position
- `0x14030F99E..9A6`: resolve `nodeAtOrderPosition[position]`
- `0x14030F9AE`: write the byte to that CMotion joint at `joint +0xF8`

Therefore the serialized third table is **indexed by hierarchy order position**, not directly by node index.

### CMotion consumers

The `joint +0xF8` value is repeatedly used as a selector. Examples:

- `0x14030E658..662`: compare `joint +0xF8` against requested group id before resetting per-group state
- `0x14030ED98..9F`: evaluate only matching joints
- `0x14030F378..382`: process only matching joints
- `0x1403101B8..1C2`: copy/evaluate channel state only for the selected group
- `0x140310348..353`: update channel state only for the selected group
- `0x140310A90..A9B`: motion command processing filtered by the same group id
- `0x1403112B8..2C1` and `0x140311408..411`: pose/world operations filtered by the same group id

This supports the safe semantic name **motion group index**. It does not prove a high-level label for individual values (for example upper body/lower body/cloth); such names remain unpromoted.

## em000 corpus evidence

Across all 226 node order positions:

- group `0`: 132 positions
- group `1`: 91 positions
- group `2`: 3 positions

Only seven large skeleton MODs contain non-zero motion groups:

- `em000_001.mod`: `{0:9, 1:13, 2:1}`
- `em000_004.mod`: `{0:9, 1:13, 2:1}`
- `em000_005.mod`: `{0:9, 1:13, 2:1}`
- `em000_008.mod`: `{0:9, 1:13}`
- `em000_013.mod`: `{0:9, 1:13}`
- `em000_018.mod`: `{0:9, 1:13}`
- `em000_019.mod`: `{0:9, 1:13}`

The remaining 28 MODs have group `0` for every order position.

The corpus also strengthens the preservation boundary for other unresolved MOD bytes:

- transform record `+0x1C`: zero in 226/226 records
- mesh `+0x0C`: zero in 147/147 meshes
- mesh `+0x38`: zero in 147/147 meshes
- mesh generated count `+0x48`: zero in 147/147 serialized meshes
- mesh `+0x4C`: zero in 147/147 meshes
- `BLENDINDICES.x`: zero in 14,804/14,804 vertices

These observations justify `RESERVED_OBSERVED_ZERO` for this em000 corpus only; they do not authorize global zeroing by a future writer.

## Canonical API boundary

ADR-0003 requires runtime semantics to remain outside the raw parser. The parser therefore continues preserving `transform_domain::adapter_table` as raw bytes.

`analysis/mod/motion_group` projects it into:

- `by_order_position` — exact serialized table
- `by_node_index` — projection through `nodeAtOrderPosition`
- `max_group_index` — observed maximum for tooling/inspection

No writer or mutation authority is implied.

## Status

- serialized location: `EXE_CONFIRMED`
- order-position indexing: `EXE_CONFIRMED`
- CMotion joint destination `+0xF8`: `EXE_CONFIRMED`
- behavior as motion-group selector: `EXE_CONFIRMED`
- meanings of group values `0/1/2`: `PRESERVED_UNDECODED`
- writer authority: `NOT_PROMOTED`
