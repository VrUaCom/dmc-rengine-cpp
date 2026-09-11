# DMC3 HD MOD — secondary zero-region census

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`  
**Evidence follow-up:** 2026-09-11

## Rule

A zero-filled region is not promoted to padding. `RESERVED_OBSERVED_ZERO` means only that the bounded corpus is zero in that region. It never authorizes writer zero-normalization.

Canonical runtime dormancy is also a behavioral result, not a serialized semantic name. A dormant field remains `PRESERVED_UNDECODED` unless stronger format evidence gives it a semantic identity.

The final header/node whole-image receipt is:

`data/reverse/dmc3-mod-header-node-shell-closure-20260911.json`

with the detailed research note:

`docs/research/dmc3-mod-header-node-shell-closure-2026-09-11.md`.

## Header shell — whole-image closed

Canonical manager/header initializer `0x1402F9570` positively consumes the known header domain including `+0x10`, `+0x11`, `+0x13`, and `+0x14`. It retains the complete source document at manager `+0x108`, so initializer-local no-read evidence was not sufficient.

The final whole-image census starts from all non-stack `+0x108 -> register` source-like loads in the 12,235 `.pdata` runtime functions and performs a separate leaf-code pass. It finds:

```text
116 pdata functions with source-like +0x108 loads
234 source-like load sites
53 raw header-target candidate functions
138 raw target-range read/write operations
14 direct family-classifier call sites
0 provenance-confirmed model-manager target reads
0 provenance-confirmed model-manager target writes
```

The raw target candidates are offset collisions in other owner layouts. Important rejected controls include the separate `0x14030B550/0x14030B5C0` owner reached from `0x14008BE50`, the resource/table owners at `0x140291D60/0x14029581C`, and the distinct CMotion/runtime-transform owner family around `0x14030E.../0x14031...`.

The leaf pass adds two source-like loads, `0x140165B23` and `0x140165B46`; both are nested-owner transform-like writes through `owner +0x448` and lack model-manager provenance.

Canonical result:

```text
header +0x08..+0x0F
header +0x18..+0x1F
header +0x28..+0x3F

bounded corpus                       RESERVED_OBSERVED_ZERO
canonical typed runtime effect       EXE_CONFIRMED: dormant / no effect
serialized semantic                  PRESERVED_UNDECODED
writer policy                        preserve exact source bytes
```

No region is renamed padding/reserved.

## Object record provenance

The canonical MOD/EFM object initializer `0x1403029E0` derives each serialized object exactly as:

```text
source_header + 0x40 + object_index * 0x40
```

at `0x140302A70..0x140302A81`.

At `0x140302AAA` it stores the **complete serialized object-record pointer** into:

```text
runtime object +0x18
```

This is not a read of serialized object `+0x18/+0x1C`.

The retained pointer means initializer-local no-read evidence is insufficient. The object secondary regions therefore required a full retained-pointer and alternative-direct-source census.

## Live MOD-specific object +0x18/+0x1C

These are not part of the secondary-zero closure. When source flag `0x00000200` or `0x00000400` is active, `0x1403029E0` reads serialized object `+0x18` / `+0x1C` and propagates them into runtime render state. They remain live `PRESERVED_UNDECODED` fields under their separate contract.

## Target object regions

The bounded corpus-zero regions are:

```text
object +0x04..+0x07
object +0x14..+0x17
object +0x20..+0x2F
```

Current corpus contains 166 objects and all bytes in those regions are zero. This is `RESERVED_OBSERVED_ZERO` corpus evidence only.

## Whole-image canonical runtime-object census

The runtime object is structurally identified by the joint model-manager fingerprint:

```text
manager +0xE8 object count
manager +0x100 runtime-object array
runtime_object = manager +0x100 + object_index * 0x380
```

A whole-executable `.pdata` census finds **32 exact runtime-object derivation roots**. The previously bounded model-core pass counted 31; the additional root is `0x14029F0B0`, which calls model-core accessor `0x140089DE0`, validates manager `+0xE8`, derives the same `0x380` runtime object from manager `+0x100`, and therefore belongs to the same canonical model-manager domain.

Conservative pointer propagation from all 32 roots reaches:

```text
61 typed interprocedural states
49 direct pointer-tagged calls
5 provenance-confirmed runtime object +0x18 loads
0 retained serialized-object pointer escapes
0 retained serialized-object indirect-call escapes
```

The five retained-pointer loads are:

| Function | Load | Result |
|---|---|---|
| `0x14029F0B0` | `0x14029F0F0` | reads serialized object `+0x08`, then enters mesh-record domain |
| `0x1402F7D60` | `0x1402F7DB4` | EFM sibling builder; reads serialized object `+0x08`, then mesh records |
| `0x1402F8000` | `0x1402F845E` | EFM render builder; saved local is never dereferenced before return |
| `0x1402FE6A0` | `0x1402FE6F4` | MOD runtime mesh builder; reads serialized object `+0x08`, then mesh records |
| `0x1402FE930` | `0x1402FED8E` | MOD render builder; saved local is never dereferenced before return |

Across all five paths, the only provenance-confirmed serialized-object field read through the retained pointer is:

```text
serialized object +0x08 mesh-table pointer   3 reads
```

There are zero reads/writes of:

```text
+0x04..+0x07
+0x14..+0x17
+0x20..+0x2F
```

No retained serialized-object pointer is stored or passed through an indirect call from the typed surface.

## Additional whole-image source-header derivation control

A separate whole-image census searched for:

```text
manager/source +0x108 -> source header
object_index * 0x40
source_header + 0x40 + index*0x40
```

Exactly five functions match:

```text
0x1402F9F20
0x1402FDB40
0x1402FDD10
0x1403029E0
0x140302F10
```

The MOD/EFM/shared planning paths consume known fields only. `0x140302F10` is the independently identified SCM `0x3C0` object initializer, and `0x1402F9F20` belongs to the same separate `0x3C0` runtime-owner domain.

Thus no alternative direct-source path revives the target object regions.

## Canonical closure of object secondary regions

```text
object +0x04..+0x07
object +0x14..+0x17
object +0x20..+0x2F

