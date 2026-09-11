# DMC3 HITS — Canonical reverse synthesis through Pass 10

**Canonical branch:** `hits`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Policy:** evidence-first; unknown fields and names remain unknown until promoted by direct evidence.

## 1. Architecture boundary

HITS support is split deliberately into three layers.

1. `formats/hits.*` — shared, address-free binary format parser/model.
2. `hits/*` — game-agnostic geometry, spatial, contact, edit and writer helpers.
3. `profiles/dmc3/hits_*` — DMC3 HD exact-build runtime/ABI evidence, SHA-gated to the canonical executable.

Stage Ops, GDSpaces, ModViz and future editors are consumers of these modules. They must not implement private HITS parsers.

## 2. Corrected format identity

Historical `HITS$` recognition is rejected. The format magic is exactly four bytes:

```text
HITS
```

The apparent `$` in historical samples is byte `0x24`, the low byte of the little-endian `end_offset` field at `+0x04` in those files.

Historical scanning for a universal `0x18060001` record marker is also rejected. `0x18060001` is an observed raw triangle flag value, not a record delimiter.

## 3. Serialized HITS layout

Header size: `0x44`.

```text
+0x00 char[4] HITS
+0x04 u32     end_offset
+0x08 vec3f   bounds_min
+0x14 vec3f   bounds_max
+0x20 vec3f   cell_size
+0x2C u32     grid_count_x
+0x30 u32     grid_count_y
+0x34 u32     grid_count_z
+0x38 u32     triangle_count
+0x3C u32     spatial_table_relative_offset
+0x40 u32     triangle_array_relative_offset
```

Both serialized offsets are relative to base `+0x08`.

Cell count is `grid_count_x * grid_count_y * grid_count_z`. The spatial table contains per-cell relative pointers to lists of triangle byte offsets. Every list terminates with signed `-1`.

Triangle stride is `0x38`:

```text
+0x00 u32    raw_flags
+0x04 vec3f  point_a
+0x10 vec3f  point_b
+0x1C vec3f  point_c
+0x28 vec3f  normal
+0x34 f32    plane_d
```

The plane contract is `dot(normal, point) + plane_d = 0`. Plane residual is used as a structural diagnostic, not as proof of higher gameplay semantics.

## 4. Purpose and runtime role

HITS is stage/spatial collision geometry with a 3-D acceleration grid. Runtime queries map a world region into cells, collect referenced triangle records, filter candidates and evaluate collision/contact behavior.

Two stage-backed static sources are EXE-confirmed:

- stage PAC member 3 -> static source 0 -> runtime owner `+0x680`, raw pointer `+0x730`;
- stage PAC member 6 -> static source 1 -> runtime owner `+0x6C8`, raw pointer `+0x738`.

A third wrapper source exists in the DMC3 query architecture but is not proven to be stage-PAC-backed.

Static runtime helper anchors on the canonical EXE include:

- initializer `0x1402D3060`;
- teardown `0x1402D29C0`;
- source binder `0x14005EBA0`;
- source selector `0x14005EBC0`;
- candidate-cell collector `0x1402D2A10`;
- cell-list resolver `0x1402D29F0`;
- record resolver `0x1402D3050`;
- plane evaluator `0x1402CF820`;
- normal classifier `0x1402CCF20`.

These addresses belong only in the DMC3 profile evidence layer.

## 5. Query ABI recovered through Pass 10

The canonical combined query wrapper is `0x14005E7A0` and is modeled as a six-argument point-query ABI. Its recovered behavior includes:

- a static HITS pass;
- two dynamic-category passes after the static pass;
- success returned in `AL`;
- output/working-point aliasing support;
- total miss copies the working point to output;
- dynamic passes start from the same static/input baseline;
- the last successful pass has output precedence.

The Pass 10 evidence profile preserves the exact canonical body ranges and SHA-256 hashes for:

- combined wrapper `0x14005E7A0..0x14005E880`;
- static HITS pass `0x14005E880..0x14005EB95`;
- dynamic category pass `0x14005BCF0..0x14005C0D6`.

Other recovered query entry points are profiled at `0x14005E460`, `0x14005EBE0`, `0x14005EE40`, `0x14005F070`, `0x14005FD10`, `0x14005FEC0`, `0x1400601E0`, and `0x140060790`. Their specialized contracts are kept separately rather than forcing one false monolithic CollisionResult ABI.

## 6. Dynamic collision categories

Pass 10 preserves six observed dynamic-category bindings:

| Activation | Manager field | Category | Static HITS reject mask |
|---:|---:|---:|---:|
| `0x00001000` | `+0x10` | `0x02` | `0x0040` |
| `0x00002000` | `+0x28` | `0x05` | `0x0002` |
| `0x00004000` | `+0x40` | `0x08` | `0x0010` |
| `0x00008000` | `+0x58` | `0x0B` | `0x0020` |
| `0x00010000` | `+0x70` | `0x0E` | `0x0000` |
| `0x00020000` | `+0x88` | `0x11` | `0x0000` |

