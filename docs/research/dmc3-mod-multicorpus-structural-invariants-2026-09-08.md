# DMC3 HD MOD multi-corpus structural invariants — 2026-09-08

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable profile:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Primary bound corpus:** `em000-extract.zip` SHA-256 `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`

## Purpose

Continue MOD reverse after the first multi-corpus unknown-field pass and record only structural conclusions supported by the bytes currently available.

This pass deliberately does **not** invent semantics for fields that remain unread/zero. Its main result is a substantially stronger physical serialization contract for the node domain and the generated-topology workspace.

The current evidence set contains 38 unique MOD payloads:

- 35 recursive MOD payloads from `em000-extract.zip`;
- 2 independently supplied `pl000`-associated MOD payloads;
- 1 `id100` Red Orb counter MOD payload.

A separately supplied 110096-byte MOD is byte-identical to `em000_001.mod` and is not double-counted.

## Bound additional samples

### pl000 main MOD

SHA-256:

`e219e89285604cb6d800b0afdd3bec6684a6b00cd1862d464a669d2861ff3c89`

Observed core:

```text
version       1.01
objects       17
meshes        18
vertices      5316
nodes         24
texture mirror 3
default joint 1
header +0x14  217
```

### pl000 cloth-associated MOD

SHA-256:

`7a2be875b3702f59a607655f7a0a412801a6aea639dcb6e3b23d9b0a09c7e740`

The adjacent supplied text resource starts with `;pl000_02.clt`; its cloth `Bone` references fit the 33-node domain of this MOD. This is provenance/correlation evidence, not a claim that every adjacent resource relationship is runtime ownership.

Observed core:

```text
version       1.01
objects       6
meshes        8
vertices      828
nodes         33
texture mirror 3
default joint 0
header +0x14  217
```

### id100 Red Orb counter MOD

SHA-256:

`9cbbaba99fdd008e257258dfe87c5dfed7fae2a13c4b1c2b08d0e318f0213b90`

Observed core:

```text
version       1.01
objects       7
meshes        7
vertices      28
nodes         2
texture mirror 1
default joint 0
header +0x14  1000000
```

---

## 1. Aggregate multi-corpus census

```text
unique MODs       38
objects          166
meshes           180
vertices       20,976
transform records 285
```

Version distribution:

```text
0.82   1
0.84   4
1.00   6
1.01  27
```

All 38 files satisfy the currently recovered fixed-record grammar without aggregate-count or bounded-range failure.

Evidence status:

`CORPUS_CONFIRMED` for this bounded multi-family sample set.

---

## 2. Expanded zero-region census

The following byte ranges/fields are zero in every corresponding record in the current 38-MOD set:

| Field/range | Non-zero | Total | Status |
|---|---:|---:|---|
| header `+0x08..+0x0F` | 0 | 38 | `MULTI_CORPUS_OBSERVED_ZERO` |
| header `+0x18..+0x1F` | 0 | 38 | `MULTI_CORPUS_OBSERVED_ZERO` |
| header `+0x28..+0x3F` | 0 | 38 | `MULTI_CORPUS_OBSERVED_ZERO` |
| object `+0x04..+0x07` | 0 | 166 | `MULTI_CORPUS_OBSERVED_ZERO` |
| object `+0x14..+0x17` | 0 | 166 | `MULTI_CORPUS_OBSERVED_ZERO` |
| object `+0x20..+0x2F` | 0 | 166 | `MULTI_CORPUS_OBSERVED_ZERO` |
| mesh `+0x0C` | 0 | 180 | `MULTI_CORPUS_OBSERVED_ZERO` |
| mesh `+0x38` | 0 | 180 | `MULTI_CORPUS_OBSERVED_ZERO` |
| mesh `+0x48` serialized | 0 | 180 | `MULTI_CORPUS_OBSERVED_ZERO` |
| mesh `+0x4C` | 0 | 180 | `MULTI_CORPUS_OBSERVED_ZERO` |
| node-domain header `+0x10..+0x1F` | 0 | 38 | `MULTI_CORPUS_OBSERVED_ZERO` |
| transform `+0x1C` | 0 | 285 | `MULTI_CORPUS_OBSERVED_ZERO` |
| `BLENDINDICES.x` | 0 | 20,976 | `MULTI_CORPUS_OBSERVED_ZERO` |

