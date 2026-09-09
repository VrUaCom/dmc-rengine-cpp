# DMC3 HD MOD — source flag 0x00100000 reverse

**Branch:** `reverse/mod-completion-20260907`  
**Canonical executable:** `dmc3.exe`  
**SHA-256:** `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`

## Serialized domain

The target is bit `0x00100000` in the serialized object source flag word at object `+0x10`.

Known corpus distribution includes 36 em000 objects, 8 objects in the pl000 slot-1 model and 1 object in the pl000 slot-12 cloth-associated model. Presence in a visually suggestive model is not semantic authority.

## Canonical executable consumer

Helper `0x140302640` receives the source/runtime flag word in `EDX` and constructs a packet/state block rooted at runtime object `+0x80`.

For an active low-nibble mode other than mode 4:

```text
0x1403026D1  mask 0x00100000

bit clear:
  packet +0x08 = 0x000000000005000D

bit set:
  packet +0x08 = 0x000000000005010D
```

The same bit is checked again at `0x14030274A`. In the active low-mode path, the clear-bit branch ORs `0x0000000100000000` into packet `+0x00`, while the set-bit branch omits that high mask.

Low mode 4 is a separate fixed path and writes packet `+0x08 = 0x50007` regardless of this bit.

Known calls into `0x140302640` include:

```text
0x140302E0C
0x140303405
0x140305949
0x1403059B5
0x140305C06
```

At the primary construction call, `0x140302E05` moves the recovered source flag word into `EDX` immediately before the helper call.

## What is closed

The following is `EXE_CONFIRMED`:

```text
source flag 0x00100000
  -> 0x140302640
  -> packet +0x08 state 0x5000D / 0x5010D
  -> packet +0x00 high-mask branch
```

This is a real runtime effect, not a corpus-only candidate.

## What is still open

The final meaning of the packet difference has not yet been traced to the terminal renderer/material/shader consumer. Therefore this pass does not assign names such as cloth, alpha mode, blend mode, culling mode, lighting mode, or any other visual semantic.

Packet-state closure is not equivalent to final artistic-semantic closure.

## C++ contract

`include/dmc_rengine/analysis/mod/object_flags.hpp` implements only the executable-proven packet projection. Compile-time regression guards require the exact constants and the mode-4 exception.

## Status

```text
serialized bit presence          CORPUS_CONFIRMED
source bit -> packet projection  EXE_CONFIRMED
final renderer/material meaning  PRESERVED_UNDECODED
writer policy                    preserve
```

## Next gate

Follow the runtime object `+0x80` packet block from its construction sites into the renderer submission path. The target is the last consumer that interprets the changed `packet+0x00/+0x08` bits, not merely another copy or queue operation.