These numeric bindings are confirmed profile data. Human-readable gameplay names are not invented where direct identity evidence is absent.

## 7. Stage-CFG collision descriptor path

The recovered DMC3 stage-CFG collision view uses:

- entry stride `0x04`;
- primitive descriptor stride `0x50`;
- primitive type at descriptor `+0x00`.

Entry layout:

```text
+0x00 u8  flags
+0x01 u8  transform_selector
+0x02 u16 descriptor_index
```

Observed slot generations:

- modern CEM stage CFG: entry table slot 39, descriptor table slot 40;
- legacy CEM008 stage CFG: entry table slot 22, descriptor table slot 23.

The canonical implementation validates descriptor references and can build a census of referenced descriptor indices, primitive types and reference counts.

**Important boundary:** transform-selector source/bounds semantics remain unresolved. `transform_selector_bounds_available()` intentionally returns false. Pass 10 Slice 16 is preservation/research provenance, not permission to invent a transform table.

## 8. Flags

`raw_flags` is preserved losslessly. The shared helpers expose upper/lower halves and caller-supplied upper-mask rejection. Some individual bits/masks are independently observed in specific query paths, but there is no evidence-backed universal material/surface enum yet.

Therefore:

- do not rename the 32-bit field to a material ID;
- do not normalize unknown bits;
- do not derive traversal/damage/surface behavior from corpus frequency alone;
- expose raw flags plus only profile-specific proven masks.

## 9. Editing and writer boundary

Current clean C++ supports:

- topology-preserving triangle geometry edits;
- normal/plane recomputation;
- raw flag preservation;
- spatial safety classification;
- exact-copy serialization where cell membership remains valid;
- separate spatial writer/rebuild work for topology/cell-membership changes.

A successful clean-C++ writer does **not** by itself prove original-game acceptance. Topology-changing output remains behind runtime/game validation.

## 10. Decompilation depth

Depth is tracked independently from evidence confidence.

- `DL0`: raw binary evidence
- `DL1`: machine decode and references
- `DL2`: ABI/layout/type recovery
- `DL3`: local algorithm semantics
- `DL4`: object/class ownership
- `DL5`: subsystem architecture
- `DL6`: end-to-end runtime integration
- `DL7`: exact game-domain semantics
- `DL8`: in-game behavioral parity

Current HITS summary:

- serialized format/layout: at least `DL2`, strongly closed structurally;
- static/dynamic query pipeline: `DL6` on the canonical EXE for recovered paths;
- stage-CFG collision binding: `DL6` for the proven slot/descriptor path;
- universal flag semantics: not closed beyond proven local ABI/filter behavior;
- transform-selector semantic identity: target deeper than current evidence, still open;
- topology-changing writer/game acceptance: no blanket `DL8` claim.

A higher DL never upgrades evidence confidence automatically.

## 11. Module map

Shared format:

- `include/dmc_rengine/formats/hits.hpp`
- `src/formats/hits.cpp`
- `include/dmc_rengine/formats/hits_binary.hpp`
- `src/formats/hits_binary.cpp`

Game-agnostic HITS logic:

- `include/dmc_rengine/hits/contact.hpp`
- `include/dmc_rengine/hits/edit.hpp`
- `include/dmc_rengine/hits/result.hpp`
- `include/dmc_rengine/hits/runtime.hpp`
- `include/dmc_rengine/hits/spatial_compare.hpp`
- `include/dmc_rengine/hits/spatial_match.hpp`
- `include/dmc_rengine/hits/writer.hpp`
- `src/hits/*`

DMC3 reverse profile:

- `include/dmc_rengine/profiles/dmc3/hits_evidence_catalog.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_query_evidence.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_dynamic_update_evidence.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_source1_query_evidence.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_primitive_shape_evidence.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_contact_normal_evidence.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_primitive_descriptor_ownership_evidence.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_primitive_type01_evidence.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_stage_cfg_pac_evidence.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_stage_cfg_collision_tables.hpp`
- `include/dmc_rengine/profiles/dmc3/hits_collision_triplet.hpp`

## 12. Remaining research gates

1. Exact semantic names for all raw HITS flag bits/masks.
2. Transform-selector source ownership and bounds in stage CFG.
3. Remaining primitive types, especially evidence-gated runtime-only cases.
4. Complete original-game query/result semantics where Pass 10 intentionally retained specialized ABIs.
5. Original-game acceptance tests for topology-changing spatial rebuilds.
6. Broader corpus/game validation before declaring any universal behavior beyond the DMC3 canonical target.

## 13. Provenance

This synthesis reconciles the repository implementation with the preserved Drive HITS corpus through Raw Passes 7–10, including the Pass 8 runtime review, Pass 9 ABI/ownership saturation, Pass 10 query/runtime evidence, referenced stage-CFG descriptor census, and Slice 16 transform-source provenance work.

Historical evidence remains valuable but never outranks a later explicit correction. In particular, `HITS$` and universal `0x18060001` marker-scanning are superseded.
