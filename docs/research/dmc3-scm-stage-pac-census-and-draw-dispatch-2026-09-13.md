# DMC3 HD SCM — stage PAC retail census and render-dispatch deep pass

**Date:** 2026-09-13  
**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Machine receipt:** `data/reverse/dmc3-scm-stage-pac-retail-census-20260913.json`

## Scope

This pass continues the SCM reader/runtime reverse from the actual canonical executable and freshly supplied retail stage PACs. It deliberately separates:

- serialized format structure;
- runtime carry/provenance;
- render-pass selection;
- command-queue construction;
- still-unresolved high-level labels.

A numeric offset match without pointer/layout provenance is not evidence.

## 1. Four stage PACs contain 51 distinct structural SCM payloads

Direct scanning of `st000.pac` through `st003.pac` found 51 `SCM ` candidates. Each candidate was accepted only after a complete structural check: object/mesh tables, fixed `0x50` mesh continuation chain, object vertex sums, all vertex streams, scene arrays/permutation, transforms and generated-index workspace footprint.

```text
st000.pac  16 SCM
st001.pac   3 SCM
st002.pac   5 SCM
st003.pac  27 SCM
-----------------
total      51 SCM
```

All 51 payloads have different SHA-256 hashes. These are therefore 51 distinct retail SCM specimens rather than repeated copies.

Aggregate material:

```text
objects       345
meshes        506
scene nodes   408
vertices   107233
```

## 2. Retail SCM version domain is wider than 1.01

Observed structural versions are:

```text
0.83   1 payload
0.90  16 payloads
1.00   1 payload
1.01  33 payloads
```

The previous parser warning that treated anything other than `1.01` as outside the confirmed DMC3-HD SCM corpus is stale. The reader must accept the four observed versions as corpus-confirmed while continuing to preserve and warn on future unseen versions rather than rejecting them solely by version.

## 3. Header +0x14 decimal structure survives, unique-ID interpretation does not

The arithmetic decomposition continues to hold:

```text
raw = family_class * 100000 + model_set * 100 + sub_index
```

The new corpus expands observed family classes to:

```text
3, 4, 7, 8
```

New bounded examples include:

```text
730507 -> family 7, model_set 305, sub_index 7
813800 -> family 8, model_set 138, sub_index 0
```

However the stronger interpretation of this field as a unique resource identity is rejected:

```text
730507 occurs in 14 different SHA-unique SCM payloads
813800 occurs in  5 different SHA-unique SCM payloads
```

The evidence-safe contract is therefore:

```text
serialized structural resource code
+ stable decimal component decomposition
+ EXE-confirmed carry to manager+0xE4
- not a unique payload/resource identifier
- high-level manager+0xE4 role still unresolved
```

A bounded model-core displacement census finds the SCM write to manager `+0xE4` but no type-proven downstream read of that same manager field. Unrelated `[reg+0xE4]` accesses are rejected until provenance reaches the SCM manager.

## 4. Header +0x13 is a lighting reference node index

The deeper executable chain now closes the technical role of `header+0x13`:

```text
SCM+0x13
 -> 0x1402F960E
 -> manager+0xFA
 -> CDrawSCM 0x1402FD040
 -> manager+0x188 + nodeIndex*0x40
 -> selected scene-node world-matrix row +0x30
 -> 0x1402EE560 lighting-state construction
 -> 0x1402FC850 constant-state upload
 -> MDL_LIGHT_MAT { Lc, Lv }
```

The embedded stage shader consumes `Lv` for light intensity and `Lc` for light colour. The evidence-safe technical field name is therefore:

```text
lighting_reference_node_index
```

The current supplied retail corpus serializes zero, but the consumer and node-index semantics are EXE-confirmed. Parser policy now treats it as a scene-node index rather than a reserved byte.

## 5. Preservation lanes survive legacy retail revisions

Across all 51 SCM payloads, including versions `0.83`, `0.90` and `1.00`, the following remain zero:

- header secondary/preservation lanes;
- object `+0x04` and `+0x14..+0x2F`;
- mesh `+0x0C`, `+0x30`, serialized generated count `+0x48`, `+0x4C`;
- scene block `+0x10..+0x1F`;
- transform `+0x1C`;
- GS CLAMP fields in all 506 meshes.

This strengthens the negative corpus boundary substantially, but does not authorize calling these bytes padding or zeroing parsed files. Parsed resources remain exact-preservation authority.

Topology also remains stable:

```text
107233 vertices
observed topology byte values = {0, 2}
```

No retail topology bit outside confirmed break bit `0x02` is observed.

## 6. SCM render-pass selector

The canonical runtime manager stores a 16-bit pass selector at `manager+0x214`. Setter `0x140302370` proves a family-specific contract:

```text
if (manager+0xE0 & 0xF0000000) == 0x30000000:
    manager+0x214 = 4
else:
    manager+0x214 = 0
```

The `0x30000000` family is independently tied to `SCM ` by the primary render-family classifier. Therefore:

```text
manager+0x214 == 4
```

is an EXE-confirmed **SCM-family render-pass selector**.

This is distinct from the model-family selector path at `0x140302540`, which assigns mode `2` only to the `0x10000000/0x20000000` families.

## 7. Mode 4 has a bucket stage and a per-object command stage

The main dispatcher `0x140300740` first converts pass mode to a per-pass bucket/index stored at `manager+0x218`.

For SCM mode `4` it calls:

```text
0x140304950
 -> 0x1402F9680(manager+0x210, 0x7E, 7)
```

`0x1402F9680` is a numeric bucket/index quantizer. It is not the world-transform updater at `0x1402F9700`; the adjacent addresses must not be conflated.

The later per-object dispatch again checks `manager+0x214`. For mode `4`:

```text
if manager+0xE0 bit1:
    0x140305440
else:
    0x140300150
```

Both receive:

```text
object-related runtime state
manager pass bucket at +0x218
per-frame manager arrays +0x138/+0x148
```

and construct encoded render-command nodes in separate command arenas.

Observed arenas include:

```text
0x140300150 -> queue base global 0x1405D9D70
0x140305440 -> queue base global 0x1405D9D88
```

The queue layouts are not arbitrary pointers: both use fixed per-frame arenas and linked encoded command handles. Queue/link helper `0x140333A50` resolves relative handles against the common command base `0x1405D9EA8` and links nodes through the runtime queue table `0x1405D9D50`.

This closes the SCM dispatch boundary to command-queue construction without inventing shader names for the two subpaths.

## 8. What is still open below the command queue

The remaining shader/resource question is now much narrower:

```text
SCM family
 -> pass selector 4
 -> bucket 0x140304950/0x1402F9680
 -> per-object 0x140305440 or 0x140300150
 -> queue/link 0x140333A50
 -> command execution/backend
```

The next promotion requires tracing the linked command nodes through the command executor into concrete shader/resource binding. Until then the two mode-4 subpaths should remain operationally named, not called opaque/translucent/material variants.

## 9. Current evidence state

Promoted in this pass:

```text
SCM versions 0.83/0.90/1.00/1.01     CORPUS_CONFIRMED
resource-code families 3/4/7/8        CORPUS_CONFIRMED
resource-code unique-ID hypothesis    REJECTED
header+0x13 lighting node semantic    EXE_CONFIRMED
SCM runtime pass selector == 4        EXE_CONFIRMED
mode4 bucket path                      EXE_CONFIRMED
mode4 two command-queue subpaths       EXE_CONFIRMED
legacy preservation-lane zeros         CORPUS_CONFIRMED_NEGATIVE
```

Still terminal/preservation or research frontier:

```text
manager+0xE4 high-level role           PRESERVED_UNDECODED
object source flag 0x00200000          PRESERVED_UNDECODED_WITH_NEGATIVE_EVIDENCE
mesh +0x0C/+0x30/+0x4C                PRESERVED_UNDECODED_WITH_DEEP_NEGATIVE_EVIDENCE
exact mode4 command -> shader mapping   RESEARCH_REQUIRED
```
