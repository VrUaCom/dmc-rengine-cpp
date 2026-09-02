# DMC3 stage container corpus — slot roles and the name manifest — 2026-08-26

**Corpus:** `st001.pac`, `st001cfg.pac`, `st001_effect.pac` and the matching
`st114` set, supplied from a retail installation.

**Status:** OBSERVED / TWO INDEPENDENT STAGES AGREE / NOT PROMOTED

Two stages agreeing is enough to record a structure and not enough to promote
it to recovered authority. Everything below is stated as observation.

## 1. Slot payloads carry a four-byte type tag

Every payload in these containers begins with its own tag. The classifier now
reads them, which is separately committed.

Observed: `LIG2`, `SEF`, `CAM`, `EVE`, `POS`, `ITM`, `STE`, `EST`, alongside the
already-known `PAC`, `PNST`, `SCM `, `HITS`, `DCA`.

The executable does **not** carry a table of these tags. A whole-image search of
`.text`, `.rdata` and `.data` finds only `LIG2`, once, at `0x14023ECCC` — and
there it is a constructor storing `0x3247494C` into `[rcx+0x08]` of an object
whose vtable is `0x1404E3128`. So the tag is an object's own type field, and the
readers are not selected from a literal tag table. How they *are* selected is
open.

## 1a. The untagged records are readable after all

Two of the three slot kinds that had no tag are now identified, and neither
identification needed a name to work from.

**Authoring text.** `stNNN.pac` slot 0 and slot 4, `stNNNcfg.pac` slot 0 and
`stNNN_effect.pac` slot 0 are text: a name manifest, the `# GAME` scene block,
the `# DOOR` table, an effect id list. They read as `bin` only because the
classifier had asked the *path* what they were — and the path was
`slot_0004.bin`, a placeholder the parser had invented itself. Reading the
bytes instead answers it.

The encoding is not ASCII. `st001.pac` slot 4 decodes as

```text
	uv			0, 14, -6, 0		; パーツ番号、テクスチャ番号、U、V
```

so the rule validates the whole record as Shift-JIS — ASCII, half-width
katakana, and JIS X 0208 double-byte pairs — with trailing NUL padding excluded
and at least one line terminator required. Across the 38 payloads of the six
corpus containers it accepts exactly the 8 that are text and rejects the other
30, including the 3.3 MiB record next door.

A name the resource actually has still outranks this: `em035_057.index` stays
`index`, because the name says more than "text" does. The byte probe only gets
to speak where the name was ours to begin with.

**The texture pack.** `stNNN.pac` slot 1 — the largest record in the stage and
the last untyped one — is the `stNNN.ptx` the manifest names. It carries no
magic:

| offset | field |
|---|---|
| `+0x00` | `u32` texture count *N* |
| `+0x04` | *N* × `u32` block size in 2,048-byte sectors |
| `+0x800` | block 0 |
| … | block *i* at `0x800 + 2048 × Σ sectors[0..i-1]` |

and each block is a `0x70` descriptor followed by a DDS image.

What identifies it is that its own arithmetic closes exactly:
`(Σ sectors + 1) × 2048` equals the stored length to the byte — 3,485,696 for
st001, 2,404,352 for st114 — and every block it predicts really does open with
`DDS ` at `+0x70`. A structure that predicts both its own length and its own
contents is stronger evidence than four magic bytes, and it is refused rather
than repaired when it does not close.

st001 slot 1 expands to 17 DXT1/DXT5 textures, 128×128 to 1024×1024, each with
a full mip chain, at their exact stored extents — the descriptor and the sector
padding stay with the container, because neither is part of the file a caller
asked for.

This container view is not a second parser. Recognition and geometry both come
from `TextureSlotFramingParser`, which already modelled the `0x70` descriptor
and is what the packed-reflow writer authors against. Keeping one authority is
what closes the cycle: a texture taken out through the resource tree is
accepted by the writer with no translation, and the rebuilt pack comes back
through the tree with identical geometry. On the real `st001` pack, editing one
byte of one texture and repacking changes exactly one byte of 3,485,696.

## 2. Slot roles are positional and stable across stages

`stNNNcfg.pac`

| slot | st001 | st114 |
|---|---|---|
| 0 | text | text |
| 1 | `LIG2` | `LIG2` |
| 2 | `PAC` (holds `SEF`) | `PAC` |
| 3 | `CAM` | `CAM` |
| 4 | `EVE` | `EVE` |
| 5 | `POS` | `POS` |
| 6 | `ITM` | `ITM` |
| 7 | `STE` | `STE` |
| 8 | `DCA` | `DCA` |
| 9 | `EST` | absent |

`stNNN.pac`

| slot | st001 | st114 |
|---|---|---|
| 0 | name manifest (text) | name manifest (text) |
| 1 | texture pack, 17 DDS | texture pack, 17 DDS |
| 2 | `SCM ` | `SCM ` |
| 3 | `HITS` | `HITS` |
| 4 | `# GAME` text | `# GAME` text |
| 5 | `PNST` | `PNST` |
| 6 | `HITS` | `HITS` |
| 7 | `PAC` | absent |

`stNNN_effect.pac` is slot 0 a version/id list (text), slot 1 a `PNST`.

Nothing in either stage is now reported as an untyped blob.

## 3. Slot 0 of `stNNN.pac` is a name manifest

```text
st001.ptx\r\nst001.scm\r\nst001.sch\r\n
st114.ptx\r\nst114.scm\r\nst114.sch\r\n
```

This is the first evidence in this project of original names living *inside* a
relative-slot container rather than being absent from it.

The apparent mapping is manifest line *i* to slot *i+1*, and two of its three
lines now check themselves:

| line | slot | what the bytes independently say |
|---|---|---|
| `st001.ptx` | 1 | a texture pack of 17 DDS images |
| `st001.scm` | 2 | payload tag `SCM ` |
| `st001.sch` | 3 | `HITS` collision data |

The `.scm` line matched a tag from the start. The `.ptx` line matched nothing
at all until slot 1 was identified structurally, and now it lands on a pack of
textures — which is what a `.ptx` should be. That is a second corroboration
arrived at without ever consulting the manifest, which is the only kind that
counts here. `.sch` on `HITS` is consistent and still unconfirmed.

The mapping is still not wired into naming. Two lines out of three, on two
stages, is a good reason to expect it holds and not yet a reason to print a
manifest name over a payload as though it were recovered truth. The third
independent source — an existing unpacker's own mapping — is what would close
it.

## 4. Sparse slots are not damage

`st001.pac` slot 5 is a `PNST` declaring 11 slots with offsets 1–9 literally
zero; `st114.pac` slot 5 declares 21 with 18 zero. A slot index is an identity,
not a position in a packed list.

## 5. What would close this

- more stages, to turn a two-sample agreement into a role table;
- the descriptor's auxiliary pair and secondary-dimension relation, which are
  validated structurally without any claim about what they mean;
- an independent unpacker's mapping to compare against, which would raise the
  manifest-to-slot association from plausible to corroborated;
- the reader selection path in the executable, since the tag table is not there.
