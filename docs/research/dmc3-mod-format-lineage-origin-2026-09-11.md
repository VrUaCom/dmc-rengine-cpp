# DMC3 MOD format lineage — origin pass 1 (2026-09-11)

**Branch:** `reverse/mod-completion-20260907`  
**Canonical DMC3 HD executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Purpose

The canonical DMC3 HD `.MOD` unknown-field consumer phase is closed, but several bytes remain `PRESERVED_UNDECODED`. In particular, node-domain byte `+0x10` is physically read by the canonical executable and then dies without a downstream effect. This pass starts a format-lineage reverse to answer a different question:

> Is that dead-read byte a fossil of an older DMC3/Capcom model ABI where it once had a live semantic?

This research deliberately does **not** use third-party importers as format authority. Public importers/tools are only acquisition hints and lineage candidates. Canonical promotion requires retail bytes and/or a retail executable consumer.

## 1. Canonical DMC3 HD baseline

Current evidence-backed document shell:

```text
+0x00  "MOD "
+0x04  f32 version
+0x10  u8 object_count
+0x11  u8 node_count
+0x12  u8 texture_slot_count mirror
+0x13  u8 default_joint_index
+0x14  u32 runtime_metadata_u32
+0x20  u64 node-domain relative offset
header size 0x40
object stride 0x40
mesh stride 0x50
node-domain header size 0x20
transform stride 0x20
```

Observed retail versions are 0.82, 0.84, 1.00 and 1.01.

Node-domain `+0x10` is a canonical dead read:

```text
0x1402FD9C0 planner
0x1402FDA0C read u8 node-domain +0x10
0x1402FDA10 store to local rbp+0x18
later local reads = 0
```

`+0x11..+0x1F` have no provenance-confirmed canonical consumer. The whole shell remains exact-byte writer-preserved.

## 2. Legacy DMC3 `MOD ` family candidate

A public Noesis DMC3 importer committed in 2012, years before the 2018 PC HD Collection, recognizes the exact integer magic:

```text
541347661 == 0x20444F4D == bytes "MOD "
```

Its parser independently places two count bytes at the exact same header positions:

```text
+0x10  u8 numMesh
+0x11  u8 numBones
```

It then reports a different older layout:

```text
+0x1C  u32 pointer named bone_ptr by the importer
+0x30  beginning of repeated ~0x30-byte submesh-like records
```

The repeated record reads a vertex count, a 32-bit pointer, three floats and follows a secondary block containing five 32-bit stream pointers. This does **not** match the HD serialized object/mesh layout.

External source:
`Techokami/noesis-plugins-official/demonsangel/fmt_dmc3_mod.py`, historical commit `c3251e6d055ea588920898a983ab8b526c526ff8`, 2012-11-24.

### Evidence boundary

The following are strong lineage clues, not canonical semantics:

```text
same magic "MOD "                 lineage candidate
same +0x10 count-byte position    lineage candidate
same +0x11 bone/node count slot   lineage candidate
legacy +0x1C pointer name         importer claim only
legacy record semantics           importer claim only
```

The importer does not consume its `bone_ptr`, so its name cannot be used as proof of the pointed block's semantic.

## 3. DMC3SE PC 2006 packaging corroboration

Independent public tooling identifies the pre-HD Windows game as the same resource ecosystem:

- Game Extractor's DMC3 PC `PAC\0` plugin recognizes `0x20444F4D` as `.mod`;
- archived Style Switcher data for the DMC3SE 2006 PC port works with `data/GData.pak`, converts/renames it to `GDATA.AFS`, and ships `data2/pl000_00_*.pac` resources;
- Steam's legacy DMC3SE depot contains `data/GData.pak` and `data2/*.pac`.

This makes the 2006 PC game the strongest currently identified pre-HDC corpus target for an exact byte-to-byte lineage test.

It still does **not** prove that the public 2012 importer targeted one exact retail revision. A retail sample is required for promotion.

## 4. MT Framework `.MOD` is a different serialized branch

Later Capcom MT Framework `.MOD` uses a materially different document ABI. A public reverse template for UMvC3/MT identifies:

```text
magic       "MOD\0" (4D 4F 44 00)
+0x04       u16 version
+0x06       u16 joint count
then        primitive/material counts and a different offset table
```

DMC3 HD instead uses:

```text
magic       "MOD " (4D 4F 44 20)
+0x04       f32 version
+0x10/+0x11 object/node count bytes
```

Therefore the hypothesis that DMC4/RE5-era MT `.MOD` is the same serialized ABI carried forward from DMC3 is `REJECTED`. It may share broader engine ideas, but same extension is not sufficient evidence of format ancestry.

## 5. DMC2 and Onimusha 3 status

No trustworthy public DMC2 model-ABI description has been recovered in this pass. Public QuickBMS sources contain a DMC2 decompressor, proving a known DMC2 compression family exists, but this says nothing about the model layout.

Onimusha 3 remains a parallel engine-lineage control. Public historical discussions connect it to early MT Framework development, but no structural match to DMC3 `MOD ` has yet been demonstrated. No DMC3 semantic is imported from that history.

## 6. Dead-read ancestry experiment

The decisive next experiment is a retail legacy DMC3 sample, preferably 2006 PC and then PS2:

```text
legacy MOD bytes
 -> verify "MOD "
 -> confirm +0x10/+0x11 counts from the bytes themselves
 -> resolve the legacy +0x1C pointer without trusting its old importer name
 -> identify the pointed node/bone domain structurally
 -> compare that domain against HDC node-domain 0x20-byte shell
 -> locate an ancestral slot corresponding to HDC node +0x10, if one exists
 -> trace that slot in the legacy retail executable
```

Possible outcomes:

1. **Legacy field exists and is consumed**: HDC `+0x10` is likely a fossilized former semantic whose consumer was removed.
2. **Legacy field exists and is already dead**: the dead read predates HDC and is older technical debt/compatibility baggage.
3. **Legacy field does not exist**: HDC `+0x10` was introduced later, likely during port/remaster ABI evolution.
4. **Legacy node block is structurally unrelated**: the shared header prefix survived while hierarchy storage was redesigned; ancestry must be traced at a higher-level structure rather than by offset.

## 7. Acquisition and authority rules

- no leaked proprietary source code;
- no Blender/Noesis semantic promotion by itself;
- public reverse tools are navigation/corroboration only;
- retail assets and retail executable consumers are authority;
- equal offsets never transfer semantics automatically;
- no writer normalization follows from lineage hypotheses;
- preserve all current HDC `PRESERVED_UNDECODED` bytes exactly.

## Current conclusion

The strongest current ancestry model is:

```text
pre-HDC DMC3 "MOD " family
        |
        |  strong shared prefix: magic + count positions
        |  serialized body/layout evolves
        v
DMC3 HD "MOD " family

DMC4/RE5/UMvC3 MT "MOD\0"
        -> separate serialized branch (same extension, different ABI)
```

The origin of node-domain `+0x10` is **not solved yet**. The first concrete ancestor candidate has now been identified and the exact test needed to solve it is defined.