### Important preservation boundary

This still does **not** authorize a writer to synthesize zero globally.

In particular:

- canonical MOD post-load `0x1402FE3B0` and runtime mesh construction `0x1402FE6A0` do not consume/relocate mesh `+0x0C/+0x38/+0x4C` in the confirmed path;
- that negative runtime evidence is stronger than “unknown bytes”, but weaker than “globally unused”;
- MOD `+0x38` must not be generalized as model-family padding because the homologous EFM field has separately recovered live COLOR0 ownership;
- all unknown source bytes remain preservation-authoritative until a global unused/reserved claim is independently proven.

---

## 3. Node-domain physical layout is now multi-corpus exact

For every 38/38 MOD payloads, serialized node-domain relative offsets equal the shared `NodeDomainCoreAbi` formulas exactly:

```text
parent_rel       = 0x20
order_rel        = 0x20 + align4(node_count)
motion_group_rel = 0x20 + 2 * align4(node_count)
transform_rel    = align16(0x20 + 3 * align4(node_count))
```

This includes node counts from 1 through the larger actor domains observed in the sample set, including 24 and 33 nodes in the pl000 samples.

All 38/38 parent/order arrays also satisfy the already proven topological evaluation contract:

```text
order position 0:
    parent = -1 / 0xFF

later order position:
    parent node has already been evaluated

nodeAtOrderPosition:
    complete permutation of 0..node_count-1
```

Evidence status:

`CORPUS_CONFIRMED`, consistent with existing EXE-confirmed hierarchy interpretation.

This strengthens layout-planning authority but does not merge SCM/MOD semantics for the adapter array. MOD `+0x08` remains the independently proven `motion_group` domain.

---

## 4. Translation magnitude invariant survives all 285 transforms

For every current transform record:

```text
serialized +0x0C
≈ sqrt(tx*tx + ty*ty + tz*tz)
```

Result:

```text
matches within absolute tolerance 1e-5: 285 / 285
largest observed absolute difference: < 5e-6
```

This is strong multi-corpus confirmation that transform `+0x0C` is genuinely the magnitude of serialized translation XYZ, not an independent scale/radius field.

Status:

`EXE_AND_CORPUS_CONFIRMED` when combined with the previously recovered MOD/EFM transform initializer behavior.

Transform `+0x1C`, by contrast, remains zero in 285/285 and `PRESERVED_UNDECODED` semantically.

---

## 5. motion_group value-domain extension

The `+0x08` node-domain array is already EXE-confirmed as the MOD motion-group selector. The expanded corpus gives this value histogram:

```text
group 0: 178
group 1: 104
group 2:   3
TOTAL:   285
```

No value outside `{0,1,2}` appears in the current 38-MOD set.

Additional sample behavior:

```text
pl000 main:             group0=11, group1=13
pl000 cloth-associated: group0=33
id100 counter:          group0=2
```

This is **not** authority to name the values “body”, “weapon”, “cloth”, etc. The value meanings remain unpromoted until executable/animation behavior proves them.

Status:

- field semantic: `EXE_CONFIRMED` (`motion_group`);
- current value domain `{0,1,2}`: `CORPUS_CONFIRMED` only.

---

## 6. Generated-topology workspace physical serialization — major closure

This pass recovered a much stronger writer-prerequisite invariant for mesh `+0x40`.

### 6.1 Address space

For a mesh record at physical file offset `mesh_record_offset`:

```text
workspace_absolute = mesh_record_offset + mesh.generated_workspace_relative_offset
```

This remains a **mesh-relative** pointer/address contract.

