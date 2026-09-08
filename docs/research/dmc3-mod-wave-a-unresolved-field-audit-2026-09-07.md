# DMC3 HD MOD Wave A unresolved-field audit — 2026-09-07

**Branch:** `reverse/mod-completion-20260907`  
**Base:** `main@1a029daace6790e1c832e13ba4841ae45004a147`  
**Canonical executable:** `dmc3.exe` SHA-256 `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Scope

Continue MOD completion after the header `+0x14` audit by reviewing the other highest-value serialized/runtime unknowns that currently block a truthful writer contract:

- local transform `+0x1C`;
- mesh `+0x0C`, `+0x38`, `+0x4C`;
- `BLENDINDICES.x`;
- object source flag `0x00100000`.

This pass promotes no new semantic names. Its purpose is to distinguish what is already strongly constrained from what still requires fresh executable/corpus evidence.

---

## A2 — Local transform `+0x1C`

### Serialized position

MOD node-indexed local transform record:

```text
stride 0x20
+0x00 float3 translation
+0x0C f32 translation_magnitude
+0x10 float3 rotation_xyz_radians
+0x1C f32 unresolved
```

The canonical MOD/EFM transform initializer is `0x1402FA080`. Existing EXE evidence directly closes translation XYZ, XYZ rotation and hierarchy/world composition. The promoted transform evidence does not assign a downstream behavior to `+0x1C`.

### Corpus constraint

In the bound `em000-extract.zip` corpus:

```text
transform +0x1C == 0.0f in 226 / 226 records
```

### Repository consumer census

Current canonical C++ references to `reserved1c` are structural/preservation/validation references. No MOD runtime-analysis module currently gives the field a behavior or semantic accessor.

### Safe result

```text
serialized layout = STRUCTURAL_CONFIRMED
em000 value        = RESERVED_OBSERVED_ZERO
MOD semantic role  = PRESERVED_UNDECODED
writer rule        = preserve source value; do not force zero globally
```

### Required closure

- fresh canonical EXE census around the MOD/EFM transform initializer and all consumers of copied runtime-node/local-transform state;
- broader actor/model corpus looking for any nonzero `+0x1C`;
- if a nonzero sample exists, compare runtime behavior before assigning any name.

---

## A3 — Mesh `+0x0C`, `+0x38`, `+0x4C`

### Strong negative runtime evidence already established

Canonical MOD post-load `0x1402FE3B0` consumes/relocates the live mesh fields at `+0x10/+0x18/+0x20/+0x28/+0x30`, uses record-relative `+0x40`, and produces `+0x48` generated count. It does **not** relocate or rewrite `+0x0C`, `+0x38`, or `+0x4C`.

Runtime mesh builder `0x1402FE6A0` transfers the confirmed live streams/workspace/count and likewise has no confirmed transfer of `+0x0C`, `+0x38`, or `+0x4C`.

Therefore all three are strongly constrained as:

```text
unconsumed in the confirmed MOD post-load/runtime-mesh construction path
```

This is stronger than “unknown bytes”, but weaker than “globally unused”.

### Corpus constraint

In em000:

```text
mesh +0x0C == 0 in 147 / 147 meshes
mesh +0x38 == 0 in 147 / 147 meshes
mesh +0x4C == 0 in 147 / 147 meshes
```

### Safe result

All three remain `PRESERVED_UNDECODED` with exact source-byte authority.

A future writer must not:

- drop the fields;
- normalize them to zero;
- repurpose `+0x38` as another pointer simply because neighboring offsets are pointers;
- copy EFM/SCM semantics by stride analogy.

### Required closure

1. global canonical EXE search for direct reads from raw MOD mesh records at these offsets outside the already-audited load path;
2. wider retail MOD corpus histogram across player, enemy, weapon/accessory and special models;
3. if still universally zero and globally unread, only then consider a stronger reserved/padding claim;
4. writer parity must preserve them before that stronger claim exists.

---

## A4 — `BLENDINDICES.x`

### What is already closed

The runtime-linked tag-5 `DMC3_MOD.hlsl` skin path uses `BLENDINDICES.y/z/w` as the starts of the three influence matrices. CPU evidence independently proves the `/4` node/bone mapping from an active blend-index lane.

The packed lower 15 bits of the companion u16 carry the three 5-bit `/31` weights; bit `0x8000` is independent topology state.

### What is not closed

No canonical semantic role has been established for `BLENDINDICES.x`.

### Corpus constraint

```text
BLENDINDICES.x == 0 in 14,804 / 14,804 em000 vertices
```

Earlier three-file skin census also observed lane 0 as zero while y/z/w obeyed the matrix-row addressing contract.

### Safe result

```text
lane x corpus behavior = RESERVED_OBSERVED_ZERO
lane x semantic role   = PRESERVED_UNDECODED
active skin lanes       = y/z/w, EXE_CONFIRMED
```

### Required closure

- inspect every runtime-selected MOD shader variant (`MOD`, `MOD_SP`, `MOD_STX`) for any `matIndex.x`/lane-0 use;
- census CPU paths that read the 4-byte blend stream, not just the already-proven lane-1 consumer;
- search broader retail MOD corpus for any nonzero x lane;
- if a nonzero variant exists, correlate it with shader tag/object flags/material path before assigning meaning.

A zero em000 lane is not authority to overwrite lane x in other revisions/models.

---

## A5 — Object source flag `0x00100000`

### Existing evidence

Common render-state helper `0x140302640` consumes the low source nibble and source bits including `0x00010000` and `0x00100000` while constructing legacy render-state state/packets.

This proves `0x00100000` is live render-path input and is not free/reserved bitmap space.

### What remains open

The current canonical MOD runtime projection deliberately does not assign a stable user-facing semantic effect to `0x00100000`, because the downstream packet behavior has not been closed sufficiently to separate artistic intent from low-level legacy state encoding.

### Safe result

```text
bit is live                = EXE_CONFIRMED
complete downstream effect = PARTIAL / OPEN
artistic semantic name     = PRESERVED_UNDECODED
writer mutation authority  = NOT_PROMOTED
```

### Required closure

- follow the exact `0x00100000` branch through `0x140302640` to the final render/material packet fields;
- identify the consumer of those packet fields;
- compare against object/shader/material corpus patterns;
- only expose a user-facing name if the behavior is stable and MOD-authorized.

---

## Wave A priority after this audit

The unresolved fields now divide into two classes.

### Class 1 — blocked primarily on fresh canonical xrefs

- header `+0x14` / manager `+0xE4`;
- transform `+0x1C`;
- mesh `+0x0C/+0x38/+0x4C` global-read census;
- `BLENDINDICES.x` CPU/shader-variant census;
- object flag `0x00100000` downstream packet behavior.

### Class 2 — blocked primarily on broader corpus

- whether transform `+0x1C` can ever be nonzero;
- whether mesh preserved fields can ever be nonzero;
- whether `BLENDINDICES.x` can ever be nonzero;
- whether current em000 motion-group value patterns generalize across actor families.

The next highest-value evidence package is therefore not another speculative parser change. It is a **canonical byte-window/xref acquisition + multi-family MOD corpus sweep**.

## Promotion rule

No field in this note should be renamed in the public API merely because it is zero in em000. Promotion requires direct behavior or a stronger proven invariant. Until then, preservation is the correct canonical behavior.
