# DMC3 light and material pipeline: Gates 17–48

2026-10-29. Target `dmc3.exe` SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`, image base
`0x140000000`. Sources: the Android ARM64 gate packages 17–43 (the user's
`DMC3_Rengine_Reverse_Archive_Gates01-43_2026-10-28.zip`: scripts, tests and
manifests, no game data; not committed) and Gates 44–48, worked out in the
DMC Native Reader (`NR-Luna-v73`, `docs/research/dmc3-gates-archive-v124.md`).
Only addresses, offsets and rules are recorded here.

## Where it lives in Rengine

| What | Where |
| --- | --- |
| Every address below that the image can show (118 checks: gates 0–48 and the effect catalogs) | `tools/reverse/dmc3_gates.cpp` (`dmc3-gates canonical.exe --json out.json`; `DMC_RENGINE_BUILD_REVERSE_TOOLS=ON`), result `data/reverse/dmc3-gates-20261029.json` |
| LIG2 table, candidate rules, top-four selection, Lc/Lv, model rig, stage rig, object masks | `include/dmc_rengine/profiles/dmc3/light_pipeline.hpp`, `src/profiles/dmc3/light_pipeline.cpp`, `tests/light_pipeline_tests.cpp` |
| Clamp register, HD WRAP/CLAMP, nearest flag, alpha control, material byte | `include/dmc_rengine/profiles/dmc3/material_state.hpp`, `tests/material_state_tests.cpp` |
| Vertex formats of `0x1400473F0` (DIFFUSE as four floats) | `include/dmc_rengine/profiles/dmc3/vertex_layouts.hpp`, `tests/vertex_layouts_tests.cpp` |
| LIG2 file framing (0x20 header, 48 × 0x30) | `include/dmc_rengine/formats/lig2.hpp` (existing) |
| Index of all gates 0–48 | `docs/research/dmc3-gates-00-48-2026-10-29.md` |

The light model was first written clean-room for the Native Reader from
behaviour specifications (its `docs/cleanroom/specs-v124/game_lights.md`
revisions 1–4 and `game_material.md`); the code here is that implementation.

## Status of the claims

| Claim | Status |
| --- | --- |
| Selector `0x1402EE560`: lists, mask rules, flag-4 rule, strict-greater entry, bubble on `<=`, single light with mask & 2 | `confirmed` (instructions; Gate 22 rules `corrected` by Gate 46) |
| Score `0x1402EE2A0`: type 0 `max(0, 1 − d²·falloff)`, otherwise 1 | `confirmed` |
| LIG2 (cfg PAC slot 1) → `CLightStatic` (`CSceneGameMain+0x1D80`) → `CLightMgr` (`+0x40`) main `CChain` → selector | `confirmed` (Gate 46 v0.7; the Gate 24 "unproven" is `corrected`) |
| LIG2 header: `+0x04` int8 = static lights kept (3), not a version; `+0x06..+0x0A` ambient for mask 2; `+0x0C..+0x10` ambient | `confirmed` (Gate 47; the "version" reading is `rejected`) |
| Model rig `0x1402FD040` / `0x1402FC850` / `0x1402FA580` | `confirmed` |
| Stage rig `0x1402FD370` / `0x1402FCED0` / `0x1402FA6D0`, K = `0x3B808081` (1/255) | `confirmed` |
| Holder masks: stage constructor `0x140089320` → mask 1, lights on; model constructor `0x140089270` → mask 2 | `confirmed` writers; that these are the drawn holders: `candidate` |
| "Stage objects start with lights off" (`0x140115FF0`) | `rejected` (another class) |
| `+0x940` of the step machine `0x1402AA2B0` is LIG2 | `rejected` (a model; objects 1 and 2 hidden) |
| `0x14031FA80` (mask 0x1C, flag-4 records) is a shadow direction | `hypothesis` |
| Stage ambient upload scale (32 = 1.0) | `hypothesis` |
| Vertex DIFFUSE is four floats (strides of all eight formats) | `confirmed`; the R8G8B8A8_UNORM reading is `rejected` |
| Two engine FVF bits (`(fvf>>16)&3 == 3` adds 8, `0x40000000` adds 4) | `candidate` (strides only) |

## Open

- The RTTI of `0x1404C9010` (model holder) and `0x1404D1E48`.
- The low mask bits of the setters `0x140089940`, `0x140089950`, `0x140089E20`.
- The upload of the stage buffer `B+0xB0..+0x14F` to the GPU, and the
  ambient's unit there.
- The name of shader id 0xD (expected `dmc3_stg`), the `DXGI_FORMAT` of
  `COLOR` in `0x140562780` / `0x140562A80`, and whether the SCM vertex fill
  scales colour bytes by 1/255.

## Material state (Gates 17–19)

`formats/mod.hpp`, `formats/model_mesh_core.hpp` and `formats/scm_render.hpp`
read these fields; `profiles/dmc3/material_state.hpp` turns them into sampler
and alpha state.

| Field | Where | Rule |
| --- | --- | --- |
| Mesh +0x04/+0x06/+0x08/+0x0A | MINU, MAXU, MINV, MAXV | `0x1402F9890`: MINU == 0 → no CLAMP (sentinel); otherwise the GS CLAMP register `(MINU<<4) \| 0x0F \| (MAXU<<14) \| (MINV<<24) \| (MAXV<<34)` (REGION_REPEAT on both axes) |
| HD bridge | `0x14002BC30` | WMS/WMT 1 or 2 → D3D11 CLAMP; 0 or 3 → WRAP. REGION_REPEAT therefore samples as WRAP on PC |
| Object flags & 0x4000 | object source flags | TEX1 = 0: nearest filtering; otherwise TEX1 = 0x60: linear |
| Object +0x01 | `alpha_control` (copied by `0x1403029E0`) | 0x80 → alpha 128/255; above 0x80 → alpha forced to 1 |
| MOD postload | `0x1402FE3B0` | command stream run after load (Gate 17) |

## Lights (Gates 22–24)

- **Selection** `0x1402EE560`: the top 4 records (stride 0x30) by score.
  Main list needs mask & 4, the secondary list mask & 8. Low flag 1 → mask
  & 3, 2 → mask & 2, 3 → mask & 1.
- **Score** `0x1402EE2A0`: type 0 → `max(0, 1 − d² · scale)`; other types 1.
  Record: flags +0x00, colour modifier +0x01, type +0x02, position +0x10,
  aux w +0x1C, scale +0x20, RGB u16 +0x24/+0x26/+0x28. Dot product helper
  `0x140030D30`.
- **Packing** `CDrawSCM` `0x1402FD040`: Lc rows = (r, g, b, scale or 1);
  Lv rows = (light position − query point, aux w, or 0 when type ≠ 0).
  `0x1402FC850` writes both into the model constants from +0x280
  (`MDL_LIGHT_MAT`). SCM +0x13 is the node whose position is the query point.
- **LIG2 as the source** (status at Gate 24: unproven; `corrected` by Gate
  46 v0.6–v0.7 below). Stage cfg PAC slot 1 (0x920 bytes = 0x20 header +
  48 × 0x30) has the record shape above, but no path from it into `CLightMgr` (constructor `0x14023ECB0`, inside `CSceneGame` at +0x4078
  and +0x1D80, switch at `0x1402AA2FE` / `0x1402AAC6C`) has been shown.

## The stage files

| Stage | LIG2 records | Type 0 (point) | Type 1 (far) | With a colour modifier |
| --- | --- | --- | --- | --- |
| st000 | 8 | 5 | 3 | 0 |
| st001 | 9 | 6 | 3 | 3 |
| st002 | 13 | 10 | 3 | 2 |
| st006 | 3 | 0 | 3 | 0 |

- **Light flags.** Type 1 lights have flags 2 (they pass mask & 2); point
  lights mostly have 3 (they pass mask & 1). Lights with flags 4 are black
  and are never picked.
- **Header.** The six u16 after the version look like a colour (st000
  220, 210, 200; st001 95, 95, 95; st002 128, 128, 140), perhaps the
  ambient. (`corrected` by Gate 47 below: the last three are the ambient,
  the first three the ambient used with mask 2.)
- **Object alpha.** MOD objects are all 0x80. SCM objects are 0x80 or
  0xC0..0xC5 (st006: 18 of 20 objects). No object of these files has the
  nearest-filter flag.

## Gates 44–45, checked against our disassembly

Gates 44–45 traced the switch at `0x1402AA2FE`: `lea rcx, [rdi+0x940]`,
then `call 0x140089DE0`, which returns `rcx + 0x80`. Read further:

- **`0x140089DE0` is not a light function.** It is the body
  `lea rax, [rcx+0x80]; ret`, called from 2 695 sites. The linker folded
  every identical "address of the member at +0x80" accessor into it, so
  its callers say nothing about lights.
- **What the branch does.** The function `0x1402AA2B0` is a step machine
  on `this+0x09` (steps 0..12). The branch at `0x1402AA300` calls
  `0x1402F7350(model, 1)` and `0x1402F7350(model, 2)` on the model at
  `this+0x940` (+0x80). Then it advances the step.
- **`0x1402F7350(model, i)`.** If `i < model+0xE8` (u16, object count),
  it clears bit 0 of the first word of object `i`. Objects are
  `model+0x100 + i·0x380`, the MOD runtime objects. It hides object `i`.
  - `0x1402F74E0` sets the same bit (show).
  - `0x1402F73A0` clears `model+0xE0` bit 0 and loops over every object
    (hide all).
- **So the scene's `+0x940` is a model** whose objects 1 and 2 are hidden
  at that step. It is not LIG2. The match with the LIG2 object's `+0x940`
  is a coincidence.

### The LIG2 object itself (constructor `0x14023ECB0`)

| Offset | Content |
| --- | --- |
| +0x00 | vtable `0x1404E3128` |
| +0x08 | `LIG2` |
| +0x28 | the 0x900 bytes of records (48 × 0x30) |
| +0x928 | 48 entries of 0x20 bytes, built by `0x140345B24` (constructor `0x1400F7FF0`, destructor `0x1400F8310`) |

Each entry `k` holds at its `+0x18` (object `+0x940 + 0x20·k`) the
pointer to record `k` (`+0x28 + 0x30·k`).

- **Who builds it.** It is constructed into `this+0x4078` by `0x14023EA50`
  and into `this+0x1D80` by `0x14023EEF0` (two different owners). The
  vtable is also written at `0x14023F616`, `0x14023F75D`, `0x14023F957` and
  `0x14023FCB9`.
- **Next lead for the binding.** Find the code that walks the entries
  (stride 0x20 from `+0x928`, pointer at `+0x18`) or calls through vtable
  `0x1404E3128`. Reaching `0x1402EE560`'s source lists from there would
  prove (or rule out) LIG2 → `CLightMgr`.

## Gate 46: the light lists `0x1402EE560` walks

Gate 46 v0.1 identified the classes from RTTI:

| Vtable | Class |
| --- | --- |
| `0x1404E3128` | `CLightStatic`, the LIG2 object |
| `0x1404E3238` | `CSceneGame` |
| `0x1404E31C8` | `CGameW` |

`CLightStatic` has one slot: `0x14023FCA0`, its scalar deleting
destructor. It destroys the 48 entries at `+0x928` and frees 0xF28 bytes
(the object's size).

### The entries are list nodes

The entry constructor `0x1400F7FF0` builds each 0x20-byte entry as:

| Offset | Content |
| --- | --- |
| +0x00 | vtable `0x1404D02B8` (base `0x1404D02A8`, set by the destructor `0x1400F8310`) |
| +0x08 | a link, set to zero |
| +0x10 | a link, set to zero |
| +0x18 | the record pointer (`CLightStatic` constructor) |

### The selector (`rcx` = manager, `rdx` = query, `r8d` = mask, `r9` = optional seed)

It walks lists of exactly these nodes:

- **Main list.** Only when mask & 4. Head `[mgr+0x20]`; each node's record
  is `[node+0x18]`; next is `[node+0x10]`.
- **Secondary list.** Only when mask & 8. Head `[mgr+0x38]`, with the same
  node layout. The record's flags are taken `& 0xF7` first.

So the nodes the selector walks have the same layout as `CLightStatic`'s
entries: links at +0x08/+0x10, record at +0x18.

**Not proven yet:** the code that links `CLightStatic` entries into
`mgr+0x20` / `+0x38`. That store is the missing step of LIG2 → light lists.

### Corrections to the Gate 22 rules (from `0x1402EE660..0x1402EE6B3`)

- **Flag bit 2 (value 4) is not ignored.** With mask & 0x10, a record whose
  flags have bit 2 is a candidate whatever its low mode. The low mode
  (flags & ~4) can only add to that:
  - 1 → mask & 3;
  - 2 → mask & 2;
  - 3 → mask & 1.

  The LIG2 records with flags 4 (black, low mode 0) are therefore picked
  when mask & 0x10.
- **Ties at the fourth slot.** The output is four 0x30-byte slots at
  `mgr+0x80`, `+0xB0`, `+0xE0` and `+0x110`, score at +0x20. They are seeded
  from `r9` (0xC0 bytes) or zeroed.
  - A new candidate is scored into the last slot only if its score is
    **greater** than the last slot's.
  - It then bubbles up while the slot above scores `<=` it.
  - With more than four equal scores, the **first four in list order** are
    kept, later ones ahead of earlier ones. They are not the highest record
    indices.
  - The list order is the linking order, which is not proven to be the
    record order.
- **Mask & 2 adds the manager's single light.** The record of node
  `[mgr+0x70]` is scored and put in the first slot. The other slots move
  down and the last one is dropped.

### Who calls the selector, and with which list bits

The five calls of `0x1402EE560` add these bits to a mask that comes from
their caller. The caller's bits (1, 2, 0x10) are not traced yet.

| Call site | Function | Mask | Seed (`r9`) |
| --- | --- | --- | --- |
| `0x1402FC7CD` | `0x1402FC720` | caller's \| 4 | none |
| `0x1402FD140` | `0x1402FD040` (`CDrawSCM`) | caller's \| 4 \| 8 | none |
| `0x1402FD485` | `0x1402FD370` | caller's \| 8 | object + 0x300 (the four slots kept from before) |
| `0x1402FD4AD` | `0x1402FD370` | caller's \| 8 \| 4 | none |
| `0x14031FB02` | `0x14031FA80` | 0x1C (4 \| 8 \| 0x10) | none |

- **`0x14031FA80`.** It asks only for flag-4 lights (low bits 0, bit
  0x10). In LIG2 those are the black records (st000 `(1000, 3000, −5000)`,
  st001 `(5000, 2500, 0)`). A light direction with no colour suggests a
  shadow direction. This is not proven: the class of `0x14031FA80` is not
  identified.
- **`CDrawSCM`** walks both lists.

### Gate 46 v0.5: `0x14022D710 → 0x140058F40` is not LIG2

- **`0x140058F40` is not a LIG2 consumer.** It is a 0x58-byte initializer
  with no loops and no calls:
  - it sets `+0xC8 = 1`;
  - it zeroes `+0xBC`, `+0xD8`, `+0xE0`, `+0xE8`, `+0xF4`, `+0x100`,
    `+0x108` and `+0x10C`;
  - it writes `1.0f` to `+0xEC` and `+0xFC`;
  - it returns `rcx`.

  It has 17 callers. Its neighbours `0x140058FA0` and `0x140058FB0` read and
  clear bytes of five-channel tables at `+0x108`. The motion script
  interpreter `0x140058FE0` follows them. So it initializes a motion-side
  object.
- **`0x14022D710` is a constructor of another class.** It calls the base
  constructor `0x140227650`, then writes three vtables:
  - `0x1404E1970` at `+0x00`;
  - `0x1404E19F0` at `+0x60`;
  - `0x1404E1AE8` at `+0xD0`.

  Its `+0x1D80` member is what `0x140058F40` initializes.
- **The class that holds LIG2 at `+0x1D80`** is the one built by
  `0x14023EEF0`. That constructor calls `0x14023ECB0` on `this+0x1D80`, and
  writes:
  - vtable `0x1404E32C8` at `+0x00`;
  - `0x1404E3118` at `+0x40`;
  - `0x1404E3108` at `+0x58`;
  - the same value at `+0x70` (`+0xC90` gets 0, not a vtable).

  Its other members get vtables `0x1404E3268`, `0x1404E3280` and
  `0x1404E3298`.
- **So the same offset `+0x1D80` belongs to different classes.** An owner
  is proven by the constructor (vtable `0x1404E32C8`), not by the offset.
  The methods of owner B are the slots of vtable `0x1404E32C8`, which can be
  read from `.rdata` with its RTTI name.

### Owner B holds two lists laid out like the selector's

The rest of constructor `0x14023EEF0` (`0x14023EF29..0x14023EFD0`):

| Owner B | Written | Same place relative to `+0x40` |
| --- | --- | --- |
| +0x40 | vtable `0x1404E3118` | +0x00 |
| +0x58 | vtable `0x1404E3108` | +0x18 |
| +0x60, +0x68 | 0 | +0x20, +0x28 |
| +0x70 | vtable `0x1404E3108` | +0x30 |
| +0x78, +0x80 | 0 | +0x38, +0x40 |
| +0xB0 | 0 | +0x70 |

- **Two lists of 0x18 bytes.** `+0x58` and `+0x70` each hold a vtable and
  two pointers (first, last), 0x18 apart.
- **They line up with the selector.** Taking `mgr = owner B + 0x40`:
  - main list `[mgr+0x20]` is the first-pointer of the list at `+0x58`;
  - secondary list `[mgr+0x38]` is the first-pointer of the list at
    `+0x70`;
  - the single light `[mgr+0x70]` is owner `+0xB0`, which is zeroed too.

  The subobject at `+0x40` has its own vtable, `0x1404E3118`.
- **Hypothesis.** The light manager (or its base) is the subobject of
  owner B at `+0x40`, and the same owner holds `CLightStatic` at `+0x1D80`.
  This is a structural match only, not proven.
- **What would prove it:**
  - the RTTI of `0x1404E3118`;
  - a call of `0x1402EE560` whose `rcx` is owner B `+ 0x40`;
  - a store of an entry address (owner `+0x26A8 + 0x20·k`) into
    `+0x60`/`+0x68`/`+0x78`/`+0x80` or into another node's `+0x08`/`+0x10`.

## LIG2 → CLightMgr: the path, instruction by instruction

Gate 46 v0.6 read the RTTI from `.rdata`:

| Vtable | Class |
| --- | --- |
| `0x1404E32C8` | `CSceneGameMain` (owner B) |
| `0x1404E3118` | `CLightMgr` (at owner `+0x40`) |
| `0x1404E3108` | `CChain` (at owner `+0x58` and `+0x70`, i.e. `CLightMgr+0x18` and `+0x30`) |

With those names, the disassembly gives the whole path.

### 1. Copying the LIG2 member (in `0x14023C3D0`, `rdi` = `CSceneGameMain`)

- **The source.** It is a PAC in memory: `rcx` = `[[r13+0x18]+0x20]`.
  - With slot count `[pac+4] >= 2`, the source is `pac + [pac+0x0C]`,
    i.e. **physical slot 1** of the stage configuration archive.
  - Slot 0 (`pac + [pac+0x08]`) goes to `0x1401A9B80` on `this+0x1F0` just
    before.
- **The copy.** `0x14023CA15..0x14023CA82` copies `0x12 × 0x80 + 0x20` =
  **0x920 bytes** to `this+0x1D88`, which is `CLightStatic+0x08`.
  - The file's `LIG2` lands on the magic field.
  - Its 0x20-byte header follows.
  - Its 48 records land at `CLightStatic+0x28`, where the entries' record
    pointers point.

### 2. Linking the records (`0x14023CA77..0x14023CA8D`)

```
lea  rdx, [rdi+0x40]      ; CLightMgr
lea  rcx, [rdi+0x1D80]    ; CLightStatic
call 0x140089000
```

### 3. `0x140089000(CLightStatic* ls, CLightMgr* mgr)`

```
lea  rbp, [rdx+0x18]      ; mgr's first CChain
call 0x1402C83A0          ; CChain::Clear(rbp)
lea  rbx, [rdi+0x940]     ; &entry[0].record
mov  esi, 0x30            ; 48 entries
add  rdi, 0x928           ; &entry[0]
loop:
  mov  rax, [rbx]         ; record
  cmp  byte [rax], 0      ; flags == 0: skip
  je   next
  mov  rdx, rdi
  mov  rcx, rbp
  call 0x1402C8310        ; CChain::Append(chain, entry)