### 6.2 Alignment

For all current meshes:

```text
workspace_absolute % 16 == 0
```

Result:

`180 / 180`.

### 6.3 Exact capacity formula

For every 180/180 current meshes:

```text
capacity_bytes = align16(6 * (vertex_count - 2))
```

The current vertex-count range is 3..773, and the formula matches the physical span of every workspace exactly.

This extends the earlier em000-only observation to pl000 and id100.

### 6.4 Workspaces tile the file trailer exactly

Sort all mesh workspaces in one MOD by physical workspace offset.

For every non-final workspace:

```text
next_workspace_offset
== current_workspace_offset + capacity_bytes
```

For the final workspace:

```text
EOF
== final_workspace_offset + capacity_bytes
```

Therefore the workspace region is not a set of arbitrary independent holes. In all 38 current MODs it is one gapless trailing physical block partitioned by the per-mesh capacity formula.

### 6.5 Serialized fill pattern

Every 180/180 workspace begins with:

```text
u16 0x1212
```

More strongly:

```text
142 non-final workspaces:
    every byte == 0x12

38 final workspaces, exactly one per MOD:
    all bytes except the last two == 0x12
    final u16 == 0x0000
```

Therefore all 38/38 files currently end with:

```text
12 12 00 00
```

### 6.6 Safe interpretation

What is now safe to say:

- mesh `+0x40` resolves to a generated-topology workspace;
- its physical capacity formula is multi-corpus confirmed;
- the workspaces form a gapless file trailer;
- `0x1212` is the observed serialized fill word;
- a final zero word is an exact per-file trailing structural pattern in the current corpus;
- serialized mesh `+0x48` remains zero before runtime generation.

What is **not** yet safe to say:

- that `0x1212` is a runtime command or index value with a known semantic name;
- that the final zero word is a proven runtime terminator rather than a serialization/layout convention;
- that future writers may regenerate this trailer without byte-preservation/round-trip proof.

Evidence status:

`CORPUS_CONFIRMED / STRUCTURAL_CONFIRMED`, writer prerequisite only.

The canonical helper is generalized to:

```cpp
mod_generated_workspace_capacity(vertex_count)
```

while the old `em000_generated_workspace_capacity` name remains as a compatibility wrapper.

---

## 7. Object flags — broader corpus frequencies

Across 166 current objects:

```text
alpha_control == 0x80: 166 / 166
```

Observed exact source flag values:

```text
0x00000000 : 92
0x00100001 : 41
0x00020001 : 11
0x00020000 : 10
0x00200000 :  7
0x00120002 :  2
0x00100201 :  2
0x00024000 :  1
```

Individual observed bit frequencies:

```text
0x00000001 : 54
0x00000002 :  2
0x00000200 :  2
0x00004000 :  1
0x00020000 : 24
0x00100000 : 45
0x00200000 :  7
```

Important consequences:

- `0x00100000` is not enemy-only; it appears in pl000 as well as em000;
- `0x00200000` remains limited to seven em000 objects in the current bounded set;
- the pl000 cloth-associated MOD does not require `0x00200000`, rejecting any universal “cloth flag” interpretation;
- exactly two current objects have nonzero serialized `+0x18/+0x1C`, both in `em000_033.mod`, values `1.25f` and `2`, consistent with the already recovered conditional transfer path.

No new artistic flag names are promoted by this census.

---

## 8. Header +0x14 remains open — prior universal decimal hypothesis stays rejected

The additional samples continue to require the corrected model:

```text
em000 examples: clustered six-digit values
pl000 main:      217
pl000 cloth:     217
id100:           1000000
```

The arithmetic projection

```text
high   = raw / 100000
middle = (raw / 100) % 1000
low    = raw % 100
```

is lossless arithmetic but not evidence of a universal MOD semantic partition.

The only canonical high-confidence flow remains:

```text
serialized MOD +0x14
    -> common model-manager initializer
    -> manager +0xE4
```

