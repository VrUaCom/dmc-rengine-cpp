# DMC3 HD MOD — source flag 0x00100000 reverse

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Serialized domain

The target is bit `0x00100000` in the serialized object source flag word at object `+0x10`.

Known corpus distribution includes 36 em000 objects, 8 objects in the pl000 slot-1 model and 1 object in the pl000 slot-12 cloth-associated model. Presence in a visually suggestive model is not semantic authority.

## Baseline/effective runtime state

Canonical object initialization copies serialized object `+0x10` into two runtime words:

```text
runtime object +0x10  baseline/source flags
runtime object +0x14  mutable/effective flags
```

The later reset path at `0x140305911..0x14030593B` distinguishes the two explicitly:

```text
effective = (effective & 0xFFEFFFF0) | (baseline & 0x0010000F)
```

Thus the low mode nibble and source bit `0x00100000` are intentionally restored from baseline. This is `EXE_CONFIRMED` state-machine behavior.

## Packet projection

Helper `0x140302640` receives the effective/source flag word in `EDX` and constructs a packet/state block rooted at runtime object `+0x80`.

For an active low-nibble mode other than mode 4:

```text
bit clear: packet +0x08 = 0x000000000005000D
bit set:   packet +0x08 = 0x000000000005010D
```

The same bit is checked again at `0x14030274A`. In the active low-mode path, the clear-bit branch ORs `0x0000000100000000` into packet `+0x00`, while the set-bit branch omits that high mask.

Low mode 4 is a separate fixed path and writes packet `+0x08 = 0x50007` regardless of this bit.

## Packet -> per-mesh render descriptor

The terminal trace now extends beyond the object-local packet.

`0x1403028F0` computes:

```text
descriptor = runtime object + 0xB0 + mesh_index * 0x50
packet     = runtime object + 0x80
```

It then copies packet state into the descriptor:

```text
packet +0x00 -> descriptor +0x10   0x14030296B..0x14030296E
packet +0x08 -> descriptor +0x20   0x140302937..0x14030293B
packet +0x10 -> descriptor +0x30   0x140302951..0x140302955
packet +0x18 -> descriptor +0x40   0x140302984..0x140302988
```

Packet byte `+0x20` is also tested and conditionally updated against the mesh bit in `0x140302991..0x1403029CD`.

Therefore the `0x00100000` difference is not an object-local dead value: its packet-state projection reaches the per-mesh runtime render descriptor.

## Render-command handoff

Command-building paths independently reconstruct the same descriptor address `object +0xB0 + mesh_index*0x50`.

Examples:

```text
0x1402F8498  mesh_index * 0x50
0x1402F84A0  lea descriptor

0x1402FEDC8  mesh_index * 0x50
0x1402FEDD0  lea descriptor
```

The address is then encoded/referenced in generated command data. This closes another propagation edge:

```text
serialized source flag 0x00100000
  -> baseline/effective runtime flags
  -> 0x140302640 packet +0x80
  -> 0x1403028F0 per-mesh descriptor
  -> render-command construction
```

## What is closed

All of the following are `EXE_CONFIRMED`:

- source bit is preserved as baseline state;
- effective bit 20 can be restored from baseline;
- bit 20 selects exact packet `+0x08` and `+0x00` state differences;
- those packet differences are copied into per-mesh render descriptors;
- command construction references those descriptors.

This is a real persistent renderer-state path, not a corpus-only candidate.

## What is still open

The final backend interpretation of descriptor `+0x10/+0x20` has not yet been assigned a proven artistic/material name. Therefore names such as cloth, alpha mode, blend mode, culling mode, lighting mode, or translucency remain unsupported.

A command-stream handoff is stronger than a local packet copy but still is not semantic authority for the final state name.

## C++ contract

`include/dmc_rengine/analysis/mod/object_flags.hpp` models only the executable-proven baseline/effective refresh and packet projection. Compile-time regression guards preserve the exact constants and mode-4 exception.

## Status

```text
serialized bit presence                CORPUS_CONFIRMED
baseline/effective state-machine       EXE_CONFIRMED
source bit -> packet projection        EXE_CONFIRMED
packet -> per-mesh descriptor          EXE_CONFIRMED
descriptor -> render-command handoff   EXE_CONFIRMED
final renderer/material state name     PRESERVED_UNDECODED
writer policy                           preserve
```

## Next gate

Trace the command/backend consumer that interprets the per-mesh descriptor words copied from packet `+0x00/+0x08`. Promotion to a visual/material semantic is allowed only when that terminal interpretation is proven; until then keep the source bit byte-preserved and semantically unnamed.
