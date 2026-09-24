# DMC3 effect bank loader (`*_effect` PNST) — EXE read site

Date: 2026-09-24
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

Samples (analysed only, not committed; SHA-256 values in
`dmc3-enemy-motion-script-2026-09-24.md`):
- em028.pac slot 9;
- em000, em006 and em007 slot 41;
- plwp_sword.pac slot 2.

Until now `EffectPackContract::manifest_read_site_found` was `false` (passes
#254 and 2026-09-10/11). This note gives the read site.

## 1. Loader `0x1402C04C0(bank, mode)` — EXE_CONFIRMED

```text
text    = bank + u32[bank+8]           (slot 0, the manifest)
records = bank + u32[bank+0xC]         (slot 1, a PNST)
tokenizer 0x140322CB0 / 0x140322CA0 on the text (the .tsc tokenizer 0x140322AB0)
mode 0: 0x140331180(..., 0x10, 0x20) first; modes 1–4 skip it
loop while 0x1402EF450(tok) == 0:
    kind = first char of token (0x140322AB0)
    if kind == '#': stop                      ("# End")
    id   = next token as int (0x140322A60)
    rec  = records slot k (null when k >= count)
    switch kind - 'A' (table 0x1405C0764, 22 entries):
      A -> 0x140322990(g 0x140CEFFD0, rec, id)
      C -> 0x1402D3BE0(g 0x140CAA190, rec, id)
      E -> 0x1402E87A0(g 0x140CAB230, rec, id)
      G -> 0x1402ECBD0(g 0x140CAE7D0, rec, id)
      M -> 0x1402E35D0(g 0x140CAA840, id, rec, slot k+1)   ; two slots
      P -> 0x140314B80(g 0x140CB9ED0, rec, id)
      T -> 0x140322F20(g 0x140CF0AA0, rec, id, 0, mode==0)
      V -> 0x140325030(g 0x140CF1270, rec, id)
      other letters: no registrar, one slot consumed
    k += 1
mode 0: 0x14024EA30(...) afterwards
```

This is where the "unnamed companion" records come from:
- An `M` line consumes the model and the slot after it: the 16-byte `0x31`
  record.
- Counting that way matches the manifests exactly:

| Archive | Lines + `M` companions | Slots |
| --- | --- | --- |
| em028 | 157 + 14 | 171 |
| em000 | 173 + 13 | 186 |
| plwp_sword | 99 + 9 | 108 |
| em006 | 94 + 6 | 100 |

Callers seen so far:
- em028 init `0x1401306BC`: slot 9, mode 2;
- em000 family `0x140097BC0`: slot 41, mode 2.

## 2. Kinds

The registrars keep id → record tables with bounded capacities:
- `M`: 0x48;
- `P`: 0x300;
- `E`, `G`, `V`: 16-byte entries.

What the payloads are:

| Kind | Size | Content | Evidence |
| --- | --- | --- | --- |
| **T** | 22 112 / 87 648 B | 112-byte texture descriptor (w/h at +0x10) + a DDS (DXT5 with mips) | DATA_CONFIRMED; the registrar calls `0x1403368F0` (upload) |
| **M** | variable | a MOD or EFM document, plus the 16-byte companion | DATA_CONFIRMED |
| **A** | 336 B | `[1, texture id, frame time, last frame index, loop, 0]` then 10-byte frames `u16 x, y, w, h, 0` in texture pixels | DATA_CONFIRMED |
| **P** | 336 / 528 / 704 / 896 B | particle definitions; hold names such as `ee028-70p0` | DATA_CONFIRMED |
| **E** | 544 B | starts with type 9 and a subtype | registrar near `CEffectClip` / `CFadeControl` |
| **G** | 96 B | | registrar near `CGenerator` |
| **V** | 368 B | | registrar near `CValue` / `CWorkRateClip` |
| **C** | | not in the samples | |

Examples for the data-confirmed kinds:
- **T:** em028 T072 is the atlas of Nevan's bats and lightning (256²); the
  128² textures are the others.
- **A:** em028 A9 plays 4 frames of 64×64 from T072 (the bats flapping), and
  A76 plays 4 lightning frames from T080. Some A records name textures that
  are not in their own bank (em028 A96 → T083). These are probably shared
  banks.

For E, G and V, the class names come only from vtables near the registrar
addresses. That is a proximity hint, not a read.

## 3. Viewer rule (Native Reader)

- A PNST whose slot 0 is a manifest is family `FXBANK`.
- Its children are the named records, `<kind><id>`:
  - T → `.dds`, opened as a texture;
  - M → `.mod` / `.efm`, opened in 3D;
  - A → a sprite view over its texture when the bank holds it;
  - the rest → the raw binary view.

## 4. Open

- The layouts of E, P, G, V and C, and the runtime that spawns them (so that
  effects can be played in the scene).
- Which archive holds the textures that A records name but their bank does
  not.
