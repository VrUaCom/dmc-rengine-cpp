# DMC3 EventTbl corpus reverse — 2026-09-12

## Scope and authority

This pass consolidates every EventTbl payload supplied for the DMC3 HD reverse work as of 2026-09-12.

Two source groups are deliberately kept separate:

- **GData.afs complete EventTbl corpus supplied by the owner:** `EventTbl00.bin` through `EventTbl09.bin`.
- **Additional supplied EventTbl samples:** `EventTbl13.bin` through `EventTbl21.bin`. Their bytes are useful corpus evidence, but this document does not relabel their source as GData.afs.

`EventTbl10.bin`, `EventTbl11.bin`, and `EventTbl12.bin` are not present in the currently supplied byte corpus. Therefore this is a complete analysis of the available 19 files, not a claim of byte-complete `00..21` stock coverage.

Canonical executable authority remains the DMC3 HD executable profile used by this project. Existing reverse evidence binds the runtime paths `eventtbl\\EventTbl00.bin` through `eventtbl\\EventTbl21.bin`, the 22-entry descriptor table, mission selection, loader, scanner, action dispatcher and condition dispatcher. The stock filename extension is therefore `.bin`; `.bin` is generic and is not the content schema. The content family is identified by the `EVT\0` magic and EventTbl grammar.

## Executive result

Across the 19 supplied payloads:

- **12,012 decoded commands**;
- **121 distinct observed opcodes**;
- highest observed opcode: **`0x8C`**;
- maximum observed argument count: **6**;
- every observed opcode has one stable argument count across the entire corpus;
- all command descriptor upper 16 bits are zero;
- all 19 payloads decode to their declared terminal command without desynchronization.

The complete GData.afs subset `00..09` contributes **5,351 commands and 103 distinct opcodes**. The additional `13..21` subset contributes **6,661 commands and 105 distinct opcodes**.

This is enough to treat EventTbl as a real bytecode/event-control format, not as an opaque `.bin` leaf.

## File grammar — structurally confirmed

```text
+0x00  char[4]  "EVT\0"
+0x04  u16 LE   revision            // 1 in supplied corpus
+0x06  u16 LE   stream_count
+0x08  u32 LE   terminal_command_offset (absolute file offset)
+0x0C  0x14     reserved/preserved bytes
+0x20  command stream / first stream root
```

Each command is self-describing:

```text
u32 descriptor
  bits  0..7   opcode
  bits  8..15  argument_count (u32 arguments)
  bits 16..31  zero in the supplied corpus
u32 arguments[argument_count]

serialized_size = 4 + 4 * argument_count
```

The command at `terminal_command_offset` is always `opcode 0x20`, arity 0 in the supplied corpus.

The first stream root is implicit at `0x20`. For `stream_count > 1`, exactly `stream_count - 1` additional `u32` stream-root offsets are serialized immediately after the terminal command. Remaining physical tail bytes are zero padding; supplied payload sizes are `0x20` aligned.

### Multi-stream boundary contract

Every additional stream root in the complete GData.afs `00..09` corpus:

- lands on a decoded command boundary;
- points to `opcode 0x57`, arity 1, with argument 0 at the stream root;
- is immediately preceded by `opcode 0x00`, arity 0.

Single-stream files end with the observed pair `0x01 -> 0x20`; multi-stream files end with `0x0E -> 0x20`.

## Complete GData.afs file census

| File | Size | SHA-256 | Streams | Commands | Terminal | Stream roots |
|---|---:|---|---:|---:|---:|---|
| EventTbl00.bin | 1440 | `2bdfcc56733c0e2867bf54b70031e206758e262d0155374ee2fcd0fc4315fedb` | 1 | 211 | `0x590` | `0x20` |
| EventTbl01.bin | 608 | `cedb5f494267812aac4901c278097330666285a929067f81bf34ec9c471801e9` | 1 | 92 | `0x248` | `0x20` |
| EventTbl02.bin | 1152 | `c338f3673e331887baa6750835a180c36747baf5e73c47a99d5e25cd36f20f41` | 2 | 172 | `0x478` | `0x20, 0x280` |
| EventTbl03.bin | 4256 | `8829123ae479747424593c4a53957fda3d80c11a57462632a3b0a8598d9dd61f` | 2 | 679 | `0x107C` | `0x20, 0x708` |
| EventTbl04.bin | 3840 | `8cbad909cdbb392b8642176021c225b73f0a84aafcd59004ff2d650aec7ec392` | 1 | 625 | `0xEDC` | `0x20` |
| EventTbl05.bin | 7552 | `6055f97bb0948147172349795112c149a4c963656e761b6cfd99edc6351328df` | 2 | 1211 | `0x1D64` | `0x20, 0x2B0` |
| EventTbl06.bin | 6880 | `2dd939f6bcc9018334051f3f8f7a21d28fe30f43f07b88714893357a9a74b01c` | 5 | 1071 | `0x1AB0` | `0x20, 0xD8, 0x308, 0xE7C, 0xF68` |
| EventTbl07.bin | 15584 | `887e145b948516c0f7a48a2752f4ec29556c71bcbcc2e15add341c837e6bcffa` | 6 | 2476 | `0x3CA4` | `0x20, 0x738, 0x958, 0x2898, 0x2C48, 0x2DE0` |
| EventTbl08.bin | 4032 | `6d8704bffef8a94f36703112561a0b2eaedc9cc79e745b77192d498e52f88886` | 1 | 678 | `0xFA4` | `0x20` |
| EventTbl09.bin | 3584 | `3c16c653bccb11b10dd83a9a98d9e56b5bbaa2fcd7e013cc1913324d47eb4b35` | 1 | 577 | `0xDF0` | `0x20` |

