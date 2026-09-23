# DMC3 .tsc texture UV scroll (CDrawUV)

Date: 2026-09-23
Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
Samples (analysed only, not committed): `em028_013.tsc` (em028.pac slot 13),
`em000_024.tsc` (em000.pac slot 24).
Contract: `include/dmc_rengine/profiles/dmc3/tsc_uv_scroll_contract.hpp`,
test `tests/tsc_uv_scroll_contract_tests.cpp`.
This corrects `dmc3-em028-nevan-assembly-2026-09-23.md`, which described slot
13 as a "small BIN for the dress cloth": slot 13 is this scroll script.

## 1. Text

```
.TSC
	# RELATIVE
		<Start
			ScrlNo		0
			ScrlType	1
			TexNo		3
			DirUV		stay,   up
			TimeUV		0, 	90
		End>
		...
		<Finish>
$
	# ABSOLUTE
```

The tokenizer `0x140322AB0` splits on NUL, tab, LF, CR, space and `,`. A `;`
skips to the end of the line, and `$` ends the text. The ABSOLUTE part after
`$` is therefore never read.

The parser `0x14030A9B0` works as a small state machine:

- The first token must be `.TSC`.
- In state 0, `#` moves to state 1.
- In state 1, `RELATIVE` moves to state 100 and `ABSOLUTE` moves to state 200.
  State 200 has no handler, so everything after it is skipped.
- In state 100, `<Start` calls the block parser `0x14030ABE0` and `<Finish>`
  returns to state 0.

A block must start with `ScrlNo n` and then `ScrlType t`. Other keys follow
until `End>`, and unknown keys are ignored. `ScrlType 10` also sets bit 2 of
model `+0x70`.

## 2. Record (0x80 bytes, `model+0x80` + ScrlNo * 0x80)

The loader `0x14030B5C0` takes the record count from MOD header byte `+0x18`
and clears every record's flag word before parsing.

| Offset | Content |
| --- | --- |
| `+0x00` | Flags. `0x80000000` means the record is live, 4 means MinimumUV, 2 means RndUV. |
| `+0x04`, `+0x06` | ScrlNo, ScrlType. |
| `+0x07` | JntNo. It defaults to MOD `+0xFA` and is clamped to the node count. |
| `+0x08` | TexNo. -1 means every mesh of the object. |
| `+0x0C`, `+0x0E` | Output u, v as u16, in units of 1/4096 texture. |
| `+0x10`, `+0x18` | Phase u, v. `+0x14` and `+0x1C` are the MinimumUV phase. |
| `+0x20`, `+0x28` | RateUV (u, v). |
| `+0x24`, `+0x2C` | MinimumUV (u, v). |
| `+0x30`, `+0x38` | TimeUV (u, v). The default is 1. |
| `+0x34`, `+0x3C` | TurnTimeUV (u, v). The default is 1. |
| `+0x40`, `+0x48` | InterUV countdown. |
| `+0x50`, `+0x54` | InterUV reload values. |
| `+0x58`, `+0x5A` | DirUV as i16: u is 1 for left and -1 for right; v is 1 for up and -1 for down; stay is 0. |
| `+0x60` | Model pointer. |
| `+0x68`..`+0x74` | RndUV. |
| `+0x78` | dt, copied from model `+0x240`. |

## 3. Per frame

`CDrawUV` update (`0x14008BEE0`) stores dt at model `+0x240` and calls
`0x14030BDD0`. For every record, `0x14030C1C0` skips records that are not live,
and returns early when dt ≤ 0. Otherwise it jumps through the table at
`0x14030C2A0` by ScrlType.

**Step and rate by type.** Each axis has its own u or v worker:

| Type | Rate per step | Shape | Workers |
| --- | --- | --- | --- |
| 0 | RateUV | linear | `0x14030C2D0` / `0x14030C4F0` |
| 1 | `dt / TimeUV` | linear | `0x14030C710` / `0x14030C7B0`, then the type-0 workers |
| 2 | RateUV | eased | `0x14030C850` / `0x14030CC30` |
| 3 | `dt / TimeUV` | eased | `0x14030B780`, then the type-2 workers |

- Types 1 to 3 stop with an offset of 0 when TimeUV is 0 or DirUV is `stay`.
- The InterUV counter drops by dt each frame. A step happens when it reaches
  0 or below, and the counter then reloads to InterUV.
- Each step does `phase += dir · rate`, wrapped to [0, 1).

**Output.**
- Linear types output the phase directly.
- Eased types output `(cos((1 − phase)·π) + 1)·0.5`. The constants are π at
  `0x140371918` and 0.5 at `0x14035D52C`. When the MinimumUV flag is set, a
  second phase that grows by `dir · MinimumUV` per step is added.
- The result is stored as `value · 4096`.

**Types 4, 5 and 10** (`0x14030B820`, `0x14030B980`, `0x14030BB50`) use
TurnTimeUV and are not decoded yet.

**RndUV.** When the RndUV flag is set, `rand · (b − a) + a` is added
(`0x140059390`).

## 4. Applying the offset

`0x14030BDD0` walks the MOD's runtime objects: records of 0x380 bytes at
`+0x100`, with the count at `+0xE8`. It only visits objects that have runtime
flag `0x8000` and a scroll index at byte `+0x0D`.

The MOD loader sets both of these from the source object's flags at `+0x10`
(`0x1403033B3`): when bits 24-27 are non-zero, it sets `0x8000` and stores
`nibble − 1` at `+0x0D`.

For TexNo < 0, `0x1403098B0` sends the offset to every mesh of the object. For
other values, `0x140309920` sends it only to meshes whose texture index
(`+0x08`) equals TexNo.

`0x140309570` stores `u & 0xFFF` at mesh `+0xF0`, `v & 0xFFF` at `+0xF4`, and
sets byte `+0xF8 = 3`. The shaders sample at `texcoord + texOffset`, so the
offset adds to the UVs.

## 5. Bindings

| Archive | TSC slot | Model | Owner |
| --- | --- | --- | --- |
| em028 | 13 | slot 5, bat dress | CDrawUV at CEm028 `this+0x2D00` (`0x1401307FD`) |
| em028 | 13 | slot 6, sleeves | CDrawUV at `this+0x2D38` (`0x14013089E`) |
| em000 | 24 | slot 23, EFM | `CEm005Shl01` init (`0x1400AD620`): EFM slot 23 is loaded as a model with PTX slot 32, motions from slot 37, `.clt` slot 22, TSC slot 24 |

**em028 objects.**
- Slot 5 objects 2 and 3 have flags `0x01020000`, which is scroll 0 with
  texture 3. These are the lightning strip, which scrolls up one texture every
  90 frames.
- Slot 5 objects 0 and 1 and slot 6 object 0 have flags `0x02000000`, which is
  scroll 1 with texture 2. These are the bat fabric. It eases sideways one
  texture every 400 frames and drifts 0.0001 per frame.
- The MOD header byte `+0x18` is 2 in both slots.

**em000.** None of the MODs in em000.pac has a scroll object. Its TSC belongs
to the EFM model of `CEm005Shl01`.

## 6. Open

- Types 4, 5 and 10, which use TurnTimeUV.
- JntNo, which is stored but not used by the offset path.
- The EFM layout (see the `.efm` task).