A MOD-specific downstream consumer census is still required before a public semantic name can replace `runtime_metadata_u32`.

---

## 9. Historical external-tool cross-check — non-authoritative

As a hypothesis/corroboration source only, the public historical `igrbn/DMC1-3_Blender_Tools` DMC3 importer was inspected.

Its DMC3 model importer leaves the same still-open areas unnamed/skipped, including header `+0x13/+0x14`, much of mesh `+0x04..+0x0F`, mesh `+0x38`, and the transform tail. Its shared mesh reader also explicitly discards the first byte of each four-byte blend-index record and uses the following three bytes divided by four as bone indices.

This is useful historical corroboration for the already canonical y/z/w skin-lane interpretation and for the fact that these fields were not semantically understood by that external importer. It is **not** executable evidence and promotes no field by itself.

Status:

`EXTERNAL_TOOL_OBSERVATION / SALVAGE_ONLY`.

Reference:

`https://github.com/igrbn/DMC1-3_Blender_Tools`

---

## 10. Canonical code changes from this pass

Updated:

```text
include/dmc_rengine/analysis/mod/corpus_observations.hpp
src/analysis/mod/corpus_observations.cpp
```

The code now records:

- 38-file version distribution;
- expanded zero-range invariants;
- exact node-domain layout count;
- topological hierarchy count;
- translation-magnitude agreement count;
- expanded motion-group histogram;
- generated-workspace capacity/alignment/fill/trailing-block invariants;
- broader object flag frequencies needed by follow-up reverse;
- generalized `mod_generated_workspace_capacity` helper.

No parser monolith or writer code was added.

---

## 11. What remains unresolved after this pass

### Requires direct canonical EXE evidence

1. header `+0x14` / manager `+0xE4` downstream consumers;
2. transform `+0x1C` read/write census;
3. raw-MOD direct reads of mesh `+0x0C/+0x38/+0x4C` outside the known load path;
4. `BLENDINDICES.x` CPU readers and every runtime-selected `DMC3_MOD`, `DMC3_MOD_SP`, `DMC3_MOD_STX` shader variant;
5. final packet effect/user-facing meaning of object source `0x00100000`;
6. final behavior of object source `0x00200000`;
7. runtime meaning, if any, of the final zero word in the serialized workspace trailer.

### Requires broader retail corpus

1. player/enemy/item/weapon/special MOD families beyond the current 38 unique payloads;
2. a nonzero variant, if one exists, for transform `+0x1C`;
3. a nonzero variant, if one exists, for mesh `+0x0C/+0x38/+0x4C`;
4. a nonzero `BLENDINDICES.x` sample, if one exists;
5. motion-group values outside `{0,1,2}`, if any exist.

---

## 12. Writer-authority boundary

This pass substantially improves layout planning, especially for the generated-topology trailer, but MOD writing is still **NOT AUTHORIZED**.

Minimum writer proof remains:

```text
parse source
-> typed IR
-> preserve every unknown/source-owned byte
-> no-edit serialize
-> byte-identical or explicitly proven canonical-equivalent output
-> reparse
-> semantic equality
-> texture companion coherence
-> PAC/NBZ reintegration
-> canonical dmc3.exe acceptance
```

Only after that may edited-transform/geometry/UV/texture-binding tests promote MOD to reversible authoring.

## Conclusion

The important result of this pass is not a speculative field rename. It is that several parts of MOD physical serialization have now survived a wider multi-family corpus:

- exact node-domain physical layout: 38/38;
- topological hierarchy contract: 38/38;
- translation magnitude relation: 285/285;
- unresolved zero fields remain zero across 180 meshes / 285 transforms / 20,976 vertices;
- generated-topology workspace capacity and trailing-block serialization are exact across 180/180 meshes and 38/38 MOD files.

These are now suitable canonical structural contracts and future writer prerequisites, while the remaining semantic unknowns stay explicitly preserved and unpromoted.