## Additional supplied samples

All of these use revision 1 and one stream in the supplied bytes.

| File | Size | SHA-256 | Commands |
|---|---:|---|---:|
| EventTbl13.bin | 2272 | `263787383896ea9ffe7d4c31e723ebcfc8c8f249caf866ff3c9ede1ac09e9ce9` | 246 |
| EventTbl14.bin | 6880 | `2b3a4ad4a4a6a0c7136107874923a502280d2b5a2dc023f10f45fd1fadcf9a22` | 668 |
| EventTbl15.bin | 5760 | `dcc839902a43987a6895402d1984234e495e67bfdeaf6336e323499e99804396` | 597 |
| EventTbl16.bin | 5152 | `db3ae3bd51b27544871d87689d9a2b180fa8f91ad46adb21fa6a3fa779875c1a` | 580 |
| EventTbl17.bin | 4128 | `7cbf67ee022fb3abd2fd81a134543165d71b6c58dad9bd2be627631699f9c46b` | 484 |
| EventTbl18.bin | 6816 | `7f73b7f197c1316194ee7dd01f0e30b02f5a2785656a41d8bb2d0be30543af01` | 758 |
| EventTbl19.bin | 4256 | `f5658942495254780ebd1fe1df01935b08246de3338039541347427a51c7b84e` | 462 |
| EventTbl20.bin | 416 | `3d5bbeb17c582d784df4aec2e2d01887563cff8335957281df72204f3c55ce8b` | 51 |
| EventTbl21.bin | 31968 | `ae5519388153032822c906a63017d84fb8cf72eacf1019faf0044fb60db7ca6a` | 2815 |

## Structural scopes recovered from the corpus

Four begin/end families are count-balanced and well-nested across every supplied payload:

| Begin | End | Begin arity | End arity | Status |
|---:|---:|---:|---:|---|
| `0x07` | `0x08` | 1 | 0 | `CORPUS_STRUCTURAL_CONFIRMED`; `0x07` also has EXE evidence as section/label selector |
| `0x09` | `0x0A` | 1 | 0 | `CORPUS_STRUCTURAL_CONFIRMED` |
| `0x0B` | `0x0C` | 1 | 0 | `CORPUS_STRUCTURAL_CONFIRMED` |
| `0x0D` | `0x0E` | 2 | 0 | `CORPUS_STRUCTURAL_CONFIRMED`; `0x0D` also has EXE subtype/ordinal evidence |

Maximum observed nesting depth is 2. Multi-stream files contain additional `0x0E` closures associated with stream/file boundary structure, so `0x0E` is not assigned a narrower public semantic name yet.

## Event kinds that are currently justified

These are the only semantic families promoted to the canonical API in this pass.

### Structural / flow-control

- `0x00` — stream boundary marker — `CORPUS_STRUCTURAL_CONFIRMED`
- `0x01` — stream end — `EXE_CONFIRMED`
- `0x07` — section selector — `EXE_CONFIRMED`
- `0x08`, `0x09`, `0x0A`, `0x0B`, `0x0C`, `0x0E` — paired scope structure — `CORPUS_STRUCTURAL_CONFIRMED`
- `0x0D` — subtype/ordinal command — `EXE_CONFIRMED`
- `0x20` — file terminal marker — `CORPUS_STRUCTURAL_CONFIRMED`
- `0x57` — timeline/position marker candidate — `CORPUS_SEMANTIC_CANDIDATE`; values are monotonic inside many streams and reset at stream roots. The unit is deliberately not called frames, time, ticks, or seconds without EXE confirmation.

### Controller/event state

- `0x02..0x05` — modify controller return/event state — behavior family `EXE_CONFIRMED`; exact public names remain deliberately generic.
- `0x5E` — writes a global state location near the previously recovered runtime global — `EXE_CONFIRMED`; exact gameplay label remains open.
- `0x5F` — writes controller field `+0x128` — `EXE_CONFIRMED`.
- `0x60` — writes controller field `+0x12C` — `EXE_CONFIRMED`.

### Conditions

- `0x15(itemId)` — acquired/inventory condition with tri-state behavior — `EXE_CONFIRMED`.
- `0x16(itemId, packedContext)` — true when the item resolver returns `-1`; this is **not** safely equivalent to “item collected” — `EXE_CONFIRMED`.