next:
  add  rdi, 0x20 ; add rbx, 0x20 ; dec esi ; jne loop
```

### 4. The `CChain` methods (`CChain`: vtable, `+0x08` first, `+0x10` last; node: `+0x08` prev, `+0x10` next)

| Method | What it does |
| --- | --- |
| `0x1402C8310` (Append) | Empty chain: first = last = node, links 0. Otherwise `last->next = node`, `node->prev = last`, `node->next = 0`, last = node. |
| `0x1402C8350` (Remove) | Unlinks a node. |
| `0x1402C83A0` (Clear) | Zeroes every node's links, then first and last. |

### 5. The selector

`0x1402EE560` reads the main list from `[mgr+0x20]`. That is the first
pointer of the chain at `mgr+0x18`, the chain filled above. It follows
`node+0x10` and reads `node+0x18`.

### Result

**LIG2 (cfg PAC slot 1) → `CLightStatic` at `CSceneGameMain+0x1D80` →
the main list of `CLightMgr` at `CSceneGameMain+0x40` → `0x1402EE560`.**

This is proven by instructions, with one step left open: that `rdi` in
`0x14023C3D0` is a `CSceneGameMain`. Its offsets `+0x40` / `+0x1D80` match
that class's constructor.

What follows for a renderer:
- **The list.** LIG2 lights are the **main list** (mask bit 4).
- **The order.** The list order is the record order (appended k = 0..47).
- **Skipped records.** Records with flags byte 0 are not linked. They are
  never candidates anyway.
- **The secondary list.** It holds lights registered at run time by
  `0x1402EE9F0`:
  - a node at `obj+0xD8`;
  - flags & 8 also makes it the manager's single light (`mgr+0x70`);
  - flags & 0xF7 appends it to the chain at `mgr+0x30`.

Still not proven:
- the low mask bits of the scene and model calls;
- how Lc/Lv become the shader constants;
- the model's query point.

### `this` of `0x14023C3D0`

- **Inside the function.** The only write to `rdi` before the link call is
  `0x14023C3F5 mov rdi, rcx` (after `movzx eax, [rcx+0x24]`; the function
  steps on `this+0x24`, 0..10). `rdi` is callee-saved, so it is still the
  entry `rcx` at `0x14023CA77`.
- **The caller.** The only direct caller is `0x14023B427` in `0x14023B3E0`:

  ```
  0x14023B3ED  mov rbx, rcx
  ...          switch on [rbx+0x20] (0..12, table at image+0x23B500)
  0x14023B424  mov rcx, rbx
  0x14023B427  call 0x14023C3D0
  ```

  `0x14023B3E0` has no direct callers, so it is reached through a vtable.
- **The last step.** If `0x14023B3E0` is a slot of `CSceneGameMain`'s
  vtable `0x1404E32C8`, then `rdi` is a `CSceneGameMain` and the path is
  proven end to end.

### Closed (Gate 46 v0.7, checked on the canonical EXE)

- **The vtable slot.** `CSceneGameMain` vtable `0x1404E32C8`, slot 3
  (`0x1404E32E0`), is `0x14023B3E0`.
- **The jump table.** Its jump table at image `+0x23B500` has 13 entries.
  Case 0 goes to `0x14023B424`, which calls `0x14023C3D0` with its own
  `this`.

The path **LIG2 → `CLightStatic` → `CLightMgr` main `CChain` →
`0x1402EE560`** is proven by static instructions, from the
`CSceneGameMain` virtual method to the selector.

Open:
- the low mask bits of each call;
- the model's query point;
- the Lc/Lv packing into GPU constants (with the colour scale).

## Gate 47: from the four selected lights to the model's light constants

### `0x1402FD040(model, mgr, mask)` (the archive's `CDrawSCM`)

- **The query point.** Take `m = 0x1403025E0(model)`, or `model` itself
  when that returns null. The point is the translation row (`+0x30`) of
  matrix `[m+0x188] + 0x40·[m+0xFA]`, i.e. the world position of node
  `[m+0xFA]` (u16).
- **The selection.** `0x1402EE560(mgr, &query, mask|4|8, null)`: both lists.
- **Lv and Lc.** Lv = the four slots' `+0x00` rows (vector), Lc = their
  `+0x10` rows (colour). The function calls `0x1402FC850(model, Lc, Lv)`.
- **The ambient.** It calls `0x1402FA580(model, mgr+0x60)` when the caller's
  mask is exactly 2, otherwise `0x1402FA580(model, mgr+0x50)`.

### `0x1402FC850(model, Lc, Lv)`: the constants at `model + 0x280 + 0x80·k`

`k` is `[[0x140D6D300]+0x0E]`, a frame buffer index. The block is two
4×4 matrices: colours `C[0..3]`, then directions `D[0..3]` at `+0x40`.

1. `C[i] = Lc[i]` for i = 0..2. Only three lights; slot 3 is not used.
2. `D[0..3] = Lv[0..3]` (`0x140030D00`, a 4×4 copy).
3. For i = 0..2:
   - if `D[i].w != 0`: `a = max(0, 1 − dot3(D[i], D[i]) · D[i].w)` and
     `C[i] *= a`. `dot3` is `0x140030D30`. `D[i].w` is the record's `+0x1C`
     (aux) for type 0, and 0 for type 1;
   - then `C[i].w = 0`.
4. `D[i].w = 0` for i = 0..2. Then `D[i] = normalize(D[i])` (`0x140330390`,
   with a zero-length guard).
5. `D[3] = (0, 0, 0, 1)`.
6. `D = transpose(D)` (`0x140031240`).

### `0x1402FA580(model, ambient)`

`C[3] = *ambient`, and `D` row 3 = (0, 0, 0, 1) again.

### Where the ambient comes from (`0x14023CA92..0x14023CB28`, after the link)

| `CLightMgr` field | Source in the LIG2 file |
| --- | --- |
| `+0x10` (int) | `(int8) file[+0x04]` |
| `+0x50` (rgb, w 0) | floats of the u16 at `+0x0C`, `+0x0E`, `+0x10` |
| `+0x60` (rgb, w 0) | floats of the u16 at `+0x06`, `+0x08`, `+0x0A` |

The last three header words are the **ambient**: st000 220, 210, 200;
st001 95, 95, 95; st002 128, 128, 140; st006 120, 132, 138. The first
three (all 0x20, or 0x18 in st006) are the ambient used with mask 2.

### The static lights of an object (`0x1402FC720`)

- **Selection.** It zeroes the object's four slots at `+0x300` and selects
  from the main list (mask | 4) at a matrix's translation.
- **Kept slots.** It copies the first `[mgr+0x10]` slots there. That is
  file `+0x04`, **3** in every sample. It is the number of static lights
  kept, not a version.
- **Per frame.** `0x1402FD370` reuses `+0x300` as the seed with
  mask | 8: the dynamic lights are added to the static ones each frame.

### So the model's light rig (`model_lit`, 128 = 1) is

| Part | Value |
| --- | --- |
| Lights 0..2 | direction `normalize(light − query)`; colour `rgb × max(0, 1 − d²·aux)` (×1 when aux = 0, which is every type 0 record in the samples). The score (+0x20) only ranks; it does not dim the colour. |
| Constant term | the LIG2 ambient (header `+0x0C..+0x10`), in the same 0..255 units. |
| Lights kept | at most 3, and at most file `+0x04` for static lights. |

Still open:
- the low mask bits;
- whether node `[m+0xFA]` matches the Native Reader's model centre;
- how the stage shader's point lights (`u_light_x/y/z`, falloff) are
  filled: `0x1402FC850` writes directions, not positions.

## Gate 48: the stage path (`stage_lit`'s point lights)

### The callers and their masks (the masks are fields of the drawn object)

| Function | Does | Arguments |
| --- | --- | --- |
| `0x140089900` (holder `X`) | sets `X+0x573 = 1` | jumps to `0x1402FC670(X+0x50, [0x140C90E38], mask = byte [X+0x572])` |
| `0x14008B5D0` | unless `X+0x570`, runs every frame | `0x1402FD2B0(X+0x50, [0x140C90E38], seeded = ([X+0x573] == 1), mask = byte [X+0x572])` |
| `0x14008B500` (holder `Y`, models) | unless `Y+0x740` | `0x1402FD040(Y+0x80, [0x140C90E38], mask = dword [Y+0x744])` |

`[0x140C90E38]` is the global `CLightMgr` pointer. `0x1402D58B0` also
writes its `+0x70`, the single light.

### `0x1402FC670` / `0x1402FD2B0`: one call per object of the model

- **The objects.** Count `[m+0xE8]` (u16), records of 0x3C0 bytes at
  `[m+0x100]`. Each object's node is `byte [obj+0x0E]`, and its matrix is
  `[m+0x188] + 0x40·node`.
- **Static lights.** `0x1402FC670` calls `0x1402FC720(obj, mgr, matrix,
  mask)` once. That stores the static lights at `obj+0x300` (Gate 48 v0.1).
- **Every frame.** `0x1402FD2B0` calls `0x1402FD370(obj, mgr, matrix,
  !seeded, mask)`.

### `0x1402FD370(obj, mgr, matrix, full, mask)`

1. **Query.** `q` = the matrix's translation, with w = 1.
2. **Inverse rotation.** `M = transpose(matrix)` (`0x140031240`), then
   `M[0..2].w = 0`: the rotation's inverse, with no translation.
3. **Selection.**
   - seeded (`full == 0`): `0x1402EE560(mgr, q, mask|8, seed = obj+0x300)`,
     i.e. the static lights plus the dynamic list;
   - otherwise: `0x1402EE560(mgr, q, mask|8|4, null)`.
4. **Light positions.** `Lv` = the four slots' `+0x00` rows, transformed by
   `M` (`0x140030E40`). This gives the light positions in the object's
   **local** space, relative to the node's origin.
5. **Colours.** `Lc` = the slots' `+0x10` rows.
6. **Packing.** `0x1402FCED0(obj, Lc, Lv_local)`.
7. **Ambient.** `0x1402FA6D0(obj, mgr+0x50)` when `mask == 1` exactly,
   otherwise `0x1402FA6D0(obj, mgr+0x60)`. This is the opposite test from
   the models, where `mask == 2` picks `+0x60`.

### `0x1402FCED0(obj, Lc, Lv)`: the buffer `B = [obj + 0x50 + 8·k]`

- **Colours.** `B+0xB0..+0xEF`: the four `Lc` rows, each through
  `0x1400311E0(row, row, K)`:
  - xyz are multiplied by `K = float [0x14035D558]`;
  - w is kept, by the `andps` mask at `0x1405D9F30`.
  - **K is in `.rdata`; not read yet.**
- **Positions.** `B+0xF0..+0x12F`: the four `Lv` rows, then transposed.
  The rows become the shader's `u_light_x`, `u_light_y`, `u_light_z` and
  `u_light_falloff`.
- **All four lights are used** (the model uses three).
- **Falloff = `Lv.w`.** That is the record's `+0x1C` (aux) for type 0, and
  0 for type 1. It is not the `+0x20` falloff the selector ranks with.

### `0x1402FA6D0(obj, ambient)`

`B+0x140 = *ambient`, copied as is: no K, no scaling.

### Open

- **The value of `K`** (`[0x14035D558]`, 31 references) and the mask at
  `0x1405D9F30`.
- **The ambient's units in the stage shader.** It is copied raw (0..255),
  while the colours get K.
- **Who sets the object masks** `X+0x572` and `Y+0x744`.

### Gate 48 v0.2 results (canonical EXE) and what they imply

- **The tail call.** `0x14008991A jmp 0x1402FC670` is confirmed: it is a
  tail call, not a `call`.
- **K.** `float [0x14035D558]` = `0x3B808081` ≈ **1/255**. The stage
  colours are the record's rgb / 255.
- **The `andps` mask.** `[0x1405D9F30]` (`.data`, filled by `0x140001920`
  from `.rdata` `0x14035D340`) is `(0, 0, 0, 0xFFFFFFFF)`. `0x1400311E0`
  therefore scales xyz and keeps w.

What this means for `stage_lit`, with the Native Reader's SCM normals (they are
unit length on st000 and st006):

- **Facing is not normalised.** The shader's facing `dot(light − vertex,
  normal)` has the light's distance in world units, i.e. hundreds.
- **A point light would saturate.** With aux (`+0x1C`) = 0, as for every
  st000 point light, it is not attenuated. It would add about
  `distance × rgb / 255` and turn white.
- **So stage objects probably ask for no point lights.** In the game the
  stage geometry likely uses a mask (`X+0x572`) without the low bits 1/2,
  so the main list gives it none. Its picture is then the vertex colours
  times the ambient, as the Native Reader draws it without its Stage lights setting. This is
  a hypothesis: the writers of `X+0x572` are not found yet.
- **A unit for the ambient.** The stage ambient is `mgr+0x60` (header
  `+0x06..+0x0A`) for every mask but 1. It is 0x20 = 32 in st000..st002
  and 0x18 = 24 in st006. If the GPU upload uses 32 as 1.0, that gives
  1.0, the neutral value the Native Reader set by eye (255/256), and 0.75 in
  st006. Not proven: the upload of `B+0xB0..+0x14F` to the GPU is not
  traced.

### Gate 48 v0.3 leads: who writes the masks

| Field | Writer | Value |
| --- | --- | --- |
| `Y+0x744` (model mask) | `0x140089270`, a constructor (vtables `0x1404C9010`, `0x1404C91F0`; 8+ callers) | **2**, with `Y+0x740 = 0` (lights on) |
| `Y+0x744` | `0x1401B09F0` | `r12d` (r12 serves as zero there) |
| `Y+0x744` | `0x140089940` | `edx` (a setter with no direct callers, likely a vtable slot) |
| `X+0x572` (stage mask) | `0x140089950` | `dl` (a setter with no direct callers) |
| `+0x570..+0x573` | `0x140115FF0`, a constructor (vtables `0x1404D1E48`, `0x1404D1E90`, `0x1404D1F88`) | `0xFF` each |

What follows, if these owners are the drawn holders (their RTTI is still
to be read):

- **A model holder built by `0x140089270`** draws with mask 2. Its
  selection is then `2|4|8`:
  - the main and secondary lists are both walked;
  - low mode 1 and 2 (directional, type 1) pass, low mode 3 (the point
    lights) does not;
  - the ambient is `mgr+0x60` (header `+0x06..+0x0A`, 32 32 32), because
    the mask is exactly 2.
- **The holder built by `0x140115FF0`** starts with `+0x570 = 0xFF`.
  `0x14008B5D0` skips the stage light update while that byte is non-zero,
  so such objects get no per-frame lights until something clears it.

### Correction: the stage holder's own constructor (`0x140089320`)

`+0x570` is written by many unrelated classes. The ones in the light
holder's own code (`0x1400892xx..0x14008Bxxx`, next to `0x140089900`,
`0x14008B5D0` and the mask setters) are:

| Writer | Effect |
| --- | --- |
| `0x140089320` (constructor, 22 callers), `0x14008938E` | `dword [+0x570] = 0x10000`: `+0x570 = 0` (lights on), `+0x571 = 0`, **`+0x572 = 1`** (mask), `+0x573 = 0` (static lights not built) |
| `0x140089E20` | `byte [+0x570] = dl` (setter) |
| `0x140089F20`, `0x140089FB0` | `byte [+0x570] = 1` (lights off) |

- **Stage objects start with mask 1 and lights on**, if `0x140089320` is
  the constructor of the drawn holder X:
  - the static selection uses `1|4`, so point lights (low mode 3) and low
    mode 1 pass, and directional ones (2) do not;
  - the per-frame selection adds the dynamic list (`|8`);
  - the ambient is `mgr+0x50` (header `+0x0C..+0x10`, e.g. 220 210 200),
    because the mask is exactly 1.
- **`0x140115FF0`** (`+0x570..+0x573 = 0xFF`) is probably another class
  with its own fields at those offsets. The "lights off by default" reading
  above is withdrawn.
- **So hypothesis A (no point lights on the stage) is unlikely.** The open
  question moves to the scale: with unit normals, aux = 0 and K = 1/255,
  the `stage_lit` formula as specified would saturate. Either the GPU
  upload rescales, or the specification's "not normalised" needs
  rechecking against the shader.

### Gate 48 v0.3d leads: the vertex formats (`0x1400473F0`)

`0x1400473F0` builds the engine's vertex formats. For each format it:
- takes a shader record from the table at `0x140C0CFA0` with
  `0x1400461B0(table, id)`;
- creates two input layouts with `ID3D11Device::CreateInputLayout` (slot
  `+0x58`) from static `D3D11_INPUT_ELEMENT_DESC` arrays in `.rdata`, with
  the element count at record `+0x10`;
- stores a D3D9-style FVF code (`+0x28`) and the stride (`+0x30`).

| Shader id | Layouts (`.rdata`) | FVF code | Stride |
| --- | --- | --- | --- |
| 5 | `0x140562620`, `0x140562920` | `0x80030112` | 0x28 |
| 8 | `0x1405626C0`, `0x1405629C0` | `0x80030152` | 0x38 |
| 0xD | `0x140562780`, `0x140562A80` | `0x80000152` | 0x30 |
| 0xF | `0x140562800` (both) | `0x2` | 0x0C |
| 0xE | `0x140562820` (both) | `0x40000002` | 0x10 |
| 4 | `0x140562880`, `0x140562B40` | `0x102` | 0x14 |
| 1 | `0x1405628C0`, `0x140562B80` | `0x142` | 0x24 |
| (next) | `0x140562840`, `0x140562B00` | `0x42` | 0x1C |

The strides fit the FVF bits only if `DIFFUSE` (0x40) takes 16 bytes:
- `0x42` (XYZ, DIFFUSE) is 12 + 16 = 28;
- `0x142` adds TEX1: 12 + 16 + 8 = 36;
- `0x80000152` (XYZ, NORMAL, DIFFUSE, TEX1) is 12 + 12 + 16 + 8 = 48;
- `0x80030152` adds 8 bytes more: 56.

So in the HD vertex buffers the colour is **four floats**, not four
bytes, and the `R8G8B8A8_UNORM` reading above does not hold. Whether the
floats are 0..255 or 0..1 is decided by the code that fills the buffer,
not by the format.

To check:
- the `DXGI_FORMAT` of `COLOR` (`+0x0C` of its element) in
  `0x140562780` / `0x140562A80`;
- which shader name id 0xD is (expected `dmc3_stg`);
- the SCM vertex fill (colour bytes → floats, and whether ×1/255 is
  applied).

The legacy GS vertex emitter `0x14002BAD0` is another path. It writes
`DIFFUSE` as a packed dword and the fog byte as ×1/255.