bounded corpus                       RESERVED_OBSERVED_ZERO
canonical typed runtime effect       EXE_CONFIRMED: dormant / no effect
serialized semantic                  PRESERVED_UNDECODED
writer policy                        preserve exact source bytes
```

## Node-domain shell — whole-image closed

`0x1402F1DB0` binds only the four relative dwords `+0x00/+0x04/+0x08/+0x0C`; it does not consume the secondary shell.

Whole-image source/node propagation leaves three raw candidates. Two are separate resource owners and are rejected. The one canonical model path is the layout planner `0x1402FD9C0`:

```text
0x1402FD9F1  manager +0x108 -> source
0x1402FDA00  source +0x20   -> node-domain
0x1402FDA0C  read byte [node +0x10]
0x1402FDA10  store byte -> [rbp+0x18]
```

The local `[rbp+0x18]` value has **zero later reads** before return. `node +0x10` therefore has one canonical dead read, not “no read”. `node +0x11..+0x1F` has no provenance-confirmed model-domain consumer, and the raw shell pointer does not escape.

Canonical result:

```text
node-domain +0x10..+0x1F

bounded corpus                       RESERVED_OBSERVED_ZERO
canonical typed runtime effect       EXE_CONFIRMED: dormant / no effect
serialized semantic                  PRESERVED_UNDECODED
writer policy                        preserve exact source bytes
```

## Rejected claims

- zero shell implies padding — `REJECTED`;
- no initializer read implies no later read — `REJECTED`;
- raw `+0x108` displacement equality proves model-manager provenance — `REJECTED`;
- node-domain `+0x10` is never read — `REJECTED`; it has one dead read at `0x1402FDA0C`;
- `0x140302AAA` reads serialized object `+0x18/+0x1C` — `REJECTED`; it stores the serialized record pointer;
- equal `+0x18` displacement in another owner proves MOD use — `REJECTED`;
- the SCM `0x3C0` retained-pointer path is a MOD consumer because it also uses 0x40 source records — `REJECTED`;
- canonical runtime dormancy authorizes writer zero-normalization — `REJECTED`.

## Phase result

The secondary header, object and node-domain runtime-consumption gates are closed. Together with the previously closed transform, mesh, BLENDINDICES and source-flag gates, there is no remaining direct-EXE unknown-field consumer gate in the current canonical MOD contract.

The next separate phase is byte-preserving MOD writer promotion and exact no-edit round-trip proof. Source preservation remains mandatory for every undecoded field.