### Inventory mutation

- `0x5C(itemId, amount)` — direct inventory add, bypassing the ordinary cap guard — `EXE_CONFIRMED`.
- `0x5D(itemId, amount)` — direct inventory subtract — `EXE_CONFIRMED`.

### Spawn / object state

- `0x61(itemId, packedContext, x, y, z, flag)` — registered item spawn through the resolver — `EXE_CONFIRMED`.
- `0x62()` — changes active/visible state of the most recently spawned object — `EXE_CONFIRMED`.
- `0x63()` — same recovered object-state family from executable reverse evidence; not observed in the currently supplied 19-file corpus.
- `0x8B(x, y, z, itemId)` — direct item spawn without registry — `EXE_CONFIRMED`.

`0x61` has a recovered fallback path: resolver failure can fall back to a direct spawn using hardcoded item ID 4. It must not be interpreted as an automatic Gold Orb replacement.

## EventTbl21 / Bloody Palace correlation

Independent DMC3 runtime/community-source enums use mission IDs 1..20 for the twenty missions and ID 21 for Bloody Palace. The canonical executable selects among 22 `EventTbl00..21` descriptors by current mission state. Therefore the `EventTbl21 -> Bloody Palace` mapping is **HIGH_CONFIDENCE**, while `EventTbl00` exact purpose remains open.

Within the supplied `EventTbl21.bin`:

- opcode `0x8B` appears **56 times** and appears in no other supplied EventTbl file;
- its item-id argument is split evenly between IDs `0x0C` and `0x0F` (28 each);
- every XYZ triple appears exactly twice, once with each of those two item IDs.

Existing item-system evidence places those IDs in the Green/White orb families. This strongly supports paired direct reward-spawn logic in the special EventTbl21 program, but this document does not invent a more specific reward/room label without runtime confirmation.

## Full observed opcode census

The machine-readable authoritative census for this pass is:

`docs/research/dmc3-eventtbl-opcode-census-2026-09-12.json`

Every observed opcode keeps its fixed arity and count/file-count evidence there. Unknown opcodes remain `PRESERVED_UNDECODED`.

Not observed in the supplied 19 files include `0x23..0x28`, `0x42`, `0x4C`, `0x4F`, `0x52`, `0x54`, `0x5A`, `0x63`, `0x85..0x88`, among other gaps. Absence from this corpus is not evidence that the runtime lacks handlers for them.

## Native Reader display taxonomy — evidence boundary

The desired EventFlow UI can eventually use human categories such as:

- Mission Start / Mission End
- Gameplay Start / Gameplay State
- Cutscene / Camera / Dialogue
- Enemy Spawn / Boss
- Item / Inventory
- Door / Room Transition
- Condition / Flag
- Sound / Effect / Checkpoint

However only categories backed by recovered handler semantics may be emitted as authoritative node labels. `CUTSCENE`, `GAMEPLAY START`, `ENEMY SPAWN`, `BOSS`, `DOOR`, `SOUND`, etc. remain **target taxonomy**, not decoded EventTbl semantics, until their handlers are bound to concrete opcodes.

Unknown nodes must continue to display their raw opcode, arity, arguments, source offset and evidence level.

## Canonical executable reverse points already bound to EventTbl

- first EventTbl path string file offset: `0x004EBCA8`
- descriptor table file offset: `0x005B0020`
- descriptor table VA: `0x1405B1A20`
- mission EventTbl loader: `0x14023A43D`
- general resource loader: `0x1401B8DF0`
- stream initializer/scanner: `0x1401AA500`
- main action dispatcher: `0x1401A6510`
- condition dispatcher: `0x1401A8320`

These addresses are evidence anchors, not permission to assign names to handlers that have not been individually recovered.

## Product integration performed by this pass

The canonical C++ EventTbl layer now exposes an evidence-aware opcode descriptor containing:

- opcode;
- stable display/technical name where justified;
- semantic class;
- evidence level.

Promoted classes are deliberately narrow: `structural`, `condition`, `inventory`, `spawn`, `object_state`, `controller_state`, otherwise `unknown`.

The parser remains lossless: unknown opcode arguments and all preserved byte regions are retained exactly. No EventTbl writer is promoted by this research pass.

## Remaining reverse frontier

1. Acquire stock `EventTbl10.bin`, `EventTbl11.bin`, `EventTbl12.bin` bytes and rerun the same census.
2. Walk the full action dispatcher at `0x1401A6510` and condition dispatcher at `0x1401A8320`; bind every handler target to opcode, arity, state reads/writes and called subsystems.
3. Resolve the `0x57` unit and stream scheduling semantics.
4. Bind concrete opcodes to cutscene, camera, enemy, door/room, sound/effect, checkpoint and mission-transition subsystems only from EXE/xref evidence.
5. Build typed branch/edge semantics before drawing authoritative TRUE/FALSE or jump edges in EventFlowView.
6. Only after lossless no-op round-trip and in-game acceptance should an EventTbl writer/editor be considered.
